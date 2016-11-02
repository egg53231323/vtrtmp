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

import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
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
				//������һ��activity
				Intent intent=new Intent(MainActivity.this,VideoInfoActivity.class);
				Bundle mp4_info=new Bundle();
				mp4_info.putString("filename", txt);
				intent.putExtras(mp4_info);
				startActivity(intent);
			}
        		 
        });
        
        //ɨ��İ�ť
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
				//������һ��activity
				Intent intent=new Intent(MainActivity.this,VideoInfoActivity.class);
				Bundle mp4_info=new Bundle();
				mp4_info.putString("filename", txt);
				intent.putExtras(mp4_info);
				startActivity(intent);
			}
		});
		editText_URL=(EditText)findViewById(R.id.editText_URL);
    }

    //����������ִ�е����������������̨����ִ�еĽ��ȡ�������̨�����������͡�
    private class EnumDiskTask extends AsyncTask<String,Integer,String>{
       	
    	private ArrayList<String> enumResult;
    	//onPreExecute����������ִ�к�̨����ǰ��һЩUI����
    	@Override
    	protected void onPreExecute() {
    		
    	}
    	
    	//doInBackground�����ڲ�ִ�к�̨����,�����ڴ˷������޸�UI
    	@Override
    	protected String doInBackground(String...param) {
    		
    		enumResult=DiskEnum.scanVideoAndUpdateUI();
    		return null;
    	}
    	
    	//onProgressUpdate�������ڸ��½�����Ϣ
    	@Override
    	protected void onProgressUpdate(Integer... progresses) {
    				
    	}
    			
    	//onPostExecute����������ִ�����̨��������UI,��ʾ���
    	@Override
    	protected void onPostExecute(String result) {
    		ArrayAdapter<String> adpt=new ArrayAdapter<String> (MainActivity.this,android.R.layout.simple_list_item_1,enumResult);
			lv.setAdapter(adpt);
			button_scan.setEnabled(true);
			Toast.makeText(getApplicationContext(), "Scan Done! res num="+enumResult.size(), 0).show();  
    	}
    			
    	//onCancelled����������ȡ��ִ���е�����ʱ����UI
    	@Override
    	protected void onCancelled() {
    				
    				
    	}
    }
}
