package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import com.cubic.e3box.utils.CharUtils;

import android.os.SystemProperties;
import android.util.Log;

public class SetVideo  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetVideo");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONObject reqJsonObjectVideo = jsonReq.getJSONObject("Video");
	    	String user = reqJsonObjectVideo.getString("user");
	    	String pass = reqJsonObjectVideo.getString("pass");
	    	if(CharUtils.isAlphabetic(user) && user.length() <= 64){
		    	SystemProperties.set(Constants.BLE_PROP_video_user, user);
		    	SystemProperties.set(Constants.BLE_PROP_video_pass, pass);
		    	jsonRsp.put("method", "SetVideo");
		    	jsonRsp.put("result", 200);
		    	jsonRsp.put("reason", "OK");
	    	}else{
	    		jsonRsp.put("method", "SetVideo");
			    jsonRsp.put("result", 488);
			    jsonRsp.put("reason", "Invalid characters in username");
	    	}
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
