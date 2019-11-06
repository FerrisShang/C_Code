#ifndef __RE_PARAM_H__
#define __RE_PARAM_H__

#define ENABLE_REGEX 0
#include <stdio.h>

#if ENABLE_REGEX
#include <regex>
#endif

#define RE_PARAM_NAME        "[A-Za-z][A-Za-z0-9_]*"
#define RE_PARAM_WIDTH       "(([-]?[0-9]+)|([0-9]+/[0-9]+/((8)|(16)|(32))))?"
//#define RE_PARAM_WIDTH       "(([A-Za-z][A-Za-z0-9_]+)|([-]?[0-9]+)|([0-9]+/[0-9]+/((8)|(16)|(32))))?"
#define RE_PARAM_TYPE        RE_PARAM_NAME
#define RE_PARAM_KEY         "[A-Za-z][A-Za-z0-9_]+"
#define RE_PARAM_SUB_KEY     "(([A-Za-z][A-Za-z0-9_]+)|0)?"
#define RE_PARAM_VALUE       "(0x[A-Fa-f0-9]+)|[-]?[0-9]+"
#define RE_PARAM_RANGE       "(((0x[A-Fa-f0-9]+)|[0-9]+)-((0x[A-Fa-f0-9]+)|[0-9]+)(\\|((0x[A-Fa-f0-9]+)|[0-9]+)-((0x[A-Fa-f0-9]+)|[0-9]+))*)?"
#define RE_PARAM_DEFAULT     "((0x[A-Fa-f0-9]+)|[-]?[0-9]+)?"
#define RE_PARAM_CFG         "(0x[A-Fa-f0-9]+)?"
#define RE_FORMAT_PARAM      "[A-Za-z][A-Za-z0-9_]+([ ]+[A-Za-z][A-Za-z0-9_]+)?([ ]*\\|[ ]*[A-Za-z][A-Za-z0-9_]+([ ]+[A-Za-z][A-Za-z0-9_]+)?)*"
#define RE_PARAM_WIDTH_NUM   "[-]?[0-9]+"
#define RE_PARAM_WIDTH_BIT   "[0-9]+/[0-9]+/[0-9]+"

bool re_enabled = true;

#if ENABLE_REGEX
#define RE(s, r) ((!re_enabled) || regex_match(s, regex(r)))
#else
#define RE(s, r) (true)
#endif

#endif /* __RE_PARAM_H__ */
