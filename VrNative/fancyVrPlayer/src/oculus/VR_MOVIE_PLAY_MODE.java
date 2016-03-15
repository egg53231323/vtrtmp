package oculus;

//定义了播放影片的方式
public enum VR_MOVIE_PLAY_MODE {


	NoPanorama_NoStereo, 			//非全景 非立体的影片，也就是最普通的影片
	NoPanorama_Stereo_Left_Right, 	//非全景的 左右格式的立体影片
	NoPanorama_Stereo_Up_Down,		//非全景的 上下格式的立体影片
	
	Panorama_NoStereo,				//全景 非立体的影片
	Panorama_Stereo_Left_Right,		//全景的 左右格式的立体影片
	Panorama_Stereo_Up_Down,		//全景的 上下格式的立体影片
}
