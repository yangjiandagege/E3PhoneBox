package com.cubic.e3box.ble.json.operations;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class SetRadioList  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetRadioList");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONArray jsonReqRadioList = jsonReq.getJSONArray("RadioList"); 
	    	JSONObject jsonReqRadioListChannelTmp = null;
	    	JSONObject jsonReqRadioListChannelTmp1 = null;
	    	String strReqRadioListChannelName = null;
	    	String strReqRadioListChannelUrl = null;
	    	String strReqRadioListChannelFreq = null;
	    	int iRadioListLength = 0;
	    	if(iRadioListLength > 12){
	    		iRadioListLength = 12;
	    	}else{
	    		iRadioListLength = jsonReqRadioList.length();
	    	}
	    	for(int i = 0 ; i < iRadioListLength ; i++){
	    		jsonReqRadioListChannelTmp = jsonReqRadioList.getJSONObject(i);
	    		jsonReqRadioListChannelTmp1 = jsonReqRadioListChannelTmp.getJSONObject("Channel");
	    		strReqRadioListChannelName = jsonReqRadioListChannelTmp1.getString("name");
	    		
	    		try {
	    			strReqRadioListChannelUrl = jsonReqRadioListChannelTmp1.getString("url");
	    		} catch (JSONException e) {
	    			strReqRadioListChannelUrl = null;
	    		}
	    		try {
	    			strReqRadioListChannelFreq = jsonReqRadioListChannelTmp1.getString("freq");
	    		} catch (JSONException e) {
	    			strReqRadioListChannelFreq = null;
	    		}
	    		SystemProperties.set(Constants.BLE_PROP_radio_name_no+i, strReqRadioListChannelName);
	    		if(strReqRadioListChannelUrl != null){
	    			SystemProperties.set(Constants.BLE_PROP_radio_value_no+i, "url:"+strReqRadioListChannelUrl);
	    		}else if(strReqRadioListChannelFreq != null){
	    			SystemProperties.set(Constants.BLE_PROP_radio_value_no+i, "freq:"+strReqRadioListChannelFreq);
	    		}
	    	}
	    	
	    	jsonRsp.put("method", "SetRadioList");
	    	jsonRsp.put("result", 200); 
	    	jsonRsp.put("reason", "OK"); 
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
