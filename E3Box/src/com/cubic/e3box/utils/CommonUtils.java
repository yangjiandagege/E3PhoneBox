package com.cubic.e3box.utils;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import com.cubic.e3box.Constants;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.Context;
import android.os.SystemProperties;
import android.util.Base64;
import android.util.Log;

public class CommonUtils {
	public static final int PASSWORD_TYPE_PSTN = 0;
	public static final int PASSWORD_TYPE_API  = 1;
	
	public static boolean isServiceWorked(Context context,String className) {
        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);  
        ArrayList<RunningServiceInfo> runningService = (ArrayList<RunningServiceInfo>) activityManager.getRunningServices(30);  
        for (int i = 0; i < runningService.size(); i++) {  
            if (runningService.get(i).service.getClassName().toString().equals(className)) {
                return true;  
            }
        }
        return false;
    }
	
	public static String getTimestamp(){
		DateFormat date = new SimpleDateFormat("yyyy-MM-dd");
		DateFormat time = new SimpleDateFormat("HH:mm:ss");
		String strTimestamp = date.format(new Date())+"T"+time.format(new Date())+"Z";
		return strTimestamp;
	}

	public static String getDigestToString(String strSrc){
		String strDigest = null;
		try {
			MessageDigest messageDigest;
			messageDigest = MessageDigest.getInstance("SHA-256");
			messageDigest.update(strSrc.getBytes("UTF8"));
			byte[] bDigest = messageDigest.digest();
			strDigest = CharUtils.bytesToHexString(bDigest);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
		return strDigest;
	}
	
	public static String getDigestAndEncodeBase64(String strSrc){
		String strDigestRsp = null;
		try {
			MessageDigest messageDigest;
			messageDigest = MessageDigest.getInstance("SHA-256");
			messageDigest.update(strSrc.getBytes("UTF8"));
			byte[] bDigest = messageDigest.digest();
			String strDigest = Base64.encodeToString(bDigest, Base64.DEFAULT);
			StringBuilder strBuilderTmp = new StringBuilder(strDigest);
			strBuilderTmp.deleteCharAt(strBuilderTmp.length()-1);
			strDigestRsp = strBuilderTmp.toString();
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
		
		return strDigestRsp;
	}
	
	public static Boolean checkPasswordDigest(String strXwsse,int iPasswordType){
		String strPass = null;
		String strPasswordDigest = strXwsse.substring(strXwsse.indexOf("PasswordDigest=\"")+"PasswordDigest=\"".length(), strXwsse.indexOf("\",Nonce"));
		String strNonce = strXwsse.substring(strXwsse.indexOf("Nonce=\"")+"Nonce=\"".length(), strXwsse.indexOf("\",Created"));
		String strCreated = strXwsse.substring(strXwsse.indexOf("Created=\"")+"Created=\"".length(), strXwsse.lastIndexOf("\""));

		switch(iPasswordType){
		case PASSWORD_TYPE_PSTN:
			strPass = SystemProperties.get(Constants.BLE_PROP_sip_pstn_pass,Constants.BLE_DEFAULT_VALUE_NULL);
			break;
		case PASSWORD_TYPE_API:
			strPass = SystemProperties.get(Constants.BLE_PROP_activate_pass,Constants.BLE_DEFAULT_VALUE_NULL);
			break;
		default:
			break;
		}
		String strPasswordDigestTmp = CommonUtils.getDigestAndEncodeBase64(strNonce+strCreated+strPass);
		return strPasswordDigest.equals(strPasswordDigestTmp);
	}
}
