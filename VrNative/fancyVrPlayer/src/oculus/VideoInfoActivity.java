package oculus;
import com.fancyTech.fancyVrPlayer.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
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
		try{
			Bundle bundle=getIntent().getExtras();
			final String mp4_fn=bundle.getString("filename");
			if(mp4_fn!=null){
				Log.d(TAG,"Get String from bundle "+ mp4_fn);
				TextView tv=(TextView)findViewById(R.id.textView1);
				tv.setText("movie: "+mp4_fn);			
			}
			//set up play button 
			Button playBtn=(Button)findViewById(R.id.button_play);
			playBtn.setOnClickListener(new View.OnClickListener() 
			{
				public void onClick(View v)
				{
					
					int sel_tc_id=GetTcMode();
					int sel_screen_id = GetScreenMode();
					//启动观看影片的activity
					Intent intent=new Intent(VideoInfoActivity.this,movieViewActivity.class);
					Bundle mp4_info=new Bundle();

					//mp4_info.putSerializable("class", this.getClass());
					mp4_info.putString("filename", mp4_fn); //文件名
					mp4_info.putFloat("ratio", 0.75f); //宽高比
					mp4_info.putInt("screen", sel_screen_id);//screen方式
					mp4_info.putInt("tc", sel_tc_id);//tc方式
					intent.putExtras(mp4_info);
					startActivity(intent);
				}
			});
		}catch(Exception e){
			
		}

		
		

	}
	
	int GetScreenMode()
	{
		final RadioGroup groupScreen = (RadioGroup)this.findViewById(R.id.radioGroup1);
		int sel_screen_id = groupScreen.getCheckedRadioButtonId();
		RadioButton rb = (RadioButton)findViewById(sel_screen_id);
		String txt=(String) rb.getText();
		if(txt.equals("球体"))
			return VR_MOVIE_PLAY_MODE.SCREEN_SPHERE;
		else if(txt.equals("幕布"))
			return VR_MOVIE_PLAY_MODE.SCREEN_QUAD;
		else
			return VR_MOVIE_PLAY_MODE.SCREEN_THREATRE;
	}

	int GetTcMode()
	{
		final RadioGroup groupScreen = (RadioGroup)this.findViewById(R.id.radioGroup2);
		int sel_tc_id = groupScreen.getCheckedRadioButtonId();
		RadioButton rb = (RadioButton)findViewById(sel_tc_id);
		String txt=(String) rb.getText();
		if(txt.equals("全屏"))
			return VR_TC_MODE.MM_WHOLE;
		else if(txt.equals("左右"))
			return VR_TC_MODE.MM_LEFT_RIGHT;
		else if(txt.equals("上下"))
			return VR_TC_MODE.MM_TOP_BOTTOM;
		else if(txt.equals("左半部分"))
			return VR_TC_MODE.MM_ONLY_LEFT;
		else if(txt.equals("上半部分"))
			return VR_TC_MODE.MM_ONLY_TOP;
		else if(txt.equals("vrporn"))
			return VR_TC_MODE.MM_VR_PORN;
		else
			return VR_TC_MODE.MM_WHOLE;
			
	}
}
