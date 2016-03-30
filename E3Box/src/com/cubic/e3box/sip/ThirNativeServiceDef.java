package com.cubic.e3box.sip;

import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Parcel;
import android.os.RemoteException;

import java.lang.String; 

public class ThirNativeServiceDef {

	public  static final String DESCRIPTOR = "me.thirchina.SipNaticeService";
	private static final int    TRANSACTION_sendCmd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
	private static final int    TRANSACTION_sendData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
	private static final int    TRANSACTION_regBinder = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);


	public interface IService extends IInterface {
		public int sendCmd( int code, String info ) throws android.os.RemoteException;
		public int sendData( byte[] data ) throws android.os.RemoteException;
		public int regBinder(IBinder binder) throws android.os.RemoteException;
	}
	
	public static abstract class BnThirNativeService extends Binder implements IService {
		public BnThirNativeService(){
			this.attachInterface(this, DESCRIPTOR);
		}

		@Override
		public IBinder asBinder() {
			return this;
		}
		
		@Override
		protected boolean onTransact(int code, Parcel data, Parcel reply, int flags) throws RemoteException {
			switch(code) {
			case INTERFACE_TRANSACTION: 
			{
				reply.writeString(DESCRIPTOR);
				return true;
			}
			case TRANSACTION_sendCmd:
			{
				data.enforceInterface(DESCRIPTOR);
				int cmd_code = data.readInt();
				String cmd_info = data.readString();
				int ret = this.sendCmd(cmd_code, cmd_info);
				reply.writeInt(ret);
				return true;
			}
			case TRANSACTION_sendData:
			{
				data.enforceInterface(DESCRIPTOR);
				int data_size = data.readInt();
				byte[] data_array = new byte[data_size];
				data.readByteArray(data_array);
				int ret = this.sendData(data_array);
				reply.writeInt(ret);
				return true;
			}
			case TRANSACTION_regBinder:
			{
				data.enforceInterface(DESCRIPTOR);
				IBinder binder = data.readStrongBinder();
				int ret = this.regBinder(binder);
				reply.writeInt(ret);
				return true;
			}
			default:
				break;
			}
			return super.onTransact(code, data, reply, flags);
		}
	}
	
	public static class BpThirNativeService implements IService {
		private IBinder m_remote;

		public BpThirNativeService( IBinder remote )
		{
			m_remote = remote;
		}
		
		@Override
		public IBinder asBinder() {
			return m_remote;
		}

		@Override
		public int sendCmd( int code, String info ) throws RemoteException {
			Parcel req = Parcel.obtain();
			Parcel rsp = Parcel.obtain();
			int ret;
			
			try{
				req.writeInterfaceToken(DESCRIPTOR);
				req.writeInt(code);
				req.writeString(info);
				m_remote.transact(TRANSACTION_sendCmd, req, rsp, 0);
				ret = rsp.readInt();
			}
			finally {
				req.recycle();
				rsp.recycle();
			}
			return ret;
		}

		@Override
		public int sendData( byte[] data ) throws RemoteException {
			Parcel req = Parcel.obtain();
			Parcel rsp = Parcel.obtain();
			int ret;
			
			try{
				req.writeInterfaceToken(DESCRIPTOR);
				req.writeByteArray(data);
				m_remote.transact(TRANSACTION_sendData, req, rsp, 0);
				ret = rsp.readInt();
			}
			finally {
				req.recycle();
				rsp.recycle();
			}
			return ret;
		}
		
		@Override
		public int regBinder( IBinder binder ) throws RemoteException {
			Parcel req = Parcel.obtain();
			Parcel rsp = Parcel.obtain();
			int ret;
			
			try{
				req.writeInterfaceToken(DESCRIPTOR);
				req.writeStrongBinder(binder);
				m_remote.transact(TRANSACTION_regBinder, req, rsp, 0);
				ret = rsp.readInt();
			}
			finally {
				req.recycle();
				rsp.recycle();
			}
			return ret;
		}
	}
}
