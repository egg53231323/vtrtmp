/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import java.io.IOException;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.Toast;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences.Editor;
import android.graphics.SurfaceTexture;

import com.oculusvr.vrlib.VrActivity;
import com.oculusvr.vrlib.VrLib;

public class MainActivity extends VrActivity implements SurfaceHolder.Callback,
android.graphics.SurfaceTexture.OnFrameAvailableListener,
MediaPlayer.OnVideoSizeChangedListener,
MediaPlayer.OnCompletionListener,
MediaPlayer.OnErrorListener,
AudioManager.OnAudioFocusChangeListener {
	public static final String TAG = "FancyTech";

	SurfaceTexture movieTexture = null;
	Surface movieSurface = null;
	MediaPlayer mediaPlayer = null;	
	AudioManager audioManager = null;
	
    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );
	public static native SurfaceTexture nativePrepareNewVideo(long appPtr );
	public static native void nativeFrameAvailable( long appPtr );
	public static native void nativeVideoCompletion( long appPtr );
	public static native void nativeSetVideoSize( long appPtr, int width, int height );
   // public static native void nativeDestroy(long appPtr );

	static
	{
		 System.loadLibrary("ovrapp");
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
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

       
        
		Intent intent = getIntent();
		
		
		String commandString = VrLib.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrLib.getPackageStringFromIntent( intent );
		String uriString = VrLib.getUriStringFromIntent( intent );

		appPtr = nativeSetAppInterface( this, fromPackageNameString, commandString, uriString );
		audioManager = (AudioManager) getSystemService( Context.AUDIO_SERVICE );
		
		String videoDirPath= Environment.getExternalStorageDirectory().getPath()+"/oculus/360videos/";
		String fn=videoDirPath+"3d_tag2_2_videofile.mp4";

		if(fn==null || fn.isEmpty())
		{
		    Toast.makeText(getApplicationContext(), "Movie文件名为空", 0).show();  
		    return;
		}
		else
			Toast.makeText(getApplicationContext(), "Movie文件名="+fn, 0).show();
		//play movie
		startMovie( fn );
    }

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		if(event.getKeyCode()==KeyEvent.KEYCODE_BACK)
		{
			Log.d(TAG, "dispatchKeyEvent  KEYCODE_BACK");
			super.onPause();
			returnToMain();
		}
		return false;
		//return super.dispatchTouchEvent(e);
		
	}
	
	private void returnToMain()
	{
		//在这里编写返回的逻辑
		//Intent intent=new Intent(firstActitity.this,MainActivity.class);
		//startActivity(intent);
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
		
		// Abandon audio focus if we still hold it
		releaseAudioFocus();

		super.onDestroy();	
	}
//	@Override
//	protected void OnStop() {	
//		Log.d(TAG, "OnStop");
//		
//		super.OnStop();	
//	}
	// --> MediaPlayer.OnErrorListener START
	public boolean onError( MediaPlayer mp, int what, int extra ) {
		Log.e( TAG, "MediaPlayer.OnErrorListener - what : " + what + ", extra : " + extra );
		return false;
	}
	
	// <-- MediaPlayer.OnErrorListener END
	
	public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
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

	public void onCompletion(MediaPlayer mp) {
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
					mediaPlayer = new MediaPlayer();
				}
				mediaPlayer.setOnVideoSizeChangedListener(this);
				mediaPlayer.setOnCompletionListener(this);
				mediaPlayer.setSurface(movieSurface);

				try {
					Log.v(TAG, "mediaPlayer.setDataSource()");
					mediaPlayer.setDataSource(pathName);
				} catch (IOException t) {
					Log.e(TAG, "mediaPlayer.setDataSource failed");
				}

				try {
					Log.v(TAG, "mediaPlayer.prepare");
					mediaPlayer.prepare();
				} catch (IOException t) {
					Log.e(TAG, "mediaPlayer.prepare failed:" + t.getMessage());
				}
				Log.v(TAG, "mediaPlayer.start");

				// If this movie has a saved position, seek there before starting
				// This seems to make movie switching crashier.
				final int seekPos = getPreferences(MODE_PRIVATE).getInt(pathName + "_pos", 0);
				if (seekPos > 0) {
					try {
						mediaPlayer.seekTo(seekPos);
					}
					catch( IllegalStateException ise ) {
						Log.d( TAG, "mediaPlayer.seekTo(): Caught illegalStateException: " + ise.toString() );
					}
				}

				mediaPlayer.setLooping(false);

				try {
					mediaPlayer.start();
				}
				catch( IllegalStateException ise ) {
					Log.d( TAG, "mediaPlayer.start(): Caught illegalStateException: " + ise.toString() );
				}
				
				mediaPlayer.setVolume(1.0f, 1.0f);

				// Save the current movie now that it was successfully started
				Editor edit = getPreferences(MODE_PRIVATE).edit();
				edit.putString("currentMovie", pathName);
				edit.commit();
			}

			Log.v(TAG, "returning");
		}
}
