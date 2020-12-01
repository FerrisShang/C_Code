#ifndef __OM_IMPLICIT_SEND_H__
#define __OM_IMPLICIT_SEND_H__

void Om_InitImplicitSend(void);
void Om_ImplicitStartTestCase(char* strTestCaseName);
void Om_ImplicitTestCaseFinished(void);
char* Om_ImplicitSendStyle(char* strMmiText, UINT mmiStyle);
char* Om_ImplicitSendStyleEx(char* strMmiText, UINT mmiStyle, char* strBdAddr);

#endif /* __OM_IMPLICIT_SEND_H__ */
