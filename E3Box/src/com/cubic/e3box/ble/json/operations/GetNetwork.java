package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class GetNetwork  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetNetwork");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspNetwork = new JSONObject(); 
	    JSONObject jsonRspNetworkWired = new JSONObject(); 
	    JSONObject jsonRspNetworkWireless = new JSONObject(); 

		String strWiredDhcp = SystemProperties.get(Constants.BLE_PROP_wired_dhcp, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWiredDns1 = SystemProperties.get(Constants.BLE_PROP_wired_dns1, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWiredDns2 = SystemProperties.get(Constants.BLE_PROP_wired_dns2, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWiredGate = SystemProperties.get(Constants.BLE_PROP_wired_gate, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWiredIp = SystemProperties.get(Constants.BLE_PROP_wired_ip, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWiredMask = SystemProperties.get(Constants.BLE_PROP_wired_mask, Constants.BLE_DEFAULT_VALUE_NULL );
		  
		String strWirelessSsid = SystemProperties.get(Constants.BLE_PROP_wireless_ssid, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessPass = SystemProperties.get(Constants.BLE_PROP_wireless_pass, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessDhcp = SystemProperties.get(Constants.BLE_PROP_wireless_dhcp, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessIp = SystemProperties.get(Constants.BLE_PROP_wireless_ip, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessMask = SystemProperties.get(Constants.BLE_PROP_wireless_mask, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessGate = SystemProperties.get(Constants.BLE_PROP_wireless_gate, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessDns1 = SystemProperties.get(Constants.BLE_PROP_wireless_dns1, Constants.BLE_DEFAULT_VALUE_NULL );
		String strWirelessDns2 = SystemProperties.get(Constants.BLE_PROP_wireless_dns2, Constants.BLE_DEFAULT_VALUE_NULL );
	    
	    try {
	    	jsonRsp.put("method", "GetNetwork");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspNetworkWired.put("dhcp", strWiredDhcp);
	    	jsonRspNetworkWired.put("ip", strWiredIp);
	    	jsonRspNetworkWired.put("mask", strWiredMask);
	    	jsonRspNetworkWired.put("gate", strWiredGate);
	    	jsonRspNetworkWired.put("dns1", strWiredDns1);
	    	jsonRspNetworkWired.put("dns2", strWiredDns2);
			
	    	jsonRspNetworkWireless.put("ssid", strWirelessSsid);
	    	jsonRspNetworkWireless.put("pass", strWirelessPass);
	    	jsonRspNetworkWireless.put("dhcp", strWirelessDhcp);
	    	jsonRspNetworkWireless.put("ip", strWirelessIp);
	    	jsonRspNetworkWireless.put("mask", strWirelessMask);
	    	jsonRspNetworkWireless.put("gate", strWirelessGate);
	    	jsonRspNetworkWireless.put("dns1", strWirelessDns1);
	    	jsonRspNetworkWireless.put("dns2", strWirelessDns2);
			jsonRspNetwork.put("Wired", jsonRspNetworkWired);
			jsonRspNetwork.put("Wireless", jsonRspNetworkWireless);
			jsonRsp.put("Network", jsonRspNetwork);
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}

}
