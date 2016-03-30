package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class PrepareLocalOwnerTransfer  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation PrepareLocalOwnerTransfer");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONObject reqJsonObjectFutureOwner = jsonReq.getJSONObject("FutureOwner");
	    	String strClientId = reqJsonObjectFutureOwner.getString("client");
	    	SystemProperties.set(Constants.BLE_PROP_islocalowner_client, strClientId);
	    	jsonRsp.put("method", "PrepareLocalOwnerTransfer");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
