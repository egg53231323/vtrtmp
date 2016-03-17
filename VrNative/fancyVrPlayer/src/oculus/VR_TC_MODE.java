package oculus;

//注意，这里的int的值必须和cpp中枚举的定义一样
/*
 //贴图映射的方式
 enum MOVIE_MAPPING
 {
 MM_LEFT_RIGHT=0,	//左眼映射 影片的左半部分，右眼映射影片的右半部分
 MM_TOP_BOTTOM,	//左眼映射 影片的上半部分，右眼映射影片的下半部分
 MM_ONLY_LEFT,	//左右眼都映射 影片的左半部分
 MM_ONLY_TOP,	//左右眼都映射 影片的上半部分
 MM_WHOLE,		//左右眼都看到影片的全部
 MM_VR_PORN		//vr porn模式
 };

 */
public class VR_TC_MODE {
	public final static int MM_LEFT_RIGHT = 0; // 左眼映射 影片的左半部分，右眼映射影片的右半部分
	public final static int MM_TOP_BOTTOM = 1; // 左眼映射 影片的上半部分，右眼映射影片的下半部分
	public final static int MM_ONLY_LEFT = 2; // 左右眼都映射 影片的左半部分
	public final static int MM_ONLY_TOP = 3; // 左右眼都映射 影片的上半部分
	public final static int MM_WHOLE = 4; // 左右眼都看到影片的全部
	public final static int MM_VR_PORN = 5; // vr porn模式
}
