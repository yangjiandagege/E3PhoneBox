package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class GetDeviceStatus  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetDeviceStatus");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspStatus = new JSONObject(); 
	    JSONObject jsonRspStatusGeneral = new JSONObject(); 
	    JSONObject jsonRspStatusCalls = new JSONObject(); 
	    try {
	    	jsonRsp.put("method", "GetDeviceStatus");
	    	jsonRsp.put("result", 200);  // 500
	    	jsonRsp.put("reason", "OK"); // "Server Internal Error"
	    	jsonRspStatusGeneral.put("firm", 1001);
	    	jsonRspStatusGeneral.put("mac", "00c09fa19d4a");
	    	jsonRspStatusGeneral.put("activated", true);
	    	jsonRspStatusCalls.put("reg_status", "registered");
	    	jsonRspStatusCalls.put("reg_result", 200);
	    	jsonRspStatusCalls.put("call_status", "idle");
	    	jsonRspStatusCalls.put("call_result", 0);
	    	jsonRspStatusCalls.put("pstn_status", "connected");
			jsonRspStatus.put("General", jsonRspStatusGeneral);
			jsonRspStatus.put("Calls", jsonRspStatusCalls);
			jsonRsp.put("Status", jsonRspStatus);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
