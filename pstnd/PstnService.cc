/**
 * file: PstnService.cc
 * auth: thirchina
 * brif: android c++ service for PSTN
 */

#ifndef _PSTN_SERVICE_CC_
#define _PSTN_SERVICE_CC_ 1

#include "ThirAndroidLog.h"
#include "IThirNativeService.cc"
#include "PstnCaller.cc"


class PstnService : public BnThirNativeService, public IPstnUser
{
public:
	enum
	{
		// command from user
		PSTN_CMD_ECHO = 1,
		PSTN_CMD_DAIL,
		PSTN_CMD_HANGUP,
		PSTN_CMD_DTMF,
		PSTN_CMD_ACCEPT,
		PSTN_CMD_GET_STAT,
		
		// command to user
		PSTN_RPT_STAT = 100,
		PSTN_RPT_INCOMMING_CALL,
	};

private:
	BpThirNativeService* m_user;
	PstnCaller m_caller;

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
	PstnService()
		: m_user(NULL)
		, m_caller(this)
	{
        m_caller.init();
	};

	virtual ~PstnService()
	{
		m_caller.deinit();
		if(m_user)
		{
			delete m_user;
		}
	};

	// **************************** call by user ********************************

	// command from user
	virtual int32_t sendCmd(int32_t code, const char16_t* info)
	{
		static const int INFO_LEN_MAX = 128;
		char s_info[INFO_LEN_MAX+4] = {0};

		LOGD("PstnService::sendCmd(code=%d)", code);
		switch( code )
		{
		case PSTN_CMD_ECHO:
			return 0;

		case PSTN_CMD_DAIL:
			strToC(info, s_info, INFO_LEN_MAX);
			if(!m_caller.dail(s_info))
			{
				return -1;
			}
			return 0;

		case PSTN_CMD_HANGUP:
			m_caller.hangup();
			return 0;

		case PSTN_CMD_DTMF:
            strToC(info, s_info, INFO_LEN_MAX);
			m_caller.dtmf( s_info );
            return 0;

		case PSTN_CMD_ACCEPT:
			if( !m_caller.accept() )
			{
				return -1;
			}
			return 0;

		case PSTN_CMD_GET_STAT:
			return m_caller.getStat();

		default:
			break;
		};
		return -2;
	};

	// data from user
	virtual int32_t sendData(int32_t size, const void* data)
	{
        m_caller.writeSound( data, size );
		return 0;
	};

	virtual int32_t regBinder(sp<IBinder> &binder)
	{
		LOGD("PstnService::regBinder()");
		if( m_user != NULL )
		{
			delete m_user;
            m_user = NULL;
		}
        m_user = new BpThirNativeService(binder);
		return 0;
	};

	
	// ******************** call by PSTN caller *********************
	virtual bool onIncommingCall( const char *number )
	{
		static const int NUMBER_LEN_MAX = 64;
		char16_t s_number[NUMBER_LEN_MAX+4] = {0};
		
		if(m_user)
		{
			strToC16(number, s_number, NUMBER_LEN_MAX);
			if( 0 > m_user->sendCmd(PSTN_RPT_INCOMMING_CALL, s_number) )
		    {
		        delete m_user;
                m_user = NULL;
                return false;
		    }
		}
        return true;
	};

    virtual void onSound(const void* data, int size)
    {
    	if(m_user)
		{
			if( m_user->sendData(size, data) )
			{
		        delete m_user;
                m_user = NULL;
		    }
		}
    };
};

#endif // _PSTN_SERVICE_CC_
