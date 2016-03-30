package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.util.Log;
import com.cubic.e3box.ble.JsonOperation;

public class EnableRadio  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation EnableRadio");
	    JSONObject jsonRsp = new JSONObject(); 

	    try {
	    	jsonRsp.put("method", "EnableRadio");
	    	jsonRsp.put("result", "200");
	    	jsonRsp.put("reason", "ok");
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}
}