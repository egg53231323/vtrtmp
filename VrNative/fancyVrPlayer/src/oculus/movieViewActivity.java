package oculus;


import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.Toast;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences.Editor;
import android.graphics.SurfaceTexture;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;

import com.oculusvr.vrlib.VrActivity;
import com.oculusvr.vrlib.VrLib;

import tv.danmaku.ijk.media.player.AndroidMediaPlayer;
import tv.danmaku.ijk.media.player.IMediaPlayer;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnBufferingUpdateListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnCompletionListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnErrorListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnInfoListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnPreparedListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnSeekCompleteListener;
import tv.danmaku.ijk.media.player.IMediaPlayer.OnVideoSizeChangedListener;
import tv.danmaku.ijk.media.player.IjkMediaPlayer;

public class movieViewActivity extends VrActivity implements SurfaceHolder.Callback,
android.graphics.SurfaceTexture.OnFrameAvailableListener,
IMediaPlayer.OnVideoSizeChangedListener,
IMediaPlayer.OnCompletionListener,
IMediaPlayer.OnErrorListener,
IMediaPlayer.OnPreparedListener,
AudioManager.OnAudioFocusChangeListener {
	public static final String TAG = "FancyTech";
	
	String currentPathName = null;
	
	SurfaceTexture movieTexture = null;
	Surface movieSurface = null;
	IMediaPlayer mediaPlayer = null;	
	AudioManager audioManager = null;
	int screenMode=VR_MOVIE_PLAY_MODE.SCREEN_SPHERE;
	int tcMode=VR_TC_MODE.MM_WHOLE;
	
	private UsbDeviceConnection connection = null;
	private UsbInterface gyroDataIntf_ = null; //陀螺仪的接口
	private UsbInterface hidInputIntf_ = null; //按钮的接口
	private UsbEndpoint hidEndpoint_ = null;	//通信节点
	private boolean forceClaim = true; 
	private boolean buttonDown_ = false; //按钮是否按下
	private byte[] bytes; //保持从usb拿到的数据
	
	protected String Get_USB_PERMISSION_Str(){return "oculus.USB_PERMISSION";}
    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );
	public static native SurfaceTexture nativePrepareNewVideo(long appPtr );
	public static native void nativeFrameAvailable( long appPtr );
	public static native void nativeVideoCompletion( long appPtr );
	public static native void nativeSetVideoSize( long appPtr, int width, int height );
	public static native void nativeSetScreenTcMode( long appPtr, int screenmode,int tcmode );
	public static native void nativeUnloadAppInterface( long appPtr );
	public static native void nativeInitVrLib( long appPtr );
	public static native void nativeSetLocalPreference(long appPtr, String key, String val);
    //public native void PushData(byte[] buffer, int length);	
    public native void setupUsbDevice(int fd, int deviceType, boolean startThread);
	static
	{
		 System.loadLibrary("ovrapp");
	}
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        Log.d(TAG,"Movie Wnd onCreate");
        nativeInitVrLib(0);
		Intent intent = getIntent();

		String commandString = VrLib.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrLib.getPackageStringFromIntent( intent );
		String uriString = VrLib.getUriStringFromIntent( intent );

		appPtr = nativeSetAppInterface( this, fromPackageNameString, commandString, uriString );
		audioManager = (AudioManager) getSystemService( Context.AUDIO_SERVICE );
		
		
		Bundle bundle=getIntent().getExtras();
		final String fn=bundle.getString("filename");
		screenMode=bundle.getInt("screen");
		tcMode=bundle.getInt("tc");
		
		Log.d(TAG,"Movie filename="+fn);
		Log.d(TAG,"Movie screen="+screenMode);
		Log.d(TAG,"Movie tc="+tcMode);
		
		String strVal = bundle.getString("rendermode", "0"); 
		nativeSetLocalPreference(appPtr, "frontBuffer", strVal);
		
		strVal = bundle.getString("showfps", "0");
		nativeSetLocalPreference(appPtr, "showFPS", strVal);
		
		strVal = bundle.getString("lensSeparation", "0");
		if (strVal == "0")
		{
			nativeSetLocalPreference(appPtr, "UseDefaultDistortionFile", "0");
			nativeSetLocalPreference(appPtr, "lensSeparation", strVal);
		}
		else
		{
			nativeSetLocalPreference(appPtr, "UseDefaultDistortionFile", "1");
			nativeSetLocalPreference(appPtr, "lensSeparation", strVal);
		}
		
		strVal = bundle.getString("eyeTextureFov", "0");
		nativeSetLocalPreference(appPtr, "eyeTextureFov", strVal);
		
		nativeSetScreenTcMode(appPtr, screenMode,tcMode);


		if(fn==null || fn.isEmpty())
		{
		    Toast.makeText(getApplicationContext(), "Movie文件名为空", 0).show();  
		    return;
		}
		else
			Toast.makeText(getApplicationContext(), "Movie文件名="+fn, 0).show();
		//String videoDirPath= Environment.getExternalStorageDirectory().getPath()+"/oculus/360videos/";
		//String fn=videoDirPath+"3d_tag2_2_60fps.mp4";
        IntentFilter filter = new IntentFilter();
        filter.addAction(Get_USB_PERMISSION_Str());
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(mUsbReceiver, filter);
        
        ScanUsb();
        
        
		startMovie( fn );//play movie
    }

	//掃描當前usb端口是否有設備連接著，如果有也需要申請權限
	protected void ScanUsb(){
		UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
		HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		while(deviceIterator.hasNext()){
			UsbDevice device=deviceIterator.next();
			PendingIntent mPermissionIntent = PendingIntent.getBroadcast(getApplicationContext(), 0, new Intent(Get_USB_PERMISSION_Str()), 0);
            manager.requestPermission(device, mPermissionIntent);
		}
	}
	protected final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
		   public void onReceive(Context context, Intent intent) {
		        String action = intent.getAction();
		        //处理usb attach 后用户同意获得权限的事件
		        if (Get_USB_PERMISSION_Str().equals(action)) {
		            synchronized (this) {
		                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
		                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
		                    if(device != null)
		                    	InitUsbDevice(device);		                    
		                } 
		                else {
		                    Log.d(TAG, "permission denied for device " + device);
		                }
		            }
		        }
		        //处理usb attach 后弹出对话框，让用户给予权限
		        else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
			           UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
			           String deviceName = device.getDeviceName();
			           Toast.makeText(getApplicationContext(), "UsbDevice attached ="+deviceName, 0).show();
			        PendingIntent mPermissionIntent = PendingIntent.getBroadcast(context, 0, new Intent(Get_USB_PERMISSION_Str()), 0);
                    UsbManager mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
                    mUsbManager.requestPermission(device, mPermissionIntent);
                    Log.d(TAG, "after requestPermission");
			    }

		        //处理usb dettach 后的事件
			    else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
			           UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
			           String deviceName = device.getDeviceName();
			           Toast.makeText(getApplicationContext(), "UsbDevice dettached ="+deviceName, 0).show();  
			    }		        
		    }
		};
		
	protected int CheckUsbDevice(UsbDevice device) {
		int deviceType = -1;
		if (device.getVendorId() == 10291 && device.getProductId() == 1) {
			deviceType = 1;
		} else if ((device.getVendorId() == 1155 && device.getProductId() == 22336)
				|| (device.getVendorId() == 1155 && device.getProductId() == 22352)
				|| (device.getVendorId() == 949 && device.getProductId() == 1)) {
			deviceType = 0;
		}
		return deviceType;
	}
	protected int FindButtonGyro(UsbDevice device) {
		
		int nInterface = device.getInterfaceCount();
		if (nInterface < 1) {
			return -1;
		}

		Log.d(TAG, "usb device found interface count " + nInterface);
		for (int i = 0; i < nInterface; i++) {
			UsbInterface intf = device.getInterface(i);
			String str = String.format("interface %d %d.%d.%d", intf.getId(),
					intf.getInterfaceClass(), intf.getInterfaceSubclass(),
					intf.getInterfaceProtocol());
			Log.i(TAG, str);

			if (intf.getInterfaceClass() == 10
					&& intf.getInterfaceSubclass() == 0
					&& intf.getInterfaceProtocol() == 0) {
				gyroDataIntf_ = intf;
				Log.i(TAG, "found cdc interface " + intf.toString());
			}

			if (intf.getInterfaceClass() == 0x03
					&& intf.getInterfaceSubclass() == 0
					&& intf.getInterfaceProtocol() == 0) {
				this.hidInputIntf_ = intf;
				Log.i(TAG, "found hid interface " + intf.toString());
			}

			int numEndpoint = intf.getEndpointCount();
			for (int j = 0; j < numEndpoint; j++) {
				UsbEndpoint endpoint = intf.getEndpoint(j);
				String endpointInfo = String.format(
						"endpoint %d addr 0x%x number %d attri %d", j,
						endpoint.getAddress(), endpoint.getEndpointNumber(),
						endpoint.getAttributes());
				Log.i(TAG, endpointInfo);
				if (this.hidInputIntf_ == intf) {
					this.hidEndpoint_ = endpoint;
				}

			}
		}
		UsbManager manager = (UsbManager) getSystemService(Context.USB_SERVICE);
		connection = manager.openDevice(device);
		if (gyroDataIntf_ != null) {
			boolean bRes = connection.claimInterface(gyroDataIntf_, forceClaim);
			Log.i(TAG, String.format("claim gyro data interface %s",
					bRes ? "succeeded" : "failed"));
		}

		if (hidInputIntf_ != null) {
			boolean bRes = connection.claimInterface(hidInputIntf_, forceClaim);
			Log.i(TAG, String.format("claim hid input interface %s",
					bRes ? "succeeded" : "failed"));
		}
		return 1;
	}

	protected void InitUsbDevice(UsbDevice device) {

		int deviceType=CheckUsbDevice(device);
		if(deviceType==-1)
			return;

		int res=FindButtonGyro(device);
		if(res==-1)
			return;

		int fd = connection.getFileDescriptor();
		Log.i(TAG, String.format("FileDescriptor %d", fd));

		setupUsbDevice(fd, deviceType, false);

		// use native thread
		StartUsbIOThread();

		// 0xa1 : device to host / Type : class / Recipien : interface
		// request : 1 (get report)
		// value : 0x300 HID_FEATURE 0x3 FEATURE_CALIBRATE

		// size = connection.controlTransfer(0xa1, 1, 0x303, 0, bytes, 256, 0);
		// addText("control read " + size);

		// size = connection.controlTransfer(0xa1, 1, 0x304, 0, bytes, 256, 0);
		// addText("control read " + size);			

	}
	
	void HandleHidInputData(byte[] data, int size)
	{
		if (size == 0){
			return;
		}
					
		if (data[6] == (byte)0xf) {
			if(!this.buttonDown_)
				//OnButtonDown();
			this.buttonDown_ = true;
			
		} else if (buttonDown_ && data[6] == (byte)0xf0) {	
			if(this.buttonDown_)
				//OnButtonUp();
			this.buttonDown_ = false;
		}		
	}
	
	void StartUsbIOThread() {
		bytes = new byte[256];
		
		new Thread(new Runnable() {
			public void run() {
							
				while (true) {	
					int size = connection.bulkTransfer(hidEndpoint_, bytes, bytes.length, 0);
					HandleHidInputData(bytes, size);
					//PushData(bytes, size);
				}				
			}
		}).start();
	}
	void requestAudioFocus()
	{
		// Request audio focus
		int result = audioManager.requestAudioFocus( this, AudioManager.STREAM_MUSIC,
			AudioManager.AUDIOFOCUS_GAIN );
		if ( result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED ) 
		{
			Log.d(TAG,"startMovie(): GRANTED audio focus");
		}
		else if ( result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED ) 
		{
			Log.d(TAG,"startMovie(): FAILED to gain audio focus");
		}
	}
	
	void releaseAudioFocus()
	{
		audioManager.abandonAudioFocus( this );
	}
	
	
	public void pauseMovie() {
		Log.d(TAG, "pauseMovie()" );
		try {
			if (mediaPlayer != null) {
				Log.d(TAG, "movie paused" );
				mediaPlayer.pause();
			}
		}
		catch( IllegalStateException ise ) {
			Log.d( TAG, "pauseMovie(): Caught illegalStateException: " + ise.toString() );
		}
	}

	public void resumeMovie() {
		Log.d(TAG, "resumeMovie()" );
		try {
			if (mediaPlayer != null) {
				Log.d(TAG, "movie started" );
				mediaPlayer.start();
				mediaPlayer.setVolume(1.0f, 1.0f);
			}
		}
		catch( IllegalStateException ise ) {
			Log.d( TAG, "resumeMovie(): Caught illegalStateException: " + ise.toString() );
		}
	}



	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		if(event.getKeyCode()==KeyEvent.KEYCODE_BACK)
		{
			Log.d(TAG, "dispatchKeyEvent  KEYCODE_BACK");
			super.onPause();
			finish();
			//returnToMain();
		}
		return false;
		//return super.dispatchTouchEvent(e);
		
	}
	
	private void returnToMain()
	{
		//在这里编写返回的逻辑
		Intent intent=new Intent(movieViewActivity.this,VideoInfoActivity.class);
		startActivity(intent);
		//finish();
	}
    
    
    private String GetVideoFilename()
    {
    	Intent intent=getIntent();
		String video_fn=intent.getStringExtra("filename");
		return video_fn;
    }
	@Override
	protected void onDestroy() {	
		Log.d(TAG, "onDestroy");
		releaseAudioFocus();
    	unregisterReceiver(mUsbReceiver);
    	//nativeUnloadAppInterface(appPtr);
		// Abandon audio focus if we still hold it
		

		super.onDestroy();	
	}
//	@Override
//	protected void OnStop() {	
//		Log.d(TAG, "OnStop");
//		
//		super.OnStop();	
//	}
	// --> MediaPlayer.OnErrorListener START
	public boolean onError( IMediaPlayer mp, int what, int extra ) {
		Log.e( TAG, "MediaPlayer.OnErrorListener - what : " + what + ", extra : " + extra );
		return false;
	}
	
	// <-- MediaPlayer.OnErrorListener END
	public void onVideoSizeChanged(IMediaPlayer mp, int width, int height, int sar_num, int sar_den) {
		Log.v(TAG, String.format("onVideoSizeChanged: %dx%d", width, height));
		if ( width == 0 || height == 0 )
		{
			Log.e( TAG, "The video size is 0. Could be because there was no video, no display surface was set, or the value was not determined yet." );
		}
		else
		{
			nativeSetVideoSize(appPtr, width, height);
		}
	}
	
	public void onPrepared(IMediaPlayer mp) {
		// TODO Auto-generated method stub
		Log.i(TAG, "IMediaPlayer onPrepared called");
		
		// If this movie has a saved position, seek there before starting
		// This seems to make movie switching crashier.
		if (null != currentPathName) {
			final int seekPos = getPreferences(MODE_PRIVATE).getInt(currentPathName + "_pos", 0);
			if (seekPos > 0) {
				try {
					mediaPlayer.seekTo(seekPos);
				}
				catch( IllegalStateException ise ) {
					Log.d( TAG, "mediaPlayer.seekTo(): Caught illegalStateException: " + ise.toString() );
				}
			}
		}

		//mediaPlayer.setLooping(false);

		try {
			Log.v(TAG, "mediaPlayer.start");
			mediaPlayer.start();
		}
		catch( IllegalStateException ise ) {
			Log.d( TAG, "mediaPlayer.start(): Caught illegalStateException: " + ise.toString() );
		}
		
		mediaPlayer.setVolume(1.0f, 1.0f);
	}
	
	public void onCompletion(IMediaPlayer mp) {
		Log.v(TAG, String.format("onCompletion"));
		nativeVideoCompletion(appPtr);
	}

	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		nativeFrameAvailable(appPtr);
	}
	
	   public void onAudioFocusChange(int focusChange) {
			switch( focusChange ) {
				case AudioManager.AUDIOFOCUS_GAIN:
					// resume() if coming back from transient loss, raise stream volume if duck applied
					Log.d(TAG, "onAudioFocusChangedListener: AUDIOFOCUS_GAIN");
					break;
				case AudioManager.AUDIOFOCUS_LOSS:				// focus lost permanently
					// stop() if isPlaying
					Log.d(TAG, "onAudioFocusChangedListener: AUDIOFOCUS_LOSS");		
					break;
				case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:	// focus lost temporarily
					// pause() if isPlaying
					Log.d(TAG, "onAudioFocusChangedListener: AUDIOFOCUS_LOSS_TRANSIENT");	
					break;
				case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:	// focus lost temporarily
					// lower stream volume
					Log.d(TAG, "onAudioFocusChangedListener: AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK");		
					break;
				default:
					break;
				}
		}

		public void stopMovie() {
			Log.d(TAG, "stopMovie() from java" );

			if (mediaPlayer != null) {
				
				mediaPlayer.stop();
				Log.d(TAG, "movie had stoped" );
			}

			releaseAudioFocus();
		}
		
		// called from native code for starting movie
		public void startMovieFromNative( final String pathName ) {
			Log.d( TAG, "startMovieFromNative" );
	    	runOnUiThread( new Thread() {
				@Override
	    		public void run() {
					Log.d( TAG, "startMovieFromNative" );
				 	startMovie( pathName );
	    		}
	    	});
		}

		public void startMovie(String pathName) {
			Log.v(TAG, "startMovie " + pathName);
			
			synchronized (this) 
			{
				// Request audio focus
				requestAudioFocus();
			
				// Have native code pause any playing movie,
				// allocate a new external texture,
				// and create a surfaceTexture with it.
				movieTexture = nativePrepareNewVideo(appPtr);
				movieTexture.setOnFrameAvailableListener(this);
				movieSurface = new Surface(movieTexture);

				if (mediaPlayer != null) {
					mediaPlayer.release();
				}

				Log.v(TAG, "MediaPlayer.create");

				synchronized (this) {
					//mediaPlayer = new IjkMediaPlayer();
					//IjkMediaPlayer ijkMediaPlayer = (IjkMediaPlayer)mediaPlayer;
					//ijkMediaPlayer.setOption(IjkMediaPlayer.OPT_CATEGORY_PLAYER, "overlay-format", IjkMediaPlayer.SDL_FCC_YV12);
					mediaPlayer = new AndroidMediaPlayer();
				}
				mediaPlayer.setOnVideoSizeChangedListener(this);
				mediaPlayer.setOnCompletionListener(this);
				mediaPlayer.setOnPreparedListener(this);
				mediaPlayer.setSurface(movieSurface);

				try {
					Log.v(TAG, "mediaPlayer.setDataSource()");
					mediaPlayer.setDataSource(pathName);
				} catch (IOException t) {
					Log.e(TAG, "mediaPlayer.setDataSource failed");
				}
				
				currentPathName = pathName;
				mediaPlayer.prepareAsync();

				// Save the current movie now that it was successfully started
				Editor edit = getPreferences(MODE_PRIVATE).edit();
				edit.putString("currentMovie", pathName);
				edit.commit();
			}

			Log.v(TAG, "returning");
		}
}