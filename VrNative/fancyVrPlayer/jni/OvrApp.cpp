/************************************************************************************

Filename    :   OvrApp.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
#include <jni.h>

#include "OvrApp.h"
#include "PathUtils.h"
#include <android/keycodes.h>
#include "Kernel\OVR_Log.h"

extern "C" {

static jclass	GlobalActivityClass;
jlong Java_oculus_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	SSSA_LOG_FUNCALL(1);
	LOG( "PlaygroundActivity_nativeSetAppInterface" );
	GlobalActivityClass = (jclass)jni->NewGlobalRef( clazz );
	return (new OvrApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}


void Java_oculus_MainActivity_nativeFrameAvailable( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->SetFrameAvailable( true );
}

jobject Java_oculus_MainActivity_nativePrepareNewVideo( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
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

void Java_oculus_MainActivity_nativeSetVideoSize( JNIEnv *jni, jclass clazz, jlong interfacePtr, int width, int height ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeSetVideoSizes: width=%i height=%i", width, height );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "video %i %i", width, height );
}

void Java_oculus_MainActivity_nativeVideoCompletion( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeVideoCompletion" );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "completion" );
}

void Java_oculus_MainActivity_nativeVideoStartError( JNIEnv *jni, jclass clazz, jlong interfacePtr ) {
	SSSA_LOG_FUNCALL(1);
	LOG( "nativeVideoStartError" );

	OvrApp * panoVids = ( OvrApp * )( ( ( App * )interfacePtr )->GetAppInterface() );
	panoVids->app->GetMessageQueue().PostPrintf( "startError" );
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
}

OvrApp::~OvrApp()
{
	SSSA_LOG_FUNCALL(1);
	//nativeDestroy();
}
void OvrApp::UninstallShader()
{
	DeleteProgram(PanoramaProgram);
	DeleteProgram(PanoramaProgram3DV);
	DeleteProgram(PanoramaProgramVRP);
	DeleteProgram(FadedPanoramaProgram);
	DeleteProgram(blackProgram);
}
void OvrApp::InitShader() {
	SSSA_LOG_FUNCALL(1);
	PanoramaProgram =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	PanoramaProgram3DV =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	float eye = Eye.x;\n	highp vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n	texc = texc + eye * vec2(0.0,0.5);\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	//用于放大毛
	PanoramaProgramVRP =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n	float eye = Eye.x;\n	highp vec2 texc = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n	texc = vec2( texc.x * 2.0 , texc.y );\n	texc = clamp(texc,vec2(0.0,0.0),vec2(0.5,1.0));\n	texc = texc + eye * vec2(0.5,0.0);\n   oTexCoord = texc;\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform lowp vec4 UniformColor;\nuniform lowp vec4 ColorBias;\nuniform lowp vec4 Eye;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	highp vec2 texc = oTexCoord;\n   if (all(lessThanEqual(vec2(texc.x), vec2(0.0 + Eye.x * 0.5))))\n            texc = vec2(0.25, 0.0);\n   if (all(greaterThanEqual(vec2(texc.x), vec2(0.5 + Eye.x * 0.5))))\n            texc = vec2(0.25, 0.0);\n	gl_FragColor = ColorBias + UniformColor * texture2D( Texture0, texc );\n}\n");
	FadedPanoramaProgram =
			BuildProgram(
					"uniform highp mat4 Mvpm;\nuniform highp mat4 Texm;\nattribute vec4 Position;\nattribute vec2 TexCoord;\nvarying  highp vec2 oTexCoord;\nvoid main()\n{\n   gl_Position = Mvpm * Position;\n   oTexCoord = vec2( Texm * vec4( TexCoord, 0, 1 ) );\n}\n",
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES Texture0;\nuniform sampler2D Texture1;\nuniform lowp vec4 UniformColor;\nvarying highp vec2 oTexCoord;\nvoid main()\n{\n	lowp vec4 staticColor = texture2D( Texture1, oTexCoord );\n	lowp vec4 movieColor = texture2D( Texture0, oTexCoord );\n	gl_FragColor = UniformColor * mix( movieColor, staticColor, staticColor.w );\n}\n");

	blackProgram=
			BuildProgram(
					"uniform highp mat4 Mvpm;\nattribute vec4 Position;\nvoid main()\n{\ngl_Position = Mvpm * Position;\n}\n",
					"void main(){gl_FragColor = vec4(1,1,1,1);}");
}

void OvrApp::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	// This is called by the VR thread, not the java UI thread.
	LOG( "--------------- Oculus360Videos OneTimeInit ---------------" );

	VideoMode = 0;
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	app->GetStoragePaths().PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);


	app->GetVrParms().colorFormat = COLOR_8888;
	app->GetVrParms().depthFormat = DEPTH_16;
	app->GetVrParms().multisamples = 2;

	LOG( "InitShader" );
	InitShader();

	LOG( "Creating Globe" );
	Globe = BuildGlobe();
	eye_quad = BuildTesselatedQuad(1,1);



	MaterialParms materialParms;
	materialParms.UseSrgbTextureFormats = ( app->GetVrParms().colorFormat == COLOR_8888_sRGB );
	const char * scenePath = "oculus/tuscany.ovrscene";
	String SceneFile;
	GetFullPath( SearchPaths, scenePath, SceneFile );

	//Scene.LoadWorldModel( SceneFile, materialParms );
	//Scene.YawOffset = -M_PI / 2;

	// Stay exactly at the origin, so the panorama globe is equidistant
	// Don't clear the head model neck length, or swipe view panels feel wrong.
	VrViewParms viewParms = app->GetVrViewParms();
	viewParms.EyeHeight = 0.0f;
	app->SetVrViewParms( viewParms );
	app->SetShowFPS(false);

	//播放影片需要关闭time wrap
	ovrModeParms param=app->GetVrModeParms();
	param.AsynchronousTimeWarp = false;
	app->SetVrModeParms(param);

	// Optimize for 16 bit depth in a modest theater size
	Scene.Znear = 0.1f;
	Scene.Zfar = 2000.0f;


}

void OvrApp::OneTimeShutdown()
{
	// Free GL resources
	SSSA_LOG_FUNCALL(1);

	UninstallShader();
	Globe.Free();
	eye_quad.Free();

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
	SSSA_LOG_FUNCALL(1);
	if (keyCode == AKEYCODE_BACK)
	{
		StopVideo();
		//return true;

	}
	else
	{
		LOG( "Unknown Key Event:keyCode=%d",keyCode );
	}

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
	SSSA_LOG_FUNCALL(1);
	int tmpIdxer=0;
	LOG("DrawEyeView CP %d",tmpIdxer++);
	const Matrix4f mvp = Scene.DrawEyeView( eye, fovDegrees );
	if(MovieTexture)
	{
		LOG("DrawEyeView CP %d",tmpIdxer++);
		//LOG("fovDegrees=%f", fovDegrees);
		// draw animated movie panorama
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_EXTERNAL_OES, MovieTexture->textureId );

		//glActiveTexture( GL_TEXTURE1 );
		//glBindTexture( GL_TEXTURE_2D, BackgroundTexId );
		const float eyeOffset = ( eye ? -1 : 1 ) * 0.5f * Scene.ViewParms.InterpupillaryDistance;
		const Matrix4f view =  Matrix4f::Translation( eyeOffset, 0.0f, 0.0f );

		//const Matrix4f view = Scene.ViewMatrixForEye(0);//VideoMode == 0 ? Scene.ViewMatrixForEye(0) * Matrix4f::RotationY(M_PI / 2) : Scene.ViewMatrixForEye(0) /** Matrix4f::RotationY( M_PI /2 )*/;
		const Matrix4f proj = Scene.ProjectionMatrixForEye( 0, fovDegrees );

		//先绘制背景，用一个黑色的球体
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		GlProgram* prog = &blackProgram;
		glUseProgram( prog->program );
		glUniformMatrix4fv( prog->uMvp, 1, GL_FALSE, ( proj * view ).Transposed().M[ 0 ] );
		Globe.Draw();


		VideoMode =0;
		prog = &PanoramaProgram;
		LOG("DrawEyeView CP %d",tmpIdxer++);
		glUseProgram( prog->program );
		glUniform4f( prog->uColor, 1.0f, 1.0f, 1.0f, 1.0f );

		// Videos have center as initial focal point - need to rotate 90 degrees to start there
//		const Matrix4f view = VideoMode == 0 ? Scene.ViewMatrixForEye(0) * Matrix4f::RotationY(M_PI / 2) : Scene.ViewMatrixForEye(0) /** Matrix4f::RotationY( M_PI /2 )*/;
//		const Matrix4f proj = Scene.ProjectionMatrixForEye( 0, fovDegrees );
//		const int toggleStereo = 0;//eye;//VideoMenu->IsOpenOrOpening() ? 0 : eye;
//
//		glUniformMatrix4fv( prog->uTexm, 1, GL_FALSE, TexmForVideo( toggleStereo ).Transposed().M[ 0 ] );
//		glUniformMatrix4fv( prog->uMvp, 1, GL_FALSE, ( proj * view ).Transposed().M[ 0 ] );
//		glUniform4f(prog->uEye, (float)eye, 0.0f, 0.0f, 0.0f);
//
//		LOG("DrawEyeView CP %d",tmpIdxer++);
//		Globe.Draw();
//		//eye_quad.Draw();
//		LOG("DrawEyeView CP %d",tmpIdxer++);



		Matrix4f world = Matrix4f::Identity();

		//world.Translation(90000,0,0);
		//world=Matrix4f::RotationX(-3.14159 / 2) * world;
		world=world.Scaling(1.5,0.75,1) * world.Translation(0,0,-1.5f) * world;

		glUniformMatrix4fv( prog->uTexm, 1, GL_FALSE, TexmForVideo( eye ).Transposed().M[ 0 ] );
		glUniformMatrix4fv( prog->uMvp, 1, GL_FALSE, ( proj * view * world ).Transposed().M[ 0 ] );
		glUniform4f(prog->uEye, (float)eye, 0.0f, 0.0f, 0.0f);

		eye_quad.Draw();
		glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );	// don't leave it bound
	}

	return mvp;
}
Matrix4f	OvrApp::TexmForVideo( const int eye )
{
//	return eye ?
//		Matrix4f(
//		0.5, 0, 0, 0,
//		0, 1, 0, 0,
//		0, 0, 1, 0,
//		0, 0, 0, 1 )
//		:
//		Matrix4f(
//		0.5, 0, 0, 0.5,
//		0, 1, 0, 0,
//		0, 0, 1, 0,
//		0, 0, 0, 1 );


	return eye ?  //左右交换一下

		Matrix4f(
		0.5, 0, 0, 0.5,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 )
		:
		Matrix4f(
		0.5, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 )
		;

	SSSA_LOG_FUNCALL(1);
	if (VideoMode == 0)
	{//普通360模式
		return Matrix4f::Identity();

	}else if (VideoMode == 1)
	{// VirtualRealPorn 模式
		return Matrix4f(
			0.5, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
			);
	}
	else if (VideoMode == 2)
	{//3D360模式
		return Matrix4f(
			1, 0, 0, 0,
			0, 0.5, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
			);
	}
	else if (VideoMode == 3) //left right 3d video
	{	// left / right stereo panorama
		return eye ?
			Matrix4f(
			0.5, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f(
			0.5, 0, 0, 0.5,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
	}
	return Matrix4f::Identity();

	if ( strstr( VideoName.ToCStr(), "_TB.mp4" ) )
	{	// top / bottom stereo panorama
		return eye ?
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0.5,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
	}
	if ( strstr( VideoName.ToCStr(), "_BT.mp4" ) )
	{	// top / bottom stereo panorama
		return ( !eye ) ?
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0.5,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
	}
	if ( strstr( VideoName.ToCStr(), "_LR.mp4" ) )
	{	// left / right stereo panorama
		return eye ?
			Matrix4f( 0.5, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f( 0.5, 0, 0, 0.5,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
	}
	if ( strstr( VideoName.ToCStr(), "_RL.mp4" ) )
	{	// left / right stereo panorama
		return ( !eye ) ?
			Matrix4f( 0.5, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f( 0.5, 0, 0, 0.5,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );
	}

	// default to top / bottom stereo
	if ( CurrentVideoWidth == CurrentVideoHeight )
	{	// top / bottom stereo panorama
		return eye ?
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0.5,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f( 1, 0, 0, 0,
			0, 0.5, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );

		// We may want to support swapping top/bottom
	}
	return Matrix4f::Identity();
}

Matrix4f	OvrApp::TexmForBackground( const int eye )
{
	SSSA_LOG_FUNCALL(1);
	if ( BackgroundWidth == BackgroundHeight )
	{	// top / bottom stereo panorama
		return eye ?
			Matrix4f(
			1, 0, 0, 0,
			0, 0.5, 0, 0.5,
			0, 0, 1, 0,
			0, 0, 0, 1 )
			:
			Matrix4f(
			1, 0, 0, 0,
			0, 0.5, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 );

		// We may want to support swapping top/bottom
	}
	return Matrix4f::Identity();
}

Matrix4f OvrApp::Frame(const VrFrame vrFrame)
{
	SSSA_LOG_FUNCALL(1);
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
		//LOG( "MovieTexture->Update()" );
		glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );
		FrameAvailable = false;
	}

	// We could disable the srgb convert on the FBO. but this is easier
	app->GetVrParms().colorFormat = COLOR_8888;//UseSrgb ? COLOR_8888_sRGB : COLOR_8888;

	// Draw both eyes
	app->DrawEyeViewsPostDistorted( Scene.CenterViewMatrix() );

	return Scene.CenterViewMatrix();
}
