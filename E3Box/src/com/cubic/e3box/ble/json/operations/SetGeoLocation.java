package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class SetGeoLocation  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetGeoLocation");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONObject jsonReqGeoLocation = jsonReq.getJSONObject("GeoLocation");
	    	String strLatitude = jsonReqGeoLocation.getString("latitude");
	    	String strLongitude = jsonReqGeoLocation.getString("longitude");
	    	SystemProperties.set(Constants.BLE_PROP_location_latitude, strLatitude);
	    	SystemProperties.set(Constants.BLE_PROP_lacation_longitude, strLongitude);
	    	jsonRsp.put("method", "SetGeoLocation");
	    	jsonRsp.put("result", 200); 
	    	jsonRsp.put("reason", "OK");
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
