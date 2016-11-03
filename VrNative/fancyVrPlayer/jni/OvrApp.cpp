/************************************************************************************

Filename    :   OvrApp.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
#include "OvrApp.h"
#include <jni.h>
#include <android/keycodes.h>

#include "PathUtils.h"
#include "Kernel\OVR_Log.h"

#include "VrApi/LocalPreferences.h"

//
//#define  NoPanorama_NoStereo 0 				//��ȫ�� �������ӰƬ��Ҳ��������ͨ��ӰƬ
//#define  NoPanorama_Stereo_Left_Right 1 	//��ȫ���� ���Ҹ�ʽ������ӰƬ
//#define  NoPanorama_Stereo_Up_Down 2		//��ȫ���� ���¸�ʽ������ӰƬ
//#define  Panorama_NoStereo 3				//ȫ�� �������ӰƬ
//#define  Panorama_Stereo_Left_Right 4		//ȫ���� ���Ҹ�ʽ������ӰƬ
//#define  Panorama_Stereo_Up_Down 5			//ȫ���� ���¸�ʽ������ӰƬ



extern "C" {

static jclass	GlobalActivityClass;
static SphereScreenConfig cfg1;
static QuadScreenConfig cfg2;
static ThreatreConfig cfg3;

void Java_oculus_movieViewActivity_nativeInitVrLib( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeInitVrLib" );
	//ovr_OnLoad( vm );
	ovr_Init();

}

void Java_oculus_movieViewActivity_nativeUnloadAppInterface( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeUnloadAppInterface" );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	ovr_ExitActivity(panoVids->app->GetOvrMobile(),EXIT_TYPE_EXIT);
	delete panoVids;

}


jlong Java_oculus_movieViewActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	SSSA_LOG_FUNCALL(1);
	LOG( "PlaygroundActivity_nativeSetAppInterface" );
	GlobalActivityClass = (jclass)jni->NewGlobalRef( clazz );
	return (new OvrApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}


void Java_oculus_movieViewActivity_nativeFrameAvailable( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->SetFrameAvailable( true );
}

jobject Java_oculus_movieViewActivity_nativePrepareNewVideo( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);

	// set up a message queue to get the return message
	// TODO: make a class that encapsulates this work
	MessageQueue	result( 1 );
	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "newVideo %p", &result );

	result.SleepUntilMessage();
	const char * msg = result.GetNextMessage();
	jobject	texobj;
	sscanf( msg, "surfaceTexture %p", &texobj );
	free( ( void * )msg );

	return texobj;
}

void Java_oculus_movieViewActivity_nativeSetVideoSize( JNIEnv *jni, jclass clazz, jlong interfacePtr, int width, int height ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeSetVideoSizes: width=%i height=%i", width, height );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "video %i %i", width, height );
}

void Java_oculus_movieViewActivity_nativeVideoCompletion( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeVideoCompletion" );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "completion" );
}

void Java_oculus_movieViewActivity_nativeVideoStartError( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeVideoStartError" );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "startError" );
}

void Java_oculus_movieViewActivity_nativeSetScreenTcMode( JNIEnv *jni, jclass clazz, jlong interfacePtr, int screenmode ,int tcmode)
{
	SSSA_LOG_FUNCALL(1);
	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );

	cfg1.tc_mode=(MOVIE_MAPPING)tcmode;
	cfg2.tc_mode=(MOVIE_MAPPING)tcmode;
	cfg3.tc_mode=(MOVIE_MAPPING)tcmode;

	LOG( "screenmode=%d,tcmode=%d", screenmode,tcmode);
	if(screenmode==SG_SPHERE)
		panoVids->SetUseSphereScreen(cfg1);
	else if(screenmode==SG_QUAD)
		panoVids->SetUseQuadScreen(cfg2);
	else
		panoVids->SetUseTheatre(cfg3);
}

void Java_oculus_movieViewActivity_nativeSetLocalPreference( JNIEnv *jni, jclass clazz, jstring key, jstring val ) {
	SSSA_LOG_FUNCALL(1);
	const char * keystr = jni->GetStringUTFChars( key, NULL );
	const char * valstr = jni->GetStringUTFChars( val, NULL );

	LOG( "nativeSetLocalPreference: \"%s\" = \"%s\"", keystr, valstr );
	ovr_SetLocalPreferenceValueForKey(keystr, valstr);

	jni->ReleaseStringUTFChars( key, keystr );
	jni->ReleaseStringUTFChars( val, valstr );
}

} // extern "C"

OvrApp::OvrApp()
{
	MovieTexture=NULL;
	FrameAvailable=false;
	CurrentVideoWidth=0;
	CurrentVideoHeight=480;
	BackgroundWidth=0;
	BackgroundHeight= 0 ;
	VideoMode=3;

	m_pQuadScreen=new MoiveScreenQuad();
	m_pSphereScreen=new MoiveScreenSphere();
	m_pTheatre=new MoiveTheatre();
	m_pCurrentScreen=(MovieScreen*)m_pSphereScreen;
	
}

OvrApp::~OvrApp()
{
	SSSA_LOG_FUNCALL(1);

	//ovr_LeaveVrMode(app->GetOvrMobile());
	//ovr_ExitActivity(app->GetOvrMobile(),EXIT_TYPE_FINISH);
	app=0;
	//nativeDestroy();
}


void OvrApp::UninstallShaderAndScreen()
{

	delete m_pQuadScreen;
	delete m_pSphereScreen;
	delete m_pTheatre;

	m_pCurrentScreen=0;

	m_ShaderMng->CleanUpShaders();
	delete m_ShaderMng;
}

void OvrApp::InitShaderAndScreen() {
	SSSA_LOG_FUNCALL(1);
	m_ShaderMng=new ShaderManager();
	m_ShaderMng->InitShaders();

	m_pQuadScreen->Init();
	m_pSphereScreen->Init();
	m_pTheatre->Init();
}

void OvrApp::SetUseSphereScreen(const SphereScreenConfig& cfg)
{
	SSSA_LOG_FUNCALL(1);
	m_pCurrentScreen=(MovieScreen*)m_pSphereScreen;
	m_pSphereScreen->SetConfig(cfg);
}
void OvrApp::SetUseQuadScreen(const QuadScreenConfig& cfg)
{
	SSSA_LOG_FUNCALL(1);
	m_pCurrentScreen=(MovieScreen*)m_pQuadScreen;
	m_pQuadScreen->SetConfig(cfg);
}
void OvrApp::SetUseTheatre(const ThreatreConfig& cfg)
{
	m_pCurrentScreen=(MovieScreen*)m_pTheatre;
	m_pTheatre->SetConfig(cfg);
}
void OvrApp::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	// This is called by the VR thread, not the java UI thread.
	LOG( "--------------- OvrApp OneTimeInit ---------------" );

	VideoMode = 0;
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);


	app->GetVrParms().colorFormat = COLOR_8888;
	app->GetVrParms().depthFormat = DEPTH_16;
	app->GetVrParms().multisamples = 2;

	LOG( "InitShader" );
	InitShaderAndScreen();

	// Stay exactly at the origin, so the panorama globe is equidistant
	// Don't clear the head model neck length, or swipe view panels feel wrong.
	VrViewParms viewParms = app->GetVrViewParms();
	viewParms.EyeHeight = 0.0f;
	app->SetVrViewParms( viewParms );
	app->SetShowFPS(true);

	//����ӰƬ��Ҫ�ر�time wrap
	ovrModeParms param=app->GetVrModeParms();
	param.AsynchronousTimeWarp = false;
	app->SetVrModeParms(param);

	// Optimize for 16 bit depth in a modest theater size
	Scene.Znear = 0.1f;
	Scene.Zfar = 2000.0f;

}

void OvrApp::ConfigureVrMode( ovrModeParms & modeParms )
{
	// We need very little CPU for pano browsing, but a fair amount of GPU.
	// The CPU clock should ramp up above the minimum when necessary.
	//LOG( "ConfigureClocks: Oculus360Videos only needs minimal clocks" );
	modeParms.CpuLevel = 1;
	modeParms.GpuLevel = 0;

	// When the app is throttled, go to the platform UI and display a
	// dismissable warning. On return to the app, force 30hz timewarp.
	modeParms.AllowPowerSave = true;

	// All geometry is blended, so save power with no MSAA
	app->GetVrParms().multisamples = 1;
}



void OvrApp::OneTimeShutdown()
{
	// Free GL resources
	SSSA_LOG_FUNCALL(1);

	UninstallShaderAndScreen();


	if ( MovieTexture != NULL )
	{
		delete MovieTexture;
		MovieTexture = NULL;
	}

}

void OvrApp::Command( const char * msg )
{
	SSSA_LOG_FUNCALL(1);
	if ( MatchesHead( "newVideo ", msg ) )
	{
		delete MovieTexture;
		MovieTexture = new SurfaceTexture( app->GetVrJni() );
		LOG( "RC_NEW_VIDEO texId %i", MovieTexture->textureId );

		MessageQueue	* receiver;
		sscanf( msg, "newVideo %p", &receiver );

		receiver->PostPrintf( "surfaceTexture %p", MovieTexture->javaObject );

		// don't draw the screen until we have the new size
		CurrentVideoWidth = 0;

		return;
	}
	else if ( MatchesHead( "video ", msg ) )
	{
		sscanf( msg, "video %i %i", &CurrentVideoWidth, &CurrentVideoHeight );
		return;
	}
	else if ( MatchesHead( "resume ", msg ) )
	{
		ResumeVideo();
		return;	// allow VrLib to handle it, too
	}
	else if ( MatchesHead( "pause ", msg ) )
	{
		PauseVideo( false );
		return;	// allow VrLib to handle it, too
	}
	else
	{
		LOG( "unknow command %s", msg );
	}

}

bool OvrApp::OnKeyEvent( const int keyCode, const KeyState::eKeyEventType eventType )
{
//	SSSA_LOG_FUNCALL(1);
//	if (keyCode == AKEYCODE_BACK)
//	{
//		StopVideo();
//		//ovr_ExitActivity(app->GetOvrMobile(),EXIT_TYPE_FINISH);
//		//return true;
//	}
//	else
//	{
//		LOG( "Unknown Key Event:keyCode=%d",keyCode );
//	}

	return false;
}
void OvrApp::ResumeVideo()
{
	SSSA_LOG_FUNCALL(1);
	LOG( "ResumeVideo()" );

	//app->GetGuiSys().CloseMenu( app, Browser, false );

	jmethodID methodId = app->GetVrJni()->GetMethodID( GlobalActivityClass,
		"resumeMovie", "()V" );
	if ( !methodId )
	{
		LOG( "Couldn't find resumeMovie methodID" );
		return;
	}

	app->GetVrJni()->CallVoidMethod( app->GetJavaObject(), methodId );
}

void OvrApp::PauseVideo( bool const force )
{
	SSSA_LOG_FUNCALL(1);
	LOG( "PauseVideo()" );

	jmethodID methodId = app->GetVrJni()->GetMethodID( GlobalActivityClass,
		"pauseMovie", "()V" );
	if ( !methodId )
	{
		LOG( "Couldn't find pauseMovie methodID" );
		return;
	}

	app->GetVrJni()->CallVoidMethod( app->GetJavaObject(), methodId );
}

void OvrApp::StopVideo()
{
	SSSA_LOG_FUNCALL(1);
	LOG( "StopVideo()" );

	jmethodID methodId = app->GetVrJni()->GetMethodID( GlobalActivityClass,
		"stopMovie", "()V" );
	if ( !methodId )
	{
		LOG( "Couldn't find stopMovie methodID" );
		return;
	}

	app->GetVrJni()->CallVoidMethod( app->GetJavaObject(), methodId );

	delete MovieTexture;
	MovieTexture = NULL;
}

Matrix4f OvrApp::DrawEyeView( const int eye, const float fovDegrees )
{
	//SSSA_LOG_FUNCALL(1);
	const Matrix4f mvp = Scene.DrawEyeView( eye, fovDegrees );
	if(MovieTexture)
	{
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_EXTERNAL_OES, MovieTexture->textureId );

		if(m_pCurrentScreen)
			m_pCurrentScreen->Render(&Scene,MovieTexture,m_ShaderMng,eye,fovDegrees);

		glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );	// don't leave it bound
	}

	return mvp;
}


Matrix4f OvrApp::Frame(const VrFrame vrFrame)
{
	//SSSA_LOG_FUNCALL(1);
	// Disallow player foot movement, but we still want the head model
	// movement for the swipe view.
	VrFrame vrFrameWithoutMove = vrFrame;
	vrFrameWithoutMove.Input.sticks[ 0 ][ 0 ] = 0.0f;
	vrFrameWithoutMove.Input.sticks[ 0 ][ 1 ] = 0.0f;
	Scene.Frame( app->GetVrViewParms(), vrFrameWithoutMove, app->GetSwapParms().ExternalVelocity );
	// Check for new video frames
	// latch the latest movie frame to the texture.
	//LOG( "MovieTexture=%x,CurrentVideoWidth=%d",MovieTexture,CurrentVideoWidth);
	if ( MovieTexture && CurrentVideoWidth ) {

		glActiveTexture( GL_TEXTURE0 );
		MovieTexture->Update();
		glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );
		FrameAvailable = false;
	}

	// We could disable the srgb convert on the FBO. but this is easier
	app->GetVrParms().colorFormat = COLOR_8888;//UseSrgb ? COLOR_8888_sRGB : COLOR_8888;

	// Draw both eyes
	app->DrawEyeViewsPostDistorted( Scene.CenterViewMatrix() );

	return Scene.CenterViewMatrix();
}


