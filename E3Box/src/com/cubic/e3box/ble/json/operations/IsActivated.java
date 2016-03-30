package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import android.os.SystemProperties;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;

public class IsActivated  extends JsonOperation{
	String TAG = "BleStation";

	public JSONObject jsonOperation(JSONObject jsonReq){
	    JSONObject jsonRsp = new JSONObject(); 
	    try {
	    	String strClientIdFromE3phone = jsonReq.getString("client");
	    	String strClientIdFromE3box = SystemProperties.get(Constants.BLE_PROP_islocalowner_client, Constants.BLE_DEFAULT_VALUE_NULL);
	    	if(strClientIdFromE3phone.equals(strClientIdFromE3box))
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

