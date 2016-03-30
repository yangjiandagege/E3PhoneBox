package com.cubic.e3box;

public class Constants {
	
	public final static int MESSAGE_SIP_INCOMING_CALL = 1;
	public final static int MESSAGE_PSTN_INCOMING_CALL = 2;
	public final static int MESSAGE_SIP_DISCONNECT = 3;
	public final static int MESSAGE_SIP_CONFIRM = 4;
	public final static int MESSAGE_SIP_REGISTERED = 5;
	public final static int MESSAGE_SIP_UNREGISTERED = 6;
	public final static int MESSAGE_PSTN_HANGUP = 7;
	
	/*****************pjsip state******************/
	public final static int PJSIP_INV_STATE_NULL          = 0;
	public final static int PJSIP_INV_STATE_CALLING       = 1;
	public final static int PJSIP_INV_STATE_INCOMING      = 2;
	public final static int PJSIP_INV_STATE_EARLY         = 3;
	public final static int PJSIP_INV_STATE_CONNECTING    = 4;
	public final static int PJSIP_INV_STATE_CONFIRMED     = 5;
	public final static int PJSIP_INV_STATE_DISCONNECTED  = 6;
	
	public final static int EVT_IDLE = 0;
	/********key conditions : standby******************/
	public final static int EVT_PSTN_DIAL_MIDDLE = 1;
	public final static int EVT_PSTN_DIAL_RIGHT = 2;
	public final static int EVT_PSTN_DIAL_LEFT = 3;
	public final static int EVT_ENABLE_FM = 4;
	/********key conditions : incoming call / in a call************/
	public final static int EVT_ANSWER_PHONE = 5;
	public final static int EVT_HANG_UP_PHONE = 6;
	/********key conditions : in a call / listen in FM radio*******/
	public final static int EVT_VOLUME_CALL = 7;
	public final static int EVT_VOLUME_FM = 8;
	/********key conditions : listen in FM radio******************/
	public final static int EVT_DISABLE_FM = 9;
	public final static int EVT_PREVIOUS_CHANNEL = 10;
	public final static int EVT_NEXT_CHANNEL = 11;
	
	/********BLE state*******************/
    public static final int BLE_STATE_BEGIN = BoxStateMachine.STATE_BEGIN;
	public static final int BLE_STATE_IDLE = BLE_STATE_BEGIN + 1;
	public static final int BLE_STATE_RECEIVING_REQ = BLE_STATE_BEGIN + 2;
	public static final int BLE_STATE_PROCESSING_REQ = BLE_STATE_BEGIN + 3;
	public static final int BLE_STATE_SENDING_RSP = BLE_STATE_BEGIN + 4;
	
	/********BLE event*******************/
	public static final int BLE_EVT_BEGIN = 0;
	public static final int BLE_EVT_REQ_START = 1;
	public static final int BLE_EVT_REQ_RECEIVING = 2;
	public static final int BLE_EVT_REQ_END = 3;
	public static final int BLE_EVT_RSP_SENDING = 4;
	public static final int BLE_EVT_RSP_END = 5;
	public static final int BLE_EVT_RECEIVE_ERROR = 6;
	public static final int BLE_EVT_SEND_ERROR = 7;
	public static final int BLE_EVT_TIMEOUT = 8;
	
	/********BLE UUIDs*******************/
	public static final String SERVICE_UUID = "922D84D0-1AB3-4A6C-BD7A-FB00780A3DEE";
	public static final String REQUEST_DATA = "89DAFB6E-BB39-4AAD-A7A3-78CD1D050470";
	public static final String REQUEST_START = "424C164E-2589-4E81-867E-FF8EB4BC85F4";
	public static final String REQUEST_END = "5FC614F4-3429-4814-8A05-52B1A515AE5B";
	public static final String RESPONSE_DATA = "C3D2CDCF-FC9D-4E99-9D18-0CC98C61B557";
	public static final String RESPONSE_START = "6AC603F8-E0F5-4907-B5CE-61496BD819B4";
	public static final String RESPONSE_INCREMENT = "827F70E3-188D-4565-9930-B3EB98AB5506";
	
	public static final int BLE_SET_TIMEOUT = 5;
	
	/********ACTIONs*********************/
	public static final String SIP_ACCOUNT_CHANGE_ACTION = "android.intent.action.SIP_ACCOUNT_CHANGE";
	public static final String NET_CHANGE_ACTION = "android.net.conn.CONNECTIVITY_CHANGE";
	
	/********BLE Setting Properties***************/
	public static final String BLE_PROP_activate_api = "persist.forge.activate.api";
	public static final String BLE_PROP_activate_pass = "persist.forge.activate.pass";
	public static final String BLE_PROP_activate_group = "persist.forge.activate.group";
	public static final String BLE_PROP_islocalowner_client = "persist.forge.owner.client";
	//"SetSip" ; "GetSuo"
	public static final String BLE_PROP_sip_sport = "persist.forge.sip.sport";
	public static final String BLE_PROP_sip_mport_start = "persist.forge.sip.mport_start";
	public static final String BLE_PROP_sip_mport_end = "persist.forge.sip.mport_end";
	public static final String BLE_PROP_sip_domain = "persist.forge.sip.domain";
	public static final String BLE_PROP_sip_acc_user = "persist.forge.sip.acc_user";
	public static final String BLE_PROP_sip_acc_pass = "persist.forge.sip.acc_pass";
	public static final String BLE_PROP_sip_pstn_pass = "persist.forge.sip.pstn_pass";
	//"SetDialButtons" ; "GetDialButtons"
	public static final String BLE_PROP_buttons_btn1 = "persist.forge.buttons.btn1";
	public static final String BLE_PROP_buttons_btn2 = "persist.forge.buttons.btn2";
	public static final String BLE_PROP_buttons_btn3 = "persist.forge.buttons.btn3";
	//"SetVideo" ; "GetVideo"
	public static final String BLE_PROP_video_user = "persist.forge.video.user";
	public static final String BLE_PROP_video_pass = "persist.forge.video.pass";
	//"SetNetwork" ; "GetNetwork"
	public static final String BLE_PROP_wired_dhcp = "persist.forge.wired.dhcp";
	public static final String BLE_PROP_wired_ip = "persist.forge.wired.ip";
	public static final String BLE_PROP_wired_mask = "persist.forge.wired.mask";
	public static final String BLE_PROP_wired_gate = "persist.forge.wired.gate";
	public static final String BLE_PROP_wired_dns1 = "persist.forge.wired.dns1";
	public static final String BLE_PROP_wired_dns2 = "persist.forge.wired.dns2";
	public static final String BLE_PROP_wireless_ssid = "persist.forge.wireless.ssid";
	public static final String BLE_PROP_wireless_pass = "persist.forge.wireless.pass";
	public static final String BLE_PROP_wireless_dhcp = "persist.forge.wireless.dhcp";
	public static final String BLE_PROP_wireless_ip = "persist.forge.wireless.ip";
	public static final String BLE_PROP_wireless_mask = "persist.forge.wireless.mask";
	public static final String BLE_PROP_wireless_gate = "persist.forge.wireless.gate";
	public static final String BLE_PROP_wireless_dns1 = "persist.forge.wireless.dns1";
	public static final String BLE_PROP_wireless_dns2 = "persist.forge.wireless.dns2";
	//"SetAudio" ; "GetAudio"
	public static final String BLE_PROP_audio_mic_vol = "persist.forge.audio.mic_vol";
	public static final String BLE_PROP_audio_call_vol = "persist.forge.audio.call_vol";
	public static final String BLE_PROP_audio_ring_vol = "persist.forge.audio.ring_vol";
	public static final String BLE_PROP_audio_radio_vol = "persist.forge.audio.radio_vol";
	//"SetRadioList" ; "GetRadioList"
	public static final String BLE_PROP_radio_name_no = "persist.forge.radio.name.no";
	public static final String BLE_PROP_radio_value_no = "persist.forge.radio.value.no";
	//"SetGeoLocation" ; "GetGeoLocation"
	public static final String BLE_PROP_location_latitude = "persist.forge.locate.latitude";
	public static final String BLE_PROP_lacation_longitude = "persist.forge.locate.longitude";
	//serial id
	public static final String BLE_PROP_serial_id = "persist.forge.serial.id";
	public static final String BLE_DEFAULT_VALUE_NULL = "null";
	
}
