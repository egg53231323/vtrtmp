/************************************************************************************

Filename    :   VideoMenu.cpp
Content     :
Created     :
Authors     :

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Oculus360Videos/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "VideoMenu.h"

#include "VRMenu/VRMenuMgr.h"
#include "VRMenu/GuiSys.h"
#include "VRMenu/DefaultComponent.h"
#include "VRMenu/ActionComponents.h"
#include "VRMenu/FolderBrowser.h"

#include "OvrApp.h"

namespace OVR {

const VRMenuId_t OvrVideoMenu::ID_CENTER_ROOT( 1000 );
const VRMenuId_t OvrVideoMenu::ID_BROWSER_BUTTON( 1000 + 1011 );
const VRMenuId_t OvrVideoMenu::ID_VIDEO_BUTTON( 1000 + 1012 );

char const * OvrVideoMenu::MENU_NAME = "VideoMenu";

static const Vector3f FWD( 0.0f, 0.0f, 1.0f );
static const Vector3f RIGHT( 1.0f, 0.0f, 0.0f );
static const Vector3f UP( 0.0f, 1.0f, 0.0f );
static const Vector3f DOWN( 0.0f, -1.0f, 0.0f );

static const int BUTTON_COOL_DOWN_SECONDS = 0.25f;

//==============================
// OvrVideoMenuRootComponent
// This component is attached to the root of VideoMenu 
class OvrVideoMenuRootComponent : public VRMenuComponent
{
public:
	OvrVideoMenuRootComponent( OvrVideoMenu & videoMenu )
		: VRMenuComponent( VRMenuEventFlags_t( VRMENU_EVENT_FRAME_UPDATE ) | VRMENU_EVENT_OPENING )
		, VideoMenu( videoMenu )
	{
	}

private:
	virtual eMsgStatus	OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
		VRMenuObject * self, VRMenuEvent const & event )
	{
		switch ( event.EventType )
		{
		case VRMENU_EVENT_FRAME_UPDATE:
			return OnFrame( app, vrFrame, menuMgr, self, event );
		case VRMENU_EVENT_OPENING:
			return OnOpening( app, vrFrame, menuMgr, self, event );
		case VRMENU_EVENT_TOUCH_DOWN:
		{
				Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

				//VRMenuObject * object = GetMenuObject();
				//OVR_ASSERT( object );

				Posef const &  localPose = self->GetLocalPose();

				// move hit position into local space
				//const Posef modelPose = Background->GetWorldPose();
				Vector3f localHit = localPose.Orientation.Inverted().Rotate( hitPos - localPose.Position );

				Bounds3f bounds = self->GetLocalBounds( app->GetDefaultFont() ) * localPose;
				float progress = ( localHit.x - bounds.GetMins().x ) / bounds.GetSize().x;
				if ( ( progress >= 0.0f ) && ( progress <= 1.0f ) )
				{
					app->ShowInfoText( 1.0f, "click at %f",progress );
				}
			return MSG_STATUS_ALIVE;
		}
		default:
			OVR_ASSERT( !"Event flags mismatch!" ); // the constructor is specifying a flag that's not handled
			return MSG_STATUS_ALIVE;
		}


	}

	eMsgStatus OnOpening( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr, VRMenuObject * self, VRMenuEvent const & event )
	{
//		CurrentVideo = (OvrVideosMetaDatum *)( VideoMenu.GetVideos()->GetActiveVideo() );
//		// If opening VideoMenu without a Video selected, bail
//		if ( CurrentVideo == NULL )
//		{
//			app->GetGuiSys().CloseMenu( app, &VideoMenu, false );
//		}
//		LoadAttribution( self );
		return MSG_STATUS_CONSUMED;
	}

	void LoadAttribution( VRMenuObject * self )
	{
//		if ( CurrentVideo )
//		{
//			self->SetText( CurrentVideo->Title );
//		}
	}

	eMsgStatus OnFrame( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr, VRMenuObject * self, VRMenuEvent const & event )
	{
		return MSG_STATUS_ALIVE;
	}

private:
	OvrVideoMenu &			VideoMenu;
};

//==============================
// OvrVideoMenu
OvrVideoMenu * OvrVideoMenu::Create( App * app, OvrApp * videos, OvrVRMenuMgr & menuMgr, BitmapFont const & font,float fadeOutTime, float radius )
{
	return new OvrVideoMenu( app, videos, menuMgr, font, fadeOutTime, radius );
}

OvrVideoMenu::OvrVideoMenu( App * app, OvrApp * videos, OvrVRMenuMgr & menuMgr, BitmapFont const & font,
	float fadeOutTime, float radius )
	: VRMenu( MENU_NAME )
	, AppPtr( app )
	, MenuMgr( menuMgr )
	, Font( font )
	, Videos( videos )
	, LoadingIconHandle( 0 )
	, AttributionHandle( 0 )
	, BrowserButtonHandle( 0 )
	, VideoControlButtonHandle( 0 )
	, Radius( radius )
	, ButtonCoolDown( 0.0f )
	, OpenTime( 0.0 )
{
	// Init with empty root
	Init( menuMgr, font, 0.0f, VRMenuFlags_t() );

	// Create Attribution info view
	Array< VRMenuObjectParms const * > parms;
	Array< VRMenuComponent* > comps;
	Array< VRMenuSurfaceParms > surfParms;

	VRMenuId_t attributionPanelId( ID_CENTER_ROOT.Get() + 10 );

	comps.PushBack( new OvrVideoMenuRootComponent( *this ) );
	//comps.PushBack( new OvrDefaultComponent( Vector3f( 0.0f, 0.0f, 0.05f ), 1.05f, 0.25f, 0.0f, Vector4f( 1.0f ), Vector4f( 1.0f ) ) );
	Quatf rot( DOWN, 0.0f );
	Vector3f dir( -FWD );
	Posef panelPose( rot, dir * Radius );

	const VRMenuFontParms fontParms( true, true, false, false, true, 0.525f, 0.45f, 1.0f );

	//Posef videoButtonPose( Quatf(), DOWN * ICON_HEIGHT * 2.0f );
	surfParms.PushBack( VRMenuSurfaceParms( "browser",
		"assets/nav_restart_off.png", SURFACE_TEXTURE_DIFFUSE,
		NULL, SURFACE_TEXTURE_MAX, NULL, SURFACE_TEXTURE_MAX ) );

	VRMenuObjectParms controlButtonParms(
			VRMENU_BUTTON,
			comps,
			surfParms,
			"",
			panelPose, //pos
			Vector3f( 3.0f,1.0f,1.0f ), //scale
			Posef(), //text local pos
			Vector3f( 1.0f ),  //text local scale
			fontParms,
			ID_VIDEO_BUTTON,  //id
			VRMenuObjectFlags_t( VRMENUOBJECT_DONT_HIT_TEXT ),
			VRMenuObjectInitFlags_t( VRMENUOBJECT_INIT_FORCE_POSITION ));


	parms.PushBack( &controlButtonParms );

	AddItems( MenuMgr, Font, parms, GetRootHandle(), false );
	parms.Clear();
	comps.Clear();


}

OvrVideoMenu::~OvrVideoMenu()
{

}

void OvrVideoMenu::Open_Impl( App * app, OvrGazeCursor & gazeCursor )
{
	ButtonCoolDown = BUTTON_COOL_DOWN_SECONDS;

	OpenTime = ovr_GetTimeInSeconds();
}

void OvrVideoMenu::Frame_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr, BitmapFont const & font, BitmapFontSurface & fontSurface, gazeCursorUserId_t const gazeUserId )
{

}

void OvrVideoMenu::OnItemEvent_Impl( App * app, VRMenuId_t const itemId, VRMenuEvent const & event )
{

}

}
