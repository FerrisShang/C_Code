#ifndef __PARAM_H__
#define __PARAM_H__

#include <cstring>
#include <string.h>
#include <map>

#define PARAM_RSV_LINE     1
enum {
	PARAM_NAME_COL,
	PARAM_WIDTH_COL,
	PARAM_TYPE_COL,
	PARAM_KEY_SUB_COL,
	PARAM_VAL_RNE_COL,
	PARAM_DEF_COL,
	PARAM_CONFIG_COL,
	PARAM_DESC_COL,
	PARAM_MAX_COL,
};

enum {
	BC_OUTPUT_SUBKEY,
	BC_INC_INDENT,
	BC_OUTPUT_BIT,
	BC_STREAM_LESS,
	BC_PACK_LENGTH,
	BC_UNUSED,
};
#define isSet(data, bit) ((data) & (1 << (bit)))

#define PTYPE_ENUM        "enum"
#define PTYPE_BITMAP      "bitmap"
#define PTYPE_STREAM      "stream"
#define PTYPE_UNSIGNED    "unsigned"
#define PTYPE_SIGNED      "signed"
#define PTYPE_HEX         "hex"
#define PTYPE_ADDRESS     "address"
#define PTYPE_T0_625      "T0_625ms"
#define PTYPE_T1_25       "T1_25ms"
#define PTYPE_T10         "T10ms"
#define PROTO_ROOT        "PROTO_ALL"
#define PROTO_UNUSED      "Unused"

inline bool isBasicType(const string& t){
	return ((t == PTYPE_ENUM) || (t == PTYPE_BITMAP) || (t == PTYPE_STREAM) || (t == PTYPE_UNSIGNED) ||
			(t == PTYPE_SIGNED) || (t == PTYPE_HEX) || (t == PTYPE_ADDRESS) || (t == PTYPE_T0_625) ||
			(t == PTYPE_T1_25) || (t == PTYPE_T10));
}

class CParam{
	public:
		class CBitWidth {
			public:
			CCell cell;
			int p, w, m;
			CBitWidth(void){}
			CBitWidth(const CCell &cell){
				this->cell = cell;
				if(!cell.text.length()){ return; }
				if(strstr(cell.text.c_str(), "/")){
					char *str = strdup(cell.text.c_str()), *p=str, *w=NULL, *m=NULL;
					FOR(i, cell.text.length()-1){
						if(str[i] == '/'){
							if(!w){ w = &str[i+1]; }
							else{ m = &str[i+1]; }
							str[i] = '\0';
						}
					}
					this-> p = atoi(p);
					this-> w = atoi(w);
					this-> m = atoi(m);
					free(str);
				}else{
					p = w = m = stoi(cell.text, nullptr, 0);
				}
			}
			bool isBitShare(void){ return !(m == w); }
		};
		class CRange {
			public:
			CCell cell;
			CRange(const CCell &cell=CCell()){ this->cell = cell; }
		};
		class CEnum{
			public:
			CCell key;
			CCellPos valuePos;
			int value;
			string desciption;
			CEnum(const CCell &cell_key=CCell(), const CCell &cell_value=CCell(), string desc=""){
					if(!cell_value.text.length()){ return; }
					this->key = cell_key;
					this->valuePos = cell_value.pos;
					this->value = stoi(cell_value.text, nullptr, 0);
					this->desciption = desc;
			}
		};
		class CDefValue{
			public:
			CCell cell;
			CDefValue(const CCell &cell=CCell()){ this->cell = cell; }
		};
		class CPackLen{
			public:
			int len;
			int operator()(){ return len; }
			void init(unsigned int flag, const CCell& defValue){
				if(isSet(flag, BC_PACK_LENGTH)){
					len = stoi(defValue.text, nullptr, 0);
				}
			}
		};
		class CParams{
			public:
			CCell name;
			CBitWidth bitWidth;
			CCell type;
			CCell subKey;
			CRange range;
			map<int, CEnum> enums;
			CDefValue def_value;
			unsigned int configFlag;
			string description;
			CPackLen packLen;
			CParams(){}
			CParams(const CCell& name, const CCell& bitWidth, const CCell& type, const CCell& subKey,
				const CRange& range, const CCell& def_value, unsigned int configFlag=0, string description=""){
				this->name = name;
				this->bitWidth = CBitWidth(bitWidth);
				this->type = type;
				if(type.text != PTYPE_ENUM && type.text != PTYPE_BITMAP){
					this->subKey = subKey;
					this->range = CRange(range);
				}
				this->def_value = def_value;
				this->configFlag = configFlag;
				this->description = description;
				this->packLen.init(configFlag, def_value);
			}
			void addEnum(const CCell &cell_key, const CCell &cell_value, string desc){
					int value = stoi(cell_value.text, nullptr, 0);
					enums[value] = CEnum(cell_key, cell_value, desc);
			}
		};
		map<string, CParams> params;
		CParams& operator[](string name){ return params[name]; }
		void addParam(const CCell& name, const CCell& bitWidth, const CCell& type, const CCell& sub_key,
						const CCell& range, const CCell& def_value, unsigned int configFlag=0, string description=""){
				params[name.text] = CParams(name, bitWidth, type, sub_key, range, def_value, configFlag, description);
		}
};
#endif /* __PARAM_H__ */
