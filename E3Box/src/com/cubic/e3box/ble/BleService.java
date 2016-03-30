package com.cubic.e3box.ble;

import java.io.UnsupportedEncodingException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;
import org.json.JSONException;
import org.json.JSONObject;
import android.annotation.SuppressLint;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.SystemProperties;
import android.util.Log;
import com.cubic.e3box.BoxStateMachine;
import com.cubic.e3box.CarrierApp;
import com.cubic.e3box.Constants;
import com.cubic.e3box.IAction;
import com.cubic.e3box.utils.AESUtils;
import com.cubic.e3box.utils.CharUtils;

public class BleService extends Service{
	
	private static final String TAG = "BleService";
	private static void logout(String log) {
		Log.d(TAG, TAG + " " + log );
	}
	private Timer mTimeOut = null;
	
	private BluetoothManager mBluetoothManager; 
    private BluetoothGattServer  mGattServer; 
    private BluetoothAdapter mAdapter;
    private BluetoothLeAdvertiser mLeAdvertiser;
    private AdvertiseCallback mAdvertiseCallback; 
    
    private BluetoothGattCharacteristic mWriteReqDataChar;
    private BluetoothGattCharacteristic mWriteReqStartChar;
    private BluetoothGattCharacteristic mWriteReqEndChar;
    private BluetoothGattCharacteristic mNotifyRspStartChar;
    private BluetoothGattCharacteristic mReadRspDataChar;
    private BluetoothGattCharacteristic mWriteRspIncrementChar;
    
    private int mRspOffset = 0;
	private byte[]   mReqJsonBytes = null;
	private byte[][] mRspJsonBytes = null;
    
    private int mReadRspState = 0;
    
    private static final int  RESPONSE_STATE_IDLE = 0;
    private static final int  RESPONSE_STATE_READY = 1;
    private static final int  RESPONSE_STATE_END = 2;

    BoxStateMachine mBleStateMachine = new BoxStateMachine();
    
    private StartReqAction mStartReqAction = new StartReqAction();
    private ReceivingReqAction mReceivingReqAction = new ReceivingReqAction();
    private ProcessingReqAction mProcessingReqAction = new ProcessingReqAction();
    private SendingRspAction mSendingRspAction = new SendingRspAction();
    private ReturnIdleAction mReturnIdleAction = new ReturnIdleAction();
    
	@Override
	public void onCreate() {
		super.onCreate();
		initService();
	}
	
	@SuppressLint("NewApi") 
	private void initService(){
		SystemProperties.set(Constants.BLE_PROP_serial_id, "512346789492Izw");
		if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) 
        {
            Log.e(TAG, "BLE is not supported");
            BleService.this.stopSelf();
        }
		
		mBluetoothManager = (BluetoothManager)getSystemService(Context.BLUETOOTH_SERVICE);
		mAdapter = mBluetoothManager.getAdapter();
		if (mAdapter == null) 
		{
			Log.e(TAG, "Bluetooth is not supported.");
			BleService.this.stopSelf();
		}else{
			Log.e(TAG, "Bluetooth is supported.");
		}
		
		if(!mAdapter.isEnabled())
		{
		    mAdapter.enable();	
		}
		String strSerial = SystemProperties.get(Constants.BLE_PROP_serial_id, Constants.BLE_DEFAULT_VALUE_NULL);
		String strBleName = strSerial.substring(9, 15);
		mAdapter.setName(strBleName);
		
		mLeAdvertiser = mAdapter.getBluetoothLeAdvertiser();
        if(mLeAdvertiser == null)
        {
        	Log.e(TAG,"The device not support peripheral");
        	BleService.this.stopSelf();
        }
        
        mAdvertiseCallback = new E3boxAdvertiseCallback();
        
        startAdvertising();
        
        CarrierApp.updateNetStates(this);
        
        mBleStateMachine.insertAction(BoxStateMachine.STATE_BEGIN, Constants.BLE_STATE_IDLE,Constants.BLE_EVT_BEGIN,null);
        mBleStateMachine.insertAction(Constants.BLE_STATE_IDLE,Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_EVT_REQ_START,mStartReqAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_EVT_REQ_RECEIVING,mReceivingReqAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_STATE_PROCESSING_REQ,Constants.BLE_EVT_REQ_END,mProcessingReqAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_PROCESSING_REQ,Constants.BLE_STATE_SENDING_RSP,Constants.BLE_EVT_RSP_SENDING,mSendingRspAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_SENDING_RSP,Constants.BLE_STATE_SENDING_RSP,Constants.BLE_EVT_RSP_SENDING,mSendingRspAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_SENDING_RSP,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_RSP_END,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_RECEIVE_ERROR,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_SENDING_RSP,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_SEND_ERROR,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_RECEIVING_REQ,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_TIMEOUT,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_PROCESSING_REQ,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_TIMEOUT,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_SENDING_RSP,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_TIMEOUT,mReturnIdleAction);
        mBleStateMachine.insertAction(Constants.BLE_STATE_IDLE,Constants.BLE_STATE_IDLE,Constants.BLE_EVT_TIMEOUT,mReturnIdleAction);
        mBleStateMachine.inputEvent(Constants.BLE_EVT_BEGIN, null);
	}
	
	public void startAdvertising() {
        if (mLeAdvertiser != null) {
        	// build SC Station Gatt Service
            mGattServer = mBluetoothManager.openGattServer(this, mGattServerCallback);
    		mWriteReqDataChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.REQUEST_DATA), 
    				BluetoothGattCharacteristic.PROPERTY_WRITE,BluetoothGattCharacteristic.PERMISSION_WRITE);
    		mWriteReqStartChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.REQUEST_START), 
    				BluetoothGattCharacteristic.PROPERTY_WRITE,BluetoothGattCharacteristic.PERMISSION_WRITE);    
    		mWriteReqEndChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.REQUEST_END), 
    				BluetoothGattCharacteristic.PROPERTY_WRITE,BluetoothGattCharacteristic.PERMISSION_WRITE); 
    		mReadRspDataChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.RESPONSE_DATA), 
    				BluetoothGattCharacteristic.PROPERTY_READ,BluetoothGattCharacteristic.PERMISSION_READ);
    		mNotifyRspStartChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.RESPONSE_START), 
    				BluetoothGattCharacteristic.PROPERTY_READ,BluetoothGattCharacteristic.PERMISSION_READ);
    		mWriteRspIncrementChar = new BluetoothGattCharacteristic(UUID.fromString(Constants.RESPONSE_INCREMENT), 
    				BluetoothGattCharacteristic.PROPERTY_WRITE,BluetoothGattCharacteristic.PERMISSION_WRITE);
    		BluetoothGattService RequestResponseService = new BluetoothGattService(UUID.fromString(Constants.SERVICE_UUID),BluetoothGattService.SERVICE_TYPE_PRIMARY);
    		RequestResponseService.addCharacteristic(mWriteReqDataChar);
    		RequestResponseService.addCharacteristic(mWriteReqStartChar);
    		RequestResponseService.addCharacteristic(mWriteReqEndChar);
    		RequestResponseService.addCharacteristic(mReadRspDataChar);
            RequestResponseService.addCharacteristic(mNotifyRspStartChar);
            RequestResponseService.addCharacteristic(mWriteRspIncrementChar);
            mGattServer.addService(RequestResponseService);
            
            mLeAdvertiser.startAdvertising(buildAdvertiseSettings(), buildAdvertiseData(),mAdvertiseCallback);
        } 
    }
	
	private AdvertiseSettings buildAdvertiseSettings() {
        AdvertiseSettings.Builder settingsBuilder = new AdvertiseSettings.Builder();
        settingsBuilder.setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_POWER);
        return settingsBuilder.build();
    }
	
	private AdvertiseData buildAdvertiseData() {
    	AdvertiseData.Builder dataBuilder = new AdvertiseData.Builder();
    	dataBuilder.addServiceUuid(ParcelUuid.fromString(Constants.SERVICE_UUID));
		dataBuilder.setIncludeDeviceName(true);
        return dataBuilder.build();
    }
	
	/*************START : BLE Action******************/
	private class StartReqAction implements IAction{
		public boolean performAction(byte[] value) {
			logout("step 1 : Received a start flag , data type is REQUEST_START");
			mTimeOut = new Timer();
			mTimeOut.schedule(new TimerTask() {
				public void run() {
					logout("Start timeout with 5s");
					mBleStateMachine.inputEvent(Constants.BLE_EVT_TIMEOUT, null);
				}
			}, Constants.BLE_SET_TIMEOUT*1000);
			mNotifyRspStartChar.setValue("");
			mReqJsonBytes = null;
			return true;
		}
	}
	
	private class ReceivingReqAction implements IAction{
		public boolean performAction(byte[] value) {
			logout("step 2 : Received a data chunk , data type is RECEIVING_REQUEST , size = "+value.length);//+"value = "+Utils.bytesToHexString(value)
    		mReqJsonBytes = CharUtils.byteMerger(mReqJsonBytes, value);
			return true;
		}
	}
	
	private class ProcessingReqAction implements IAction{
		public boolean performAction(byte[] value) {
    		String recString = null;
    		JSONObject reqJsonObject = null;
    		JSONObject rspJsonObject = null;
    		JsonOperation jsonOper = null;
    		byte[] rspData = null;

        	try {
        		if(mReqJsonBytes.length <= 3000){
        			recString = new String(AESUtils.getInstance().decrypt(mReqJsonBytes) ,"UTF-8");
                	try {
    					reqJsonObject = new JSONObject(recString);
    				} catch (JSONException e) {
    					reqJsonObject = null;
    					e.printStackTrace();
    				}
                	if(reqJsonObject != null){
                		logout("--step 3 : Received all of the data chunks , request json is "+reqJsonObject);//"--step" means the info is very important
                		jsonOper = JsonOperationFactory.createJsonOperation(reqJsonObject.getString("method"),BleService.this);
                	}else{
                		logout("--step 3 : Received all of the data chunks , IsParsingJsonError");//"--step" means the info is very important
                		jsonOper = JsonOperationFactory.createJsonOperation("IsParsingJsonError",null);
                	}
        		}else{
        			logout("--step 3 : Received all of the data , IsRequestTooLong ");//"--step" means the info is very important
        			jsonOper = JsonOperationFactory.createJsonOperation("IsRequestTooLong",null);
        		}
        		AuthenticationFinery authenticationFinery = new AuthenticationFinery();
        		authenticationFinery.Decorate(jsonOper);
	            rspJsonObject = authenticationFinery.jsonOperation(reqJsonObject);
	            logout("--step 4 : Get the response json , response Json = "+rspJsonObject);//"--step" means the info is very important
	            
				rspData = AESUtils.getInstance().encrypt(rspJsonObject.toString().getBytes("UTF8"));
				logout("step 5 : Encrypt the response json "+", rspData.length = "+rspData.length+" , rspData = "+CharUtils.bytesToHexString(rspData));
	            
	            mRspJsonBytes = null;
	            mRspJsonBytes = CharUtils.bytesSplit(rspData,512);
	            logout("step 6 : Split the encrypted response data to many parts so that send to SC Client");
	            
	            mReadRspState = RESPONSE_STATE_READY;
	            mRspOffset=0;
	            logout("step 7 : Notify the SC Client to read response data");
	        	
	            mReqJsonBytes = null;
			} catch (UnsupportedEncodingException e1) {
				e1.printStackTrace();
			} catch (Exception e1) {
				e1.printStackTrace();
			}
			return true;
		}
	}
	
	private class SendingRspAction implements IAction{
		public boolean performAction(byte[] value) {
    		mRspOffset ++;
    		if(mRspOffset < mRspJsonBytes.length){
    			logout("step 8 : SC Client read response data from SC Station , mRspOffset = "+mRspOffset+" , State change to SENDING_RESPONSE from PROCESSING_REQUEST");
            	mReadRspState = RESPONSE_STATE_READY;
    		}else{
    			logout("step 9 : SC Client read response data Over , State change to IDLE from SENDING_RESPONSE");
            	mReadRspState = RESPONSE_STATE_END;
    		}
			return true;
		}
		
	}
	
	private class ReturnIdleAction implements IAction{
		public boolean performAction(byte[] value) {
			logout("End timeout");
			mTimeOut.cancel();
			return true;
		}
	}
	/**************END : BLE Action*****************/
	
	private final BluetoothGattServerCallback mGattServerCallback = new BluetoothGattServerCallback()
	    {
	        @Override
	        public void onConnectionStateChange(BluetoothDevice device, int status, int newState)
	        {
	            super.onConnectionStateChange(device, status, newState);
	        }

	        @Override
	        public void onServiceAdded(int status, BluetoothGattService service) 
	        {
	            super.onServiceAdded(status, service);
	        }

	        @Override
	        public void onCharacteristicReadRequest(BluetoothDevice device, int requestId, int offset, 
	                                                BluetoothGattCharacteristic characteristic) 
	        {
	        	byte[] sentinel_array = new byte[0];
	            if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.RESPONSE_START)){
	            	try {
		            	switch(mReadRspState){
		            		case RESPONSE_STATE_IDLE:
		            			mNotifyRspStartChar.setValue("".getBytes("UTF8"));
		            			break;
		            		case RESPONSE_STATE_READY:
		            		case RESPONSE_STATE_END:
								mNotifyRspStartChar.setValue("ready".getBytes("UTF8"));
		            			break;	
		            	}
	        		} catch (UnsupportedEncodingException e) {
						e.printStackTrace();
					}
	            }
	            
	        	if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.RESPONSE_DATA)){
	            	switch(mReadRspState){
	        		case RESPONSE_STATE_IDLE:
	        			logout("Read response data , state is idle");
		        		mReadRspDataChar.setValue(sentinel_array);
	        			break;
	        		case RESPONSE_STATE_READY:
	        			logout("Read response data , state is ready , offset = "+mRspOffset);
	        			mReadRspDataChar.setValue(mRspJsonBytes[mRspOffset]);
	        			mReadRspState = RESPONSE_STATE_IDLE;
	        			break;
	        		case RESPONSE_STATE_END:
	        			logout("step 10 : Read response data over , state is end");
	            		mReadRspDataChar.setValue(sentinel_array);
	            		mRspOffset = 0;
	            		mReadRspState = RESPONSE_STATE_IDLE;
	            		mBleStateMachine.inputEvent(Constants.BLE_EVT_RSP_END, null);
	        			break;
		        	}
	        	}

	            super.onCharacteristicReadRequest(device, requestId, offset, characteristic);
	            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, offset, characteristic.getValue());
	        }

	        @Override
	        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId, BluetoothGattCharacteristic characteristic, 
	                                                 boolean preparedWrite, boolean responseNeeded, int offset, byte[] value) 
	        {
	        	if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.REQUEST_START)){
	        		mBleStateMachine.inputEvent(Constants.BLE_EVT_REQ_START, null);
	        	}else if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.REQUEST_DATA)){
	        		if(mBleStateMachine.inputEvent(Constants.BLE_EVT_REQ_RECEIVING, value) == -1){
	        			mBleStateMachine.inputEvent(Constants.BLE_EVT_RECEIVE_ERROR, null);
	        		}
	        	}else if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.REQUEST_END)){
	        		mBleStateMachine.inputEvent(Constants.BLE_EVT_REQ_END, null);
	        	}else if(characteristic.getUuid().toString().equalsIgnoreCase(Constants.RESPONSE_INCREMENT)){
	        		if(mBleStateMachine.inputEvent(Constants.BLE_EVT_RSP_SENDING, null) == -1){
	        			mBleStateMachine.inputEvent(Constants.BLE_EVT_SEND_ERROR, null);
	        		}
	        	}else{
	        		logout("write characteristic error !");
	        	}
	        	mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, offset, null);
	        	super.onCharacteristicWriteRequest(device, requestId, characteristic, preparedWrite, responseNeeded, offset, value);
	        }

	        @Override
	        public void onNotificationSent(BluetoothDevice device, int status)
	        {
	            super.onNotificationSent(device, status);
	        }
	        
	        @Override
	        public void onDescriptorReadRequest(BluetoothDevice device, int requestId, int offset, 
	                                            BluetoothGattDescriptor descriptor) 
	        {
	            super.onDescriptorReadRequest(device, requestId, offset, descriptor);
	        }

	        @Override
	        public void onDescriptorWriteRequest(BluetoothDevice device, int requestId, 
	                                             BluetoothGattDescriptor descriptor, boolean preparedWrite, 
	                                             boolean responseNeeded, int offset, byte[] value) 
	        {
	            super.onDescriptorWriteRequest(device, requestId, descriptor, preparedWrite, responseNeeded, offset, value);
	        }

	        @Override
	        public void onExecuteWrite(BluetoothDevice device, int requestId, boolean execute) 
	        {
	            super.onExecuteWrite(device, requestId, execute);
	            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, null);
	        }
	    };
	    
	    private class E3boxAdvertiseCallback extends AdvertiseCallback {
	    	 
	 	   @Override
	 	    public void onStartFailure(int errorCode) {
	 			super.onStartFailure(errorCode);
	 			if(errorCode == ADVERTISE_FAILED_DATA_TOO_LARGE)
	 			{
	 			}else if(errorCode == ADVERTISE_FAILED_TOO_MANY_ADVERTISERS)
	 			{
	 			}else if(errorCode == ADVERTISE_FAILED_ALREADY_STARTED)
	 			{
	 			}else if(errorCode == ADVERTISE_FAILED_INTERNAL_ERROR)
	 			{
	 			}else if(errorCode == ADVERTISE_FAILED_FEATURE_UNSUPPORTED)
	 			{
	 			}
	 	    }
	        @Override
	         public void onStartSuccess(AdvertiseSettings settingsInEffect) {
	             super.onStartSuccess(settingsInEffect);
	         }
	     }
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
}
