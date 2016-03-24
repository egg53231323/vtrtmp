/************************************************************************************

Filename    :   UIContainer.h
Content     :
Created     :	1/5/2015
Authors     :   Jim Dos�

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant 
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#if !defined( UIContainer_h )
#define UIContainer_h

#include "VRMenu/VRMenu.h"
#include "UI/UIWidget.h"

using namespace OVR;
namespace OVR{
class App;
}

class CinemaApp;

class UIContainer : public UIWidget
{
public:
										UIContainer( App &cinema );
										~UIContainer();

	void 								AddToMenu( UIMenu *menu, UIWidget *parent = NULL );
};


#endif // UIContainer_h
