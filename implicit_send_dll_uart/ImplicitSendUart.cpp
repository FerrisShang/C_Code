#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include "windows.h"
#include "win_serial.h"
using namespace std;
void RedirectIOToConsole(char c);

#define ENV_BUF_SIZE 2048
typedef struct env {
    int env_inited;
    int console_inited;
    CBtIO* btio;
    FILE* fp;
    int port;
    int baud;
    char b[ENV_BUF_SIZE];
} env_t;

static env_t* env = NULL;
static char console_inited;

env_t* get_global_buf(env_t* env)
{
    if (env) return env;
#define BUF_SIZE 4096
    char* szName = (char*)"Global\\ImplicitEnvMappingObject";
    HANDLE hMapFile;
    LPCTSTR pBuf;
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
    if (hMapFile == NULL) {
        hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, szName);
    }
    env = (struct env*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
    memset(env, BUF_SIZE, 0);
    return env;
    //UnmapViewOfFile(pBuf);
    //CloseHandle(hMapFile);
}
#define log printf
#define ulog(...) \
    do{ \
        char b[1024]; \
        sprintf(b, __VA_ARGS__); \
        log("<<< %s\n", b); \
        env->btio->send((uint8_t*)b, strlen(b)+1); \
        fputs("<<< ", env->fp); \
        fputs(b, env->fp); \
        fputs("\n", env->fp); \
        fflush(env->fp); \
    }while(0) \

void Om_InitImplicitSend(void)
{
    env = get_global_buf(env);
    if (!console_inited) {
        RedirectIOToConsole(!env->console_inited);
        env->console_inited = 1;
    }
    console_inited = 1;
    if (!env->env_inited) {
        env->btio = NULL;
        vector<int> ports = CSerial::scan_port();
        log("*** Available port: ");
        int i;
        for (i = 0; i < ports.size(); i++) {
            log("COM%d ", ports[i]);
        }
        if (!i) {
            log("None\n");
        } else {
            log("\n");
        }
        log("*** Input Serial port and baudrate.\n");
        cin >> env->port >> env->baud; cin.ignore();
        log("*** InitImplicitSend\n");
        env->env_inited = 1;
    }
}
char* style_2_str(int style)
{
    if (style == 0x11041) return const_cast<char*>( "OK/Cancel buttons" );
    if (style == 0x11141) return const_cast<char*>( "Cancel button" );
    if (style == 0x11040) return const_cast<char*>( "OK button" );
    if (style == 0x11044) return const_cast<char*>( "Yes, No buttons" );
    if (style == 0x11043) return const_cast<char*>( "Yes, No, Cancel" );
    if (style == 0x11042) return const_cast<char*>( "Abort, Retry buttons" );
    if (style == 0x12040) return const_cast<char*>( "OK, Cancel buttons" );
    if (style == 0x12140) return const_cast<char*>( "OK, Cancel buttons" );
    if (style == 0x00000) return const_cast<char*>( "MMI_Style_Helper" );
    return const_cast<char*>( "Unknown");
}

void Om_ImplicitStartTestCase(char* strTestCaseName)
{
    env = get_global_buf(env);
    if (!env->btio) {
        env->btio = new CBtIO();
        env->fp = fopen("c:\\log.txt", "w");
        if (env->btio->start(env->port, env->baud)) {
            log("*** Open serial port failed\n");
        }
    }
    ulog("ImplicitStartTestCase: %s", strTestCaseName);
}

void Om_ImplicitTestCaseFinished(void)
{
    env = get_global_buf(env);
    ulog("ImplicitTestCaseFinished");
    if (env->btio) {
        env->btio->stop();
        delete env->btio;
        env->btio = NULL;
        fclose(env->fp);
        env->fp = NULL;
    }
    log("-----------------------------------------\n");
}

char* Om_ImplicitSendStyle(char* strMmiText, UINT mmiStyle)
{
    env = get_global_buf(env);
    enum ImplicitSendMessageStyles {
        MMI_Style_Ok_Cancel1 =     0x11041,    // Simple prompt           | OK, Cancel buttons      | Default: OK
        MMI_Style_Ok_Cancel2 =     0x11141,    // Simple prompt           | Cancel button           | Default: Cancel
        MMI_Style_Ok1 =            0x11040,    // Simple prompt           | OK button               | Default: OK
        MMI_Style_Yes_No1 =        0x11044,    // Simple prompt           | Yes, No buttons         | Default: Yes
        MMI_Style_Yes_No_Cancel1 = 0x11043,    // Simple prompt           | Yes, No, Cancel buttons | Default: Yes
        MMI_Style_Abort_Retry1 =   0x11042, // Simple prompt           | Abort, Retry buttons    | Default: Abort
        MMI_Style_Edit1 =          0x12040, // Request for data input  | OK, Cancel buttons      | Default: OK
        MMI_Style_Edit2 =          0x12140,    // Select item from a list | OK, Cancel buttons      | Default: OK
    };
    log("*** Style:0x%X [%s]\n", mmiStyle, style_2_str(mmiStyle));
    ulog("ImplicitSendStyle:0x%05x:%s", mmiStyle, strMmiText);
    memset(env->b, 0, ENV_BUF_SIZE);
    int len = env->btio->recv((uint8_t*)env->b, ENV_BUF_SIZE, 500);
    if (len > 0) {
        log(">>> %s\n", env->b);
        if (!strcmp("RETURN_NULL", env->b)) {
            return NULL;
        } else {
            return env->b;
        }
    } else {
        log(">>> [Timeout]\n");
    }
    return (const_cast<char*>("OK"));
}

char* Om_ImplicitSendStyleEx(char* strMmiText, UINT mmiStyle, char* strBdAddr)
{
    return Om_ImplicitSendStyle(strMmiText, mmiStyle);
}

