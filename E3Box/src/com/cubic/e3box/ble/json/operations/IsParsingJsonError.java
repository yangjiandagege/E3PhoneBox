package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.util.Log;
import com.cubic.e3box.ble.JsonOperation;

public class IsParsingJsonError extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation IsParsingJsonError");
	    JSONObject jsonRsp = new JSONObject();
	    try {
	    	jsonRsp.put("method", "IsParsingJsonError");
	    	jsonRsp.put("result", -1);
	    	jsonRsp.put("reason", "400 Bad Request");
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}
}
