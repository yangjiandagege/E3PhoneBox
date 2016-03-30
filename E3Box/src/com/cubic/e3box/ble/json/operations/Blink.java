package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.util.Log;
import com.cubic.e3box.ble.JsonOperation;

public class Blink  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation Blink");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	jsonRsp.put("method", "Blink");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}