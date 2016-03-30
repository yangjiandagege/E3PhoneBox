package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.os.SystemProperties;
import android.util.Log;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;

public class GetVideo  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetVideo");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspVideo = new JSONObject(); 
	    
	    try {
	    	String strUser = SystemProperties.get(Constants.BLE_PROP_video_user, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strPass = SystemProperties.get(Constants.BLE_PROP_video_pass, Constants.BLE_DEFAULT_VALUE_NULL);
	    	jsonRsp.put("method", "GetVideo");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspVideo.put("user", strUser);
	    	jsonRspVideo.put("pass", strPass);
			jsonRsp.put("Video", jsonRspVideo);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}