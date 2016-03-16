/*
 * MovieScreen.h
 *
 *  Created on: 2016-3-16
 *      Author: Administrator
 */

#ifndef MOVIESCREEN_H_
#define MOVIESCREEN_H_

#include "GlGeometry.h"

//屏幕的几何体
enum SCREEN_GEOMETRY
{
	SG_SPHERE, 	//球体 用于播放全景影片
	SG_QUAD		//面片 用于播放非全景影片
};

//贴图映射的方式
enum MOVIE_MAPPING
{
	MM_LEFT_RIGHT,	//左眼映射 影片的左半部分，右眼映射影片的右半部分
	MM_TOP_BOTTOM,	//左眼映射 影片的上半部分，右眼映射影片的下半部分
	MM_ONLY_LEFT,	//左右眼都映射 影片的左半部分
	MM_ONLY_TOP,	//左右眼都映射 影片的上半部分
	MM_WHOLE,		//左右眼都看到影片的全部
	MM_VR_PORN		//vr porn模式
};

struct SphereScreenConfig
{
	SphereScreenConfig()
	{
		tc_mode=MM_LEFT_RIGHT;
	}
	MOVIE_MAPPING tc_mode;
};
struct QuadScreenConfig
{
	QuadScreenConfig()
	{
		width_scale=1.5;
		height_scale=0.75;
		distance_to_eye=-1.5;
		tc_mode=MM_LEFT_RIGHT;
	}
	float width_scale;
	float height_scale;
	float distance_to_eye;
	MOVIE_MAPPING tc_mode;
};

class SurfaceTexture;
class MovieScreen
{
public :
	MovieScreen(){}
	virtual ~MovieScreen(){}
	virtual void Render(OVR::SurfaceTexture* MovieTexture)=0;
};

class MoiveScreenSphere:public MovieScreen
{
public:
	MoiveScreenSphere();
	virtual ~MoiveScreenSphere();
	virtual void Render(SurfaceTexture* MovieTexture);

	void SetConfig(const SphereScreenConfig& cfg);
private:
	GlGeometry	Globe;
};

class MoiveScreenQuad:public MovieScreen
{
public:
	MoiveScreenQuad();
	virtual ~MoiveScreenQuad();
	virtual void Render(SurfaceTexture* MovieTexture);
	void SetConfig(const QuadScreenConfig& cfg);
private:
	GlGeometry eye_quad;
};
#endif /* MOVIESCREEN_H_ */
