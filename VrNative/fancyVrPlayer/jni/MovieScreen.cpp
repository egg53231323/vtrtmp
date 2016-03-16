/*
 * MovieScreen.cpp
 *
 *  Created on: 2016-3-16
 *      Author: Administrator
 */



#include "MovieScreen.h"
#include "App.h"

MoiveScreenSphere::MoiveScreenSphere()
{
	Globe = BuildGlobe();
}
MoiveScreenSphere::~MoiveScreenSphere()
{
	Globe.Free();
}
void MoiveScreenSphere::Render(OVR::SurfaceTexture* MovieTexture)
{

}

MoiveScreenQuad::MoiveScreenQuad()
{
	eye_quad = BuildTesselatedQuad(1,1);
}
MoiveScreenQuad::~MoiveScreenQuad()
{
	eye_quad.Free();
}
void MoiveScreenQuad::Render(SurfaceTexture* MovieTexture)
{

}

//
////绘制全景视频
//void OvrApp::DrawMoviePanoramaMode( const int eye, const float fovDegrees )
//{
//	const Matrix4f view = (m_Mode == Panorama_NoStereo) ?
//					Scene.ViewMatrixForEye(0) * Matrix4f::RotationY(M_PI / 2) :// Videos have center as initial focal point - need to rotate 90 degrees to start there
//					Scene.ViewMatrixForEye(0) /** Matrix4f::RotationY( M_PI /2 )*/;	//const Matrix4f view = Scene.ViewMatrixForEye(0);//VideoMode == 0 ? Scene.ViewMatrixForEye(0) * Matrix4f::RotationY(M_PI / 2) : Scene.ViewMatrixForEye(0) /** Matrix4f::RotationY( M_PI /2 )*/;
//	const Matrix4f proj = Scene.ProjectionMatrixForEye( 0, fovDegrees );
//
//	GlProgram* prog = &PanoramaProgram;
//	glUseProgram( prog->program );
//	glUniform4f( prog->uColor, 1.0f, 1.0f, 1.0f, 1.0f );
//
//	glUniformMatrix4fv(prog->uTexm, 1, GL_FALSE,
//			TexmForVideo(eye).Transposed().M[0]);
//	glUniformMatrix4fv(prog->uMvp, 1, GL_FALSE,
//			(proj * view).Transposed().M[0]);
//	glUniform4f(prog->uEye, (float) eye, 0.0f, 0.0f, 0.0f);
//
//	Globe.Draw();
//}
//
////绘制非全景视频
//void OvrApp::DrawMovieNoPanoramaMode( const int eye, const float fovDegrees )
//{
//	const float eyeOffset = ( eye ? -1 : 1 ) * 0.5f * Scene.ViewParms.InterpupillaryDistance;
//	const Matrix4f view =  Matrix4f::Translation( eyeOffset, 0.0f, 0.0f ); //固定不动
//	//const Matrix4f view = Scene.ViewMatrixForEye(0);//VideoMode == 0 ? Scene.ViewMatrixForEye(0) * Matrix4f::RotationY(M_PI / 2) : Scene.ViewMatrixForEye(0) /** Matrix4f::RotationY( M_PI /2 )*/;
//	const Matrix4f proj = Scene.ProjectionMatrixForEye( 0, fovDegrees );
//
//	//先绘制背景，用一个黑色的球体
//	glDisable( GL_DEPTH_TEST );
//	glDisable( GL_CULL_FACE );
//	GlProgram* prog = &blackProgram;
//	glUseProgram( prog->program );
//	glUniformMatrix4fv( prog->uMvp, 1, GL_FALSE, ( proj * view ).Transposed().M[ 0 ] );
//	Globe.Draw();
//
//	prog = &PanoramaProgram;
//	glUseProgram( prog->program );
//	glUniform4f( prog->uColor, 1.0f, 1.0f, 1.0f, 1.0f );
//
//	Matrix4f world = Matrix4f::Identity();
//	//注意，x轴朝右，y轴朝上，z轴朝向屏幕外侧，这里把绘制的片往远处挪了1.5个单位
//	world=world.Scaling(m_WidthScale,m_HeightScale,1) * world.Translation(0,0,-1.5f) * world;
//
//	glUniformMatrix4fv( prog->uTexm, 1, GL_FALSE, TexmForVideo( eye ).Transposed().M[ 0 ] );
//	glUniformMatrix4fv( prog->uMvp, 1, GL_FALSE, ( proj * view * world ).Transposed().M[ 0 ] );
//	glUniform4f(prog->uEye, (float)eye, 0.0f, 0.0f, 0.0f);
//
//	eye_quad.Draw();
//}
//
//Matrix4f	OvrApp::TexmForVideo( const int eye )
//{
//	SSSA_LOG_FUNCALL(1);
//	if (m_Mode == VirtualRealPorn)// VirtualRealPorn 模式
//	{
//		return Matrix4f(
//			0.5, 0, 0, 0,
//			0, 1, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1
//			);
//	}
//
//	else if (m_Mode == Panorama_Stereo_Left_Right || m_Mode == NoPanorama_Stereo_Left_Right) //left right 3d video
//	{
//		return eye ?  //左右交换一下
//
//			Matrix4f(
//			0.5, 0, 0, 0.5,
//			0, 1, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1 )
//			:
//			Matrix4f(
//			0.5, 0, 0, 0,
//			0, 1, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1 )
//			;
//	}
//	else if (m_Mode == Panorama_Stereo_Up_Down || m_Mode == NoPanorama_Stereo_Up_Down) //left right 3d video
//	{
//		return eye ?
//			Matrix4f( 1, 0, 0, 0,
//			0, 0.5, 0, 0.5,
//			0, 0, 1, 0,
//			0, 0, 0, 1 )
//			:
//			Matrix4f( 1, 0, 0, 0,
//			0, 0.5, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1 );
//	}
//	return Matrix4f::Identity();
//
//}
//
//Matrix4f	OvrApp::TexmForBackground( const int eye )
//{
//	SSSA_LOG_FUNCALL(1);
//	if ( BackgroundWidth == BackgroundHeight )
//	{	// top / bottom stereo panorama
//		return eye ?
//			Matrix4f(
//			1, 0, 0, 0,
//			0, 0.5, 0, 0.5,
//			0, 0, 1, 0,
//			0, 0, 0, 1 )
//			:
//			Matrix4f(
//			1, 0, 0, 0,
//			0, 0.5, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1 );
//
//		// We may want to support swapping top/bottom
//	}
//	return Matrix4f::Identity();
//}

