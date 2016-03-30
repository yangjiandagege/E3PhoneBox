package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import com.cubic.e3box.utils.CharUtils;

import android.os.SystemProperties;
import android.util.Log;

public class SetAudio  extends JsonOperation{
	String TAG = "BleStation";
	
	private Boolean isCorrectVol(String strVol){
		if(CharUtils.isNumeric(strVol)){
			int iVol = Integer.valueOf(strVol).intValue();
			if(iVol <= 100 || iVol >= 0){
				return true;
			}
			return false;
		}else{
			return false;
		}
	}
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation SetAudio");
	    JSONObject jsonRsp = new JSONObject(); 
	    
	    try {
	    	JSONObject jsonReqAudio = jsonReq.getJSONObject("Audio");
	    	JSONObject jsonReqAudioMic = jsonReqAudio.getJSONObject("Mic");
	    	JSONObject jsonReqAudioCall = jsonReqAudio.getJSONObject("Call");
	    	JSONObject jsonReqAudioRing = jsonReqAudio.getJSONObject("Ring");
	    	JSONObject jsonReqAudioRadio = jsonReqAudio.getJSONObject("Radio");
	    	String strAudioMicVol = jsonReqAudioMic.getString("vol");
	    	String strAudioCallVol = jsonReqAudioCall.getString("vol");
	    	String strAudioRingVol = jsonReqAudioRing.getString("vol");
	    	String strAudioRadioVol = jsonReqAudioRadio.getString("vol");
	    	if(isCorrectVol(strAudioMicVol) && 
	    			isCorrectVol(strAudioCallVol) &&
	    			isCorrectVol(strAudioRingVol) && 
	    			isCorrectVol(strAudioRadioVol)){
		    	SystemProperties.set(Constants.BLE_PROP_audio_mic_vol, strAudioMicVol);
		    	SystemProperties.set(Constants.BLE_PROP_audio_call_vol, strAudioCallVol);
		    	SystemProperties.set(Constants.BLE_PROP_audio_ring_vol, strAudioRingVol);
		    	SystemProperties.set(Constants.BLE_PROP_audio_radio_vol, strAudioRadioVol);
		    	
		    	jsonRsp.put("method", "SetAudio");
		    	jsonRsp.put("result", 200);
		    	jsonRsp.put("reason", "OK"); 
	    	}else{
	    		String strErrorReason = "Invalid";
	    		if(isCorrectVol(strAudioMicVol) == false){
	    			strErrorReason = strErrorReason + " mic";
	    		}
	    		if(isCorrectVol(strAudioCallVol) == false){
	    			strErrorReason = strErrorReason + " call";
	    		}
	    		if(isCorrectVol(strAudioRingVol) == false){
	    			strErrorReason = strErrorReason + " ring";
	    		}
	    		if(isCorrectVol(strAudioRadioVol) == false){
	    			strErrorReason = strErrorReason + " radio";
	    		}
	    		strErrorReason = strErrorReason + " volume (5000)";
	    		
	    		jsonRsp.put("method", "SetAudio");
	    		jsonRsp.put("result", 488);
	    		jsonRsp.put("reason", strErrorReason);
	    	}
		} catch (JSONException e) {
			e.printStackTrace();
		}	
		return jsonRsp;
	}
}
