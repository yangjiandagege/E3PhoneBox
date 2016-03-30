package com.cubic.e3box.pstn;

import java.util.Timer;
import java.util.TimerTask;
import com.cubic.e3box.ICallCenter;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;


public class PstnService{
	public static final String TAG="PstnServiceJ";

	// command to service
	private static final int PSTN_CMD_ECHO = 1;
	private static final int PSTN_CMD_DAIL = 2;
	private static final int PSTN_CMD_HANGUP =3;
	private static final int PSTN_CMD_DTMF = 4;
	private static final int PSTN_CMD_ACCEPT = 5;
	private static final int PSTN_CMD_GET_STAT = 6;
		
	// command from service
	private static final int PSTN_RPT_STAT = 100;
	private static final int PSTN_RPT_INCOMMING_CALL = 101; 

	private static void logout(String log) {
		Log.e(TAG, TAG + " " + log );
	}

	private ThirNativeServiceDef.BpThirNativeService mNativeService = null;
	private ThirNativeServiceDef.BnThirNativeService mNativeServiceCallback = new ThirNativeServiceDef.BnThirNativeService() {

		// data from pstnd
		@Override
		public int sendData(byte[] data) {
			if( mCallCenter != null )
			{
				mCallCenter.onSoundData(data);
			}
			return 0;
		}

		// report from pstnd
		@Override
		public int sendCmd(int code, String info) {
			switch(code)
			{
			case PSTN_RPT_STAT:
				// do nothing
				break;

			case PSTN_RPT_INCOMMING_CALL:
				if( mCallCenter != null )
				{
					logout("PSTN_RPT_INCOMMING_CALL info=" + info);
					mCallCenter.onIncommingCall(info);
				}
				break;
			};
			return 0;
		}

		@Override
		public int regBinder(IBinder binder) throws RemoteException {
			// empty function, do nothing
			return 0;
		}
	};

	private TimerTask mTimerTask = new TimerTask() {
		@Override
		public void run() {
			// if not connected
			if(mNativeService == null){
				logout("ServiceManager.TimerTask.run: try to connect service ..");
				// init service proxy
				IBinder binder = ServiceManager.getService(ThirNativeServiceDef.DESCRIPTOR);
				if( binder == null ) {
					logout("ServiceManager.getService failed !");
					return;
				}

				mNativeService = new ThirNativeServiceDef.BpThirNativeService(binder);
				if( mNativeService == null ) {
					logout("mNativeService create Failed!");
					return;
				}

				// regiester service
				try
				{
					int ret = mNativeService.regBinder(mNativeServiceCallback);
					logout("mNativeService.doAction.regBinder ret="+ret);
				}
				catch( Exception e )
				{
					e.printStackTrace();
				}
			} else {
				// test if service still online
				// if service offline, reconnect it
				if( sendCmd(PSTN_CMD_ECHO) < 0 ) {
					logout("ServiceManager.TimerTask.run: lost service !");
					mNativeService = null;
				}
			}
		}
	};
	private Timer mTimer = new Timer();

	private ICallCenter mCallCenter = null;

	private int sendCmd( int code, String info ) {
		int ret = 0;

		if(mNativeService == null) {
			return -100;
		}
		
		try
		{
			ret = mNativeService.sendCmd(code ,info);
		}
		catch( Exception e )
		{
			e.printStackTrace();
			ret = -99;
		}
		return ret;
	};
	private int sendCmd( int code ) {
		return sendCmd(code, "");
	};

	public boolean init(ICallCenter callCenter) {
		logout("init");
		mTimer.schedule(mTimerTask, 100, 2000);
		mCallCenter = callCenter;
		return true;
	};

	public void deinit() {
		mTimer.cancel();
	};

	public boolean dial(String number) {
		if( 0 > sendCmd(PSTN_CMD_DAIL, number) )
		{
			return false;
		}
		return true;
	};

	public boolean dtmf(String number) {
		if( 0 > sendCmd(PSTN_CMD_DTMF, number) )
		{
			return false;
		}
		return true;
	};

	public void hangup() {
		sendCmd(PSTN_CMD_HANGUP);
	};

	public void accept() {
		sendCmd(PSTN_CMD_ACCEPT);
	};

	public void writeSound( byte[] data ) {
		if( mNativeService == null ) {
			return;
		}

		try {
			mNativeService.sendData(data);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	};
}
