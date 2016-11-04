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
		//�õ���Ƶ���ļ���
		try{
			Bundle bundle=getIntent().getExtras();
			final String mp4_fn=bundle.getString("filename");
			final int rendermode=bundle.getInt("rendermode");
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
					//�����ۿ�ӰƬ��activity
					Intent intent=new Intent(VideoInfoActivity.this,movieViewActivity.class);
					Bundle mp4_info=new Bundle();

					//mp4_info.putSerializable("class", this.getClass());
					mp4_info.putString("filename", mp4_fn); //�ļ���
					mp4_info.putFloat("ratio", 0.75f); //��߱�
					mp4_info.putInt("screen", sel_screen_id);//screen��ʽ
					mp4_info.putInt("tc", sel_tc_id);//tc��ʽ
					mp4_info.putString("rendermode", GetRenderMode());
					mp4_info.putString("showfps", GetShowFPS());
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
		if(txt.equals("����"))
			return VR_MOVIE_PLAY_MODE.SCREEN_SPHERE;
		else if(txt.equals("Ļ��"))
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
		if(txt.equals("ȫ��"))
			return VR_TC_MODE.MM_WHOLE;
		else if(txt.equals("����"))
			return VR_TC_MODE.MM_LEFT_RIGHT;
		else if(txt.equals("����"))
			return VR_TC_MODE.MM_TOP_BOTTOM;
		else if(txt.equals("��벿��"))
			return VR_TC_MODE.MM_ONLY_LEFT;
		else if(txt.equals("�ϰ벿��"))
			return VR_TC_MODE.MM_ONLY_TOP;
		else if(txt.equals("vrporn"))
			return VR_TC_MODE.MM_VR_PORN;
		else
			return VR_TC_MODE.MM_WHOLE;
			
	}
	
	String GetRenderMode()
	{
		String strRenderMode = "0";
		final RadioGroup groupRenderMode = (RadioGroup)this.findViewById(R.id.radioGroup_renderMode);
		int sel_tc_id = groupRenderMode.getCheckedRadioButtonId();
		RadioButton rb = (RadioButton)findViewById(sel_tc_id);
		if (rb.getId() == R.id.radio_frontBuffer) {
			strRenderMode = "1";
		}
		return strRenderMode;
	}
	
	String GetShowFPS()
	{
		String strShowFPS = "0";
		final RadioGroup groupShowFPS = (RadioGroup)this.findViewById(R.id.radioGroup_showFPS);
		int sel_tc_id = groupShowFPS.getCheckedRadioButtonId();
		RadioButton rb = (RadioButton)findViewById(sel_tc_id);
		if (rb.getId() == R.id.radio_showFPS_yes) {
			strShowFPS = "1";
		}
		return strShowFPS;
	}
}
