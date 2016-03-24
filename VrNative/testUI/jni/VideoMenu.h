/************************************************************************************

Filename    :	VideoMenu.h
Content     :   VRMenu shown when within panorama
Created     :   October 9, 2014
Authors     :   Warsam Osman

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Oculus360Videos/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

************************************************************************************/

#if !defined( OVR_VideoMenu_h )
#define OVR_VideoMenu_h

#include "VRMenu/VRMenu.h"
#include "UI/UITexture.h"
#include "UI/UIMenu.h"
#include "UI/UIContainer.h"
#include "UI/UILabel.h"
#include "UI/UIImage.h"
#include "UI/UIButton.h"

class OvrApp;
namespace OVR {


class SearchPaths;
class OvrVideoMenuRootComponent;

class OvrMetaData;
struct OvrMetaDatum;

class ControlsGazeTimer : public VRMenuComponent
{
public:
	static const int 		TYPE_ID = 152413;

							ControlsGazeTimer();

	virtual int				GetTypeId() const { return TYPE_ID; }

	void					SetGazeTime();
	double					GetLastGazeTime() const { return LastGazeTime; }

	bool					IsFocused() const { return HasFocus; }

private:
	double					LastGazeTime;
	bool					HasFocus;

private:
    virtual eMsgStatus      OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
};

class ScrubBarComponent : public VRMenuComponent
{
public:
	static const int 		TYPE_ID = 152414;

							ScrubBarComponent();

	virtual int				GetTypeId() const { return TYPE_ID; }

	void					SetDuration( const int duration );
	void					SetOnClick( void ( *callback )( ScrubBarComponent *, void *, float ), void *object );
	void					SetWidgets( UIWidget *background, UIWidget *scrubBar, UILabel *currentTime, UILabel *seekTime, const int scrubBarWidth );
	void 					SetProgress( const float progress );

	bool					IsScrubbing() const { return TouchDown; }

private:
	bool					HasFocus;
	bool					TouchDown;

	float					Progress;
	int						Duration;

	UIWidget *				Background;
	UIWidget *				ScrubBar;
	UILabel *				CurrentTime;
	UILabel *				SeekTime;
	int 					ScrubBarWidth;

	void 					( *OnClickFunction )( ScrubBarComponent *button, void *object, float progress );
	void *					OnClickObject;

private:
    virtual eMsgStatus      OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );

    eMsgStatus 				OnFrame( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
            VRMenuObject * self, VRMenuEvent const & event );

    void 					OnClick( App * app, VrFrame const & vrFrame, VRMenuEvent const & event );

    void 					SetTimeText( UILabel *label, const int time );
};

}

#endif
