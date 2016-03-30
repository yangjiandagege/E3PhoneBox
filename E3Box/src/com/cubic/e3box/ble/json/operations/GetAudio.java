package com.cubic.e3box.ble.json.operations;

import org.json.JSONException;
import org.json.JSONObject;
import com.cubic.e3box.Constants;
import com.cubic.e3box.ble.JsonOperation;
import android.os.SystemProperties;
import android.util.Log;

public class GetAudio  extends JsonOperation{
	String TAG = "BleStation";
	public JSONObject jsonOperation(JSONObject jsonReq){
		Log.e(TAG, " jsonOperation GetAudio");
	    JSONObject jsonRsp = new JSONObject(); 
	    JSONObject jsonRspAudio = new JSONObject(); 
	    JSONObject jsonRspAudioMic = new JSONObject(); 
	    JSONObject jsonRspAudioCall = new JSONObject(); 
	    JSONObject jsonRspAudioRing = new JSONObject(); 
	    JSONObject jsonRspAudioRadio = new JSONObject(); 
	    try {
	    	String strCallsVol = SystemProperties.get(Constants.BLE_PROP_audio_call_vol, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strMicVol = SystemProperties.get(Constants.BLE_PROP_audio_mic_vol, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strRadioVol = SystemProperties.get(Constants.BLE_PROP_audio_radio_vol, Constants.BLE_DEFAULT_VALUE_NULL);
	    	String strRingVol = SystemProperties.get(Constants.BLE_PROP_audio_ring_vol, Constants.BLE_DEFAULT_VALUE_NULL);
	    	jsonRsp.put("method", "GetAudio");
	    	jsonRsp.put("result", 200);
	    	jsonRsp.put("reason", "OK");
	    	jsonRspAudioMic.put("vol", strMicVol);
	    	jsonRspAudioCall.put("vol", strCallsVol);
	    	jsonRspAudioRing.put("vol", strRingVol);
	    	jsonRspAudioRadio.put("vol", strRadioVol);
			jsonRspAudio.put("Mic", jsonRspAudioMic);
			jsonRspAudio.put("Call", jsonRspAudioCall);
			jsonRspAudio.put("Ring", jsonRspAudioRing);
			jsonRspAudio.put("Radio", jsonRspAudioRadio);
			jsonRsp.put("Audio", jsonRspAudio);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return jsonRsp;
	}

}
