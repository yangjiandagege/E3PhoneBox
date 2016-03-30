package com.cubic.e3box.ble;

import java.util.HashMap;
import org.json.JSONException;
import org.json.JSONObject;
import android.os.SystemProperties;
import com.cubic.e3box.Constants;

public class AuthenticationFinery extends JsonOperation{
	private JsonOperation m_component = null;
	private static HashMap<String, Integer> hmForOperaOfLocalOwner = new HashMap<String, Integer>();
	private static final Integer INT_LOCAL_ADMIN  = 1;
	private static final Integer INT_LOCAL_OWNER  = 2;
	private static final Integer INT_LOCAL_MEMBER = 3;
	
	public AuthenticationFinery(){
		hmForOperaOfLocalOwner.put("GetKey", INT_LOCAL_ADMIN);
		hmForOperaOfLocalOwner.put("Activate", INT_LOCAL_ADMIN);
		
		hmForOperaOfLocalOwner.put("SetSip", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("SetDialButtons", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("SetVideo", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GetSip", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GetDialButtons", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GetVideo", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GetDeviceStatus", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GetLog", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("PrepareLocalOwnerTransfer", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("GroupOwnerTransfer", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("FinishLocalOwnerTransfer", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("IsLocalOwner", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("NotifyNewFirmware", INT_LOCAL_OWNER);
		hmForOperaOfLocalOwner.put("UpgradeFirmware", INT_LOCAL_OWNER);
		
		hmForOperaOfLocalOwner.put("SetNetwork", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("SetAudio", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("SetRadioList", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("SetGeoLocation", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("IsActivated", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("GetNetwork", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("GetAudio", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("GetRadioList", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("GetRadioStatus", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("Blink", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("SetPresetChannel", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("SetCustomChannel", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("FmSearch", INT_LOCAL_MEMBER);
		hmForOperaOfLocalOwner.put("EnableRadio", INT_LOCAL_MEMBER);
	}
	
	public void Decorate(JsonOperation component)
	{
		m_component = component;
	}
	
	public JSONObject jsonOperation(JSONObject jsonObject){
	    JSONObject jsonRsp = new JSONObject(); 
	    try {
			String strClientIdFromE3phone = jsonObject.getString("client");
			String strMethod = jsonObject.getString("method");
			String strClientIdInE3box = SystemProperties.get(Constants.BLE_PROP_islocalowner_client, Constants.BLE_DEFAULT_VALUE_NULL);	
			Integer iUserType = 0;
			Boolean bConfirmed = false;
			if(strClientIdInE3box.equals(Constants.BLE_DEFAULT_VALUE_NULL)){
				iUserType = INT_LOCAL_ADMIN;
			}else if(strClientIdFromE3phone.equals(strClientIdInE3box)){
				iUserType = INT_LOCAL_OWNER;
			}else{
				iUserType = INT_LOCAL_MEMBER;
			}
			if(iUserType == INT_LOCAL_ADMIN){
				if(hmForOperaOfLocalOwner.get(strMethod) == INT_LOCAL_ADMIN || 
						strMethod.equals("GetDeviceStatus")){
					bConfirmed = true;
				}
			}else if(iUserType == INT_LOCAL_OWNER){
				if(hmForOperaOfLocalOwner.get(strMethod) == INT_LOCAL_OWNER ||
						hmForOperaOfLocalOwner.get(strMethod) == INT_LOCAL_MEMBER){
					bConfirmed = true;
				}
			}else if(iUserType == INT_LOCAL_MEMBER){
				if(hmForOperaOfLocalOwner.get(strMethod) == INT_LOCAL_MEMBER){
					bConfirmed = true;
				}
			}
			JSONObject jsonRspTmp = new JSONObject(); 
			jsonRspTmp.put("method", jsonObject.getString("method"));
			jsonRspTmp.put("result", 403);
			jsonRspTmp.put("reason", "Forbidden");
			jsonRsp = (bConfirmed == true)?m_component.jsonOperation(jsonObject):jsonRspTmp;
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}
}
