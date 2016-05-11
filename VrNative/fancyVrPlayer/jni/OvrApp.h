/************************************************************************************

Filename    :   OvrApp.h
Content     :   Trivial use of VrLib
Created     :   February 10, 2014
Authors     :   John Carmack

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


************************************************************************************/

#ifndef OVRAPP_H
#define OVRAPP_H

#include "App.h"
#include "ModelView.h"
#include "ShaderManager.h"
#include "MovieScreen.h"

class OvrApp : public OVR::VrAppInterface
{
public:
						OvrApp();
    virtual				~OvrApp();

	virtual void		OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI );
	virtual void		OneTimeShutdown();
	virtual Matrix4f 	DrawEyeView( const int eye, const float fovDegrees );
	virtual Matrix4f 	Frame( VrFrame vrFrame );
	virtual void		Command( const char * msg );
	virtual bool 		OnKeyEvent( const int keyCode, const KeyState::eKeyEventType eventType );

	void				SetFrameAvailable( bool const a ) { FrameAvailable = a; }
	Matrix4f			TexmForVideo( const int eye );
	Matrix4f			TexmForBackground( const int eye );
	void 				PauseVideo( bool const force );
	void 				ResumeVideo();
	void				StopVideo();

	void				SetUseSphereScreen(const SphereScreenConfig& cfg);
	void				SetUseQuadScreen(const QuadScreenConfig& cfg);
	void				SetUseTheatre(const ThreatreConfig& cfg);

	void				ConfigureVrMode( ovrModeParms & modeParms );
	// video vars
	String				VideoName;
	SurfaceTexture	* 	MovieTexture;

	// Set when MediaPlayer knows what the stream size is.
	// current is the aspect size, texture may be twice as wide or high for 3D content.
	int					CurrentVideoWidth;	// set to 0 when a new movie is started, don't render until non-0
	int					CurrentVideoHeight;

	int					BackgroundWidth;
	int					BackgroundHeight;

	bool				FrameAvailable;


	//0 为普通，1 为 VirtualRealPorn，2 为 3D360
	int					VideoMode;
	OvrSceneView		Scene;

	Array<String> 		SearchPaths;

	MovieScreen*		m_pCurrentScreen;
	MoiveScreenSphere*	m_pSphereScreen;
	MoiveScreenQuad*	m_pQuadScreen;
	MoiveTheatre*		m_pTheatre;

	ShaderManager*		m_ShaderMng;
private:
	void UninstallShaderAndScreen();
	void InitShaderAndScreen();
};

#endif
