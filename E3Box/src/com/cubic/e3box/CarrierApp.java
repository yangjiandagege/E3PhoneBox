package com.cubic.e3box;

import android.app.Application;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public class CarrierApp extends Application {
	String TAG = "CarrierApp";
	private final static int NET_TYPE_WIFI = 1;
	private final static int NET_TYPE_ETHERNET = 7;
	
	private static MainActivity mainActivity;
    private static ConnectivityManager mConnectivityManager;  
    private static NetworkInfo mNetworkInfo[];
    
	private static boolean mNetWifiState = false;
	private static boolean mNetEthernetState = false;
	
	public CarrierApp() {
		// TODO Auto-generated constructor stub
		mainActivity = null;
	}
	
	public static void setMainActivity(MainActivity activity){
		mainActivity = activity;
	}
	
	public static MainActivity getMainActivity(){
		return mainActivity;
	}

	public static void updateNetStates(Context context){
    	mConnectivityManager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);  
    	mNetworkInfo = mConnectivityManager.getAllNetworkInfo();
		mNetWifiState = mNetworkInfo[NET_TYPE_WIFI].getState().toString().equals("CONNECTED");
		mNetEthernetState = mNetworkInfo[NET_TYPE_ETHERNET].getState().toString().equals("CONNECTED");
	}
	
	public static boolean getNetWifiState(){
		return mNetWifiState;
	}
	
	public static boolean getNetEthernetState(){
		return mNetEthernetState;
	}
}
