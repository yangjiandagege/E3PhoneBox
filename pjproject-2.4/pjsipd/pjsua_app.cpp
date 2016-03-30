/* $Id: pjsua_app.c 4848 2014-05-22 04:52:53Z ming $ */
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
#include "pjsua_app.h"
#include "SipCaller.h"
#include "ThirAndroidLog.h"
extern "C"
{
#include <pjmedia-audiodev/app_callback.h>
}
extern callback_func callback_player_func;

#define THIS_FILE	"pjsua_app.c"

#define current_acc	pjsua_acc_get_default()

static pj_status_t app_init();
static pj_status_t app_destroy();

static pjsua_app_cfg_t app_cfg;
pj_str_t		    uri_arg;

/*****************************************************************************
 * Callback 
 */
static void on_player_func(void *buf, int size)
{
	SipCaller::getSipUser()->onSound(buf,size);
}

/* Callback from timer when the maximum call duration has been
 * exceeded.
 */
static void call_timeout_callback(pj_timer_heap_t *timer_heap,
				  struct pj_timer_entry *entry)
{
    pjsua_call_id call_id = entry->id;
    pjsua_msg_data msg_data;
    pjsip_generic_string_hdr warn;
    pj_str_t hname = pj_str("Warning");
    pj_str_t hvalue = pj_str("399 pjsua \"Call duration exceeded\"");

    PJ_UNUSED_ARG(timer_heap);

    if (call_id == PJSUA_INVALID_ID) {
	PJ_LOG(1,(THIS_FILE, "Invalid call ID in timer callback"));
	return;
    }
    
    /* Add warning header */
    pjsua_msg_data_init(&msg_data);
    pjsip_generic_string_hdr_init2(&warn, &hname, &hvalue);
    pj_list_push_back(&msg_data.hdr_list, &warn);

    /* Call duration has been exceeded; disconnect the call */
    PJ_LOG(3,(THIS_FILE, "Duration (%d seconds) has been exceeded "
			 "for call %d, disconnecting the call",
			 app_config.duration, call_id));
    entry->id = PJSUA_INVALID_ID;
    pjsua_call_hangup(call_id, 200, NULL, &msg_data);
}

/*
 * Handler when invite state has changed.
 */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info call_info;
	char info[128];

    PJ_UNUSED_ARG(e);
	if (current_call==PJSUA_INVALID_ID)
	{
	    current_call = call_id;
    }
    pjsua_call_get_info(call_id, &call_info);

	switch(call_info.state){
		case PJSIP_INV_STATE_DISCONNECTED:
			SipCaller::getSipUser()->onHangup();
			break;
		case PJSIP_INV_STATE_CONFIRMED:
			SipCaller::getSipUser()->onAccept();
			break;
		default :
			break;
	}
}

static void getCorrectString(char* tar, char* src)
{
	int i=0;
	for(;i<strlen(src);i++)
	{
		if(isprint(src[i]))
		{
			tar[i] = src[i];
		}
	}
}

/**
 * Handler when there is incoming call.
 */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,pjsip_rx_data *rdata)
{
	static const int INFO_MAX = 2048;
    pjsua_call_info call_info;
	char info[INFO_MAX+4];
	char cWsse[INFO_MAX+4];
	char cPstnNumber[INFO_MAX+4];
	pjsip_generic_string_hdr *hdrWSSE = NULL;
	pjsip_generic_string_hdr *hdrPstnNumber = NULL;
	memset(info,0,sizeof(info));
	memset(cWsse,0,sizeof(cWsse));
	memset(cPstnNumber,0,sizeof(cPstnNumber));
	
    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &call_info);
	
    if (current_call==PJSUA_INVALID_ID)
	current_call = call_id;
	pj_str_t name;
	name = pj_str("X-WSSE");        
	hdrWSSE = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg,&name,NULL);
	name = pj_str("X-PSTN-Number");
	hdrPstnNumber = pjsip_msg_find_hdr_by_name(rdata->msg_info.msg,&name,NULL);
	if(hdrWSSE != NULL && hdrPstnNumber != NULL)
	{
		getCorrectString(cWsse,hdrWSSE->hvalue.ptr);
		getCorrectString(cPstnNumber,hdrPstnNumber->hvalue.ptr);
		snprintf(info,INFO_MAX,"%s::%s",cPstnNumber,cWsse);
		SipCaller::getSipUser()->onIncommingCall(info);
		return;
	}
	//SipCaller::getSipUser()->onIncommingCall(call_info.remote_info.ptr);
	SipCaller::getSipUser()->onIncommingCall("0");
}

/*
 * Handler when a transaction within a call has changed state.
 */
static void on_call_tsx_state(pjsua_call_id call_id,
			      pjsip_transaction *tsx,
			      pjsip_event *e)
{
    const pjsip_method info_method = 
    {
	PJSIP_OTHER_METHOD,
	{ "INFO", 4 }
    };

    if (pjsip_method_cmp(&tsx->method, &info_method)==0) {
	/*
	 * Handle INFO method.
	 */
	const pj_str_t STR_APPLICATION = { "application", 11};
	const pj_str_t STR_DTMF_RELAY  = { "dtmf-relay", 10 };
	pjsip_msg_body *body = NULL;
	pj_bool_t dtmf_info = PJ_FALSE;
	
	if (tsx->role == PJSIP_ROLE_UAC) {
	    if (e->body.tsx_state.type == PJSIP_EVENT_TX_MSG)
		body = e->body.tsx_state.src.tdata->msg->body;
	    else
		body = e->body.tsx_state.tsx->last_tx->msg->body;
	} else {
	    if (e->body.tsx_state.type == PJSIP_EVENT_RX_MSG)
		body = e->body.tsx_state.src.rdata->msg_info.msg->body;
	}
	
	/* Check DTMF content in the INFO message */
	if (body && body->len &&
	    pj_stricmp(&body->content_type.type, &STR_APPLICATION)==0 &&
	    pj_stricmp(&body->content_type.subtype, &STR_DTMF_RELAY)==0)
	{
	    dtmf_info = PJ_TRUE;
	}

	if (dtmf_info && tsx->role == PJSIP_ROLE_UAC && 
	    (tsx->state == PJSIP_TSX_STATE_COMPLETED ||
	       (tsx->state == PJSIP_TSX_STATE_TERMINATED &&
	        e->body.tsx_state.prev_state != PJSIP_TSX_STATE_COMPLETED))) 
	{
	    /* Status of outgoing INFO request */
	    if (tsx->status_code >= 200 && tsx->status_code < 300) {
		PJ_LOG(4,(THIS_FILE, 
			  "Call %d: DTMF sent successfully with INFO",
			  call_id));
	    } else if (tsx->status_code >= 300) {
		PJ_LOG(4,(THIS_FILE, 
			  "Call %d: Failed to send DTMF with INFO: %d/%.*s",
			  call_id,
		          tsx->status_code,
			  (int)tsx->status_text.slen,
			  tsx->status_text.ptr));
	    }
	} else if (dtmf_info && tsx->role == PJSIP_ROLE_UAS &&
		   tsx->state == PJSIP_TSX_STATE_TRYING)
	{
	    /* Answer incoming INFO with 200/OK */
	    pjsip_rx_data *rdata;
	    pjsip_tx_data *tdata;
	    pj_status_t status;

	    rdata = e->body.tsx_state.src.rdata;

	    if (rdata->msg_info.msg->body) {
		status = pjsip_endpt_create_response(tsx->endpt, rdata,
						     200, NULL, &tdata);
		if (status == PJ_SUCCESS)
		    status = pjsip_tsx_send_msg(tsx, tdata);

		PJ_LOG(3,(THIS_FILE, "Call %d: incoming INFO:\n%.*s", 
			  call_id,
			  (int)rdata->msg_info.msg->body->len,
			  rdata->msg_info.msg->body->data));
	    } else {
		status = pjsip_endpt_create_response(tsx->endpt, rdata,
						     400, NULL, &tdata);
		if (status == PJ_SUCCESS)
		    status = pjsip_tsx_send_msg(tsx, tdata);
	    }
	}
    }
}

/* General processing for media state. "mi" is the media index */
static void on_call_generic_media_state(pjsua_call_info *ci, unsigned mi,
                                        pj_bool_t *has_error)
{
    const char *status_name[] = {
        "None",
        "Active",
        "Local hold",
        "Remote hold",
        "Error"
    };

    PJ_UNUSED_ARG(has_error);

    pj_assert(ci->media[mi].status <= PJ_ARRAY_SIZE(status_name));
    pj_assert(PJSUA_CALL_MEDIA_ERROR == 4);
}

/* Process audio media state. "mi" is the media index. */
static void on_call_audio_state(pjsua_call_info *ci, unsigned mi,
                                pj_bool_t *has_error)
{
    PJ_UNUSED_ARG(has_error);

    /* Stop ringback */
    //ring_stop(ci->id);

    /* Connect ports appropriately when media status is ACTIVE or REMOTE HOLD,
     * otherwise we should NOT connect the ports.
     */
    if (ci->media[mi].status == PJSUA_CALL_MEDIA_ACTIVE ||
	ci->media[mi].status == PJSUA_CALL_MEDIA_REMOTE_HOLD)
    {
	pj_bool_t connect_sound = PJ_TRUE;
	pj_bool_t disconnect_mic = PJ_FALSE;
	pjsua_conf_port_id call_conf_slot;

	call_conf_slot = ci->media[mi].stream.aud.conf_slot;

	/* Loopback sound, if desired */
	if (app_config.auto_loop) {
	    pjsua_conf_connect(call_conf_slot, call_conf_slot);
	    connect_sound = PJ_FALSE;
	}

	/* Automatically record conversation, if desired */
	if (app_config.auto_rec && app_config.rec_port != PJSUA_INVALID_ID) {
	    pjsua_conf_connect(call_conf_slot, app_config.rec_port);
	}

	/* Stream a file, if desired */
	if ((app_config.auto_play || app_config.auto_play_hangup) && 
	    app_config.wav_port != PJSUA_INVALID_ID)
	{
	    pjsua_conf_connect(app_config.wav_port, call_conf_slot);
	    connect_sound = PJ_FALSE;
	}

	/* Stream AVI, if desired */
	if (app_config.avi_auto_play &&
	    app_config.avi_def_idx != PJSUA_INVALID_ID &&
	    app_config.avi[app_config.avi_def_idx].slot != PJSUA_INVALID_ID)
	{
	    pjsua_conf_connect(app_config.avi[app_config.avi_def_idx].slot,
			       call_conf_slot);
	    disconnect_mic = PJ_TRUE;
	}

	/* Put call in conference with other calls, if desired */
	if (app_config.auto_conf) {
	    pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	    unsigned call_cnt=PJ_ARRAY_SIZE(call_ids);
	    unsigned i;

	    /* Get all calls, and establish media connection between
	     * this call and other calls.
	     */
	    pjsua_enum_calls(call_ids, &call_cnt);

	    for (i=0; i<call_cnt; ++i) {
		if (call_ids[i] == ci->id)
		    continue;
		
		if (!pjsua_call_has_media(call_ids[i]))
		    continue;

		pjsua_conf_connect(call_conf_slot,
				   pjsua_call_get_conf_port(call_ids[i]));
		pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]),
		                   call_conf_slot);

		/* Automatically record conversation, if desired */
		if (app_config.auto_rec && app_config.rec_port !=
					   PJSUA_INVALID_ID)
		{
		    pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]), 
				       app_config.rec_port);
		}

	    }

	    /* Also connect call to local sound device */
	    connect_sound = PJ_TRUE;
	}

	/* Otherwise connect to sound device */
	if (connect_sound) {
	    pjsua_conf_connect(call_conf_slot, 0);
	    if (!disconnect_mic)
		pjsua_conf_connect(0, call_conf_slot);

	    /* Automatically record conversation, if desired */
	    if (app_config.auto_rec && app_config.rec_port != PJSUA_INVALID_ID)
	    {
		pjsua_conf_connect(call_conf_slot, app_config.rec_port);
		pjsua_conf_connect(0, app_config.rec_port);
	    }
	}
    }
}

/*
 * Callback on media state changed event.
 * The action may connect the call to sound device, to file, or
 * to loop the call.
 */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info call_info;
    unsigned mi;
    pj_bool_t has_error = PJ_FALSE;

    pjsua_call_get_info(call_id, &call_info);

    for (mi=0; mi<call_info.media_cnt; ++mi) {
	on_call_generic_media_state(&call_info, mi, &has_error);

	switch (call_info.media[mi].type) {
	case PJMEDIA_TYPE_AUDIO:
	    on_call_audio_state(&call_info, mi, &has_error);
	    break;
	default:
	    /* Make gcc happy about enum not handled by switch/case */
	    break;
	}
    }

    if (has_error) {
	pj_str_t reason = pj_str("Media failed");
	pjsua_call_hangup(call_id, 500, &reason, NULL);
    }
}

/*
 * DTMF callback.
 */
static void call_on_dtmf_callback(pjsua_call_id call_id, int dtmf)
{
    PJ_LOG(3,(THIS_FILE, "Incoming DTMF on call %d: %c", call_id, dtmf));
}


/*
 * Handler registration status has changed.
 */
static void on_reg_state2(pjsua_acc_id acc_id , pjsua_reg_info *info)
{
	SipCaller::getSipUser()->onRegisterStat(info->cbparam->code);
}

/**
 * Call transfer request status.
 */
static void on_call_transfer_status(pjsua_call_id call_id,
				    int status_code,
				    const pj_str_t *status_text,
				    pj_bool_t final,
				    pj_bool_t *p_cont)
{
    PJ_LOG(3,(THIS_FILE, "Call %d: transfer status=%d (%.*s) %s",
	      call_id, status_code,
	      (int)status_text->slen, status_text->ptr,
	      (final ? "[final]" : "")));

    if (status_code/100 == 2) {
	PJ_LOG(3,(THIS_FILE, 
	          "Call %d: call transferred successfully, disconnecting call",
		  call_id));
	pjsua_call_hangup(call_id, PJSIP_SC_GONE, NULL, NULL);
	*p_cont = PJ_FALSE;
    }
}


/*
 * NAT type detection callback.
 */
static void on_nat_detect(const pj_stun_nat_detect_result *res)
{
    if (res->status != PJ_SUCCESS) {
	pjsua_perror(THIS_FILE, "NAT detection failed", res->status);
    } else {
	PJ_LOG(3, (THIS_FILE, "NAT detected as %s", res->nat_type_name));
    }
}

/*
 * Transport status notification
 */
static void on_transport_state(pjsip_transport *tp, 
			       pjsip_transport_state state,
			       const pjsip_transport_state_info *info)
{
    char host_port[128];

    pj_ansi_snprintf(host_port, sizeof(host_port), "[%.*s:%d]",
		     (int)tp->remote_name.host.slen,
		     tp->remote_name.host.ptr,
		     tp->remote_name.port);

    switch (state) {
    case PJSIP_TP_STATE_CONNECTED:
	{
	    PJ_LOG(3,(THIS_FILE, "SIP %s transport is connected to %s",
		     tp->type_name, host_port));
	}
	break;

    case PJSIP_TP_STATE_DISCONNECTED:
	{
	    char buf[100];

	    snprintf(buf, sizeof(buf), "SIP %s transport is disconnected "
		    "from %s", tp->type_name, host_port);
	    pjsua_perror(THIS_FILE, buf, info->status);
	}
	break;

    default:
	break;
    }

#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0

    if (!pj_ansi_stricmp(tp->type_name, "tls") && info->ext_info &&
	(state == PJSIP_TP_STATE_CONNECTED || 
	 ((pjsip_tls_state_info*)info->ext_info)->
			         ssl_sock_info->verify_status != PJ_SUCCESS))
    {
	pjsip_tls_state_info *tls_info = (pjsip_tls_state_info*)info->ext_info;
	pj_ssl_sock_info *ssl_sock_info = tls_info->ssl_sock_info;
	char buf[2048];
	const char *verif_msgs[32];
	unsigned verif_msg_cnt;

	/* Dump server TLS cipher */
	PJ_LOG(4,(THIS_FILE, "TLS cipher used: 0x%06X/%s",
		  ssl_sock_info->cipher,
		  pj_ssl_cipher_name(ssl_sock_info->cipher) ));

	/* Dump server TLS certificate */
	pj_ssl_cert_info_dump(ssl_sock_info->remote_cert_info, "  ",
			      buf, sizeof(buf));
	PJ_LOG(4,(THIS_FILE, "TLS cert info of %s:\n%s", host_port, buf));

	/* Dump server TLS certificate verification result */
	verif_msg_cnt = PJ_ARRAY_SIZE(verif_msgs);
	pj_ssl_cert_get_verify_status_strings(ssl_sock_info->verify_status,
					      verif_msgs, &verif_msg_cnt);
	PJ_LOG(3,(THIS_FILE, "TLS cert verification result of %s : %s",
			     host_port,
			     (verif_msg_cnt == 1? verif_msgs[0]:"")));
	if (verif_msg_cnt > 1) {
	    unsigned i;
	    for (i = 0; i < verif_msg_cnt; ++i)
		PJ_LOG(3,(THIS_FILE, "- %s", verif_msgs[i]));
	}

	if (ssl_sock_info->verify_status &&
	    !app_config.udp_cfg.tls_setting.verify_server) 
	{
	    PJ_LOG(3,(THIS_FILE, "PJSUA is configured to ignore TLS cert "
				 "verification errors"));
	}
    }

#endif

}

/*
 * Notification on ICE error.
 */
static void on_ice_transport_error(int index, pj_ice_strans_op op,
				   pj_status_t status, void *param)
{
    PJ_UNUSED_ARG(op);
    PJ_UNUSED_ARG(param);
    PJ_PERROR(1,(THIS_FILE, status,
	         "ICE keep alive failure for transport %d", index));
}

/*
 * A simple registrar, invoked by default_mod_on_rx_request()
 */
static void simple_registrar(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    const pjsip_expires_hdr *exp;
    const pjsip_hdr *h;
    unsigned cnt = 0;
    pjsip_generic_string_hdr *srv;
    pj_status_t status;

    status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(),
					 rdata, 200, NULL, &tdata);
    if (status != PJ_SUCCESS)
    return;

    exp = (pjsip_expires_hdr *)pjsip_msg_find_hdr(rdata->msg_info.msg, 
						  PJSIP_H_EXPIRES, NULL);

    h = rdata->msg_info.msg->hdr.next;
    while (h != &rdata->msg_info.msg->hdr) {
	if (h->type == PJSIP_H_CONTACT) {
	    const pjsip_contact_hdr *c = (const pjsip_contact_hdr*)h;
	    int e = c->expires;

	    if (e < 0) {
		if (exp)
		    e = exp->ivalue;
		else
		    e = 3600;
	    }

	    if (e > 0) {
		pjsip_contact_hdr *nc = (pjsip_contact_hdr *)pjsip_hdr_clone(
								tdata->pool, h);
		nc->expires = e;
		pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)nc);
		++cnt;
	    }
	}
	h = h->next;
    }

    srv = pjsip_generic_string_hdr_create(tdata->pool, NULL, NULL);
    srv->name = pj_str("Server");
    srv->hvalue = pj_str("pjsua simple registrar");
    pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)srv);

    pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(),
		       rdata, tdata, NULL, NULL);
}

/*****************************************************************************
 * A simple module to handle otherwise unhandled request. We will register
 * this with the lowest priority.
 */

/* Notification on incoming request */
static pj_bool_t default_mod_on_rx_request(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    pjsip_status_code status_code;
    pj_status_t status;

    /* Don't respond to ACK! */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
			 &pjsip_ack_method) == 0)
	return PJ_TRUE;

    /* Simple registrar */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
                         &pjsip_register_method) == 0)
    {
	simple_registrar(rdata);
	return PJ_TRUE;
    }

    /* Create basic response. */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method, 
			 &pjsip_notify_method) == 0)
    {
	/* Unsolicited NOTIFY's, send with Bad Request */
	status_code = PJSIP_SC_BAD_REQUEST;
    } else {
	/* Probably unknown method */
	status_code = PJSIP_SC_METHOD_NOT_ALLOWED;
    }
    status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(), 
					 rdata, status_code, 
					 NULL, &tdata);
    if (status != PJ_SUCCESS) {
	pjsua_perror(THIS_FILE, "Unable to create response", status);
	return PJ_TRUE;
    }

    /* Add Allow if we're responding with 405 */
    if (status_code == PJSIP_SC_METHOD_NOT_ALLOWED) {
	const pjsip_hdr *cap_hdr;
	cap_hdr = pjsip_endpt_get_capability(pjsua_get_pjsip_endpt(), 
					     PJSIP_H_ALLOW, NULL);
	if (cap_hdr) {
	    pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr *)pjsip_hdr_clone(
							 tdata->pool, cap_hdr));
	}
    }

    /* Add User-Agent header */
    {
	pj_str_t user_agent;
	char tmp[80];
	const pj_str_t USER_AGENT = { "User-Agent", 10};
	pjsip_hdr *h;

	pj_ansi_snprintf(tmp, sizeof(tmp), "PJSUA v%s/%s", 
			 pj_get_version(), PJ_OS_NAME);
	pj_strdup2_with_null(tdata->pool, &user_agent, tmp);

	h = (pjsip_hdr*) pjsip_generic_string_hdr_create(tdata->pool,
							 &USER_AGENT,
							 &user_agent);
	pjsip_msg_add_hdr(tdata->msg, h);
    }

    pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(), rdata, tdata, 
			       NULL, NULL);

    return PJ_TRUE;
}

/* The module instance. */
static pjsip_module mod_default_handler = 
{
    NULL, NULL,				/* prev, next.		*/
    { "mod-default-handler", 19 },	/* Name.		*/
    -1,					/* Id			*/
    PJSIP_MOD_PRIORITY_APPLICATION+99,	/* Priority	        */
    NULL,				/* load()		*/
    NULL,				/* start()		*/
    NULL,				/* stop()		*/
    NULL,				/* unload()		*/
    &default_mod_on_rx_request,		/* on_rx_request()	*/
    NULL,				/* on_rx_response()	*/
    NULL,				/* on_tx_request.	*/
    NULL,				/* on_tx_response()	*/
    NULL,				/* on_tsx_state()	*/

};

static pj_status_t app_init()
{
    pjsua_transport_id transport_id = -1;
    pjsua_transport_config tcp_cfg;
    unsigned i;
    pj_pool_t *tmp_pool;
    pj_status_t status;

    /** Create pjsua **/
    status = pjsua_create();
    if (status != PJ_SUCCESS)
	return status;

    /* Create pool for application */
    app_config.pool = pjsua_pool_create("pjsua-app", 1000, 1000);
    tmp_pool = pjsua_pool_create("tmp-pjsua", 1000, 1000);;

    /** Parse args **/
    status = load_config(app_cfg.argc, app_cfg.argv, &uri_arg);
    if (status != PJ_SUCCESS) {
	pj_pool_release(tmp_pool);
	return status;
    }
	
	callback_player_func = &on_player_func;

    /* Initialize application callbacks */
    app_config.cfg.cb.on_call_state = &on_call_state;
    app_config.cfg.cb.on_call_media_state = &on_call_media_state;
    app_config.cfg.cb.on_incoming_call = &on_incoming_call;
    app_config.cfg.cb.on_call_tsx_state = &on_call_tsx_state;
    app_config.cfg.cb.on_dtmf_digit = &call_on_dtmf_callback;
	app_config.cfg.cb.on_reg_state2 = &on_reg_state2;
    app_config.cfg.cb.on_call_transfer_status = &on_call_transfer_status;
    app_config.cfg.cb.on_nat_detect = &on_nat_detect;
    app_config.cfg.cb.on_transport_state = &on_transport_state;
    app_config.cfg.cb.on_ice_transport_error = &on_ice_transport_error;

    /* Set sound device latency */
    if (app_config.capture_lat > 0)
	app_config.media_cfg.snd_rec_latency = app_config.capture_lat;
    if (app_config.playback_lat)
	app_config.media_cfg.snd_play_latency = app_config.playback_lat;

    if (app_cfg.on_config_init)
	(*app_cfg.on_config_init)(&app_config);

    /* Initialize pjsua */
    status = pjsua_init(&app_config.cfg, &app_config.log_cfg,
			&app_config.media_cfg);
    if (status != PJ_SUCCESS) {
	pj_pool_release(tmp_pool);
	return status;
    }

    /* Initialize our module to handle otherwise unhandled request */
    status = pjsip_endpt_register_module(pjsua_get_pjsip_endpt(),
					 &mod_default_handler);
    if (status != PJ_SUCCESS)
	return status;

    /* Initialize calls data */
    for (i=0; i<PJ_ARRAY_SIZE(app_config.call_data); ++i) {
	app_config.call_data[i].timer.id = PJSUA_INVALID_ID;
	app_config.call_data[i].timer.cb = &call_timeout_callback;
    }

    pj_memcpy(&tcp_cfg, &app_config.udp_cfg, sizeof(tcp_cfg));

    /* Add UDP transport unless it's disabled. */
    if (!app_config.no_udp) {
	pjsua_acc_id aid;
	pjsip_transport_type_e type = PJSIP_TRANSPORT_UDP;

	status = pjsua_transport_create(type,
					&app_config.udp_cfg,
					&transport_id);
	if (status != PJ_SUCCESS)
	    goto on_error;

	/* Add local account */
	pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

	/* Adjust local account config based on pjsua app config */
	{
	    pjsua_acc_config acc_cfg;
	    pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);
	    acc_cfg.rtp_cfg = app_config.rtp_cfg;
	    pjsua_acc_modify(aid, &acc_cfg);
	}

	pjsua_acc_set_online_status(current_acc, PJ_TRUE);

	if (app_config.udp_cfg.port == 0) {
	    pjsua_transport_info ti;
	    pj_sockaddr_in *a;

	    pjsua_transport_get_info(transport_id, &ti);
	    a = (pj_sockaddr_in*)&ti.local_addr;

	    tcp_cfg.port = pj_ntohs(a->sin_port);
	}
    }

    /* Add TCP transport unless it's disabled */
    if (!app_config.no_tcp) {
	pjsua_acc_id aid;

	status = pjsua_transport_create(PJSIP_TRANSPORT_TCP,
					&tcp_cfg, 
					&transport_id);
	if (status != PJ_SUCCESS)
	    goto on_error;

	/* Add local account */
	pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

	/* Adjust local account config based on pjsua app config */
	{
	    pjsua_acc_config acc_cfg;
	    pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);
	    acc_cfg.rtp_cfg = app_config.rtp_cfg;
	    pjsua_acc_modify(aid, &acc_cfg);
	}

	pjsua_acc_set_online_status(current_acc, PJ_TRUE);

    }


#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0
    /* Add TLS transport when application wants one */
    if (app_config.use_tls) {

	pjsua_acc_id acc_id;

	/* Copy the QoS settings */
	tcp_cfg.tls_setting.qos_type = tcp_cfg.qos_type;
	pj_memcpy(&tcp_cfg.tls_setting.qos_params, &tcp_cfg.qos_params, 
		  sizeof(tcp_cfg.qos_params));

	/* Set TLS port as TCP port+1 */
	tcp_cfg.port++;
	status = pjsua_transport_create(PJSIP_TRANSPORT_TLS,
					&tcp_cfg, 
					&transport_id);
	tcp_cfg.port--;
	if (status != PJ_SUCCESS)
	    goto on_error;
	
	/* Add local account */
	pjsua_acc_add_local(transport_id, PJ_FALSE, &acc_id);

	/* Adjust local account config based on pjsua app config */
	{
	    pjsua_acc_config acc_cfg;
	    pjsua_acc_get_config(acc_id, tmp_pool, &acc_cfg);
	    acc_cfg.rtp_cfg = app_config.rtp_cfg;
	    pjsua_acc_modify(acc_id, &acc_cfg);
	}

	pjsua_acc_set_online_status(acc_id, PJ_TRUE);
    }
#endif

    if (transport_id == -1) {
	PJ_LOG(1,(THIS_FILE, "Error: no transport is configured"));
	status = -1;
	goto on_error;
    }

    if (app_config.capture_dev  != PJSUA_INVALID_ID ||
        app_config.playback_dev != PJSUA_INVALID_ID) 
    {
	status = pjsua_set_snd_dev(app_config.capture_dev, 
				   app_config.playback_dev);
	if (status != PJ_SUCCESS)
	    goto on_error;
    }

    /* Init call setting */
    pjsua_call_setting_default(&call_opt);
    call_opt.aud_cnt = app_config.aud_cnt;
    call_opt.vid_cnt = app_config.vid.vid_cnt;

    pj_pool_release(tmp_pool);
    return PJ_SUCCESS;

on_error:
    pj_pool_release(tmp_pool);
    app_destroy();
    return status;
}

pj_status_t pjsua_app_init(const pjsua_app_cfg_t *cfg)
{
    pj_status_t status;
    pj_memcpy(&app_cfg, cfg, sizeof(app_cfg));

    status = app_init();
    return status;
}

pj_status_t pjsua_app_run(pj_bool_t wait_telnet_cli)
{
    pj_thread_t *stdout_refresh_thread = NULL;
    pj_status_t status;

    status = pjsua_start();
    if (status != PJ_SUCCESS)
	{
		return status;
	}

	if (app_cfg.on_started) {
	    (*app_cfg.on_started)(status, "Ready");
	}

	legacy_main();
    return status;
}

static pj_status_t app_destroy()
{
    pj_status_t status = PJ_SUCCESS;
    unsigned i;
    int cli_fe = 0;
    pj_uint16_t cli_telnet_port = 0;

    /* Close avi devs and ports */
    for (i=0; i<app_config.avi_cnt; ++i) {
	if (app_config.avi[i].slot != PJSUA_INVALID_ID)
	    pjsua_conf_remove_port(app_config.avi[i].slot);
#if PJMEDIA_HAS_VIDEO && PJMEDIA_VIDEO_DEV_HAS_AVI
	if (app_config.avi[i].dev_id != PJMEDIA_VID_INVALID_DEV)
	    pjmedia_avi_dev_free(app_config.avi[i].dev_id);
#endif
    }

    /* Close ringback port */
    if (app_config.ringback_port && 
	app_config.ringback_slot != PJSUA_INVALID_ID) 
    {
	pjsua_conf_remove_port(app_config.ringback_slot);
	app_config.ringback_slot = PJSUA_INVALID_ID;
	pjmedia_port_destroy(app_config.ringback_port);
	app_config.ringback_port = NULL;
    }

    /* Close ring port */
    if (app_config.ring_port && app_config.ring_slot != PJSUA_INVALID_ID) {
	pjsua_conf_remove_port(app_config.ring_slot);
	app_config.ring_slot = PJSUA_INVALID_ID;
	pjmedia_port_destroy(app_config.ring_port);
	app_config.ring_port = NULL;
    }

    /* Close tone generators */
    for (i=0; i<app_config.tone_count; ++i) {
	pjsua_conf_remove_port(app_config.tone_slots[i]);
    }

    if (app_config.pool) {
	pj_pool_release(app_config.pool);
	app_config.pool = NULL;
    }

    status = pjsua_destroy();

    /* Reset config */
    pj_bzero(&app_config, sizeof(app_config));

    return status;
}

pj_status_t pjsua_app_destroy()
{
    pj_status_t status;
    status = app_destroy(); 
    return status;
}
