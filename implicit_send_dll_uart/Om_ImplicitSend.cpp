#include <windows.h>
#include <ctype.h>
#include <string>
#include "ImplicitSendAPI.h"
#include "Om_ImplicitSend.h"

extern "C" bool WINAPI InitImplicitSend(void);
extern "C" void WINAPI ImplicitStartTestCase(std::string &strTestCaseName);
extern "C" void WINAPI ImplicitTestCaseFinished(void);
extern "C" char* WINAPI ImplicitSendStyle(std::string &strMmiText, UINT mmiStyle);
extern "C" char* WINAPI ImplicitSendStyleEx(std::string &strMmiText, UINT    mmiStyle, std::string &strBdAddr);
extern "C" char* WINAPI ImplicitSendPinCodeEx(std::string &strBdAddr);
extern "C" char* WINAPI ImplicitSendPinCode(void);
#if 0
#define STR2PCHAR(p) (char*)(*((int*)(p) + 1))
#else
static char* STR2PCHAR(void* p)
{
    size_t str_addr = (size_t)p + sizeof(size_t);
    if ((*(size_t*)str_addr & 0xF0000000) == 0 ||
            !isprint(*((char*)str_addr + 0)) || !isprint(*((char*)str_addr + 1)) ||
            !isprint(*((char*)str_addr + 2)) || !isprint(*((char*)str_addr + 3))) {
        return (char*) (size_t) * (size_t*)str_addr;
    } else {
        return (char*) str_addr;
    }
}
#endif
extern "C" bool WINAPI InitImplicitSend(void)
{
    Om_InitImplicitSend();
    return (true);
}

extern "C" void WINAPI ImplicitStartTestCase(std::string &strTestCaseName)
{
    Om_ImplicitStartTestCase(STR2PCHAR(&strTestCaseName));
}

extern "C" void WINAPI ImplicitTestCaseFinished(void)
{
    Om_ImplicitTestCaseFinished();
}

extern "C" char* WINAPI ImplicitSendStyle(std::string &strMmiText, UINT mmiStyle)
{
    if ((mmiStyle != MMI_Style_Ok_Cancel1)
            && (mmiStyle != MMI_Style_Ok_Cancel2)
            && (mmiStyle != MMI_Style_Ok1)
            && (mmiStyle != MMI_Style_Yes_No1)
            && (mmiStyle != MMI_Style_Yes_No_Cancel1)
            && (mmiStyle != MMI_Style_Abort_Retry1)
            && (mmiStyle != MMI_Style_Edit1)
            && (mmiStyle != MMI_Style_Edit2)
            && (mmiStyle !=
                MMI_Style_Helper))    // This style is used internally by PTS development and is intentionally undocumented
        // PTS users should not expect to see this style.
    {
        MessageBox(NULL, "Invalid style value passed to ImplicitSendStyle()!", "Implicit Send", MB_ICONWARNING);
        goto ABORT;
    }
    if (mmiStyle == MMI_Style_Helper)
        return (const_cast<char*>("OK"));

    /*
     * In the standard Implicit Send DLL we interact with the user/test operator via a dialog box. The rest of this function
     * implements the use of the ImplicitSendDialog class found in ImplicitSendDialog.cpp
     *
     * If you are creating your own Implicit Send DLL remove the code from here to the comment that says "End Dialog Work" and
     * replace it with whatever you need to do.
     */
    return Om_ImplicitSendStyle(STR2PCHAR(&strMmiText), mmiStyle);
    /*
     * Hand off to the internal development helper if that's what has been requested
     */

ABORT:
    return (const_cast<char*>("OK"));
}

extern "C" char* WINAPI ImplicitSendStyleEx(std::string &strMmiText, UINT mmiStyle, std::string &strBdAddr)
{
    if ((mmiStyle != MMI_Style_Ok_Cancel1) && (mmiStyle != MMI_Style_Ok_Cancel2) && (mmiStyle != MMI_Style_Ok1)
            && (mmiStyle != MMI_Style_Yes_No1) && (mmiStyle != MMI_Style_Yes_No_Cancel1) && (mmiStyle != MMI_Style_Abort_Retry1)
            && (mmiStyle != MMI_Style_Edit1) && (mmiStyle != MMI_Style_Edit2)
            && (mmiStyle != MMI_Style_Helper)) {
        MessageBox(NULL, "Invalid style value passed to ImplicitSendStyle()!", "Implicit Send", MB_ICONWARNING);
        goto ABORT;
    }
    return Om_ImplicitSendStyleEx(STR2PCHAR(&strMmiText), mmiStyle, STR2PCHAR(&strBdAddr));
ABORT:
    return (const_cast<char*>("OK"));
}


extern "C" char* WINAPI ImplicitSendPinCode(void)
{
    std::string    strPrompt = "Please enter a PIN Code:";
    return (ImplicitSendStyle(strPrompt, MMI_Style_Edit1));
}
extern "C" char* WINAPI ImplicitSendPinCodeEx(std::string &strBdAddr)
{
    std::string    strPrompt = "Please enter a PIN Code:";
    return (ImplicitSendStyleEx(strPrompt, MMI_Style_Edit1, strBdAddr));
}

