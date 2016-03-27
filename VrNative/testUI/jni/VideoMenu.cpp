#include "videoMenu.h"
#include "VRMenu/VRMenuMgr.h"
#include "Kernel/OVR_String_Utils.h"
#include "vrapi/VrApi.h"
#include "input.h"
#include "app.h"
namespace OVR
{
ControlsGazeTimer::ControlsGazeTimer() :
	VRMenuComponent( VRMenuEventFlags_t( VRMENU_EVENT_FRAME_UPDATE ) |
			VRMENU_EVENT_FOCUS_GAINED |
            VRMENU_EVENT_FOCUS_LOST ),
    LastGazeTime( 0 ),
    HasFocus( false )

{
}

void ControlsGazeTimer::SetGazeTime()
{
	LastGazeTime = ovr_GetTimeInSeconds();
}

eMsgStatus ControlsGazeTimer::OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
        VRMenuObject * self, VRMenuEvent const & event )
{
    switch( event.EventType )
    {
    	case VRMENU_EVENT_FRAME_UPDATE:
    		if ( HasFocus )
    		{
    			LastGazeTime = ovr_GetTimeInSeconds();
    		}
    		return MSG_STATUS_ALIVE;
        case VRMENU_EVENT_FOCUS_GAINED:
        	HasFocus = true;
        	LastGazeTime = ovr_GetTimeInSeconds();
    		return MSG_STATUS_ALIVE;
        case VRMENU_EVENT_FOCUS_LOST:
        	HasFocus = false;
    		return MSG_STATUS_ALIVE;
        default:
            OVR_ASSERT( !"Event flags mismatch!" );
            return MSG_STATUS_ALIVE;
    }
}

/*************************************************************************************/

ScrubBarComponent::ScrubBarComponent() :
	VRMenuComponent( VRMenuEventFlags_t( VRMENU_EVENT_TOUCH_DOWN ) |
		VRMENU_EVENT_TOUCH_DOWN |
		VRMENU_EVENT_FRAME_UPDATE |
		VRMENU_EVENT_FOCUS_GAINED |
        VRMENU_EVENT_FOCUS_LOST ),
	HasFocus( false ),
	TouchDown( false ),
	Progress( 0.0f ),
	Duration( 0 ),
	Background( NULL ),
	ScrubBar( NULL ),
	CurrentTime( NULL ),
	SeekTime( NULL ),
	OnClickFunction( NULL ),
	OnClickObject( NULL )

{
}

void ScrubBarComponent::SetDuration( const int duration )
{
	Duration = duration;

	SetProgress( Progress );
}

void ScrubBarComponent::SetOnClick( void ( *callback )( ScrubBarComponent *, void *, float ), void *object )
{
	OnClickFunction = callback;
	OnClickObject = object;
}

void ScrubBarComponent::SetWidgets( UIWidget *background, UIWidget *scrubBar, UILabel *currentTime, UILabel *seekTime, const int scrubBarWidth )
{
	Background 		= background;
	ScrubBar 		= scrubBar;
	CurrentTime 	= currentTime;
	SeekTime 		= seekTime;
	ScrubBarWidth	= scrubBarWidth;

	SeekTime->SetVisible( false );
}
float PixelScale( const float x )
{
	return x * VRMenuObject::DEFAULT_TEXEL_SCALE;
}

Vector3f PixelPos( const float x, const float y, const float z )
{
	return Vector3f( PixelScale( x ), PixelScale( y ), PixelScale( z ) );
}

void ScrubBarComponent::SetProgress( const float progress )
{
	Progress = progress;
	const float seekwidth = ScrubBarWidth * progress;

	Vector3f pos = ScrubBar->GetLocalPosition();
	pos.x = PixelScale( ( ScrubBarWidth - seekwidth ) * -0.5f );
	ScrubBar->SetLocalPosition( pos );
	ScrubBar->SetSurfaceDims( 0, Vector2f( seekwidth, 40.0f ) );
	ScrubBar->RegenerateSurfaceGeometry( 0, false );

	pos = CurrentTime->GetLocalPosition();
	pos.x = PixelScale( ScrubBarWidth * -0.5f + seekwidth );
	CurrentTime->SetLocalPosition( pos );
	SetTimeText( CurrentTime, Duration * progress );
}

void ScrubBarComponent::SetTimeText( UILabel *label, const int time )
{
	int seconds = time / 1000;
	int minutes = seconds / 60;
	int hours = minutes / 60;
	seconds = seconds % 60;
	minutes = minutes % 60;

	if ( hours > 0 )
	{
		label->SetText( StringUtils::Va( "%d:%02d:%02d", hours, minutes, seconds ) );
	}
	else if ( minutes > 0 )
	{
		label->SetText( StringUtils::Va( "%d:%02d", minutes, seconds ) );
	}
	else
	{
		label->SetText( StringUtils::Va( "0:%02d", seconds ) );
	}
}

eMsgStatus ScrubBarComponent::OnEvent_Impl( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
        VRMenuObject * self, VRMenuEvent const & event )
{
    switch( event.EventType )
    {
		case VRMENU_EVENT_FOCUS_GAINED:
			HasFocus = true;
			return MSG_STATUS_ALIVE;

		case VRMENU_EVENT_FOCUS_LOST:
			HasFocus = false;
			return MSG_STATUS_ALIVE;

    	case VRMENU_EVENT_TOUCH_DOWN:
    		TouchDown = true;
    		OnClick( app, vrFrame, event );
    		return MSG_STATUS_ALIVE;

    	case VRMENU_EVENT_FRAME_UPDATE:
    		return OnFrame( app, vrFrame, menuMgr, self, event );

        default:
            OVR_ASSERT( !"Event flags mismatch!" );
            return MSG_STATUS_ALIVE;
    }
}

eMsgStatus ScrubBarComponent::OnFrame( App * app, VrFrame const & vrFrame, OvrVRMenuMgr & menuMgr,
        VRMenuObject * self, VRMenuEvent const & event )
{
	if ( ( vrFrame.Input.buttonState & ( BUTTON_A | BUTTON_TOUCH ) ) != 0 )
			{
				OnClick( app, vrFrame, event );
			}

	return MSG_STATUS_ALIVE;
}

void ScrubBarComponent::OnClick( App * app, VrFrame const & vrFrame, VRMenuEvent const & event )
{

	Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

	// move hit position into local space
	const Posef modelPose = Background->GetWorldPose();
	Vector3f localHit = modelPose.Orientation.Inverted().Rotate( hitPos - modelPose.Position );

	Bounds3f bounds = Background->GetMenuObject()->GetLocalBounds( app->GetDefaultFont() ) * Background->GetParent()->GetWorldScale();
	const float progress = ( localHit.x - bounds.GetMins().x ) / bounds.GetSize().x;

	if ( ( progress >= 0.0f ) && ( progress <= 1.0f ) )
	{
		( *OnClickFunction )( this, OnClickObject, progress );
	}
}
}
