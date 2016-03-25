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
#include "VRMenu/GuiSys.h"
#include "UI/UITexture.h"
#include "UI/UIMenu.h"
#include "UI/UIContainer.h"
#include "UI/UILabel.h"
#include "UI/UIImage.h"
#include "UI/UIButton.h"
#include "VideoMenu.h"
namespace OVR
{
	class OvrVideoMenu;
	class OvrGuiSys;
}

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

	void CreateMenu( App * app, OvrVRMenuMgr & menuMgr, BitmapFont const & font );
	OvrSceneView		Scene;

	void ShowUI();
	void HideUI();
	gazeCursorUserId_t		GazeUserId;	// id unique to this swipe view for interacting with gaze cursor
	UITexture				BackgroundTintTexture;

	UITexture				RWTexture;
	UITexture				RWHoverTexture;
	UITexture				RWPressedTexture;

	UITexture				FFTexture;
	UITexture				FFHoverTexture;
	UITexture				FFPressedTexture;

	UITexture				PlayTexture;
	UITexture				PlayHoverTexture;
	UITexture				PlayPressedTexture;

	UITexture				PauseTexture;
	UITexture				PauseHoverTexture;
	UITexture				PausePressedTexture;

	UITexture				CarouselTexture;
	UITexture				CarouselHoverTexture;
	UITexture				CarouselPressedTexture;

	UITexture				SeekbarBackgroundTexture;
	UITexture				SeekbarProgressTexture;

	UITexture				SeekPosition;

	UITexture				SeekFF2x;
	UITexture				SeekFF4x;
	UITexture				SeekFF8x;
	UITexture				SeekFF16x;
	UITexture				SeekFF32x;

	UITexture				SeekRW2x;
	UITexture				SeekRW4x;
	UITexture				SeekRW8x;
	UITexture				SeekRW16x;
	UITexture				SeekRW32x;

	UIMenu *				MoveScreenMenu;
	UILabel* 				MoveScreenLabel;
	//Lerp					MoveScreenAlpha;

	UIMenu *				PlaybackControlsMenu;
	UIContainer* 			PlaybackControlsPosition;
	UIContainer* 			PlaybackControlsScale;
	UILabel* 				MovieTitleLabel;

	UIImage*				SeekIcon;

	UIImage*				ControlsBackground;
	ControlsGazeTimer		GazeTimer;

	UIButton*				RewindButton;
	UIButtonGaze*			PlayButton;
	UIButton*				FastForwardButton;
	UIButton*				CarouselButton;

	UIImage*				SeekbarBackground;
	UIImage	*				SeekbarProgress;
	ScrubBarComponent 		ScrubBar;

	UILabel* 				CurrentTime;
	UILabel* 				SeekTime;
};

#endif
