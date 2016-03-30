#ifndef _SIP_CALLER_H_
#define _SIP_CALLER_H_ 1

class ISipUser
{
public:
    virtual ~ISipUser(){};
	virtual void onIncommingCall( const char *number ) = 0;
	virtual void onHangup() = 0;
    virtual void onSound(const void* data, int size) = 0;
	virtual void onRegisterStat(int regCode) = 0;
	virtual void onAccept() = 0;
};

class SipCaller
{
private:
	static ISipUser *m_user;
	
public:
	SipCaller( int argc, char *argv[], ISipUser *p_user )
	{
		m_user = p_user;
		pjsipd_init(argc,argv);
	};
	
	~SipCaller()
	{
		pjsipd_destroy();
	};
	
	static ISipUser* getSipUser();
	int pjsipd_init(int argc, char *argv[]);
	void pjsipd_destroy();
	void sipRegister(char* regInfo);
	void dail(char* number );
	void hangup();
	void accept();
	void writeSound(void*  data, int data_sz );
	int getStat();
	
};
#endif
