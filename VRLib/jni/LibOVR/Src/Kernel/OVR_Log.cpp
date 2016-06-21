/************************************************************************************

Filename    :   OVR_Log.cpp
Content     :   Logging support
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

************************************************************************************/

#include "OVR_Log.h"
#include "OVR_Std.h"
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(OVR_OS_WIN32)
#include <windows.h>
#elif defined(OVR_OS_ANDROID)
#include <android/log.h>
#endif
#include "Kernel/OVR_Atomic.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Array.h"
namespace OVR {

// Global Log pointer.
Log* volatile OVR_GlobalLog = 0;
FILE* pfLog = NULL;

void Log::Init()
{
	if(access("/sdcard/FancyTech/",0)!=0) //0 表示存在
		mkdir("/sdcard/FancyTech/",S_IRWXU);
	const char* filename = "/sdcard/FancyTech/log.txt";
	FILE* pf = fopen(filename, "w+");
	if (pf == NULL) {
		//LOGE("create log file failed! %s", filename);
		::exit(1);
	}

	//LOGI("create log file %s", filename);
	pfLog = pf;
}

//-----------------------------------------------------------------------------------
// ***** Log Implementation

Log::~Log()
{
    // Clear out global log
    if (this == OVR_GlobalLog)
    {
        // TBD: perhaps we should ASSERT if this happens before system shutdown?
        OVR_GlobalLog = 0;
    }
}

void Log::LogMessageVarg(LogMessageType messageType, const char* fmt, va_list argList)
{
    if ((messageType & LoggingMask) == 0)
        return;
#ifndef OVR_BUILD_DEBUG
    if (IsDebugMessage(messageType))
        return;
#endif

    char buffer[MaxLogBufferMessageSize];
    FormatLog(buffer, MaxLogBufferMessageSize, messageType, fmt, argList);
    DefaultLogOutput(messageType, buffer);
}

void OVR::Log::LogMessage(LogMessageType messageType, const char* pfmt, ...)
{
    va_list argList;
    va_start(argList, pfmt);
    LogMessageVarg(messageType, pfmt, argList);
    va_end(argList);
}


void Log::FormatLog(char* buffer, unsigned bufferSize, LogMessageType messageType,
                    const char* fmt, va_list argList)
{    
    bool addNewline = true;

    switch(messageType)
    {
    case Log_Error:         OVR_strcpy(buffer, bufferSize, "Error: ");     break;
    case Log_Debug:         OVR_strcpy(buffer, bufferSize, "Debug: ");     break;
    case Log_Assert:        OVR_strcpy(buffer, bufferSize, "Assert: ");    break;
    case Log_Text:       buffer[0] = 0; addNewline = false; break;
    case Log_DebugText:  buffer[0] = 0; addNewline = false; break;
    default:        
        buffer[0] = 0;
        addNewline = false;
        break;
    }

    UPInt prefixLength = OVR_strlen(buffer);
    char *buffer2      = buffer + prefixLength;
    OVR_vsprintf(buffer2, bufferSize - prefixLength, fmt, argList);

    if (addNewline)
        OVR_strcat(buffer, bufferSize, "\n");
}


void Log::DefaultLogOutput(LogMessageType messageType, const char* formattedText)
{

#if defined(OVR_OS_WIN32)
    // Under Win32, output regular messages to console if it exists; debug window otherwise.
    static DWORD dummyMode;
    static bool  hasConsole = (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) &&
                              (GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dummyMode));

    if (!hasConsole || IsDebugMessage(messageType))
    {
        ::OutputDebugStringA(formattedText);
    }
    else
    {
         fputs(formattedText, stdout);
    }    

#elif defined(OVR_OS_ANDROID)

    int logPriority = ANDROID_LOG_INFO;
    switch(messageType)
    {
    case Log_DebugText:    
    case Log_Debug:     logPriority = ANDROID_LOG_DEBUG; break;
    case Log_Assert:
    case Log_Error:     logPriority = ANDROID_LOG_ERROR; break;
    default:
        logPriority = ANDROID_LOG_INFO;       
    }

	__android_log_write(logPriority, "OVR", formattedText);

	fprintf(pfLog, "%s\n", formattedText);

	fflush(pfLog);

#else
    fputs(formattedText, stdout);

#endif

    // Just in case.
    OVR_UNUSED2(formattedText, messageType);
}


//static
void Log::SetGlobalLog(Log *log)
{
    OVR_GlobalLog = log;
}
//static
Log* Log::GetGlobalLog()
{
// No global log by default?
//    if (!OVR_GlobalLog)
//        OVR_GlobalLog = GetDefaultLog();
    return OVR_GlobalLog;
}

//static
Log* Log::GetDefaultLog()
{
    // Create default log pointer statically so that it can be used
    // even during startup.
    static Log defaultLog;
    return &defaultLog;
}


//-----------------------------------------------------------------------------------
// ***** Global Logging functions

#define OVR_LOG_FUNCTION_IMPL(Name)  \
    void Log##Name(const char* fmt, ...) \
    {                                                                    \
        if (OVR_GlobalLog)                                               \
        {                                                                \
            va_list argList; va_start(argList, fmt);                     \
            OVR_GlobalLog->LogMessageVarg(Log_##Name, fmt, argList);  \
            va_end(argList);                                             \
        }                                                                \
    }

OVR_LOG_FUNCTION_IMPL(Text)
OVR_LOG_FUNCTION_IMPL(Error)

#ifdef OVR_BUILD_DEBUG
OVR_LOG_FUNCTION_IMPL(DebugText)
OVR_LOG_FUNCTION_IMPL(Debug)
OVR_LOG_FUNCTION_IMPL(Assert)
#endif
	        

	sssa_log_obj::sssa_log_obj(const char* filename,int line,const char* funcname)
	{
		char buff[ 512 ];


		OVR_sprintf( buff, 512, "sssa LOG: fuc=%s", funcname);
		m_text = buff;
		LogText("%s, begin", m_text.ToCStr());
	}
	sssa_log_obj::~sssa_log_obj()
	{
		LogText("%s, end", m_text.ToCStr());
	}

	//////
	class intent_manager //日志的缩进管理，每个线程单独管理
	{
	public :
		intent_manager()
		{

		}
		~intent_manager()
		{

		}
		static intent_manager* GetIns()
			{
				static intent_manager* ins = 0;
				if (!ins)
					ins = new intent_manager;
				return ins;
			}
		int get_intent_thread(int tid,int enterOrExit)
		{
			Lock::Locker sopelock(&m_lock);
			for(int i=0;i<m_thread_intent.GetSizeI();++i)
			{
				intentInfo& _info=m_thread_intent[i];
				if(_info.m_tid==tid)
					return (_info.m_intent_value+=enterOrExit,_info.m_intent_value);
			}
			m_thread_intent.PushBack(intentInfo(tid));
			return 0;
		}

		//当要记录一个函数begin时，调用该方法，返回值是日志应该采用的缩进
		int on_enter_call()
		{
			int tid=(int)(pthread_self());
			return get_intent_thread(tid,1);
		}
		int on_exit_call()
		{
			int tid=(int)(pthread_self());
			return get_intent_thread(tid,-1);
		}
	private:
		struct intentInfo
		{
			intentInfo(int tid)
			{
				m_tid=tid;
				m_intent_value=0;
			}
			int m_tid;
			int m_intent_value;
		};
		Lock m_lock;
		Array<intentInfo> m_thread_intent;
		//int m_thread_intent;

	};
	void sssa_log_obj_ex::BuildIntentString(int intentval)
	{
		for(int i=0;i<intentval;++i)
			m_text.AppendChar(' ');
	}
	sssa_log_obj_ex::sssa_log_obj_ex(const char* filename,int line,const char* funcname)
	{
		m_text.Clear();
		int intentval=intent_manager::GetIns()->on_enter_call();
		BuildIntentString(intentval);

		m_text.AppendFormat(" [%d]sssaLog: file=%s,line=%d,fuc=%s", intentval,filename, line, funcname);
		LogText("%s, begin", m_text.ToCStr());

	}
	sssa_log_obj_ex::~sssa_log_obj_ex()
	{
		LogText("%s, end", m_text.ToCStr());
		intent_manager::GetIns()->on_exit_call();
	}
} // OVR
