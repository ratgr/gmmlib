/*******************************************************************************
**
** Copyright (c) Intel Corporation (2003-2013).
**
** DISCLAIMER OF WARRANTY
** NEITHER INTEL NOR ITS SUPPLIERS MAKE ANY REPRESENTATION OR WARRANTY OR
** CONDITION OF ANY KIND WHETHER EXPRESS OR IMPLIED (EITHER IN FACT OR BY
** OPERATION OF LAW) WITH RESPECT TO THE SOURCE CODE.  INTEL AND ITS SUPPLIERS
** EXPRESSLY DISCLAIM ALL WARRANTIES OR CONDITIONS OF MERCHANTABILITY OR
** FITNESS FOR A PARTICULAR PURPOSE.  INTEL AND ITS SUPPLIERS DO NOT WARRANT
** THAT THE SOURCE CODE IS ERROR-FREE OR THAT OPERATION OF THE SOURCE CODE WILL
** BE SECURE OR UNINTERRUPTED AND HEREBY DISCLAIM ANY AND ALL LIABILITY ON
** ACCOUNT THEREOF.  THERE IS ALSO NO IMPLIED WARRANTY OF NON-INFRINGEMENT.
** SOURCE CODE IS LICENSED TO LICENSEE ON AN 'AS IS' BASIS AND NEITHER INTEL
** NOR ITS SUPPLIERS WILL PROVIDE ANY SUPPORT, ASSISTANCE, INSTALLATION,
** TRAINING OR OTHER SERVICES.  INTEL AND ITS SUPPLIERS WILL NOT PROVIDE ANY
** UPDATES, ENHANCEMENTS OR EXTENSIONS.
**
** File Name: gfxDebug.h
**
** Description:
**
** Environment:
**
** Notes:
**
*******************************************************************************/

#pragma once

#ifndef _GFXDEBUG_H_
#define _GFXDEBUG_H_

#if defined(D3D) && !defined (USERMODE_DRIVER)
// The D3D XP Kernel Mode driver will continue to use the old scheme until it 
// has been updated.
#else

// This portion is used by new stile debug scheme.
// NOTE: There is some overlap between defines.


//========================================================================
// Controlling Debug Messages: There are two types of output messages - 
// debug or release output message. These are controlled through the two
// flags listed below. 

#ifdef _DEBUG
  #define __DEBUG_MESSAGE   1
  #define __RELEASE_MESSAGE 1
#else 
  #define __DEBUG_MESSAGE   0
  #define __RELEASE_MESSAGE 1
#endif

// !!! HAVE TO FIGURE OOUT HOW TO INCLUDE THIS CORRECTLY
// #include "video.h"

//==== Define global debug message routine ===================

#if __DEBUG_MESSAGE || __RELEASE_MESSAGE
#ifdef __UMD
#include "stdlib.h"
#define MESSAGE_FUNCTION(FUNCTION_NAME,COMPONENT_ID)                        \
                                                                            \
    void FUNCTION_NAME(ULONG MessageLevel, const char *MessageFmt, ...)     \
{                                                                           \
    DWORD Length = 0;                                                       \
    char *PrintBuffer = NULL;                                               \
    char *Prefix = NULL;                                                    \
    va_list ArgList;                                                        \
                                                                            \
    ULONG ComponentId   = COMPONENT_ID;                                     \
    ULONG ComponentMask = 1 << COMPONENT_ID;                                \
                                                                            \
    /* Ensure that CRITICAL messages are printed if the setting of the */   \
    /* global debug variables flag is NORMAL or VERBOSE. Similarly, if */   \
    /* the setting of the debug variables flag is NORMAL, ensure that  */   \
    /* the VERBOSE message are printed out too!                        */   \
                                                                            \
    if (MessageLevel & GFXDBG_CRITICAL)                                     \
    {                                                                       \
        MessageLevel |= GFXDBG_NORMAL | GFXDBG_VERBOSE;                     \
    }                                                                       \
    else if (MessageLevel & GFXDBG_NORMAL)                                  \
    {                                                                       \
        MessageLevel |= GFXDBG_VERBOSE;                                     \
    }                                                                       \
                                                                            \
    /* Some of routines need to call the debug message functionality    */  \
    /* before the DebugControl variable has been initialized. Hence, if */  \
    /* pDebugControl is NULL, unconditionally print the debug message.  */  \
                                                                            \
    if ((pDebugControl == NULL) ||                                          \
    ((pDebugControl->DebugEnableMask & ComponentMask) &&                    \
    (MessageLevel & pDebugControl->DebugLevel[ComponentId])))               \
    {                                                                       \
        va_start (ArgList, MessageFmt);                                     \
        Length = _vscprintf(MessageFmt, ArgList) + 1;                       \
        PrintBuffer = malloc(Length * sizeof(char));                        \
        if (PrintBuffer)                                                    \
        {                                                                   \
            vsprintf_s(PrintBuffer, Length, MessageFmt, ArgList);           \
            Prefix = ComponentIdStrings[ComponentId];                       \
            __MESSAGE_PRINT(Prefix, PrintBuffer);                           \
            free(PrintBuffer);                                              \
        }                                                                   \
        else                                                                \
        {                                                                   \
            __MESSAGE_PRINT("INTC DEBUG: ",                                 \
                            "Can not allocate print buffer\n");             \
            __debugbreak();                                                 \
        }                                                                   \
        va_end(ArgList);                                                    \
    }                                                                       \
}

#else

#ifndef __linux__
    #include "igdKrnlEtwMacros.h"
#endif

#define MESSAGE_FUNCTION(FUNCTION_NAME,COMPONENT_ID)                        \
                                                                            \
    void FUNCTION_NAME(ULONG MessageLevel, const char *MessageFmt, ...)     \
{                                                                           \
    INT32 Length = 0;                                                       \
    char PrintBuffer[GFX_MAX_MESSAGE_LENGTH];                               \
    char *Prefix = NULL;                                                    \
    va_list ArgList;                                                        \
    ULONG GfxDbgLvl = MessageLevel;                                         \
    ULONG ComponentId   = COMPONENT_ID;                                     \
    ULONG ComponentMask = 1 << COMPONENT_ID;                                \
                                                                            \
    /* Ensure that CRITICAL messages are printed if the setting of the */   \
    /* global debug variables flag is NORMAL or VERBOSE. Similarly, if */   \
    /* the setting of the debug variables flag is NORMAL, ensure that  */   \
    /* the VERBOSE message are printed out too!                        */   \
                                                                            \
    if (MessageLevel & GFXDBG_CRITICAL)                                     \
    {                                                                       \
        MessageLevel |= GFXDBG_NORMAL | GFXDBG_VERBOSE;                     \
    }                                                                       \
    else if (MessageLevel & GFXDBG_NORMAL)                                  \
    {                                                                       \
        MessageLevel |= GFXDBG_VERBOSE;                                     \
    }                                                                       \
                                                                            \
    /* Some of routines need to call the debug message functionality    */  \
    /* before the DebugControl variable has been initialized. Hence, if */  \
    /* pDebugControl is NULL, unconditionally print the debug message.  */  \
                                                                            \
    va_start(ArgList, MessageFmt);                                          \
    Length = _vsnprintf_s(PrintBuffer, GFX_MAX_MESSAGE_LENGTH,              \
                        sizeof(PrintBuffer),                                \
                        MessageFmt, ArgList);                               \
    if (Length >= 0)                                                        \
    {                                                                       \
        EtwDebugPrint((USHORT)GfxDbgLvl, (USHORT)ComponentId, PrintBuffer); \
                                                                            \
        if ((pDebugControl == NULL) ||                                      \
        ((pDebugControl->DebugEnableMask & ComponentMask) &&                \
        (MessageLevel & pDebugControl->DebugLevel[ComponentId])))           \
        {                                                                   \
            Prefix = ComponentIdStrings[ComponentId];                       \
            __MESSAGE_PRINT(Prefix, PrintBuffer);                           \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        __MESSAGE_PRINT("INTC DEBUG: ",                                     \
                        "The print buffer is to small\n");                  \
        __debugbreak();                                                     \
    }                                                                       \
    va_end(ArgList);                                                        \
}                                                                           \

#endif
#endif // __DEBUG_MESSAGE || __RELEASE_MESSAGE

// Define a max size for the intermediate buffer for storing the 
// debug error message string.

#define GFX_MAX_MESSAGE_LENGTH        (512)

//==============================================================================
// From winnt.h
//
// C_ASSERT() can be used to perform many COMPILE-TIME assertions:
//            type sizes, field offsets, etc.
//
// An assertion failure results in error C2118: negative subscript.
// 
// When this assertion is to be used in the middle of a code block,
// use it within {} - e.g. {__GL_C_ASSERT (__GL_NUM == 0);}
//
// Since all components may not want to include "winnt.h", define a 
// C_ASSERT that can be used by all components.
#ifndef GFX_C_ASSERT
#define GFX_C_ASSERT(e) typedef char __GFX_C_ASSERT__[(e)?1:-1]
#endif


// Unfortunately, we cannot include "g_debug.h" before this structure
// definition since it needs this structure - but then this structure
// requires the number of components. We get around this by using a 
// large enough number for the component count.

#define MAX_COMPONENT_COUNT_DONOTUSE        (20)


//------------------------------------------------------------------------------
// Debug and assert control structure. Note that each component has
// a separate variable to control its debug level.
//------------------------------------------------------------------------------

typedef struct GFX_DEBUG_CONTROL_REC 
{
    ULONG   Version;
    ULONG   Size;
    ULONG   AssertEnableMask;
    ULONG   EnableDebugFileDump;
    ULONG   DebugEnableMask;
    ULONG   RingBufDbgMask;
    ULONG   ReportAssertEnable;
    ULONG   AssertBreakDisable;

#ifndef __UMD
    ULONG   DebugLevel[MAX_COMPONENT_COUNT_DONOTUSE];
#endif

} GFX_DEBUG_CONTROL, *PGFX_DEBUG_CONTROL;

#ifdef __cplusplus
    extern "C" {
#endif
#ifdef __GFXDEBUG_C__          // Only to be defined by "gfxDebug.c" file.
    GFX_DEBUG_CONTROL  *pDebugControl = NULL;
#else
    extern GFX_DEBUG_CONTROL  *pDebugControl;
#endif
#ifdef __cplusplus
    }
#endif

    void InitializeDebugVariables(PGFX_DEBUG_CONTROL pDebugControl);

#if (defined(_DEBUG) || defined( _RELEASE_INTERNAL ))

#if ( !defined(REPORT_ASSERT) || !defined(REPORT_ASSERT_ETW)) && defined( _WIN32 ) 
#include "AssertTracer.h"
#elif !defined(REPORT_ASSERT)
#define REPORT_ASSERT( expr )
#define REPORT_ASSERT_ETW(compId, componentMask, expr)
#endif
#endif

#ifdef _DEBUG

    // This function doesn't do anything, it exists so you can put
    // a break point at the XXX_ASSERT and when you step into it
    // you will step into this function and then you can
    // disable/enable the assert by setting AssertOn to FALSE/TRUE.
#ifdef _MSC_VER
    static __inline int ToggleAssert(int AssertOn)
#else
    static inline int ToggleAssert(int AssertOn)
#endif
    {
        return AssertOn;
    };

#define __ASSERT(pDebugControl, compId, componentMask, expr)                    \
    {                                                                           \
        static int __assertOn = TRUE;                                           \
        __assertOn = ToggleAssert(__assertOn);                                  \
        REPORT_ASSERT_ETW(compId, componentMask, expr);                         \
        if (__assertOn)                                                         \
        {                                                                       \
            if(!(pDebugControl))                                                \
            {                                                                   \
                if(!(expr))                                                     \
                {                                                               \
                    REPORT_ASSERT(expr);                                        \
                    __debugbreak();                                             \
                }                                                               \
            }                                                                   \
            else                                                                \
            {                                                                   \
                if (((componentMask) & pDebugControl->AssertEnableMask) &&      \
                !(expr))                                                        \
                {                                                               \
                    if(pDebugControl->ReportAssertEnable)                       \
                    {                                                           \
                        REPORT_ASSERT(expr);                                    \
                    }                                                           \
                    if(!pDebugControl->AssertBreakDisable)                      \
                    {                                                           \
                        __debugbreak();                                         \
                    }                                                           \
                }                                                               \
                                                                                \
            }                                                                   \
        }                                                                       \
    }

#define __ASSERTPTR(pDebugControl, compId, componentMask, expr, ret)            \
    {                                                                           \
        __ASSERT(pDebugControl, compId, componentMask, expr);                   \
        if (!expr)                                                              \
        {                                                                       \
            return ret;                                                         \
        }                                                                       \
    }

#elif defined(_RELEASE_INTERNAL)

#define __ASSERT(pDebugControl, compId, compMask, expr)                     \
        {                                                                   \
            REPORT_ASSERT_ETW(compId, compMask, expr);                      \
        }

#define __ASSERTPTR(pDebugControl, compId, compMask, expr, ret)             \
        {                                                                   \
            __ASSERT(pDebugControl, compId, compMask, expr);                \
            if (!expr)                                                      \
            {                                                               \
                return ret;                                                 \
            }                                                               \
        }

#else
    #define __ASSERT(pDebugControl, compId, compMask, expr)
    #define __ASSERTPTR(pDebugControl, compId, compMask ,expr, ret)
#endif // _DEBUG


#define __RELEASEASSERT(pDebugControl, componentMask, expr)                  \
{                                                                            \
    if (((componentMask) & pDebugControl->AssertEnableMask) && !(expr))      \
        {                                                                    \
            __debugbreak();                                                  \
        }                                                                    \
}


// The common code may use a macro such as "GFXASSERT". This is defined 
// to be UMD or KMD ASSERT based on the component being compiled.

#ifdef __UMD
    #define GFXASSERT       UMDASSERT
#else
    #define GFXASSERT       KM_ASSERT
#endif


#include "g_gfxDebug.h"     // Include automatically generated component
                            // specific macros etc.


GFX_C_ASSERT(MAX_COMPONENT_COUNT_DONOTUSE >= GFX_COMPONENT_COUNT);

// !!! WE MIGHT BE ABLE TO DELETE THE FOLLOWING, DOUBLE CHECK FIRST

// Ring buffer proxy
#define ENABLE_RINGBUF_PROXY_IN_RELEASE_BUILD       0

#endif   // D3D not defined

#endif // _GFXDEBUG_H_
