package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class GetDialButtons  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetDialButtons");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspButtons = new JSONObject(); 
	    try {
	    	String strButtonsBtn1 = SystemProperties.get(Constants.BLE_PROP_buttons_btn1, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strButtonsBtn2 = SystemProperties.get(Constants.BLE_PROP_buttons_btn2, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strButtonsBtn3 = SystemProperties.get(Constants.BLE_PROP_buttons_btn3, Constants.BLE_DEFAULT_VALUE_NULL);
	    	
	    	jsonRsp.put("method", "GetDialButtons");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspButtons.put("btn1", strButtonsBtn1);
	    	jsonRspButtons.put("btn2", strButtonsBtn2);
	    	jsonRspButtons.put("btn3", strButtonsBtn3);
			jsonRsp.put("Buttons", jsonRspButtons);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}

}
