package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import com.cubic.e3box.utils.CharUtils;

import android.os.SystemProperties;
import android.util.Log;

public class SetDialButtons  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetDialButtons");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONObject objButtons = jsonReq.getJSONObject("Buttons");
	    	String objButtonsBtn1 = objButtons.getString("btn1");
	    	String objButtonsBtn2 = objButtons.getString("btn2");
	    	String objButtonsBtn3 = objButtons.getString("btn3");
	    	if(CharUtils.isNumeric(objButtonsBtn1) == true &&
	    			CharUtils.isNumeric(objButtonsBtn2) == true &&
	    			CharUtils.isNumeric(objButtonsBtn3) == true){
		    	SystemProperties.set(Constants.BLE_PROP_buttons_btn1, objButtonsBtn1);
		    	SystemProperties.set(Constants.BLE_PROP_buttons_btn2, objButtonsBtn2);
		    	SystemProperties.set(Constants.BLE_PROP_buttons_btn3, objButtonsBtn3);
		    	jsonRsp.put("method", "GetDialButtons");
		    	jsonRsp.put("result", 200);
		    	jsonRsp.put("reason", "OK");
	    	}else{
	    		jsonRsp.put("method", "GetDialButtons");
	    		jsonRsp.put("result", 488);
				String strErrorReason = "Malformed";
				if(CharUtils.isNumeric(objButtonsBtn1) == false){
					strErrorReason = strErrorReason+" btn1";
				}
				if(CharUtils.isNumeric(objButtonsBtn2) == false){
					strErrorReason = strErrorReason+" btn2";
				}
				if(CharUtils.isNumeric(objButtonsBtn3) == false){
					strErrorReason = strErrorReason+" btn3";
				}
				jsonRsp.put("reason", strErrorReason);
	    	}
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}

}
