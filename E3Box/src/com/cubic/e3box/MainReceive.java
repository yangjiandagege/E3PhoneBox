package com.cubic.e3box;

import java.util.ArrayList;
import com.cubic.e3box.ble.BleService;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.net.wifi.WifiManager;
import android.os.SystemProperties;
import com.cubic.e3box.utils.CommonUtils;
import com.cubic.e3box.utils.WifiOperationUtils;

public class MainReceive extends BroadcastReceiver{
	String TAG = "MainReceive";
	private static String NET_CHANGE_ACTION = "android.net.conn.CONNECTIVITY_CHANGE";
	
	public static ArrayList<netEventHandler> mNetworkListeners = new ArrayList<netEventHandler>();

	@Override
	public void onReceive(Context context, Intent intent) {
		if(intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED))
		{
			Log.d(TAG,"forge : onReceive ACTION_BOOT_COMPLETED");
			if(!CommonUtils.isServiceWorked(context,"com.cubic.e3box.ble.BleService")){
				Log.d(TAG,"forge : start BleService from onReceive");
				Intent bleServiceIntent = new Intent(context,BleService.class);
				context.startService(bleServiceIntent);
			}
			
			String wireless_ssid = SystemProperties.get("persist.sys.wireless.ssid", "" );
			String wireless_pass = SystemProperties.get("persist.sys.wireless.pass", "" );
			WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
			WifiOperationUtils wifi = new WifiOperationUtils(wifiManager);
			wifi.connect(wireless_ssid, wireless_pass, WifiOperationUtils.WifiCipherType.WIFICIPHER_WPA);
		}
		
		if(intent.getAction().equals(NET_CHANGE_ACTION) || 
				intent.getAction().equals(Constants.SIP_ACCOUNT_CHANGE_ACTION)){
	    	CarrierApp.updateNetStates(context);
	    	if(mNetworkListeners.size() > 0){
                for (netEventHandler handler : mNetworkListeners) {
                    handler.onNetChange();
                }
	    	}
		}
	}
	
    public static abstract interface netEventHandler {
        public abstract void onNetChange();
    }
}