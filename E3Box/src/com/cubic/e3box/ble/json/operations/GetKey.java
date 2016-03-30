package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;
import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.SystemProperties;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.cubic.e3box.utils.CharUtils;
import com.cubic.e3box.utils.CommonUtils;

public class GetKey  extends JsonOperation{
	String TAG = "BleStation";
	Context mContext = null;
	public GetKey(Context context)
	{
		mContext = context;
	}

	public JSONObject jsonOperation(JSONObject jsonReq){
	    Log.e(TAG, " jsonOperation GetKey");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspKey = new JSONObject(); 
	    try {
			WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);  
			WifiInfo info = wifiManager.getConnectionInfo();  
			String strMacAddress = info.getMacAddress();
			String strSerialID = SystemProperties.get(Constants.BLE_PROP_serial_id, Constants.BLE_DEFAULT_VALUE_NULL);
			String strTimestamp = CommonUtils.getTimestamp();
			String strTxtSum = "mk:e3phone.info:"+strSerialID+":"+strMacAddress.replace(":","")+""+":"+strTimestamp;
			String strAccesskey = CommonUtils.getDigestToString(strTxtSum);
			jsonRsp.put("method", "GetKey");
			jsonRsp.put("result", "200");
			jsonRsp.put("reason", "ok");
			jsonRspKey.put("key",strAccesskey);
			jsonRspKey.put("time",strTimestamp);
			jsonRsp.put("Key", jsonRspKey);
		} catch (JSONException e) {
			e.printStackTrace();
		} 
		return jsonRsp;
	}

}
