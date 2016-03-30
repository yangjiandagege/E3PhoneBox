package com.cubic.e3box;


import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.media.audiofx.AcousticEchoCanceler;
import android.media.audiofx.AutomaticGainControl;
import android.os.Process;
import android.util.Log;


public class AudioControl {
	static String TAG="AudioControl";
	private static void logout(String log) {
		Log.d(TAG, TAG + " " + log );
	}

	public static final int FRAME_SIZE = 1280;
    private static final int FREQUENCY = 16000;  
    private static final int CHANNEL_INPUT = AudioFormat.CHANNEL_IN_MONO;  
    private static final int CHANNEL_OUTPUT = AudioFormat.CHANNEL_OUT_MONO;  
    private static final int SAMPLE_FORMAT = AudioFormat.ENCODING_PCM_16BIT; 

	public enum AudioType { RING, CALL, MIC, RADIO };
	private int audioTypeMap [] = { AudioManager.STREAM_RING, AudioManager.STREAM_VOICE_CALL, MediaRecorder.AudioSource.MIC, AudioManager.STREAM_MUSIC };
	
	public enum VolumeLevel { VOL_LOW, VOL_MID, VOL_MAX };
	private float volumeLevelMap [] = {0.4f, 0.7f, 1.0f};
    
    private AudioRecord callRecord;
    private AudioTrack callTrack;
    private AudioTrack musicTrack;
    private AudioTrack ringTrack;
    private AcousticEchoCanceler acousticEchoCanceler = null;
    private AutomaticGainControl automaticGainControl = null;
    private AudioManager audioManager = null;
    
    private int RECORD_BUF_SIZE = 0;
    private int PLAY_BUF_SIZE = 0; 
    
	public AudioControl()
	{
        RECORD_BUF_SIZE = AudioRecord.getMinBufferSize(FREQUENCY, CHANNEL_INPUT, SAMPLE_FORMAT);
        PLAY_BUF_SIZE = AudioTrack.getMinBufferSize(FREQUENCY, CHANNEL_OUTPUT, SAMPLE_FORMAT);

		if(RECORD_BUF_SIZE < FRAME_SIZE){
			RECORD_BUF_SIZE = FRAME_SIZE;
		}

		if(PLAY_BUF_SIZE < FRAME_SIZE){
			PLAY_BUF_SIZE = FRAME_SIZE;
		}
	}

	public boolean setup(AudioManager am) {
		audioManager = am;
        callRecord = new AudioRecord(audioTypeMap[AudioType.MIC.ordinal()], FREQUENCY, CHANNEL_INPUT, SAMPLE_FORMAT, RECORD_BUF_SIZE); 
        callTrack  = new AudioTrack(audioTypeMap[AudioType.CALL.ordinal()], FREQUENCY, CHANNEL_OUTPUT, SAMPLE_FORMAT, PLAY_BUF_SIZE, AudioTrack.MODE_STREAM, callRecord.getAudioSessionId());
		musicTrack = new AudioTrack(audioTypeMap[AudioType.RADIO.ordinal()], FREQUENCY, CHANNEL_OUTPUT, SAMPLE_FORMAT, PLAY_BUF_SIZE, AudioTrack.MODE_STREAM);
		ringTrack  = new AudioTrack(audioTypeMap[AudioType.RING.ordinal()], FREQUENCY, CHANNEL_OUTPUT, SAMPLE_FORMAT, PLAY_BUF_SIZE, AudioTrack.MODE_STREAM);
		
        // AGC
        if(AutomaticGainControl.isAvailable() == true)
        {
        	automaticGainControl = AutomaticGainControl.create(callRecord.getAudioSessionId());
			if(automaticGainControl != null) {
        		automaticGainControl.setEnabled(true);
			}
        }

        // AEC
        if(AcousticEchoCanceler.isAvailable() == true)
        {
        	acousticEchoCanceler = AcousticEchoCanceler.create(callRecord.getAudioSessionId());
			if(acousticEchoCanceler != null) {
		    	acousticEchoCanceler.setEnabled(true);
			}
	    }

        setVolume(AudioType.CALL, VolumeLevel.VOL_MID);
        setVolume(AudioType.RADIO, VolumeLevel.VOL_MAX);
        setVolume(AudioType.RING, VolumeLevel.VOL_MAX);
        return true;
	}
	
	public void shutdown() {
		if(acousticEchoCanceler != null) {
			acousticEchoCanceler.setEnabled(false);
			acousticEchoCanceler.release();
			acousticEchoCanceler = null;
		}

		if(automaticGainControl != null) {
			automaticGainControl.setEnabled(false);
			automaticGainControl.release();
			automaticGainControl = null;
		}
		
		if( callRecord != null ) {
			callRecord.stop();
			callRecord.release();
			callRecord = null;
		}
		
		if( callTrack != null ) {
			callTrack.stop();
			callTrack.release();
			callTrack = null;
		}

		if( musicTrack != null ) {
			musicTrack.stop();
			musicTrack.release();
			musicTrack = null;
		}
		
		if( ringTrack != null ) {
			ringTrack.stop();
			ringTrack.release();
			ringTrack = null;
		}
	}

	// ************************************ ring tone **************************************
	// ring tone
	public class LocalRingThread extends Thread{
		private boolean isStop = false;

		public void end() {
			isStop = true;
			try {
				this.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		};

		public void run(){
			File ringTone = new File("/system/res/sound/ringtone-16k-16b-mono.pcm");
			FileInputStream is = null;

			try {
				is = new FileInputStream(ringTone);
			} catch (FileNotFoundException e1) {
				e1.printStackTrace();
			}

			if( is == null ) {
				return;
			}

			byte [] data = new byte[AudioControl.FRAME_SIZE];
			int ret = 0;
			while(isStop == false){
				try {
					ret = is.read(data, 0, data.length);
					if(ret < AudioControl.FRAME_SIZE){
						is.getChannel().position(0);
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
				ringTrack.write(data, 0, data.length);
			}
		}
	}
	private LocalRingThread localRingThread = null;

	
	public void startRing(){
		ringTrack.play();
		if( localRingThread == null )
		{
			localRingThread = new LocalRingThread();
			localRingThread.start();
		}
	};

	public void stopRing(){
		if( localRingThread != null )
		{
			localRingThread.end();
			localRingThread = null;
		}
		ringTrack.stop();
	};

	// ************************************ call sound & record ***********************************
	private class RecordThread extends Thread {
		private ISoundRecord mSoundRecord = null;
		private boolean isStop = false;

		public RecordThread(ISoundRecord soundRecord)	{
			mSoundRecord = soundRecord;
		};

		public void end() {
			isStop = true;
			try {
				this.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		};

		public void run(){
			Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

			byte[] data = new byte[RECORD_BUF_SIZE];
			int ret = 0;

			while(isStop == false){
				ret = callRecord.read(data, 0, data.length);
				if( ret !=  RECORD_BUF_SIZE)
				{
					break;
				}
				mSoundRecord.onSound(data);
			}
		}
	};
	private RecordThread mRecordThread = null;

	public void startCall(ISoundRecord sr)
	{
		callTrack.play();
		callRecord.startRecording();
		if( mRecordThread == null )
		{
			mRecordThread = new RecordThread(sr);
			mRecordThread.start();
		}
	}

	public void stopCall()
	{
		callTrack.stop();
		callRecord.stop();
		if( mRecordThread != null )
		{
			mRecordThread.end();
			mRecordThread = null;
		}
	}
	
	public int writeCallSound(byte[] buffer)
	{
		return callTrack.write(buffer, 0, buffer.length); 
	}
	
	
	// ************************************** volume *****************************************
	public void setVolume( AudioType type, VolumeLevel level ) {
		setVolume(type, volumeLevelMap[level.ordinal()]);
	}

	public void setVolume( AudioType type, float volume ) {
		if( audioManager == null ) {
			return;
		}
		
		logout("setVolume type:" + type + " volume:" + volume );
		if(type != AudioType.MIC) {
			int vol = (int)(volume * audioManager.getStreamMaxVolume(audioTypeMap[type.ordinal()]));
			audioManager.setStreamVolume(audioTypeMap[type.ordinal()], vol, 0);
		}
	}
	
	public float getVolume(AudioType type) {
		if(type != AudioType.MIC) {
			return (float)audioManager.getStreamVolume(audioTypeMap[type.ordinal()]) / audioManager.getStreamMaxVolume(audioTypeMap[type.ordinal()]);
		}
		return 1.0f;
	}

	public VolumeLevel getVolumeLevel(AudioType type) {
		float vol = getVolume(type);
		for( int i = 0; i < volumeLevelMap.length; i++ ) {
			if( vol <= volumeLevelMap[i] ) {
				return VolumeLevel.values()[i];
			}
		}
		return VolumeLevel.VOL_MAX;
	}
	
	// ************************************* radio ***************************************
	public void startRadio()
	{
		musicTrack.play();
	}

	public void stopRadio()
	{
		musicTrack.stop();
	}

	public int writeRadioSound(byte[] buffer)
	{
		return musicTrack.write(buffer, 0, buffer.length); 
	}
}
