package oculus;

//定义了播放影片的方式
public class VR_MOVIE_PLAY_MODE {


	public final static int NoPanorama_NoStereo=0; 				//非全景 非立体的影片，也就是最普通的影片
	public final static int NoPanorama_Stereo_Left_Right=1; 	//非全景的 左右格式的立体影片
	public final static int NoPanorama_Stereo_Up_Down=2;		//非全景的 上下格式的立体影片
	
	public final static int Panorama_NoStereo=3;				//全景 非立体的影片
	public final static int Panorama_Stereo_Left_Right=4;		//全景的 左右格式的立体影片
	public final static int Panorama_Stereo_Up_Down=5;			//全景的 上下格式的立体影片
}
