/************************************************************************************

Filename    :   OVR_Android_DeviceManager.h
Content     :   Android implementation of DeviceManager.
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/

#include "OVR_Android_DeviceManager.h"

// Sensor & HMD Factories
#include "OVR_LatencyTestDeviceImpl.h"
#include "OVR_SensorDeviceImpl.h"
#include "OVR_Android_HIDDevice.h"
#include "OVR_Android_HMDDevice.h"

#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Std.h"
#include "Kernel/OVR_Log.h"
#include "Android/LogUtils.h"
#include "Kernel/OVR_Alg.h"
#include <jni.h>

jobject gRiftconnection;


#define EVENT_IDEN 23

namespace OVR { 

	int ReadSensorDataFromUsb();
	int ReadInnerSensorData();
	extern Array<SensorDeviceImpl*>  GSensorDevice;

	namespace Android {

	struct SensorObj
	{
		ASensorRef	sensor;
		int			id;
		const char*	name;
		int			type;
		float		resolution;
		int			minDelay;

		SensorObj(ASensorRef ref, int _id)
		{
			sensor = ref;
			id = _id;
			name = ASensor_getName(sensor);
			type = ASensor_getType(sensor);
			resolution = ASensor_getResolution(sensor);
			minDelay = ASensor_getMinDelay(sensor);
		}

		void Enable(ASensorEventQueue* evQueue)
		{
			int succ =  ASensorEventQueue_enableSensor(evQueue, sensor);
			if (succ < 0)
			{
				LogText("DeviceManagerThread - enable Sensor %s Failed !!!!\n", name);
			}
			else{
				ASensorEventQueue_setEventRate(evQueue, sensor, minDelay);
			}
		}

		void Disable(ASensorEventQueue* evQueue)
		{
			int succ = ASensorEventQueue_disableSensor(evQueue, sensor);

			if (succ < 0)
			{
				LogText("DeviceManagerThread - disable Sensor %s Failed !!!!\n", name);
			}
		}

		void PrintInfo()
		{
			LogText("ASensor: name = %s , type = %d , res = %f , delay = %d \n", name , type , resolution , minDelay);
		}

	};

//-------------------------------------------------------------------------------------
// **** Android::DeviceManager

DeviceManager::DeviceManager()
{
	SSSA_LOG_FUNCALL(1);
}

DeviceManager::~DeviceManager()
{    
}

bool DeviceManager::Initialize(DeviceBase*)
{
	SSSA_LOG_FUNCALL(1);
    if (!DeviceManagerImpl::Initialize(0))
    {
    	LOG("OVR::DeviceManager - initialized failed.\n");
        return false;
    }
    LOG("DeviceManager::Initialize CheckPoint 1");
    pThread = *new DeviceManagerThread();
    if (!pThread || !pThread->Start())
        return false;

    LOG("DeviceManager::Initialize CheckPoint 2");
    LOG("HEHEHE");

    // Wait for the thread to be fully up and running.
	//等待线程初始化完毕
    pThread->StartupEvent.Wait();

    LOG("DeviceManager::Initialize CheckPoint 3");
    // Do this now that we know the thread's run loop.
	//创建HID设备（USB设备）管理器
    HidDeviceManager = *HIDDeviceManager::CreateInternal(this);
         
    pCreateDesc->pDevice = this;
    LogText("OVR::DeviceManager - initialized.\n");
    return true;
}

void DeviceManager::Shutdown()
{   
    LogText("OVR::DeviceManager - shutting down.\n");

    // Set Manager shutdown marker variable; this prevents
    // any existing DeviceHandle objects from accessing device.
    pCreateDesc->pLock->pManager = 0;

    // Push for thread shutdown *WITH NO WAIT*.
    // This will have the following effect:
    //  - Exit command will get enqueued, which will be executed later on the thread itself.
    //  - Beyond this point, this DeviceManager object may be deleted by our caller.
    //  - Other commands, such as CreateDevice, may execute before ExitCommand, but they will
    //    fail gracefully due to pLock->pManager == 0. Future commands can't be enqued
    //    after pManager is null.
    //  - Once ExitCommand executes, ThreadCommand::Run loop will exit and release the last
    //    reference to the thread object.
    pThread->PushExitCommand(false);
    pThread.Clear();

    DeviceManagerImpl::Shutdown();
}

ThreadCommandQueue* DeviceManager::GetThreadQueue()
{
    return pThread;
}

ThreadId DeviceManager::GetThreadId() const
{
    return pThread->GetThreadId();
}

int DeviceManager::GetThreadTid() const
{
    return pThread->GetThreadTid();
}

void DeviceManager::SuspendThread() const
{
    pThread->SuspendThread();
}

void DeviceManager::ResumeThread() const
{
    pThread->ResumeThread();
}

bool DeviceManager::GetDeviceInfo(DeviceInfo* info) const
{
	SSSA_LOG_FUNCALL(1);
    if ((info->InfoClassType != Device_Manager) &&
        (info->InfoClassType != Device_None))
        return false;
    
    info->Type    = Device_Manager;
    info->Version = 0;
    OVR_strcpy(info->ProductName, DeviceInfo::MaxNameLength, "DeviceManager");
    OVR_strcpy(info->Manufacturer,DeviceInfo::MaxNameLength, "Oculus VR, LLC");
    return true;
}

DeviceEnumerator<> DeviceManager::EnumerateDevicesEx(const DeviceEnumerationArgs& args)
{
	SSSA_LOG_FUNCALL(1);
    // TBD: Can this be avoided in the future, once proper device notification is in place?
    pThread->PushCall((DeviceManagerImpl*)this,
                      &DeviceManager::EnumerateAllFactoryDevices, true);

    return DeviceManagerImpl::EnumerateDevicesEx(args);
}


//-------------------------------------------------------------------------------------
// ***** DeviceManager Thread 



DeviceManagerThread::DeviceManagerThread()
    : Thread(ThreadStackSize),
      Suspend( false )
{
	SSSA_LOG_FUNCALL(1);
    int result = pipe(CommandFd);
	OVR_UNUSED( result );	// no warning
    OVR_ASSERT(!result);

    AddSelectFd(NULL, CommandFd[0]);
}

DeviceManagerThread::~DeviceManagerThread()
{
    if (CommandFd[0])
    {
        RemoveSelectFd(NULL, CommandFd[0]);
        close(CommandFd[0]);
        close(CommandFd[1]);
    }
}

bool DeviceManagerThread::AddSelectFd(Notifier* notify, int fd)
{
	SSSA_LOG_FUNCALL(1);
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN|POLLHUP|POLLERR;
    pfd.revents = 0;

    FdNotifiers.PushBack(notify);
    PollFds.PushBack(pfd);

    OVR_ASSERT(FdNotifiers.GetSize() == PollFds.GetSize());
    LogText( "DeviceManagerThread::AddSelectFd %d (Tid=%d)\n", fd, GetThreadTid() );
    return true;
}

bool DeviceManagerThread::RemoveSelectFd(Notifier* notify, int fd)
{
	SSSA_LOG_FUNCALL(1);
    // [0] is reserved for thread commands with notify of null, but we still
    // can use this function to remove it.

    LogText( "DeviceManagerThread::RemoveSelectFd %d (Tid=%d)\n", fd, GetThreadTid() );
    for (UPInt i = 0; i < FdNotifiers.GetSize(); i++)
    {
        if ((FdNotifiers[i] == notify) && (PollFds[i].fd == fd))
        {
            FdNotifiers.RemoveAt(i);
            PollFds.RemoveAt(i);
            return true;
        }
    }
    LogText( "DeviceManagerThread::RemoveSelectFd failed %d (Tid=%d)\n", fd, GetThreadTid() );
    return false;
}




void DeviceManagerThread::InitInnerSensor()
{
	SSSA_LOG_FUNCALL(1);
	ASensorManager* sensorManager = NULL;
	eventQueue = NULL;
	ALooper* looper = NULL;
	sensorManager = ASensorManager_getInstance();
	Array<SensorObj> sensors;
	//获取手机上全部Sensor
	ASensorList sensorList = NULL;
	int numSensor = ASensorManager_getSensorList(sensorManager, &sensorList);
	for (int i = 0; i < numSensor; i++) {
		int type = ASensor_getType(sensorList[i]);
		if (type == SENSOR_TYPE_ACCELEROMETER || type == SENSOR_TYPE_GYROSCOPE
				|| type == SENSOR_TYPE_MAGNETIC_FIELD
				|| type == SENSOR_TYPE_TEMPERATURE) {
			sensors.PushBack(SensorObj(sensorList[i], i));
		}
	}
	//打印全部Sensor信息
	for (UPInt i = 0; i < sensors.GetSize(); i++) {
		sensors[i].PrintInfo();
	}
	looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	looper = ALooper_forThread();
	if (looper == NULL) {
		LogText("Failed to create looper!");
	}
	eventQueue = ASensorManager_createEventQueue(sensorManager, looper,
			EVENT_IDEN, NULL, NULL);
	OVR_UNUSED(looper);
	OVR_UNUSED(eventQueue);

	//启动所有Sensor
	for (UPInt i = 0; i < sensors.GetSize(); i++)
	{
		sensors[i].Enable(eventQueue);
	}

}

int DeviceManagerThread::Run()
{
	SSSA_LOG_FUNCALL(1);
    ThreadCommand::PopBuffer command;

    SetThreadName("OVR::DeviceMngr");

    LogText( "DeviceManagerThread - running (Tid=%d).\n", GetThreadTid() );
    
    // needed to set SCHED_FIFO
    DeviceManagerTid = gettid();

	InitInnerSensor();
    // Signal to the parent thread that initialization has finished.
    StartupEvent.SetEvent();

    while(!IsExiting())
    {

        // PopCommand will reset event on empty queue.
        if (PopCommand(&command))
        {
            command.Execute();
        }
        else
		{
			if(ReadSensorDataFromUsb()>0)
				continue;
			else
			{
				//从内置陀螺仪拿数据
				ReadInnerSensorData();
			}
        }
    }

    LogText( "DeviceManagerThread - exiting (Tid=%d).\n", GetThreadTid() );
    return 0;
}


void DeviceManagerThread::ReadInnerSensorData()
{
	int ident;
	int events;
	struct android_poll_source* source;

	if ((ident = ALooper_pollAll(20, NULL, &events, (void**)&source)) >= 0)
	{
		if (ident != EVENT_IDEN){
			LogText( "ReadInnerSensorData ident != EVENT_IDEN");
			return;
		}

		LogText( "7777");
		ASensorEvent event;
		while (ASensorEventQueue_getEvents((ASensorEventQueue*)eventQueue, &event, 1) > 0)
		{
			SensorEventList.PushBack(event);
		}

		LogText( "6666");
		//int64_t currTimeStamp = 0x7fffffffffffffff;
		for (UPInt i = 0; i < SensorEventList.GetSize(); i++)
		{
			ASensorEvent ev = SensorEventList.At(i);
			if (ev.type == SENSOR_TYPE_ACCELEROMETER)
			{
				LastAccel.x = ev.vector.x;
				LastAccel.y = ev.vector.y;
				LastAccel.z = ev.vector.z;
			}
			else if (ev.type == SENSOR_TYPE_MAGNETIC_FIELD)
			{
				LastMag.x = ev.vector.x;
				LastMag.y = ev.vector.y;
				LastMag.z = ev.vector.z;
			}
			else if (ev.type == SENSOR_TYPE_GYROSCOPE)
			{
				LastGyro.x = ev.vector.x;
				LastGyro.y = ev.vector.y;
				LastGyro.z = ev.vector.z;
			}
			else if (ev.type == SENSOR_TYPE_TEMPERATURE)
			{
				LastTemperature = ev.temperature;
			}

			int64_t timeDelta = ev.timestamp - LastTimeStamp;
			LastTimeStamp = ev.timestamp;

			ASensorMessage amsg;
			amsg.accel = LastAccel;
			amsg.gyro = LastGyro;
			amsg.mag = LastMag;
			amsg.temperature = LastTemperature;
			amsg.timestamp = LastTimeStamp;


			if (GSensorDevice.GetSize() > 0)	{

				LogText( "5555");
							SensorDeviceImpl* device = GSensorDevice.Back();
							//device->OnTicks(timeSeconds);
							MessageBodyFrame msg(device);
							int timeDeltaTicks = (int)(timeDelta / 1000000);

							ASensorMessage* pMsg = &amsg;
							msg.AbsoluteTimeSeconds = OVR::Timer::GetSeconds();
							msg.RotationRate = Vector3f(-pMsg->gyro.y, pMsg->gyro.x, pMsg->gyro.z);
							msg.Acceleration = Vector3f(-pMsg->accel.y, pMsg->accel.x,pMsg->accel.z);
							msg.MagneticField = pMsg->mag;
							msg.Temperature = pMsg->temperature;
							msg.TimeDelta = (float)timeDeltaTicks / 1000.0f;;
							device->onTrackerInnerSensorMsg(msg);
						}
		}

		SensorEventList.Clear();
	}

}
bool DeviceManagerThread::AddTicksNotifier(Notifier* notify)
{
	SSSA_LOG_FUNCALL(1);
     TicksNotifiers.PushBack(notify);
     return true;
}

bool DeviceManagerThread::RemoveTicksNotifier(Notifier* notify)
{
	SSSA_LOG_FUNCALL(1);
    for (UPInt i = 0; i < TicksNotifiers.GetSize(); i++)
    {
        if (TicksNotifiers[i] == notify)
        {
            TicksNotifiers.RemoveAt(i);
            return true;
        }
    }
    return false;
}

void DeviceManagerThread::SuspendThread()
{
	SSSA_LOG_FUNCALL(1);
    Suspend = true;
    LogText( "DeviceManagerThread - Suspend = true\n" );
}

void DeviceManagerThread::ResumeThread()
{
	SSSA_LOG_FUNCALL(1);
    Suspend = false;
    LogText( "DeviceManagerThread - Suspend = false\n" );
}

} // namespace Android


//-------------------------------------------------------------------------------------
// ***** Creation


// Creates a new DeviceManager and initializes OVR.
DeviceManager* DeviceManager::Create()
{
	SSSA_LOG_FUNCALL(1);
    if (!System::IsInitialized())
    {
        // Use custom message, since Log is not yet installed.
        OVR_DEBUG_STATEMENT(Log::GetDefaultLog()->
            LogMessage(Log_Debug, "DeviceManager::Create failed - OVR::System not initialized"); );
        return 0;
    }

    Ptr<Android::DeviceManager> manager = *new Android::DeviceManager;

    if (manager)
    {
        if (manager->Initialize(0))
        {
			//manager->AddFactory(&MySensorDeviceFactory::GetInstance());
            manager->AddFactory(&LatencyTestDeviceFactory::GetInstance());
            manager->AddFactory(&SensorDeviceFactory::GetInstance());
            //manager->AddFactory(&Android::HMDDeviceFactory::GetInstance());

            manager->AddRef();
        }
        else
        {
            manager.Clear();
        }

    }    

    return manager.GetPtr();
}


} // namespace OVR

