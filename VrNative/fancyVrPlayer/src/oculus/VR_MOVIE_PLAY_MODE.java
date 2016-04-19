package oculus;

//定义了屏幕的几何体
//注意，这里的int的值必须和cpp中枚举的定义一样
public class VR_MOVIE_PLAY_MODE {

	public final static int SCREEN_SPHERE = 0; // 使用球体作为屏幕的几何体
	public final static int SCREEN_QUAD = 1; // 使用quad作为屏幕的几何体
	public final static int SCREEN_THREATRE = 2; // 使用剧院模式
}
