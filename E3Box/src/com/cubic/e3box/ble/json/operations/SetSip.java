package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.CarrierApp;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;
import android.content.Intent;

public class SetSip  extends JsonOperation{
	String TAG = "BleStation";
	
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetSip");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
			JSONObject setSip = jsonReq.getJSONObject("Sip");
			String sport = setSip.getString("sport");
			String mport_start = setSip.getString("mport_start");
			String mport_end = setSip.getString("mport_end");
			String domain = setSip.getString("domain");
			String acc_user = setSip.getString("acc_user");
			String acc_pass = setSip.getString("acc_pass");
			String pstn_pass = setSip.getString("pstn_pass");

			SystemProperties.set(Constants.BLE_PROP_sip_sport, sport);
			SystemProperties.set(Constants.BLE_PROP_sip_mport_start, mport_start);
			SystemProperties.set(Constants.BLE_PROP_sip_mport_end, mport_end);
			SystemProperties.set(Constants.BLE_PROP_sip_domain, domain);
			SystemProperties.set(Constants.BLE_PROP_sip_acc_user, acc_user);
			SystemProperties.set(Constants.BLE_PROP_sip_acc_pass, acc_pass);
			SystemProperties.set(Constants.BLE_PROP_sip_pstn_pass, pstn_pass);

			if(CarrierApp.getMainActivity().getCallState() == 0){
				Intent intent = new Intent();
				intent.setAction(Constants.SIP_ACCOUNT_CHANGE_ACTION);
				CarrierApp.getMainActivity().sendOrderedBroadcast(intent,null);
				
				jsonRsp.put("method", "SetSip");
				jsonRsp.put("result", 200); 
				jsonRsp.put("reason", "OK");
			}else{
				jsonRsp.put("method", "SetSip");
				jsonRsp.put("result", 486);
				jsonRsp.put("reason", "Restart in progress");
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}

}
