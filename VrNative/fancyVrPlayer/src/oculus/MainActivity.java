/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import java.util.ArrayList;

import com.fancyTech.fancyVrPlayer.R;
import com.fancyTech.fancyVrPlayer.R.id;

import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.widget.AdapterView.OnItemClickListener;


public class MainActivity extends Activity{
	public static final String TAG = "FancyTech";
	private ListView lv;
	private Button button_scan;
	private Button button_play_URL;
	private EditText editText_URL;
	private Spinner spinner_renderMode;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
		
        lv=(ListView)findViewById(R.id.listView1);
        lv.setOnItemClickListener(new OnItemClickListener()
        {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int pos,long id) {
				String txt=arg0.getItemAtPosition(pos).toString();
				Log.d(TAG,txt);
				Toast.makeText(getApplicationContext(), "pos="+txt,Toast.LENGTH_SHORT).show();
				//启动另一个activity
				Intent intent=new Intent(MainActivity.this,VideoInfoActivity.class);
				Bundle mp4_info=new Bundle();
				mp4_info.putString("filename", txt);
				mp4_info.putInt("rendermode", spinner_renderMode.getSelectedItemPosition());
				intent.putExtras(mp4_info);
				startActivity(intent);
			}
        		 
        });
        
        //扫描的按钮
        button_scan=(Button)findViewById(R.id.button2);
		button_scan.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v)
			{
				
				final EnumDiskTask myTask =  new EnumDiskTask();
				myTask.execute("");
				button_scan.setEnabled(false);
				
				ArrayList<String> fs=DiskEnum.scanVideoAndUpdateUI();
				
			}
		});

		button_play_URL=(Button)findViewById(R.id.button_URL);
		button_play_URL.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				String txt=editText_URL.getText().toString();
				Log.d(TAG,txt);
				Toast.makeText(getApplicationContext(), "pos="+txt,Toast.LENGTH_SHORT).show();
				//启动另一个activity
				Intent intent=new Intent(MainActivity.this,VideoInfoActivity.class);
				Bundle mp4_info=new Bundle();
				mp4_info.putString("filename", txt);
				mp4_info.putInt("rendermode", spinner_renderMode.getSelectedItemPosition());
				intent.putExtras(mp4_info);
				startActivity(intent);
			}
		});
		editText_URL=(EditText)findViewById(R.id.editText_URL);
		
		spinner_renderMode=(Spinner)findViewById(R.id.spinner_rendermode);
		String[] strMode=new String[2];
		strMode[0]="frontBuffer";
		strMode[1]="normal";
		ArrayAdapter<String> adapter=new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, strMode);
		spinner_renderMode.setAdapter(adapter);
		spinner_renderMode.setSelection(0);
    }

    //“启动任务执行的输入参数”、“后台任务执行的进度”、“后台计算结果的类型”
    private class EnumDiskTask extends AsyncTask<String,Integer,String>{
       	
    	private ArrayList<String> enumResult;
    	//onPreExecute方法用于在执行后台任务前做一些UI操作
    	@Override
    	protected void onPreExecute() {
    		
    	}
    	
    	//doInBackground方法内部执行后台任务,不可在此方法内修改UI
    	@Override
    	protected String doInBackground(String...param) {
    		
    		enumResult=DiskEnum.scanVideoAndUpdateUI();
    		return null;
    	}
    	
    	//onProgressUpdate方法用于更新进度信息
    	@Override
    	protected void onProgressUpdate(Integer... progresses) {
    				
    	}
    			
    	//onPostExecute方法用于在执行完后台任务后更新UI,显示结果
    	@Override
    	protected void onPostExecute(String result) {
    		ArrayAdapter<String> adpt=new ArrayAdapter<String> (MainActivity.this,android.R.layout.simple_list_item_1,enumResult);
			lv.setAdapter(adpt);
			button_scan.setEnabled(true);
			Toast.makeText(getApplicationContext(), "Scan Done! res num="+enumResult.size(), 0).show();  
    	}
    			
    	//onCancelled方法用于在取消执行中的任务时更改UI
    	@Override
    	protected void onCancelled() {
    				
    				
    	}
    }
}
