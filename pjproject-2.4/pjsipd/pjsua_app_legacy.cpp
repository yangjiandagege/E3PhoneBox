/* $Id: pjsua_app_legacy.c 4851 2014-05-23 09:29:09Z nanang $ */
/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <pjsua-lib/pjsua.h>
#include "pjsua_app_common.h"
#include "pjsua_msg.h"
#include <CSafeQueue.cc>

#define THIS_FILE	"pjsua_app_legacy.c"

extern CSafeQueue<SipMsg> gMsgQueue;

/** UI Command **/
static void ui_make_new_call(char* buff)
{
    pjsua_msg_data msg_data;
	pj_status_t status;	
	if ((status=pjsua_verify_url(buff)) != PJ_SUCCESS) 
	{
	    pjsua_perror(THIS_FILE, "Invalid URL", status);
	    return;
	}
	pj_str_t uri = pj_str(buff);
    pjsua_msg_data_init(&msg_data);
    pjsua_call_make_call(current_acc, &uri, &call_opt, NULL,
			 &msg_data, &current_call);
}

static void ui_detect_nat_type()
{
    int i = pjsua_detect_nat_type();
    if (i != PJ_SUCCESS)
	pjsua_perror(THIS_FILE, "Error", i);
}

static void ui_answer_call(char* code)
{
    pjsua_call_info call_info;
    char buf[128];
    pjsua_msg_data msg_data;
    if (current_call != -1) {
	pjsua_call_get_info(current_call, &call_info);
    } else {
	/* Make compiler happy */
	call_info.role = PJSIP_ROLE_UAC;
	call_info.state = PJSIP_INV_STATE_DISCONNECTED;
    }
    if (current_call == -1 ||
	call_info.role != PJSIP_ROLE_UAS ||
	call_info.state >= PJSIP_INV_STATE_CONNECTING)
    {
	return;
    } else {
	int st_code;
	strcpy(buf,code);
	
	st_code = my_atoi(buf);
	if (st_code < 100)
	    return;
	pjsua_msg_data_init(&msg_data);
	if (current_call == -1) {
	    return;
	}
	pjsua_call_answer2(current_call, &call_opt, st_code, NULL, &msg_data);
    }
}

static void ui_hangup_call(char menuin[])
{
    if (current_call == -1) {
	puts("No current call");
	fflush(stdout);
	return;

    } else if (menuin[1] == 'a') {
	/* Hangup all calls */
	pjsua_call_hangup_all();
    } else {
	/* Hangup current calls */
	pjsua_call_hangup(current_call, 0, NULL, NULL);
    }
}

static void ui_add_account(pjsua_transport_config *rtp_cfg,char* buff)
{
    char id[128], registrar[128], realm[128], uname[128], passwd[30],proxy[128];
    pjsua_acc_config acc_cfg;
    pj_status_t status;
	char *result = NULL;
	pjsua_acc_id acc_ids[16];
	unsigned count = PJ_ARRAY_SIZE(acc_ids);
	pjsua_acc_info info;
	int i;

	result = strtok(buff,"&");
	sprintf(id,"%s",result);
	result = strtok( NULL, "&");
	sprintf(registrar,"%s",result);
	result = strtok( NULL, "&");
	sprintf(realm,"%s",result);
	result = strtok( NULL, "&");
	sprintf(uname,"%s",result);
	result = strtok( NULL, "&");
	sprintf(passwd,"%s",result);
	result = strtok( NULL, "&");
	sprintf(proxy,"%s",result);
	pjsua_enum_accs(acc_ids, &count);
	for (i=0; i<(int)count; ++i)
	{
		switch(i)
		{
			case 0:
			case 1:
				break;
			case 2:
				if(!app_config.use_tls)
				{
					printf("The account has changed , delete the old one\n");
					pjsua_acc_del(i);
				}
				break;
			case 3:
				printf("The account has changed , delete the old one\n");
				pjsua_acc_del(i);
				break;
			default :
				break;
		}
	}

    pjsua_acc_config_default(&acc_cfg);
    acc_cfg.id = pj_str(id);
    acc_cfg.reg_uri = pj_str(registrar);
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].scheme = pj_str("Digest");
    acc_cfg.cred_info[0].realm = pj_str(realm);
    acc_cfg.cred_info[0].username = pj_str(uname);
    acc_cfg.cred_info[0].data_type = 0;
    acc_cfg.cred_info[0].data = pj_str(passwd);
	if(proxy[0] != '*'){
		acc_cfg.proxy[acc_cfg.proxy_cnt++] = pj_str(proxy);
	}
    acc_cfg.rtp_cfg = *rtp_cfg;
    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, NULL);
    if (status != PJ_SUCCESS) {
	pjsua_perror(THIS_FILE, "Error adding new account", status);
    }
	pjsua_acc_set_online_status(pjsua_acc_get_default(), PJ_TRUE);
}

void legacy_main()
{
	pjsua_codec_info c[32];
	char buf[128];
	pj_str_t id;
	unsigned i, count = PJ_ARRAY_SIZE(c);
	SipMsg tSipMsg;
	CSafeQueue<SipMsgT>::EErrCode ret;
	memset(&tSipMsg,0,sizeof(tSipMsg));
    pjsua_codec_set_priority(pj_cstr(&id, "speex/16000"),PJMEDIA_CODEC_PRIO_HIGHEST);

	printf("List of audio codecs:\n");
	pjsua_enum_codecs(c, &count);
	for (i=0; i<count; ++i) {
		printf("  %d\t%.*s\n", c[i].priority, (int)c[i].codec_id.slen,c[i].codec_id.ptr);
	}
	
    for (;;) {
		
		/* Update call setting */
		pjsua_call_setting_default(&call_opt);
		call_opt.aud_cnt = app_config.aud_cnt;
		call_opt.vid_cnt = app_config.vid.vid_cnt;
		
		ret = gMsgQueue.pop(tSipMsg, 1000);
		if( ret != CSafeQueue<SipMsgT>::ERR_NO_ERROR ) continue; 
		switch (tSipMsg.aCmd[0]) {
			case 'r':
				ui_add_account(&app_config.rtp_cfg,tSipMsg.aMsg);
				break;
			case 'd':
				ui_make_new_call(tSipMsg.aMsg);
				break;
			case 'a':
				ui_answer_call(tSipMsg.aMsg);
				break;
			case 'h':
				ui_hangup_call("h");
				break;
			default:
				break;
		}
    }
}
