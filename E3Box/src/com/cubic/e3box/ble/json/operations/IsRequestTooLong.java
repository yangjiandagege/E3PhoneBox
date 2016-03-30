package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.util.Log;
import com.cubic.e3box.ble.JsonOperation;

public class IsRequestTooLong extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation IsRequestTooLong");
	    JSONObject jsonRsp = new JSONObject(); 
	    try {
	    	jsonRsp.put("method", "IsRequestTooLong");
	    	jsonRsp.put("result", -1);
	    	jsonRsp.put("reason", "Invalid Attribute Value Length");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
