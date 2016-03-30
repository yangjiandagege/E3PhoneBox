package com.cubic.e3box.sip;

import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;
import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.CarrierApp;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ICallCenter;
import com.cubic.e3box.MainReceive;
import com.cubic.e3box.MainReceive.netEventHandler;
import com.cubic.e3box.sip.ThirNativeServiceDef;
import com.cubic.e3box.utils.CommonUtils;
import com.cubic.e3box.utils.HttpsRequestUtils;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Base64;
import android.util.Log;

public class SipService implements netEventHandler {
	public static final String TAG="SipService";
	
	private boolean mIsSipRegistered = false;
	private int mNetType = NET_TYPE_NONE;
	
	private static final int NET_TYPE_NONE = 0;
	private static final int NET_TYPE_WIFI = 1;
	private static final int NET_TYPE_WIRED =2;
	
	// command to service
	private static final int SIP_CMD_ECHO = 1;
	private static final int SIP_CMD_DAIL = 2;
	private static final int SIP_CMD_HANGUP =3;
	private static final int SIP_CMD_DTMF = 4;
	private static final int SIP_CMD_ACCEPT = 5;
	private static final int SIP_CMD_GET_STAT = 6;
	private static final int SIP_CMD_REGISTER = 7;
	
	// command from service
	private static final int SIP_RPT_STAT = 100;
	private static final int SIP_RPT_INCOMMING_CALL = 101; 
	private static final int SIP_RPT_HANGUP = 102;
	private static final int SIP_RPT_REGISTER_STAT = 103;
	private static final int SIP_RPT_ACCEPT = 104;
	
	public SipService(){
		MainReceive.mNetworkListeners.add(this);
	}
	
	private static void logout(String log) {
		Log.e(TAG, TAG + " " + log );
	}
	
	private ThirNativeServiceDef.BpThirNativeService mNativeService = null;
	private ThirNativeServiceDef.BnThirNativeService mNativeServiceCallback = new ThirNativeServiceDef.BnThirNativeService() {

		// data from sipd
		@Override
		public int sendData(byte[] data) {
			if( mCallCenter != null )
			{
				mCallCenter.onSoundData(data);
			}
			return 0;
		}

		// report from sipd
		@Override
		public int sendCmd(int code, String info) {
			switch(code)
			{
			case SIP_RPT_STAT:
				// do nothing
				break;

			case SIP_RPT_INCOMMING_CALL:
				if( mCallCenter != null )
				{
					String[] strInfoTmp = info.split("::");
					if(strInfoTmp.length == 2){
						String strNumber = strInfoTmp[0];
						String strXwsse = strInfoTmp[1];
						
						if(CommonUtils.checkPasswordDigest(strXwsse,CommonUtils.PASSWORD_TYPE_PSTN)){
							mCallCenter.onIncommingCall(strNumber);
						}else{
							Log.e(TAG,"checkPasswordDigest error");
						}
					}else{
						mCallCenter.onIncommingCall(info);
					}
				}
				break;

			case SIP_RPT_HANGUP:
				if( mCallCenter != null )
				{
					mCallCenter.onHangup();
				}
				break;
				
			case SIP_RPT_REGISTER_STAT:
				logout("SIP_RPT_REGISTER_STAT regCode = " + info);
				mIsSipRegistered = info.equals("200");
				break;
			
			case SIP_RPT_ACCEPT:
				if( mCallCenter != null )
				{
					mCallCenter.onAccept();
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
				if( sendCmd(SIP_CMD_ECHO) < 0 ) {
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
		if( 0 > sendCmd(SIP_CMD_DAIL, number) )
		{
			return false;
		}
		return true;
	};

	public boolean dtmf(String number) {
		if( 0 > sendCmd(SIP_CMD_DTMF, number) )
		{
			return false;
		}
		return true;
	};

	public void hangup() {
		sendCmd(SIP_CMD_HANGUP);
	};

	public void accept() {
		sendCmd(SIP_CMD_ACCEPT);
	};

	public void register(String regInfo){
		sendCmd(SIP_CMD_REGISTER,regInfo);
	}
	
	public void writeSound( byte[] data ) {
		if( mNativeService == null ) {
			return;
		}

		try {
			mNativeService.sendData(data);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void onNetChange() {
    	logout("wifi state = "+CarrierApp.getNetWifiState()+" ; " +
    			"ethernet state = "+CarrierApp.getNetEthernetState());
    	if(CarrierApp.getNetWifiState() == true || CarrierApp.getNetEthernetState() == true){
			String domain = SystemProperties.get(Constants.BLE_PROP_sip_domain, Constants.BLE_DEFAULT_VALUE_NULL );
			String acc_user = SystemProperties.get(Constants.BLE_PROP_sip_acc_user, Constants.BLE_DEFAULT_VALUE_NULL );
			String acc_pass = SystemProperties.get(Constants.BLE_PROP_sip_acc_pass, Constants.BLE_DEFAULT_VALUE_NULL );
			if(domain.equals(Constants.BLE_DEFAULT_VALUE_NULL) || 
					acc_user.equals(Constants.BLE_DEFAULT_VALUE_NULL) || 
					acc_pass.equals(Constants.BLE_DEFAULT_VALUE_NULL)){
				logout("Error:register infomation is not usefull");
				return;
			}
			String id = "sip:"+acc_user+"@"+domain+";transport=tcp";
			String registrar = "sip:"+domain+";transport=tcp";
			String proxy = "sip:sip.test.e3phone.info:38060;transport=tcp";
			logout("sip register : "+id+"&"+registrar+"&"+"*"+"&"+acc_user+"&"+acc_pass+"&"+proxy);
			register(id+"&"+registrar+"&"+"*"+"&"+acc_user+"&"+acc_pass+"&"+proxy);
    		if(CarrierApp.getNetWifiState() == true){
    			mNetType = NET_TYPE_WIFI;
    		}else{
    			mNetType = NET_TYPE_WIRED;
    		}
    		/***This is a example for Https Request***
			JSONObject jsonReq = new JSONObject();  
			try {
				jsonReq.put("X-Push-Call-ID", UUID.randomUUID().toString());
				jsonReq.put("From", "075-123-4567");
				jsonReq.put("To", "03-123-4567");
			} catch (JSONException e) {
				e.printStackTrace();
			}
			HttpsRequestUtils httpsReq = new HttpsRequestUtils(HttpsRequestUtils.HTTPS_REQ_CALL_All_GROUP_MEMBERS, jsonReq);
			httpsReq.httpsPostData();*/
    	}else{
    		mNetType = NET_TYPE_NONE;
    	}
	};
}
