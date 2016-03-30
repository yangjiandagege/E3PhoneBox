package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class IsLocalOwner extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation IsLocalOwner");
	    JSONObject jsonRsp = new JSONObject(); 

	    try {
			String client = jsonReq.getString("client");
			if(client.equals(SystemProperties.get(Constants.BLE_PROP_islocalowner_client, Constants.BLE_DEFAULT_VALUE_NULL )))
			{
				jsonRsp.put("method", "IsLocalOwner");
				jsonRsp.put("result", 200);
				jsonRsp.put("reason", "OK");
			}else{
				jsonRsp.put("method", "IsLocalOwner");
				jsonRsp.put("result", 403);
				jsonRsp.put("reason", "Forbidden");
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
