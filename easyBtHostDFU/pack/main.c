#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "key_gen.h"
#include "zip.h"
#include "uECC.h"
#include "sha256.h"
#define print printf

#define DFU_CMDPKG_MARK          0x01BFDF55
#define MAX_IMG_NUM  (32)
#define MAX_BIN_SIZE (1<<24)

enum {
    IMAGE_TYPE_APP,
    IMAGE_TYPE_PATCH,
    IMAGE_TYPE_CONFIG,
    IMAGE_TYPE_CUSTOM = 0x10,
    IMAGE_TYPE_RAW    = 0x5F,
};

enum {
    DFU_CTRL_SIGN_BIT = 1 << 0,
};

typedef struct{
	uint32_t type:8;
	uint32_t rsv:24;
	uint32_t version;
	uint32_t size;
}image_normal_t;

typedef struct{
	uint32_t type:8;
	uint32_t flash_itf_id:8;
	uint32_t rsv:16;
	uint32_t address;
	uint32_t size;
}image_raw_t;

typedef union{
    uint8_t type;
    image_normal_t normal;
    image_raw_t raw;
}image_t;

typedef struct{
	uint32_t  mark;
	uint32_t  version:16;
	uint32_t  rsv:16;
	uint16_t  control;
	uint16_t  image_num;
	image_t imgs[MAX_IMG_NUM];
    uint8_t buffer[SHA256_BLOCK_SIZE+PRIV_KEY_SIZE];
}pack_info_t;

pack_info_t m_pack_info;
char        m_bin_buffer[MAX_BIN_SIZE];
uint32_t    m_bin_size;

void usage(void)
{
    print(
            "Usage:\n"
            "        Generate private/public key          : CreateFwTool.exe key\n"
            "    or\n"
            "        Generate public key from private key : CreateFwTool.exe key \"filename\"\n"
            "    or\n"
            "        Generate DFU packet: CreateFwTool.exe pack version [key file] [image info]xN\n"
            "                             [image info] : [filename type(normal)   version]\n"
            "                             [image info] : [filename type(0x5F:raw) version address]\n"
         );
}
void dump(void *p, int len)
{
	int i;
	for(i=0;i<len;i++){
		if(!(i%16)){ print("\n"); }
		print("%02X ", ((uint8_t*)p)[i]);
	}
	print("\n");
}
uint32_t str2u32(char* s)
{
	if(strlen(s)>2 && s[0] == '0' && (s[1]&0xDF) == 'X'){
		char *p = &s[2];
		uint32_t res = 0;
		while(*p){
			res <<= 4;
			if(*p>'9'){
				res += (*p&0xDF) - 'A' + 10;
			}else{
				res += *p - '0';
			}
			p++;
		}
		return res;
	}else{
		return atoi(s);
	}
}

int get_file(char *filename, char **buf)
{
	if(!filename){
		*buf = NULL;
		return 0;
	}
#define MAX_FILE_SIZE (1<<24)
	*buf = calloc(1, MAX_FILE_SIZE);
	if(!*buf){
		print("Not enough memory\n");
        return 0;
	}
	memset(*buf, 0xFF, MAX_FILE_SIZE);
	FILE *fp = fopen(filename, "rb");
	if(!fp){
		print("File %s not found\n", filename);
        return 0;
	}
	uint32_t read_size = fread(*buf, 1, MAX_FILE_SIZE, fp);
	if(read_size == MAX_FILE_SIZE){
		print("File size exceed the limit size.\n");
        return 0;
	}
	read_size += (sizeof(uint32_t)-(read_size%sizeof(uint32_t)))%sizeof(uint32_t); // firmware must be word aligned
	return read_size;
}

void create_zip(char *fw_name, char *info_buf, uint32_t info_size, char *bin_buf, uint32_t bin_size)
{
	char *manifest = "\
    {\n\
        \"manifest\": {\n\
            \"softdevice\": {\n\
                \"bin_file\": \"firmware.bin\",\n\
                \"dat_file\": \"firmware.dat\"\n\
            }\n\
        }\n\
    }\n\
	";
	struct zip_t *zip = zip_open(fw_name, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

	zip_entry_open(zip, "manifest.json");
	zip_entry_write(zip, manifest, strlen(manifest));
	zip_entry_close(zip);

	zip_entry_open(zip, "firmware.bin");
	zip_entry_write(zip, bin_buf, bin_size);
	zip_entry_close(zip);

	zip_entry_open(zip, "firmware.dat");
	zip_entry_write(zip, info_buf, info_size);
	zip_entry_close(zip);

	zip_close(zip);
}

int pack_info(int argc, char *argv[], pack_info_t *info, uint8_t *bin_buffer, uint32_t *bin_size)
{
    memset(info, 0, sizeof(pack_info_t));
	char *str_version = argv[0];
	char *str_ctrl = argv[1];
	info->mark = DFU_CMDPKG_MARK;
	info->version = str2u32(str_version);
	print("DFU Version: 0x%08X\n", info->version);

    if(str_ctrl[0] != '0'){
        info->control |= DFU_CTRL_SIGN_BIT;
    }
	print("Control Field: 0x%04X\n", info->control);
	info->image_num = 0;
	int i = 2;
	while(i < argc){
		char *filename = argv[i++];
		char *file_buf;
		if(i == argc){ break; }
		info->imgs[info->image_num].type = str2u32(argv[i++]);
		if(info->imgs[info->image_num].type != IMAGE_TYPE_RAW){
			if(i == argc){ break; }
			info->imgs[info->image_num].normal.version = str2u32(argv[i++]);
		}else{
			if(i == argc){ break; }
			info->imgs[info->image_num].raw.flash_itf_id = str2u32(argv[i++]);
			if(i == argc){ break; }
			info->imgs[info->image_num].raw.address = str2u32(argv[i++]);
		}
		int filesize = get_file(filename, &file_buf);
        if(filesize <= 0){
            return -info->image_num;
        }
		info->imgs[info->image_num].normal.size = filesize;
		if(info->imgs[info->image_num].type != IMAGE_TYPE_RAW){
			print("Image %d:\n\tFilename: %s\n\tType:     0x%08X\n\tVersion:  0x%08X\n\tSize:     %d\n",
					1+info->image_num, filename, info->imgs[info->image_num].normal.type,
					info->imgs[info->image_num].normal.version, filesize);
		}else{
			print("Image %d:\n\tFilename: %s\n\tType:     0x%08X\n\titf_idx:  0x%02X\n\tAddress:  0x%08X\n\tSize:     %d\n",
					1+info->image_num, filename, info->imgs[info->image_num].type,
					info->imgs[info->image_num].raw.flash_itf_id, info->imgs[info->image_num].raw.address, filesize);
		}
		memcpy(&bin_buffer[*bin_size], file_buf, filesize);
		*bin_size += filesize;
		free(file_buf);
		info->image_num++;
	}
    return info->image_num;
}

bool sign_info(pack_info_t *info, char *priv_key_filename, int *total_len)
{
    uint8_t *p = (uint8_t*)info + *total_len;
    memcpy(p, info->buffer, SHA256_BLOCK_SIZE);
    p += SHA256_BLOCK_SIZE;
    *total_len += SHA256_BLOCK_SIZE;
    // get private key from file
    uint8_t cipher_priv_key[PRIV_KEY_SIZE];
    uint8_t plain_pub_key[PRIV_KEY_SIZE*2];
    if(!read_key_file(priv_key_filename, cipher_priv_key, plain_pub_key)){
        print("Read key file \"%s\" failed.\n", priv_key_filename);
        return false;
    }
    if(!check_key(cipher_priv_key, plain_pub_key)){
        print("Invalid key file \"%s\", parse failed.\n", priv_key_filename);
        return false;
    }
    uint8_t info_hash[SHA256_BLOCK_SIZE];
    static SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)info, *total_len);
    sha256_final(&ctx, info_hash);
    if(uECC_sign(cipher_priv_key, info_hash, SHA256_BLOCK_SIZE, info->buffer, ECC_TYPE) < 0){
        return false;
    }else{
        memcpy(p, info->buffer, PRIV_KEY_SIZE*2);
        *total_len += PRIV_KEY_SIZE*2;
    }
#if 0 // uECC_verify
    uint8_t pub_key[PRIV_KEY_SIZE*2], hash[SHA256_BLOCK_SIZE];
    memset(pub_key, 0x5F, PRIV_KEY_SIZE*2);
    memcpy(pub_key, plain_pub_key, PRIV_KEY_SIZE*2);
    memset(hash, 0x5F, SHA256_BLOCK_SIZE);
    memcpy(hash, info_hash, SHA256_BLOCK_SIZE);
    int res = uECC_verify(pub_key, hash, SHA256_BLOCK_SIZE,
                info->buffer, ECC_TYPE);
    print("uECC_verify=%d\n", res);
#endif
    return true;
}

static int uECC_RNG(uint8_t* dest, unsigned size)
{
    for (int i = 0; i < size; i++) {
        dest[i] = rand() & 0xFF;
    }
    return 1;
}
void random_init(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    srand(t.tv_usec);
    uECC_set_rng(uECC_RNG);
}

int main(int argc, char *argv[])
{
#define CMD_KEYGEN "key"
#define CMD_PACKET "pack"
    int res;
    random_init();
    if(argc >= 2){
        if(!strcmp(argv[1], CMD_KEYGEN)){
            if(argc == 2){ // create new private key
                char key_name[128];
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(key_name, "private_key.%02d%02d%02d%02d%02d.hex",
                        tm.tm_year%100, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min);
                uint8_t priv_key[PRIV_KEY_SIZE], pub_key[PRIV_KEY_SIZE*2];
                res = generate_key(priv_key, pub_key);
                if(!res){
                    print("Generate key failed. \n");
                    exit(-1);
                }
                res = write_priv_key_file(key_name, priv_key, pub_key);
                if(!res){
                    print("Create file failed. \n");
                    exit(-1);
                }
                print("Private/Public key created.\n");
                exit(0);
            }else if(argc == 3){ // create public key from exist private key
                char *filename = argv[2];
                uint8_t cipher_priv_key[PRIV_KEY_SIZE], plain_pub_key[PRIV_KEY_SIZE*2];
                res = read_key_file(filename, cipher_priv_key, plain_pub_key);
                if(!res){
                    print("Read file \"%s\"failed. \n", filename);
                    exit(-1);
                }
                res = write_pub_key_file(filename, plain_pub_key);
                if(!res){
                    print("Create file failed. \n");
                    exit(-1);
                }
                print("Create public_key.c success.\n");
                exit(0);
            }
        }
    }
    if(argc > 3){
        if(!strcmp(argv[1], CMD_PACKET)){
            if(pack_info(argc-2, &argv[2], &m_pack_info, (uint8_t*)m_bin_buffer, &m_bin_size) <= 0){
                print("Format error.\n");
                exit(-1);
            }
            uint32_t info_len = (size_t)m_pack_info.imgs - (size_t)&m_pack_info + sizeof(image_t) * m_pack_info.image_num;
            if((m_pack_info.control & DFU_CTRL_SIGN_BIT)){
                // cal hash of images
                SHA256_CTX ctx;
                sha256_init(&ctx);
                sha256_update(&ctx, (const BYTE*)m_bin_buffer, m_bin_size);
                sha256_final(&ctx, m_pack_info.buffer);
                if(!sign_info(&m_pack_info, argv[3], &info_len)){
                    print("Sign error.\n");
                    exit(-1);
                }
            }
            char fw_name[128];
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            sprintf(fw_name, "OmFw%02d%02d%02d%02d%02d",
                    tm.tm_year%100, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            if((m_pack_info.control & DFU_CTRL_SIGN_BIT)){
				strcat(fw_name, "S");
			}

			strcat(fw_name, ".zip");
            create_zip(fw_name, (char*)&m_pack_info, info_len , m_bin_buffer, m_bin_size);
            print("\nPackage Info:");
            dump(&m_pack_info, info_len);
            print("Total Cmd Size: %d\n", info_len);
            print("Total Image Size: %d\n", m_bin_size);
            print("Fireware %s created.\n", fw_name);
            exit(0);
        }
    }
    usage();
}
