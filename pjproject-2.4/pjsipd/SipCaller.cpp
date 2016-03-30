#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "pjsua_app.h"
#include "SipCaller.h"
#include "pjsua_msg.h"
#include "CSafeQueue.cc"
extern "C"
{
#include <pjmedia-audiodev/app_callback.h>
}
CSafeQueue<SipMsg> gMsgQueue;
int mArgc;
static pjsua_app_cfg_t	    cfg;
extern callback_func recorder_func;
extern callback_func player_func;

int main_func(int argc, char *argv[])
{
	pj_status_t status = PJ_TRUE;

	pj_bzero(&cfg, sizeof(cfg));
	cfg.argc = argc;
	cfg.argv = argv;

	status = pjsua_app_init(&cfg);
	if (status == PJ_SUCCESS) {
		status = pjsua_app_run(PJ_TRUE);
	} 

	pjsua_app_destroy();
	_exit(1);
	return 0;
}

void* pjsipd_start(void *pParam)
{
	return pj_run_app(&main_func, mArgc, (char **)pParam, 0);
}

void sendMsgToSipdLoop(char *cmd,char *info)
{
	SipMsg tSipMsg;
	memset(&tSipMsg,0,sizeof(tSipMsg));
	strcpy(tSipMsg.aCmd,cmd);
	if(info != NULL)
	{
		strcpy(tSipMsg.aMsg,info);
	}
	gMsgQueue.push(tSipMsg);
}

int SipCaller::pjsipd_init(int argc, char *argv[])
{
	pthread_t thread_sipd;
	pthread_attr_t attr_sipd;
	pthread_attr_init(&attr_sipd);
	pthread_attr_setdetachstate(&attr_sipd,PTHREAD_CREATE_JOINABLE);
	mArgc = argc;

	if(pthread_create(&thread_sipd,&attr_sipd, pjsipd_start,(void *)argv) == -1)  
	{  
		return -1;
	}
	return 0;
}

static ISipUser* SipCaller::getSipUser()
{
	return m_user;
}

void SipCaller::pjsipd_destroy()
{
	pjsua_app_destroy();
	return ;
}

void SipCaller::sipRegister(char* regInfo)
{
	sendMsgToSipdLoop("r",regInfo);
}
void SipCaller::dail(char* number )
{
	sendMsgToSipdLoop("d",number);
}
void SipCaller::hangup()
{
	sendMsgToSipdLoop("h",NULL);
}
void SipCaller::accept()
{
	sendMsgToSipdLoop("a","200");
}
void SipCaller::writeSound(void*  data, int data_sz )
{
	if(recorder_func != NULL)
	{
		recorder_func(data,data_sz);
	}
	if(player_func != NULL)
	{
		player_func(NULL,0);
	}
	
	return;
}
int SipCaller::getStat()
{
	return 0;
}