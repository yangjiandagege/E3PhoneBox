#ifndef _SIP_SERVICE_CC_
#define _SIP_SERVICE_CC_ 1

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <CSafeQueue.cc>
#include "ThirAndroidLog.h"
#include "IThirNativeService.cc"
#include "SipCaller.h"

ISipUser* SipCaller::m_user = NULL;

class SipService : public BnThirNativeService, public ISipUser
{
public:
	enum
	{
		// command from user
		SIP_CMD_ECHO = 1,
		SIP_CMD_DAIL,
		SIP_CMD_HANGUP,
		SIP_CMD_DTMF,
		SIP_CMD_ACCEPT,
		SIP_CMD_GET_STAT,
		SIP_CMD_REGISTER,
		
		// command to user
		SIP_RPT_STAT = 100,
		SIP_RPT_INCOMMING_CALL,
		SIP_RPT_HANGUP,
		SIP_RPT_REGISTER_STAT,
		SIP_RPT_ACCEPT,
	};

private:
	BpThirNativeService* m_user;
	SipCaller m_caller;

	void strToC16( const char* str, char16_t* str16, int len )
	{
		const char* p = str;
		char16_t*   q = str16;
		for(int i = 0; i < len && *p; i++, p++, q++ )
		{
			*q = *p;
		}
	};

	void strToC( const char16_t* str16, char* str, int len )
	{
		const char16_t* p = str16;
		char*           q = str;
		for(int i = 0; i < len && *p; i++, p++, q++ )
		{
			*q = (char)((*p)&0xFF);
		}
	};
	
public:
	
	SipService(int argc, char *argv[])
		: m_user(NULL)
		, m_caller(argc,argv,this)
	{};
	
	~SipService()
	{
		if(m_user)
		{
			delete m_user;
		}
	};

	// **************************** call by user ********************************

	// command from user
	virtual int32_t sendCmd(int32_t code, const char16_t* info)
	{
		static const int INFO_LEN_MAX = 1024;
		char s_info[INFO_LEN_MAX+4] = {0};

		
		switch( code )
		{
		case SIP_CMD_ECHO:
			return 0;

		case SIP_CMD_DAIL:
			strToC(info, s_info, INFO_LEN_MAX);
			m_caller.dail(s_info);
			LOGD("SipService::sipd get cmd of SIP_CMD_DAIL , dailNum = %s",s_info);
			return 0;

		case SIP_CMD_HANGUP:
			LOGD("SipService::sipd get cmd of SIP_CMD_HANGUP");
			m_caller.hangup();
			return 0;

		case SIP_CMD_DTMF:
			LOGD("SipService::sipd get cmd of SIP_CMD_DTMF");
			return 0;

		case SIP_CMD_ACCEPT:
			LOGD("SipService::sipd get cmd of SIP_CMD_ACCEPT");
			m_caller.accept();
			return 0;

		case SIP_CMD_GET_STAT:
			LOGD("SipService::sipd get cmd of SIP_CMD_GET_STAT");
			return m_caller.getStat();

		case SIP_CMD_REGISTER:
			strToC(info, s_info, INFO_LEN_MAX);
			LOGD("SipService::sipd get cmd of SIP_CMD_REGISTER , regInfo = %s", s_info);
			m_caller.sipRegister(s_info);
			return 0;
			
		default:
			break;
		};
		return -2;
	};

	// data from user
	virtual int32_t sendData(int32_t size, const void* data)
	{
		m_caller.writeSound(data, size);
		return 0;
	};

	virtual int32_t regBinder(sp<IBinder> &binder)
	{
		LOGD("SipService::regBinder()");
		if( m_user == NULL )
		{
			m_user = new BpThirNativeService(binder);
		}
		return 0;
	};
	
	// ******************** call by sip caller *********************
	virtual void onRegisterStat(int regCode)
	{
		static const int REG_CODE_LEN_MAX = 64;
		char strRegCode[REG_CODE_LEN_MAX];
		sprintf(strRegCode,"%d",regCode);
		char16_t s_strRegCode[REG_CODE_LEN_MAX+4] = {0};
		if(m_user)
		{
			strToC16(strRegCode, s_strRegCode, REG_CODE_LEN_MAX);
			m_user->sendCmd(SIP_RPT_REGISTER_STAT, s_strRegCode);
		}
	}
	
	virtual void onIncommingCall( const char *number )
	{
		static const int NUMBER_LEN_MAX = 2048;
		char16_t s_number[NUMBER_LEN_MAX+4] = {0};
		
		if(m_user)
		{
			strToC16(number, s_number, NUMBER_LEN_MAX);
			m_user->sendCmd(SIP_RPT_INCOMMING_CALL, s_number);
		}
	};

	virtual void onHangup()
	{
		char16_t empty_str[] = {'\0','\0'};

		if(m_user)
		{
			m_user->sendCmd(SIP_RPT_HANGUP, empty_str);
		}
	};

	virtual void onAccept()
	{
		char16_t empty_str[] = {'\0','\0'};

		if(m_user)
		{
			m_user->sendCmd(SIP_RPT_ACCEPT, empty_str);
		}
	};
	
    virtual void onSound(const void* data, int size)
    {
    	if(m_user)
		{
			m_user->sendData(size, data);
		}
    };
};
#endif
