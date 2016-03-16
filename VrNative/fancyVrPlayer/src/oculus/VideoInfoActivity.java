package oculus;
import com.fancyTech.fancyVrPlayer.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class VideoInfoActivity extends Activity{
	public static final String TAG = "FancyTech";
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		Log.d(TAG,"VideoInfoActivity onCreate");
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_videoinfo);
		//得到视频的文件名
		Bundle bundle=getIntent().getExtras();
		final String mp4_fn=bundle.getString("filename");
		Log.d(TAG,"Get String from bundle "+ mp4_fn);
		TextView tv=(TextView)findViewById(R.id.textView1);
		tv.setText("movie: "+mp4_fn);
		
		//set up play button 
		Button playBtn=(Button)findViewById(R.id.button_play);
		playBtn.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v)
			{
				//启动观看影片的activity
				Intent intent=new Intent(VideoInfoActivity.this,movieViewActivity.class);
				Bundle mp4_info=new Bundle();
				mp4_info.putString("filename", mp4_fn); //文件名
				mp4_info.putFloat("ratio", 0.75f); //宽高比
				mp4_info.putInt("playmode", VR_MOVIE_PLAY_MODE.NoPanorama_Stereo_Left_Right);//播放方式
				intent.putExtras(mp4_info);
				startActivity(intent);
			}
		});
	}

}
