package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class UpgradeFirmware  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation UpgradeFirmware");
	    JSONObject jsonRsp = new JSONObject(); 

	    try {
	    	jsonRsp.put("method", "UpgradeFirmware");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}
}
