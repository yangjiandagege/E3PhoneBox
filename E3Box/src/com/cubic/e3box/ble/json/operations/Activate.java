package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class Activate extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation Activate");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
            JSONObject jsonActivateData = jsonReq.getJSONObject("ActivationData");
            String strClientIdFromE3phone = jsonReq.getString("client");
            String strClientIdFromE3box = SystemProperties.get(Constants.BLE_PROP_islocalowner_client, Constants.BLE_DEFAULT_VALUE_NULL);
            
            if(!strClientIdFromE3box.equals(Constants.BLE_DEFAULT_VALUE_NULL))
            {
            	jsonRsp.put("method", "Activate");
            	jsonRsp.put("result", 403);
            	jsonRsp.put("reason", "Already activated");
            }else if(jsonActivateData == null){
            	jsonRsp.put("method", "Activate");
            	jsonRsp.put("result", 403);
            	jsonRsp.put("reason", "ActivationData not found");
            }else{
                String strApi = jsonActivateData.getString("api");
                String strPassword = jsonActivateData.getString("password");
                String strGroup = jsonActivateData.getString("group");
                SystemProperties.set(Constants.BLE_PROP_activate_api,strApi);
                SystemProperties.set(Constants.BLE_PROP_activate_pass,strPassword);
                SystemProperties.set(Constants.BLE_PROP_activate_group,strGroup);
                SystemProperties.set(Constants.BLE_PROP_islocalowner_client,strClientIdFromE3phone);
                jsonRsp.put("method", "Activate");
                jsonRsp.put("result", 200);
                jsonRsp.put("reason", "OK");
            }
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}
}
