package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class GetLog  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetLog");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspLog = new JSONObject(); 
	    
	    try {
	    	jsonRsp.put("method", "GetLog");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspLog.put("type", "system");
	    	jsonRspLog.put("data", "2014-09-11 19:23:14.101 [DEBUG] System Started");
			jsonRsp.put("Log", jsonRspLog);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
