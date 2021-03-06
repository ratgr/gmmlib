/*==============================================================================
Copyright(c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files(the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and / or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
============================================================================*/

#pragma once

#if (_DEBUG || _RELEASE_INTERNAL) && !__GMM_KMD__
    // Not doing #if__cplusplus >= 201103L check because partial C++11 support may 
    // work for this. We also want to catch C++11 unavailability due to not setting
    // compiler options.
    #if (_MSC_VER >= 1800) // VS 2013+
        #define GMM_LOG_AVAILABLE 1
    #elif((__clang_major__ > 3) || ((__clang_major__ == 3) && (__clang_minor__ >= 5))) // clang 3.5+
        #define GMM_LOG_AVAILABLE 1
    #elif((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8))) // g++ 4.8+
        #define GMM_LOG_AVAILABLE 1
    #else
        #define GMM_LOG_AVAILABLE 0
        // Print out messages if GmmLog was disabled due to compiler issues
        #define STRING2(x) #x
        #define STRING(x) STRING2(x)

        #if (defined __GNUC__) 
            #pragma message "Detected g++ " STRING(__GNUC__) "." STRING(__GNUC_MINOR__) ". Minimum compiler version required for GmmLog is GCC 4.8.1. Disabling GmmLog."
        #elif (defined __clang__) 
            #pragma message "Detected clang " STRING(__clang_major__) "." STRING(__clang_minor__) ". Minimum compiler version required for GmmLog is clang 3.5. Disabling GmmLog."
        #elif (defined _MSC_VER)
            #pragma message("Detected MSVC++ version " STRING(_MSC_VER) ". Minimum compiler version required for GmmLog is MSVC++ 1800. Disabling GmmLog")
        #else
            #pragma message "Unknown compiler. Disabling GmmLog."
        #endif

        #undef STRING2
        #undef STRING
    #endif
#elif (_DEBUG || _RELEASE_INTERNAL) && __GMM_KMD__ && _WIN32
#define GMM_KMD_LOG_AVAILABLE 1

/////////////////////////////////////////////////////////////////////////////////////
/// GMM_KMD_LOG 
/// Gmm logging macro to log a message in KMD mode. Exmaple: 
///     GMM_KMD_LOG("Math Addition: %d + %d = %d \r\n", 1, 1, 2);
/////////////////////////////////////////////////////////////////////////////////////
#define GMM_KMD_LOG(message, ...)                                                                   \
        {                                                                                           \
            DWORD CurrentProcessId = (DWORD) (ULONG_PTR)PsGetCurrentProcessId();                    \
            const WCHAR *format = L"%s%d.txt";                                                      \
            WCHAR FileName[] = L"C:\\Intel\\IGfx\\GmmKmd_Proc_";                                    \
            WCHAR FullFileName[KM_FILENAME_LENGTH];                                                 \
                                                                                                    \
            KmGenerateLogFileName(&FullFileName[0], format, FileName, CurrentProcessId);            \
                                                                                                    \
            KM_FILE_IO_OBJ *pGmmKmdLog = KmFileOpen(pHwDevExt, FullFileName, FALSE, FALSE, TRUE);   \
            if (pGmmKmdLog != NULL)                                                                 \
            {                                                                                       \
                KmFilePrintf(pGmmKmdLog, message, __VA_ARGS__);                                     \
                KmFileClose(pGmmKmdLog, FALSE);                                                     \
            }                                                                                       \
        }                                                                                           \

#else // else if Release driver || KMD
    #define GMM_LOG_AVAILABLE 0
#endif

#if GMM_LOG_AVAILABLE
#ifdef __cplusplus

#include <iostream>
#include "spdlog/spdlog.h"

#define GMM_LOG_FILE_SIZE         1024 * 1024 * 5   // Once log reaches this size, it will start in new file
#define GMM_ROTATE_FILE_NUMBER    3                 // Once log is full, it'll save old log with .1/.2/.3 in name, and then start with .1
#define GMM_LOG_MASSAGE_MAX_SIZE  1024
#define GMM_LOGGER_NAME           "gmm_logger"
#define GMM_LOG_FILENAME          "./gmm_log"
#define GMM_LOG_TAG               "GmmLib"
#define GMM_UNKNOWN_PROCESS       "Unknown_Proc"

#if _WIN32
    #define GMM_LOG_REG_KEY_SUB_PATH  "SOFTWARE\\Intel\\IGFX\\GMMLOG\\"
    #define GMM_LOG_TO_FILE           "LogToFile"
    enum GmmLogLevel
    {
        Off = 0, 
        Trace,
        Info,
        Error, // default
    };
    #define GMM_LOG_LEVEL_REGKEY      "LogLevel" // GmmLogLevel
#endif //#if _WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// @file Logger.h
/// @brief This file contains the functions that are useful for logging in GmmLib
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    class Logger
    {
        private:
            /// This enum determines where the log goes
            enum GmmLogMethod 
            {
                ToOSLog,                                        ///< Log is sent to OS logging infrastructure (Debugger on Windows)
                ToFile,                                         ///< Log is written to file
            } LogMethod;                                        ///< Indicates which Logging method is preferred

            spdlog::level::level_enum       LogLevel;           ///< Indicates the max log level to print

        private:
            Logger();
            ~Logger();

            bool            GmmLogInit();

        public:
            std::shared_ptr<spdlog::logger> SpdLogger;          ///< spdlog instance

            /////////////////////////////////////////////////////////////////////////////////////
            /// Creates a Logger singlton per process
            ///
            /// @remark
            ///     If there are multiple modules loaded in one process, this singleton will be 
            ///     per module. For example: D3D10 and D3D12 are all used in process A. Both D3D10 
            ///     and D3D12 are built with GmmLib.lib. They are considered to be two modules and
            ///     each has a Logger instance (singleton).
            ///
            /// @return Logger instance
            /////////////////////////////////////////////////////////////////////////////////////
            static Logger& CreateGmmLogSingleton()
            {
                static Logger GmmLoggerPerProc;
                return GmmLoggerPerProc;
            }
    };
} // namespace GmmLib

// Macros
extern GmmLib::Logger& GmmLoggerPerProc;

#define GMM_LOG_TRACE(message, ...) if(GmmLoggerPerProc.SpdLogger) { GmmLoggerPerProc.SpdLogger->trace(message, ##__VA_ARGS__); }
#define GMM_LOG_TRACE_IF(expression, message, ...) if(GmmLoggerPerProc.SpdLogger && (expression)) { GMM_LOG_TRACE(message, ##__VA_ARGS__); }

#define GMM_LOG_INFO(message, ...) if(GmmLoggerPerProc.SpdLogger) { GmmLoggerPerProc.SpdLogger->info(message, ##__VA_ARGS__); }
#define GMM_LOG_INFO_IF(expression, message, ...) if(GmmLoggerPerProc.SpdLogger && (expression)) { GMM_LOG_INFO(message, ##__VA_ARGS__); }

#define GMM_LOG_ERROR(message, ...) if(GmmLoggerPerProc.SpdLogger) { GmmLoggerPerProc.SpdLogger->error(message, ##__VA_ARGS__); }
#define GMM_LOG_ERROR_IF(expression, message, ...) if(GmmLoggerPerProc.SpdLogger && (expression)) { GMM_LOG_INFO(message, ##__VA_ARGS__); }

#else // else C
    // No support for C files -- Revisit later if really necessary. Do not define any logging macros, so any usage by
    // C file can be caught as compilation error.
#endif //#ifdef __cplusplus

#else // else Gmm Log not available 

#define GMM_LOG_TRACE(message, ...) 
#define GMM_LOG_TRACE_IF(expression, message, ...) 

#define GMM_LOG_INFO(message, ...) 
#define GMM_LOG_INFO_IF(expression, message, ...) 

#define GMM_LOG_ERROR(message, ...) 
#define GMM_LOG_ERROR_IF(expression, message, ...)

#endif //#if _WIN32

#if GMM_KMD_LOG_AVAILABLE
#else

#define GMM_KMD_LOG(message, ...)

#endif
