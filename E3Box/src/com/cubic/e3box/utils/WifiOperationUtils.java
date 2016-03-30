package com.cubic.e3box.utils;

import java.util.List;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.AuthAlgorithm;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiManager;
import android.text.TextUtils;
import android.util.Log;

public class WifiOperationUtils {
	
	private static final String TAG = WifiOperationUtils.class.getSimpleName();
	WifiManager wifiManager;
	
	public enum WifiCipherType {
		WIFICIPHER_WEP, WIFICIPHER_WPA, WIFICIPHER_NOPASS, WIFICIPHER_INVALID
	}
	
	public WifiOperationUtils(WifiManager wifiManager) {
		this.wifiManager = wifiManager;
		System.out.println("jiangning WifiAutoConnectManager");
	}
	
	public void connect(String ssid, String password, WifiCipherType type) {
		Thread thread = new Thread(new ConnectRunnable(ssid, password, type));
		thread.start();
	}
	
	private WifiConfiguration isExsits(String SSID) {
		List<WifiConfiguration> existingConfigs = wifiManager.getConfiguredNetworks();
		if(existingConfigs != null)
		{
			for (WifiConfiguration existingConfig : existingConfigs) {
				if (existingConfig.SSID.equals("\"" + SSID + "\"")) {
					return existingConfig;
				}
			}
		}
		return null;
	}
	
	private WifiConfiguration createWifiInfo(String SSID, String Password, WifiCipherType Type) {
		WifiConfiguration config = new WifiConfiguration();
		config.allowedAuthAlgorithms.clear();
		config.allowedGroupCiphers.clear();
		config.allowedKeyManagement.clear();
		config.allowedPairwiseCiphers.clear();
		config.allowedProtocols.clear();
		config.SSID = "\"" + SSID + "\"";
		// nopass
		if (Type == WifiCipherType.WIFICIPHER_NOPASS) {
			config.wepKeys[0] = "";
			config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
			config.wepTxKeyIndex = 0;
		}
		// wep
		if (Type == WifiCipherType.WIFICIPHER_WEP) {
			if (!TextUtils.isEmpty(Password)) {
				if (isHexWepKey(Password)) {
					config.wepKeys[0] = Password;
				} else {
					config.wepKeys[0] = "\"" + Password + "\"";
				}
			}
			config.allowedAuthAlgorithms.set(AuthAlgorithm.OPEN);
			config.allowedAuthAlgorithms.set(AuthAlgorithm.SHARED);
			config.allowedKeyManagement.set(KeyMgmt.NONE);
			config.wepTxKeyIndex = 0;
		}
		// wpa
		if (Type == WifiCipherType.WIFICIPHER_WPA) {
			config.preSharedKey = "\"" + Password + "\"";
			config.hiddenSSID = true;
			config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
			config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
			config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);

			// config.allowedProtocols.set(WifiConfiguration.Protocol.WPA); 
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
			config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
			config.status = WifiConfiguration.Status.ENABLED;
		}
		return config;
	}

	private boolean openWifi() {
		boolean bRet = true;
		System.out.println("jiangning openWifi");
		if (!wifiManager.isWifiEnabled()) {
		bRet = wifiManager.setWifiEnabled(true);
		}
		return bRet;
	}
	
	class ConnectRunnable implements Runnable {
		private String ssid;
		private String password;
		private WifiCipherType type;
		
		public ConnectRunnable(String ssid, String password, WifiCipherType type) {
			this.ssid = ssid;
			this.password = password;
			this.type = type;
		}
	
		@Override
		public void run() {
			openWifi();
			
			while (wifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLING) {
				try {
					Thread.sleep(500);
				} catch (InterruptedException ie) {
				}
			}
			
			WifiConfiguration wifiConfig = createWifiInfo(ssid, password, type);
	
			if (wifiConfig == null) {
				Log.d(TAG, "wifiConfig is null!");
				return;
			}
			WifiConfiguration tempConfig = isExsits(ssid);
			if (tempConfig != null) {
				wifiManager.removeNetwork(tempConfig.networkId);
			}
			int netID = wifiManager.addNetwork(wifiConfig);
			boolean enabled = wifiManager.enableNetwork(netID, true);
			Log.d(TAG, "jiangning enableNetwork status enable=" + enabled);
			boolean connected = wifiManager.reconnect();
			Log.d(TAG, "jiangning enableNetwork connected=" + connected);
		}
	}
	
	private static boolean isHexWepKey(String wepKey) {
		final int len = wepKey.length();
		// WEP-40, WEP-104, and some vendors using 256-bit WEP (WEP-232?)
		if (len != 10 && len != 26 && len != 58) {
		return false;
		}
		return isHex(wepKey);
	}
	
	private static boolean isHex(String key) {
		for (int i = key.length() - 1; i >= 0; i--) {
			final char c = key.charAt(i);
			if (!(c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f')) {
				return false;
			}
		}
		return true;
	}
}