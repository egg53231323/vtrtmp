/************************************************************************************

Filename    :   OVR_System.cpp
Content     :   General kernel initialization/cleanup, including that
                of the memory allocator.
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

************************************************************************************/

#include "OVR_System.h"
#include "OVR_Threads.h"
#include "OVR_Timer.h"

namespace OVR {

// *****  OVR::System Implementation

// Initializes System core, installing allocator.
void System::Init(Log* log, Allocator *palloc)
{    
	//LogText("		System::Init begin");
    if (!Allocator::GetInstance())
    {
        Log::SetGlobalLog(log);
        Timer::initializeTimerSystem();
        Allocator::setInstance(palloc);
#ifdef OVR_ENABLE_THREADS
        Thread::InitThreadList();
#endif
    }
    else
    {
        OVR_DEBUG_LOG(("System::Init failed - duplicate call."));
    }
    //LogText("		System::Init end");
}

void System::Destroy()
{    
	OVR_DEBUG_LOG(("System::Destroy"));
    if (Allocator::GetInstance())
    {
#ifdef OVR_ENABLE_THREADS
        // Wait for all threads to finish; this must be done so that memory
        // allocator and all destructors finalize correctly.
        Thread::FinishAllThreads();
#endif

        // Shutdown heap and destroy SysAlloc singleton, if any.
		// modify by sssa2000 不注释掉这两句 切换activity的时候会出错
        //Allocator::GetInstance()->onSystemShutdown();
        //Allocator::setInstance(0);

        Timer::shutdownTimerSystem();
        Log::SetGlobalLog(Log::GetDefaultLog());
    }
    else
    {
        OVR_DEBUG_LOG(("System::Destroy failed - System not initialized."));
    }
}

// Returns 'true' if system was properly initialized.
bool System::IsInitialized()
{
    return Allocator::GetInstance() != 0;
}

} // OVR

