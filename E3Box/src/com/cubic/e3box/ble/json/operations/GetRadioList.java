package com.cubic.e3box.ble.json.operations;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

public class GetRadioList  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetRadioList");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONArray jsonArrayRspRadioList = new JSONArray(); 
	    JSONObject jsonRspRadioListChannel_1 = new JSONObject(); 
	    JSONObject jsonRspRadioListChannel_2 = new JSONObject(); 
	    try {
	    	jsonRsp.put("method", "GetRadioList");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspRadioListChannel_1.put("name", "My Favorite Internet Radio");
	    	jsonRspRadioListChannel_1.put("url", "https://myfavoriteinternetradio.com");
			jsonArrayRspRadioList.put(jsonRspRadioListChannel_1);
			jsonRspRadioListChannel_2.put("name", "FM Osaka");
			jsonRspRadioListChannel_2.put("freq", 4000.00);
			jsonArrayRspRadioList.put(jsonRspRadioListChannel_2);
			jsonRsp.put("RadioList", jsonArrayRspRadioList);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
