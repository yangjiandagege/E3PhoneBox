package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class GetSip extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetSip");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspSip = new JSONObject();
	    
	    try {

			String sport = SystemProperties.get(Constants.BLE_PROP_sip_sport, Constants.BLE_DEFAULT_VALUE_NULL );
			String mport_start = SystemProperties.get(Constants.BLE_PROP_sip_mport_start, Constants.BLE_DEFAULT_VALUE_NULL );
			String mport_end = SystemProperties.get(Constants.BLE_PROP_sip_mport_end, Constants.BLE_DEFAULT_VALUE_NULL );
			String domain = SystemProperties.get(Constants.BLE_PROP_sip_domain, Constants.BLE_DEFAULT_VALUE_NULL );
			String acc_user = SystemProperties.get(Constants.BLE_PROP_sip_acc_user, Constants.BLE_DEFAULT_VALUE_NULL );
			String acc_pass = SystemProperties.get(Constants.BLE_PROP_sip_acc_pass, Constants.BLE_DEFAULT_VALUE_NULL );
			String pstn_pass = SystemProperties.get(Constants.BLE_PROP_sip_pstn_pass, Constants.BLE_DEFAULT_VALUE_NULL );
			
			jsonRsp.put("method", "GetSip");
			jsonRsp.put("result", 200);
			jsonRsp.put("reason", "OK");
			jsonRspSip.put("sport",sport);
			jsonRspSip.put("mport_start", mport_start);
			jsonRspSip.put("mport_end", mport_end);
			jsonRspSip.put("domain", domain);
			jsonRspSip.put("acc_user", acc_user);
			jsonRspSip.put("acc_pass", acc_pass);
			jsonRspSip.put("pstn_pass", pstn_pass);
			jsonRsp.put("Sip", jsonRspSip);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
