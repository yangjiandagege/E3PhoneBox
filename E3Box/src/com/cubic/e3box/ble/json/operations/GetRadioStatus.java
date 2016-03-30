package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class GetRadioStatus  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetRadioStatus");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspStatus = new JSONObject();
	    JSONObject jsonRspStatusChannel = new JSONObject();
	    try {
	    	jsonRsp.put("method", "GetRadioStatus");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspStatusChannel.put("name", "FM Osaka");
	    	jsonRspStatusChannel.put("freq", 4000.00);
			jsonRspStatus.put("Channel", jsonRspStatusChannel);
			jsonRsp.put("Status", jsonRspStatus);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}

}
