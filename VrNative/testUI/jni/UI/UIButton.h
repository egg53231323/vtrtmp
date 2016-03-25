/************************************************************************************

Filename    :   UIButton.h
Content     :
Created     :	1/23/2015
Authors     :   Jim Dos�

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#if !defined( UIButton_h )
#define UIButton_h

#include "VRMenu/VRMenu.h"
#include "UI/UIWidget.h"
#include "VRMenu/VRMenuComponent.h"
#include "UI/UITexture.h"

using namespace OVR;

namespace OVR{
class App;
}

class CinemaApp;
class UIButton;

//==============================================================
// UIButtonComponent
class UIButtonComponent : public VRMenuComponent
{
public:
	static const int TYPE_ID = 159493;

					UIButtonComponent( UIButton &button );

	virtual int		GetTypeId() const { return TYPE_ID; }

	bool			IsPressed() const { return TouchDown; }

protected:
	UIButton &		Button;

    SoundLimiter    GazeOverSoundLimiter;
    SoundLimiter    DownSoundLimiter;
    SoundLimiter    UpSoundLimiter;

    bool			TouchDown;


    virtual eMsgStatus      OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
private:
    eMsgStatus              FocusGained( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
    eMsgStatus              FocusLost( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
};



//==============================================================
// UIButton

class UIButton : public UIWidget
{
	friend class UIButtonComponent;

	public:
											UIButton( OVR::App &cinema );
											~UIButton();

		virtual void 						AddToMenu( UIMenu *menu, UIWidget *parent = NULL );

		void								SetButtonImages( const UITexture &normal, const UITexture &hover, const UITexture &pressed );

		void								SetOnClick( void ( *callback )( UIButton *, void * ), void *object );

		void								UpdateButtonState();
		void 								OnClick();
	private:
		UIButtonComponent					ButtonComponent;
	protected:
		UITexture 							Normal;
		UITexture 							Hover;
		UITexture 							Pressed;

		void 								( *OnClickFunction )( UIButton *button, void *object );
		void *								OnClickObject;


};

class UIButtonGaze;

//带有凝视输入的button
class UIButtonComponentGaze:public VRMenuComponent
{
public:
	static const int TYPE_ID = 159597;

	UIButtonComponentGaze( UIButtonGaze &button );
	virtual ~UIButtonComponentGaze(){}
	virtual int		GetTypeId() const { return TYPE_ID; }
	//bool			IsPressed() const { return TouchDown; }
protected:
	UIButtonGaze &		Button;
    bool			HasFocus;
    double			LastGazeTime;
    SoundLimiter    GazeOverSoundLimiter;
    SoundLimiter    DownSoundLimiter;
    SoundLimiter    UpSoundLimiter;


    virtual eMsgStatus      OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
private:
    eMsgStatus              FocusGained( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
    eMsgStatus              FocusLost( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
                                    VRMenuObject * self, VRMenuEvent const & event );
};

class UIButtonGaze : public UIButton
{
	friend class UIButtonComponentGaze;
public:
	UIButtonGaze( OVR::App &cinema );
	virtual void 	AddToMenu( UIMenu *menu, UIWidget *parent = NULL );
	virtual ~UIButtonGaze(){};

private:
	UIButtonComponentGaze				gazeButtonComponent;

};


#endif // UIButton_h
