package com.cubic.e3box;

public interface ICallCenter {
	boolean onIncommingCall(String target_number);
	void onAccept();
	void onDtmf(String number);
	void onHangup();
	void onSoundData(byte[] data);
}