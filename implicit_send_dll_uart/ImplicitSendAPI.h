/***
 *** ImplicitSendAPI.h
 ***
 *** This is the source code for the standard implicit_send3.dll that is provided with the Profile Tuning Suite (PTS). It
 *** is provided to PTS users as a starting point for developing their own Implict Send DLL.
 ***
 *** Please see the "Automating - Using Implicit Send" document for more information. This document can be found in the
 *** same folder as this source code, or by selecting "Reference Documents" -> "Automating" from the PTS Start page.
 ***
 *** ----------
 ***
 *** This file provides the style values used by ImplicitSendStyle() and declarations for the five required functions that
 *** every Implicit Send DLL must provide.
 ***
 *** ----------
 ***
 *** Copyright 2012, Bluetooth SIG, Inc.
 ***
 *** ----------
 ***
 *** Tom Allebrandi (tallebrandi@bluetooth.com)
 *** Bluetooth SIG, Inc.
 ***/

#if    (!defined(IMPLICITSENDAPI_H_INCLUDED))
#define    IMPLICITSENDAPI_H_INCLUDED

/****************************************************************************************************************************
 **
 ** Includes needed by the function declarations
 **/
#include    <string>


/****************************************************************************************************************************
 **
 ** Message styles used in calls to ImplicitSendStyle()
 **/
enum ImplicitSendMessageStyles
        {
        MMI_Style_Ok_Cancel1 =     0x11041,    // Simple prompt           | OK, Cancel buttons      | Default: OK
        MMI_Style_Ok_Cancel2 =     0x11141,    // Simple prompt           | Cancel button           | Default: Cancel
        MMI_Style_Ok1 =            0x11040,    // Simple prompt           | OK button               | Default: OK
        MMI_Style_Yes_No1 =        0x11044,    // Simple prompt           | Yes, No buttons         | Default: Yes
        MMI_Style_Yes_No_Cancel1 = 0x11043,    // Simple prompt           | Yes, No, Cancel buttons | Default: Yes
        MMI_Style_Abort_Retry1 =   0x11042, // Simple prompt           | Abort, Retry buttons    | Default: Abort
        MMI_Style_Edit1 =          0x12040, // Request for data input  | OK, Cancel buttons      | Default: OK
        MMI_Style_Edit2 =          0x12140,    // Select item from a list | OK, Cancel buttons      | Default: OK
        MMI_Style_Helper =         0x00000    // This style is used internally by PTS development and is intentionally undocumented.
                                            // PTS users should not expect to see this style.
        };


/****************************************************************************************************************************
 **
 ** Function prototypes for each required function
 **
 **/

extern "C" bool  WINAPI InitImplicitSend(void);
extern "C" void  WINAPI ImplicitStartTestCase(std::string& strTestCaseName);
extern "C" char* WINAPI ImplicitSendStyle(std::string& strMmiText,UINT mmiStyle);
extern "C" char* WINAPI ImplicitSendPinCode(void);
extern "C" void  WINAPI ImplicitTestCaseFinished(void);
extern "C" char* WINAPI ImplicitSendStyleEx(std::string& strMmiText,UINT mmiStyle,std::string& strBdAddr);
extern "C" char* WINAPI ImplicitSendPinCodeEx(std::string& strBdAddr);
#endif
