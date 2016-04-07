/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class MainActivity extends VrUsbActivity {
	public static final String TAG = "TestUI";


	/** Load jni .so on initialization */
	static {
		Log.d(TAG, "LoadLibrary");
		System.loadLibrary("ovrapp");
	}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


    }   

    protected void OnButtonDown()
    {
    	//注意 這裡的調用來自于另一個線程
    	runOnUiThread(new Runnable() {
    	    public void run() {
    	    	Toast.makeText(MainActivity.this, "Button Down", 1).show();
    	    }
    	});  
    	
    }
    protected void OnButtonUp()
    {
    	runOnUiThread(new Runnable() {
    	    public void run() {
    	    	Toast.makeText(MainActivity.this, "Button Up", 1).show();
    	    }
    	});
    }
}
