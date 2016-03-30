package com.cubic.e3box.ble;

import com.cubic.e3box.ble.json.operations.Activate;
import com.cubic.e3box.ble.json.operations.Blink;
import com.cubic.e3box.ble.json.operations.EnableRadio;
import com.cubic.e3box.ble.json.operations.FinishLocalOwnerTransfer;
import com.cubic.e3box.ble.json.operations.FmSearch;
import com.cubic.e3box.ble.json.operations.GetAudio;
import com.cubic.e3box.ble.json.operations.GetDeviceStatus;
import com.cubic.e3box.ble.json.operations.GetDialButtons;
import com.cubic.e3box.ble.json.operations.GetKey;
import com.cubic.e3box.ble.json.operations.GetLog;
import com.cubic.e3box.ble.json.operations.GetNetwork;
import com.cubic.e3box.ble.json.operations.GetRadioList;
import com.cubic.e3box.ble.json.operations.GetRadioStatus;
import com.cubic.e3box.ble.json.operations.GetSip;
import com.cubic.e3box.ble.json.operations.GetVideo;
import com.cubic.e3box.ble.json.operations.SetSip;
import com.cubic.e3box.ble.json.operations.IsLocalOwner;
import com.cubic.e3box.ble.json.operations.PrepareLocalOwnerTransfer;
import com.cubic.e3box.ble.json.operations.SetAudio;
import com.cubic.e3box.ble.json.operations.SetCustomChannel;
import com.cubic.e3box.ble.json.operations.SetDialButtons;
import com.cubic.e3box.ble.json.operations.SetGeoLocation;
import com.cubic.e3box.ble.json.operations.SetNetwork;
import com.cubic.e3box.ble.json.operations.SetPresetChannel;
import com.cubic.e3box.ble.json.operations.SetRadioList;
import com.cubic.e3box.ble.json.operations.SetVideo;
import com.cubic.e3box.ble.json.operations.UpgradeFirmware;
import com.cubic.e3box.ble.json.operations.IsActivated;
import com.cubic.e3box.ble.json.operations.IsParsingJsonError;
import com.cubic.e3box.ble.json.operations.IsRequestTooLong;
import com.cubic.e3box.ble.json.operations.IsUnknowMethod;
import android.content.Context;

public class JsonOperationFactory {

	public static JsonOperation createJsonOperation(String method,Context context){
		JsonOperation jsonOper = null;
		
		if(method.equals("Activate")){
			jsonOper = new Activate(); 
		}else if(method.equals("Blink")){
			jsonOper = new Blink(); 
		}else if(method.equals("EnableRadio")){
			jsonOper = new EnableRadio(); 
		}else if(method.equals("FinishLocalOwnerTransfer")){
			jsonOper = new FinishLocalOwnerTransfer(); 
		}else if(method.equals("FmSearch")){
			jsonOper = new FmSearch(); 
		}else if(method.equals("GetAudio")){
			jsonOper = new GetAudio(); 
		}else if(method.equals("GetDeviceStatus")){
			jsonOper = new GetDeviceStatus(); 
		}else if(method.equals("GetDialButtons")){
			jsonOper = new GetDialButtons(); 
		}else if(method.equals("GetKey")){
			jsonOper = new GetKey(context); 
		}else if(method.equals("GetLog")){
			jsonOper = new GetLog(); 
		}else if(method.equals("GetNetwork")){
			jsonOper = new GetNetwork(); 
		}else if(method.equals("GetRadioList")){
			jsonOper = new GetRadioList(); 
		}else if(method.equals("GetRadioStatus")){
			jsonOper = new GetRadioStatus(); 
		}else if(method.equals("GetSip")){
			jsonOper = new GetSip(); 
		}else if(method.equals("GetVideo")){
			jsonOper = new GetVideo(); 
		}else if(method.equals("IsActivated")){
			jsonOper = new IsActivated(); 
		}else if(method.equals("IsLocalOwner")){
			jsonOper = new IsLocalOwner(); 
		}else if(method.equals("PrepareLocalOwnerTransfer")){
			jsonOper = new PrepareLocalOwnerTransfer(); 
		}else if(method.equals("SetAudio")){
			jsonOper = new SetAudio(); 
		}else if(method.equals("SetCustomChannel")){
			jsonOper = new SetCustomChannel(); 
		}else if(method.equals("SetDialButtons")){
			jsonOper = new SetDialButtons(); 
		}else if(method.equals("SetGeoLocation")){
			jsonOper = new SetGeoLocation();
		}else if(method.equals("SetNetwork")){
			jsonOper = new SetNetwork(context); 
		}else if(method.equals("SetPresetChannel")){
			jsonOper = new SetPresetChannel(); 
		}else if(method.equals("SetRadioList")){
			jsonOper = new SetRadioList(); 
		}else if(method.equals("SetSip")){
			jsonOper = new SetSip(); 
		}else if(method.equals("SetVideo")){
			jsonOper = new SetVideo(); 
		}else if(method.equals("UpgradeFirmware")){
			jsonOper = new UpgradeFirmware(); 
		}else if(method.equals("IsParsingJsonError")){
			jsonOper = new IsParsingJsonError();
		}else if(method.equals("IsRequestTooLong")){
			jsonOper = new IsRequestTooLong();
		}else{
			jsonOper = new IsUnknowMethod();
		}
		
		return jsonOper;
	}
}
