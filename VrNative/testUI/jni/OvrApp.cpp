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
#include "VideoMenu.h"
#include "VRMenu/GuiSys.h"


extern "C" {

jlong Java_oculus_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	LOG( "nativeSetAppInterface" );
	return (new OvrApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}

} // extern "C"


using namespace OVR;

OvrApp::OvrApp():
VideoMenu(0)
{
}

OvrApp::~OvrApp()
{
}

void OvrApp::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	// This is called by the VR thread, not the java UI thread.
	MaterialParms materialParms;
	materialParms.UseSrgbTextureFormats = false;
        
	const char * scenePath = "Oculus/tuscany.ovrscene";
	String	        SceneFile;
	Array<String>   SearchPaths;

	const OvrStoragePaths & paths = app->GetStoragePaths();

	paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
	paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

	if ( GetFullPath( SearchPaths, scenePath, SceneFile ) )
	{
		Scene.LoadWorldModel( SceneFile , materialParms );
	}
	else
	{
		LOG( "OvrApp::OneTimeInit SearchPaths failed to find %s", scenePath );
	}

	// Start building the VideoMenu
	VideoMenu = (OvrVideoMenu *) app->GetGuiSys().GetMenu(
			OvrVideoMenu::MENU_NAME);

	if (VideoMenu == NULL) {
		VideoMenu = OvrVideoMenu::Create(app, this, app->GetVRMenuMgr(),
				app->GetDefaultFont(), 1.0f, 2.0f);
		OVR_ASSERT(VideoMenu);

		app->GetGuiSys().AddMenu(VideoMenu);
	}

	VideoMenu->SetFlags(
			VRMenuFlags_t(VRMENU_FLAG_PLACE_ON_HORIZON)
					| VRMENU_FLAG_SHORT_PRESS_HANDLED_BY_APP);

}

void OvrApp::OneTimeShutdown()
{
	// Free GL resources
        
}

void OvrApp::Command( const char * msg )
{
}

Matrix4f OvrApp::DrawEyeView( const int eye, const float fovDegrees )
{
	const Matrix4f view = Scene.DrawEyeView( eye, fovDegrees );

	return view;
}

Matrix4f OvrApp::Frame(const VrFrame vrFrame)
{
	// Player movement
    Scene.Frame( app->GetVrViewParms(), vrFrame, app->GetSwapParms().ExternalVelocity );

    //alwarys display the menu
    app->GetGuiSys().OpenMenu( app, app->GetGazeCursor(), OvrVideoMenu::MENU_NAME );
    if ( vrFrame.Input.buttonReleased & BUTTON_TOUCH_DOUBLE )
    {
    	VideoMenu->RepositionMenu( app );
    }



	app->DrawEyeViewsPostDistorted( Scene.CenterViewMatrix() );

	return Scene.CenterViewMatrix();
}
