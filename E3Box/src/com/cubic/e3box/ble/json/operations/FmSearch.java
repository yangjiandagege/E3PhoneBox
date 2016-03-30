package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class FmSearch  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation FmSearch");
	    JSONObject jsonRsp = new JSONObject(); 

	    try {
	    	jsonRsp.put("method", "FmSearch");
	    	jsonRsp.put("result", "200");
	    	jsonRsp.put("reason", "ok");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
