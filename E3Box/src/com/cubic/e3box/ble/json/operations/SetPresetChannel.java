package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class SetPresetChannel  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetPresetChannel");
	    JSONObject jsonRsp = new JSONObject(); 

	    try {
	    	jsonRsp.put("method", "SetPresetChannel");
	    	jsonRsp.put("result", "200");
	    	jsonRsp.put("reason", "ok");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
