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

import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.widget.AdapterView.OnItemClickListener;

public class MainActivity extends Activity{
	public static final String TAG = "FancyTech";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
		
        final ListView lv=(ListView)findViewById(R.id.listView1);
        lv.setOnItemClickListener(new OnItemClickListener()
        {
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int pos,long id) {
				Toast.makeText(getApplicationContext(), "onItemClick",Toast.LENGTH_SHORT).show();
				//启动另一个activity

			}
        		 
        });
        
        //扫描的按钮
		Button button_scan=(Button)findViewById(R.id.button2);
		button_scan.setOnClickListener(new View.OnClickListener() 
		{
			public void onClick(View v)
			{
				
				ArrayList<String> fs=DiskEnum.scanVideoAndUpdateUI();
				ArrayAdapter<String> adpt=new ArrayAdapter<String> (MainActivity.this,android.R.layout.simple_list_item_1,fs);
				lv.setAdapter(adpt);
			}
		});

    }

}
