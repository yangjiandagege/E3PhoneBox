package com.cubic.e3box;


import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Timer;
import java.util.TimerTask;
import com.cubic.e3box.AudioControl;
import com.cubic.e3box.ble.BleService;
import com.cubic.e3box.pstn.PstnService;
import com.cubic.e3box.sip.SipService;
import com.cubic.e3box.utils.CommonUtils;
import com.cubic.e3box.BoxStateMachine;
import com.cubic.e3box.AudioControl.VolumeLevel;
import com.cubic.e3box.AudioControl.AudioType;
import android.app.Activity;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;

public class MainActivity extends Activity {
	public static final String TAG="MainActivity";
	private static void logout(String log) {
		Log.d(TAG, TAG + " " + log );
	}
	private static void printStackTrace() {
        StackTraceElement[] stackElements = new Throwable().getStackTrace();
        if(stackElements != null)
        {
            for(int i = 0; i < stackElements.length; i++)
            {
                logout(" >"+ stackElements[i]); 
            }
        }
    }
	
	public static final int RING_TIMEOUT = 15000; // 15s

	// ********************** for status machine ****************************
	BoxStateMachine stateMachine = new BoxStateMachine();

	// *********** status **************
	private static final int STAT_IDLE                    = BoxStateMachine.STATE_BEGIN + 0;
	private static final int STAT_RING_BY_PSTN            = BoxStateMachine.STATE_BEGIN + 1;
	private static final int STAT_RING_BY_SIP             = BoxStateMachine.STATE_BEGIN + 2;
	private static final int STAT_CALL_WITH_PSTN          = BoxStateMachine.STATE_BEGIN + 3;
	private static final int STAT_CALL_WITH_SIP           = BoxStateMachine.STATE_BEGIN + 4;
	private static final int STAT_CALL_TRANS_PSTN_AND_SIP = BoxStateMachine.STATE_BEGIN + 5;

	// *********** event **************
	private static final int EVT_TIMEOUT = 1;
	private static final int EVT_MAKE_PSTN_CALL = 2;
	private static final int EVT_INCOMMING_CALL_FROM_PSTN = 3;
	private static final int EVT_INCOMMING_CALL_FROM_SIP_TO_BOX = 4;
	private static final int EVT_INCOMMING_CALL_FROM_SIP_TO_PSTN = 5;
	private static final int EVT_HANGUP = 6;
	private static final int EVT_SIP_HANGUP = 7;
	private static final int EVT_ACCEPT = 8;
	private static final int EVT_SIP_ACCEPT = 9;
	

	// *********** action **************
	private IAction actDialPstn = new IAction(){
		@Override public boolean performAction(byte[] data) {
			cancelTimeout();
			audioControl.stopRing();
			audioControl.startCall(localSoundRecord);
			String number = new String(data);
			logout("actDialPstn.performAction number=" + number);
			return mPstnService.dial(number);
		}
	};

	private IAction actRingAll = new IAction(){
		@Override public boolean performAction(byte[] data) {
			mSipService.dial(new String(data)); // call all sip client
			audioControl.startRing();
			setupTimeout(RING_TIMEOUT);
			return true;
		}
	};

	private IAction actRing = new IAction(){
		@Override public boolean performAction(byte[] data) {
			audioControl.startRing();
			setupTimeout(RING_TIMEOUT);
			return true;
		}
	};

	private IAction actHangupPstn = new IAction(){
		@Override public boolean performAction(byte[] data) {
			cancelTimeout();
			mPstnService.hangup();
			audioControl.stopRing();
			audioControl.stopCall();
			return true;
		}
	};

	private IAction actAcceptPstn = new IAction(){
		@Override public boolean performAction(byte[] data) {
			cancelTimeout();
			mPstnService.accept();
			audioControl.stopRing();
			audioControl.startCall(localSoundRecord);
			return true;
		}
	};

	private IAction actHangupSip = new IAction(){
		@Override public boolean performAction(byte[] data) {
			cancelTimeout();
			mSipService.hangup(); // hangup to sip client
			audioControl.stopRing();
			audioControl.stopCall();
			return true;
		}
	};

	private IAction actAcceptSip = new IAction(){
		@Override public boolean performAction(byte[] data) {
			cancelTimeout();
			mSipService.accept(); // accept to sip client
			audioControl.stopRing();
			audioControl.startCall(localSoundRecord);
			return true;
		}
	};

	private void setupStatusMachine() {
		logout(".setupStatusMachine");

		// idle
		stateMachine.insertAction(STAT_IDLE,                    STAT_CALL_WITH_PSTN,          EVT_MAKE_PSTN_CALL,                    actDialPstn);
		stateMachine.insertAction(STAT_IDLE,                    STAT_RING_BY_PSTN,            EVT_INCOMMING_CALL_FROM_PSTN,          actRingAll);
		stateMachine.insertAction(STAT_IDLE,                    STAT_RING_BY_SIP,             EVT_INCOMMING_CALL_FROM_SIP_TO_BOX,    actRing);
		stateMachine.insertAction(STAT_IDLE,                    STAT_CALL_TRANS_PSTN_AND_SIP, EVT_INCOMMING_CALL_FROM_SIP_TO_PSTN,   actDialPstn);

		// ring by pstn
		stateMachine.insertAction(STAT_RING_BY_PSTN,            STAT_IDLE,                    EVT_TIMEOUT,                           actHangupPstn);
		stateMachine.insertAction(STAT_RING_BY_PSTN,            STAT_IDLE,                    EVT_HANGUP,                            actHangupPstn); // this action may need discuss
		stateMachine.insertAction(STAT_RING_BY_PSTN,            STAT_CALL_WITH_PSTN,          EVT_ACCEPT,                            actAcceptPstn);
		stateMachine.insertAction(STAT_RING_BY_PSTN,            STAT_CALL_TRANS_PSTN_AND_SIP, EVT_SIP_ACCEPT,                        actAcceptPstn);
		stateMachine.insertAction(STAT_RING_BY_PSTN,            STAT_RING_BY_PSTN,            EVT_INCOMMING_CALL_FROM_PSTN,          actRingAll);

		// ring by sip
		stateMachine.insertAction(STAT_RING_BY_SIP,             STAT_IDLE,                    EVT_TIMEOUT,                           actHangupSip);
		stateMachine.insertAction(STAT_RING_BY_SIP,             STAT_IDLE,                    EVT_HANGUP,                            actHangupSip);
		stateMachine.insertAction(STAT_RING_BY_SIP,             STAT_CALL_WITH_SIP,           EVT_ACCEPT,                            actAcceptSip);

		// call trans between pstn and sip
		stateMachine.insertAction(STAT_CALL_TRANS_PSTN_AND_SIP, STAT_IDLE,                    EVT_SIP_HANGUP,                        actHangupPstn);

		// call with pstn
		stateMachine.insertAction(STAT_CALL_WITH_PSTN,          STAT_IDLE,                    EVT_HANGUP,                            actHangupPstn);

		// call with sip
		stateMachine.insertAction(STAT_CALL_WITH_SIP,           STAT_IDLE,                    EVT_HANGUP,                            actHangupSip);
		stateMachine.insertAction(STAT_CALL_WITH_SIP,           STAT_IDLE,                    EVT_SIP_HANGUP,                        null);
	};
	
	public int getCallState(){
		return stateMachine.getState();
	}
	
	// ******************************* for timer ****************************************
	private Handler mHandler = new Handler();
	private Runnable onTimeout = new Runnable() {
		@Override public void run() {
			stateMachine.inputEvent(EVT_TIMEOUT, null);
		}
	};

	private void setupTimeout( int ms ) {
		mHandler.removeCallbacks(onTimeout);
		mHandler.postDelayed(onTimeout, ms);
	};

	private void cancelTimeout() {
		mHandler.removeCallbacks(onTimeout);
	};

	// ******************************* for caller service *******************************
	PstnService mPstnService = new PstnService();
	SipService mSipService = new SipService(); // setup sip service instance

	private ICallCenter mPstnCb = new ICallCenter (){
		public boolean onIncommingCall(String target_number) {
			logout(".mPstnCb.onIncommingCall target_number=" + target_number);
			stateMachine.inputEvent(EVT_INCOMMING_CALL_FROM_PSTN, target_number.getBytes());
			return true;
		};

		public void onAccept()
		{
			// do nothing
		};

		public void onDtmf(String number) {
			// do nothing
		};

		public void onHangup() {
			// do nothing
		};

		public void onSoundData(byte[] data) {
			switch(stateMachine.getState())
			{
			case STAT_CALL_WITH_PSTN:
				audioControl.writeCallSound(data);
				break;

			case STAT_CALL_TRANS_PSTN_AND_SIP:
				// TODO: trans sound to sip
				break;

			default:
				// ignore
				break;
			}
		};
	};

	private ICallCenter mSipCb = new ICallCenter (){
		public boolean onIncommingCall(String target_number) {
			if( target_number.equals("0") ) { // use 0 as default local number
				stateMachine.inputEvent(EVT_INCOMMING_CALL_FROM_SIP_TO_BOX, null);
			}else {
				stateMachine.inputEvent(EVT_INCOMMING_CALL_FROM_SIP_TO_PSTN, target_number.getBytes());
			}
			return true;
		};

		public void onAccept()
		{
			stateMachine.inputEvent(EVT_SIP_ACCEPT, null);
		};

		public void onDtmf(String number) {
			if( stateMachine.getState() == STAT_CALL_TRANS_PSTN_AND_SIP )
			{
				mPstnService.dtmf(number);
			}
		};

		public void onHangup() {
			stateMachine.inputEvent(EVT_SIP_HANGUP, null);
		};

		public void onSoundData(byte[] data) {
			switch(stateMachine.getState())
			{
			case STAT_CALL_WITH_SIP:
				audioControl.writeCallSound(data);
				break;

			case STAT_CALL_TRANS_PSTN_AND_SIP:
				mPstnService.writeSound(data);
				break;

			default:
				// ignore
				break;
			}
		};
	};

	private boolean setupService() {
		logout(".setupService");
		if( true != mPstnService.init(mPstnCb) )
		{
			return false;
		}

		if( true != mSipService.init(mSipCb) )
		{
			return false;
		}
		
		return true;
	};

	private boolean shutdownService() {
		mPstnService.deinit();
		mSipService.deinit();
		return true;
	};


	// ******************************* local Audio **************************
	AudioControl audioControl = new AudioControl();
	// call sound
	ISoundRecord localSoundRecord = new ISoundRecord (){
		public void onSound(byte[] data) {
			switch(stateMachine.getState())
			{
			case STAT_CALL_WITH_SIP:
				mSipService.writeSound(data);
				break;

			case STAT_CALL_WITH_PSTN:
				mPstnService.writeSound(data);
				break;

			default:
				// ignore
				break;
			}
		}
	};

	
	private void volumeUp( AudioType type, boolean direction) {
		VolumeLevel vol = audioControl.getVolumeLevel(type);
		int i = vol.ordinal();
		if(direction) {
			i ++;
			if( i >= VolumeLevel.values().length ) {
				i = 0;
			}
		}
		else {
			i --;
			if( i < 0 ) {
				i = VolumeLevel.values().length - 1;
			}
		}

		audioControl.setVolume(type, VolumeLevel.values()[i]);
	}
	
	public void volumeUp(AudioType type) {
		volumeUp(type, true);
	}

	public void volumeDown(AudioType type) {
		volumeUp(type, false);
	}
	
	// **************************** local key event ***************************************
	private enum KeyStat {KEY_STAT_OFF, KEY_STAT_PRESSED, KEY_STAT_LONGPRESSED };
	private KeyStat [] key_stat = new KeyStat[KeyEvent.getMaxKeyCode()];
	private ArrayList<KeyActConfig> key_cfg = new ArrayList<MainActivity.KeyActConfig>();
	private interface KeyAction {
		public abstract void action();
	}
	private class KeyExpect {
		public int code;
		public KeyStat stat; 
		public KeyExpect(int c, KeyStat s) {
			code = c;
			stat = s;
		}
	}
	private class KeyActConfig {
		private KeyExpect[] expect_key = null;
		private int expect_status = -1;
		private KeyAction action = null;

		public KeyActConfig(int stat, KeyExpect[] array, KeyAction act) {
			expect_status = stat;
			expect_key = array;
			action = act;
		}
		
		public boolean isActived()	{
			if( stateMachine.getState() != expect_status ) {
				return false;
			}
			
			if( expect_key == null || action == null ) {
				return false;
			}
			
			for(KeyExpect i : expect_key) {
				if( key_stat[i.code] != i.stat ) {
					return false;
				}
			}
			return true;
		}
		
		public void doAction() {
			if( action != null ) {
				action.action();
			}
		}
		
		public int compare(KeyActConfig r) {
			if( r.expect_status > expect_status ) {
				return 1;
			} else if ( r.expect_status < expect_status ) {
				return -1;
			}

			if(r.expect_key.length > expect_key.length) {
				return 1;
			}
			else if(r.expect_key.length < expect_key.length ) {
				return -1;
			}
			
			for( int i = 0; i < expect_key.length; i ++ ) {
				if( r.expect_key[i].stat.ordinal() > expect_key[i].stat.ordinal() ) {
					return 1;
				} else if ( r.expect_key[i].stat.ordinal() < expect_key[i].stat.ordinal() ) {
					return -1;
				}
			}
			return 0;
		}
	}
	
	// ******************************** actionS *******************************
	private class DialPstnAction implements KeyAction {
		private String number = null;
		public DialPstnAction(String num) {
			number = num;
		}
		@Override public void action() {
			logout("DialPstnAction number=" + number);
			if( number != null ) {
				stateMachine.inputEvent(EVT_MAKE_PSTN_CALL, number.getBytes());
			}
		}
	}
	
	private KeyAction switchFM = new KeyAction() {
		@Override public void action() {
			// TODO: switch FM on or off
		}
	};
	
	private KeyAction acceptCall = new KeyAction() {
		@Override public void action() {
			logout(".acceptCall.action");
			stateMachine.inputEvent(EVT_ACCEPT, null);
		}
	};
	
	private KeyAction hangupCall = new KeyAction() {
		@Override public void action() {
			stateMachine.inputEvent(EVT_HANGUP, null);
		}
	};
	
	private KeyAction switchCallVolume = new KeyAction() {
		@Override public void action() {
			volumeUp(AudioType.CALL);
		}
	};
	
	private void resetKeyStat() {
		for(int i = 0; i < key_stat.length; i++) {
			key_stat[i] = KeyStat.KEY_STAT_OFF;
		}
	}
	
	private KeyAction testDtmf = new KeyAction() {
		@Override public void action() {
			mPstnService.dtmf("123456");
		}
	};

	private void setupKeyHandle() {
		logout(".setupKeyHandle");

		resetKeyStat();

		// idle
		key_cfg.add(new KeyActConfig(STAT_IDLE, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_LONGPRESSED) }, new DialPstnAction("656") ));
		key_cfg.add(new KeyActConfig(STAT_IDLE, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_LONGPRESSED) }, new DialPstnAction("656") ));
		key_cfg.add(new KeyActConfig(STAT_IDLE, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_LONGPRESSED) }, new DialPstnAction("656") ));
		key_cfg.add(new KeyActConfig(STAT_IDLE, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_LONGPRESSED),  new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_LONGPRESSED) },  switchFM ));

		// incomming call
		key_cfg.add(new KeyActConfig(STAT_RING_BY_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED) }, acceptCall ));
		key_cfg.add(new KeyActConfig(STAT_RING_BY_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));

		// calling
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED)}, switchCallVolume ));
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED)}, switchCallVolume ));
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_BACK, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));

		// thirchina
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_PSTN, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));
		key_cfg.add(new KeyActConfig(STAT_CALL_WITH_SIP, new KeyExpect[]{ new KeyExpect(KeyEvent.KEYCODE_VOLUME_DOWN, KeyStat.KEY_STAT_PRESSED),  new KeyExpect(KeyEvent.KEYCODE_VOLUME_UP, KeyStat.KEY_STAT_PRESSED) }, hangupCall ));

		
		Comparator<KeyActConfig> key_act_comparator = new Comparator<KeyActConfig>() {
			@Override
			public int compare(KeyActConfig l, KeyActConfig r) {
				return l.compare(r);
			}
		};
		Collections.sort(key_cfg, key_act_comparator);
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if(event.getRepeatCount() == 0)	{
			logout("onKeyDown getRepeatCount:0 stat:"+ stateMachine.getState() +" keyCode="+keyCode);
			key_stat[keyCode] = KeyStat.KEY_STAT_PRESSED;
		} else if(event.getRepeatCount() == 40) {
			logout("onKeyDown getRepeatCount:40 stat:"+ stateMachine.getState() +" keyCode="+keyCode);
			key_stat[keyCode] = KeyStat.KEY_STAT_LONGPRESSED;
			return handleKeyEvent();
		}
		return true;
	}

	@Override
	public boolean onKeyLongPress(int keyCode, KeyEvent event) {
		logout("onKeyLongPress stat:"+ stateMachine.getState() +" keyCode="+keyCode);
		key_stat[keyCode] = KeyStat.KEY_STAT_LONGPRESSED;
		return handleKeyEvent();
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		logout("onKeyUp stat:"+ stateMachine.getState() +" keyCode="+keyCode);
		handleKeyEvent();
		key_stat[keyCode] = KeyStat.KEY_STAT_OFF;
		return true;
	}

	private boolean handleKeyEvent()
	{
		for( KeyActConfig i : key_cfg ) {
			if( i.isActived() ) {
				i.doAction();
				resetKeyStat();
				return true;
			}
		}
		return false;
	}

	// ********************************* life cycle **********************************
	private void scheduleCheckBleServices(int ms) {
		Timer tBleTimer = new Timer();
		tBleTimer.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				if(!CommonUtils.isServiceWorked(MainActivity.this,"com.cubic.e3box.ble.BleService")){
					Log.d(TAG,"forge : start BleService from MainActivity");
					Intent bleServiceIntent = new Intent(MainActivity.this,BleService.class);
					startService(bleServiceIntent);
				}
			}
		},5000, 5000);
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		logout(".onCreate");
		
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD,WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD); 
		setContentView(R.layout.activity_main);
		
		CarrierApp.setMainActivity(MainActivity.this);
		
		// setup service
		setupService();
		setupStatusMachine();
		setupKeyHandle();
		audioControl.setup((AudioManager)getSystemService(AUDIO_SERVICE));
		scheduleCheckBleServices(5000);
	}

	@Override
	protected void onDestroy() {
		audioControl.shutdown();
		shutdownService();
		super.onDestroy();
	}
};

