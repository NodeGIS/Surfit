/*
 *
 *  C++ Portable Types Library (PTypes)
 *  Version 1.3.2 Released: 26-Jun-2002
 *
 *  Copyright (C) 2001, 2002 Hovik Melikyan
 *
 *  http://www.melikyan.com/ptypes/
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pport.h"

#if defined(WIN32) && !defined(NO_CRIT_MSGBOX)
#  include <windows.h>
#  define CRIT_MSGBOX
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

// MS Visual Studio gives warnings when using 
// fopen. But fopen_s is not going to work well 
// with most compilers, and fopen_s uses different 
// syntax than fopen. (i.e., a macro won't work) 
// So, we'lll use this:
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#endif

PTYPES_BEGIN


static void defhandler(int code, const char* msg) 
{
#ifdef CRIT_MSGBOX
    char buf[2048];
    _snprintf(buf, sizeof(buf) - 1, "Fatal [%05x]: %s", code, msg);
//    MessageBox(0, buf, "Internal error", MB_OK | MB_ICONSTOP);
#else
    fprintf(stderr, "\nInternal [%04x]: %s\n", code, msg);
#endif
}

static _pcrithandler crith = defhandler;


_pcrithandler getcrithandler() 
{
    return crith;
}


_pcrithandler setcrithandler(_pcrithandler newh) 
{
    _pcrithandler ret = crith;
    crith = newh;
    return ret;
}


void fatal(int code, const char* msg)
{
    if (crith != nil)
        (*crith)(code, msg);
    exit(code);
}


PTYPES_END
