package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.util.Log;

import com.cubic.e3box.utils.WifiOperationUtils;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.SystemProperties;

public class SetNetwork  extends JsonOperation{
	String TAG = "BleStation";
	WifiOperationUtils wifiOperationUtils;

	public SetNetwork(Context context)
	{
		WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);  
		wifiOperationUtils = new WifiOperationUtils(wifiManager);
	}
	
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetNetwork");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonNetworkWireless = null;
	    JSONObject jsonNetworkWired = null;
	    JSONObject jsonNetwork = null;
	    try {
			jsonNetwork = jsonReq.getJSONObject("Network");
			try {
				 jsonNetworkWired = jsonNetwork.getJSONObject("Wired");
			}catch (JSONException e) {
				jsonNetworkWired = null;
				e.printStackTrace();
			}
			if(jsonNetworkWired != null){
				String strWiredDhcp = jsonNetworkWired.getString("dhcp");
				String strWiredIp = jsonNetworkWired.getString("ip");
				String strWiredMask = jsonNetworkWired.getString("mask");
				String strWiredGate = jsonNetworkWired.getString("gate");
				String strWiredDns1 = jsonNetworkWired.getString("dns1");
				String strWiredDns2 = jsonNetworkWired.getString("dns2");
				SystemProperties.set(Constants.BLE_PROP_wired_dhcp, strWiredDhcp);
				SystemProperties.set(Constants.BLE_PROP_wired_ip, strWiredIp);
				SystemProperties.set(Constants.BLE_PROP_wired_mask, strWiredMask);
				SystemProperties.set(Constants.BLE_PROP_wired_gate, strWiredGate);
				SystemProperties.set(Constants.BLE_PROP_wired_dns1, strWiredDns1);
				SystemProperties.set(Constants.BLE_PROP_wired_dns2, strWiredDns2);
			}
			
			try {
				jsonNetworkWireless = jsonNetwork.getJSONObject("Wireless");
			}catch (JSONException e) {
				jsonNetworkWireless = null;
				e.printStackTrace();
			}
			if(jsonNetworkWireless != null){
				String strWirelessSsid = jsonNetworkWireless.getString("ssid");
				String strWirelessPass = jsonNetworkWireless.getString("pass");
				String strWirelessDhcp = jsonNetworkWireless.getString("dhcp");
				String strWirelessIp = jsonNetworkWireless.getString("ip");
				String strWirelessMask = jsonNetworkWireless.getString("mask");
				String strWirelessGate = jsonNetworkWireless.getString("gate");
				String strWirelessDns1 = jsonNetworkWireless.getString("dns1");
				String strWirelessDns2 = jsonNetworkWireless.getString("dns2");
				SystemProperties.set(Constants.BLE_PROP_wireless_ssid, strWirelessSsid);
				SystemProperties.set(Constants.BLE_PROP_wireless_pass, strWirelessPass);
				SystemProperties.set(Constants.BLE_PROP_wireless_dhcp, strWirelessDhcp);
				SystemProperties.set(Constants.BLE_PROP_wireless_ip, strWirelessIp);
				SystemProperties.set(Constants.BLE_PROP_wireless_mask, strWirelessMask);
				SystemProperties.set(Constants.BLE_PROP_wireless_gate, strWirelessGate);
				SystemProperties.set(Constants.BLE_PROP_wireless_dns1, strWirelessDns1);
				SystemProperties.set(Constants.BLE_PROP_wireless_dns2, strWirelessDns2);
				wifiOperationUtils.connect(strWirelessSsid, strWirelessPass, WifiOperationUtils.WifiCipherType.WIFICIPHER_WPA);
			}
			jsonRsp.put("method", "SetNetwork");
			jsonRsp.put("result", 200);  // 486 , 488
			jsonRsp.put("reason", "OK"); // "Call in progress" , "Malformed dns1"
		} catch (JSONException e) {
			e.printStackTrace();
		}	

		return jsonRsp;
	}

}
