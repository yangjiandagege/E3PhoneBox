/* $Id: pjsua_app_config.c 4868 2014-07-02 18:07:12Z bennylp $ */
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
#include "pjsua_app_common.h"

#define THIS_FILE	"pjsua_app_config.c"

/*
 * Read command arguments from config file.
 */
static int read_config_file(pj_pool_t *pool, const char *filename,
			    int *app_argc, char ***app_argv)
{
    int i;
    FILE *fhnd;
    char line[200];
    int argc = 0;
    char **argv;
    enum { MAX_ARGS = 128 };

    /* Allocate MAX_ARGS+1 (argv needs to be terminated with NULL argument) */
    argv = pj_pool_calloc(pool, MAX_ARGS+1, sizeof(char*));
    argv[argc++] = *app_argv[0];

    /* Open config file. */
    fhnd = fopen(filename, "rt");
    if (!fhnd) {
	PJ_LOG(1,(THIS_FILE, "Unable to open config file %s", filename));
	fflush(stdout);
	return -1;
    }

    /* Scan tokens in the file. */
    while (argc < MAX_ARGS && !feof(fhnd)) {
	char  *token;
	char  *p;
	const char *whitespace = " \t\r\n";
	char  cDelimiter;
	pj_size_t   len;
	int token_len;

	pj_bzero(line, sizeof(line));
	if (fgets(line, sizeof(line), fhnd) == NULL) break;

	// Trim ending newlines
	len = strlen(line);
	if (line[len-1]=='\n')
	    line[--len] = '\0';
	if (line[len-1]=='\r')
	    line[--len] = '\0';

	if (len==0) continue;

	for (p = line; *p != '\0' && argc < MAX_ARGS; p++) {
	    // first, scan whitespaces
	    while (*p != '\0' && strchr(whitespace, *p) != NULL) p++;

	    if (*p == '\0')		    // are we done yet?
		break;

	    if (*p == '"' || *p == '\'') {    // is token a quoted string
		cDelimiter = *p++;	    // save quote delimiter
		token = p;

		while (*p != '\0' && *p != cDelimiter) p++;

		if (*p == '\0')		// found end of the line, but,
		    cDelimiter = '\0';	// didn't find a matching quote

	    } else {			// token's not a quoted string
		token = p;

		while (*p != '\0' && strchr(whitespace, *p) == NULL) p++;

		cDelimiter = *p;
	    }

	    *p = '\0';
	    token_len = (int)(p-token);

	    if (token_len > 0) {
		if (*token == '#')
		    break;  // ignore remainder of line

		argv[argc] = pj_pool_alloc(pool, token_len + 1);
		pj_memcpy(argv[argc], token, token_len + 1);
		++argc;
	    }

	    *p = cDelimiter;
	}
    }

    /* Copy arguments from command line */
    for (i=1; i<*app_argc && argc < MAX_ARGS; ++i)
	argv[argc++] = (*app_argv)[i];

    if (argc == MAX_ARGS && (i!=*app_argc || !feof(fhnd))) {
	PJ_LOG(1,(THIS_FILE,
		  "Too many arguments specified in cmd line/config file"));
	fflush(stdout);
	fclose(fhnd);
	return -1;
    }

    fclose(fhnd);

    /* Assign the new command line back to the original command line. */
    *app_argc = argc;
    *app_argv = argv;
    return 0;
}

/* Parse arguments. */
static pj_status_t parse_args(int argc, char *argv[],
			      pj_str_t *uri_to_call)
{
    int c;
    int option_index;
    pjsua_app_config *cfg = &app_config;
    enum { OPT_CONFIG_FILE=127, OPT_LOG_FILE, OPT_LOG_LEVEL, OPT_APP_LOG_LEVEL,
	   OPT_LOG_APPEND, OPT_COLOR, OPT_NO_COLOR, OPT_LIGHT_BG, OPT_NO_STDERR,
	   OPT_HELP, OPT_VERSION, OPT_NULL_AUDIO, OPT_SND_AUTO_CLOSE,
	   OPT_LOCAL_PORT, OPT_IP_ADDR, OPT_PROXY, OPT_OUTBOUND_PROXY,
	   OPT_REGISTRAR, OPT_REG_TIMEOUT, OPT_PUBLISH, OPT_ID, OPT_CONTACT,
	   OPT_BOUND_ADDR, OPT_CONTACT_PARAMS, OPT_CONTACT_URI_PARAMS,
	   OPT_100REL, OPT_USE_IMS, OPT_REALM, OPT_USERNAME, OPT_PASSWORD,
	   OPT_REG_RETRY_INTERVAL, OPT_REG_USE_PROXY,
	   OPT_MWI, OPT_NAMESERVER, OPT_STUN_SRV, OPT_OUTB_RID,
	   OPT_ADD_BUDDY, OPT_OFFER_X_MS_MSG, OPT_NO_PRESENCE,
	   OPT_AUTO_ANSWER, OPT_AUTO_PLAY, OPT_AUTO_PLAY_HANGUP, OPT_AUTO_LOOP,
	   OPT_AUTO_CONF, OPT_CLOCK_RATE, OPT_SND_CLOCK_RATE, OPT_STEREO,
	   OPT_USE_ICE, OPT_ICE_REGULAR, OPT_USE_SRTP, OPT_SRTP_SECURE,
	   OPT_USE_TURN, OPT_ICE_MAX_HOSTS, OPT_ICE_NO_RTCP, OPT_TURN_SRV,
	   OPT_TURN_TCP, OPT_TURN_USER, OPT_TURN_PASSWD,
	   OPT_PLAY_FILE, OPT_PLAY_TONE, OPT_RTP_PORT, OPT_ADD_CODEC,
	   OPT_ILBC_MODE, OPT_REC_FILE, OPT_AUTO_REC,
	   OPT_COMPLEXITY, OPT_QUALITY, OPT_PTIME, OPT_NO_VAD,
	   OPT_RX_DROP_PCT, OPT_TX_DROP_PCT, OPT_EC_TAIL, OPT_EC_OPT,
	   OPT_NEXT_ACCOUNT, OPT_NEXT_CRED, OPT_MAX_CALLS,
	   OPT_DURATION, OPT_NO_TCP, OPT_NO_UDP, OPT_THREAD_CNT,
	   OPT_NOREFERSUB, OPT_ACCEPT_REDIRECT,
	   OPT_USE_TLS, OPT_TLS_CA_FILE, OPT_TLS_CERT_FILE, OPT_TLS_PRIV_FILE,
	   OPT_TLS_PASSWORD, OPT_TLS_VERIFY_SERVER, OPT_TLS_VERIFY_CLIENT,
	   OPT_TLS_NEG_TIMEOUT, OPT_TLS_CIPHER,
	   OPT_CAPTURE_DEV, OPT_PLAYBACK_DEV,
	   OPT_CAPTURE_LAT, OPT_PLAYBACK_LAT, OPT_NO_TONES, OPT_JB_MAX_SIZE,
	   OPT_STDOUT_REFRESH, OPT_STDOUT_REFRESH_TEXT, OPT_IPV6, OPT_QOS,
#ifdef _IONBF
	   OPT_STDOUT_NO_BUF,
#endif
	   OPT_AUTO_UPDATE_NAT,OPT_USE_COMPACT_FORM,OPT_DIS_CODEC,
	   OPT_DISABLE_STUN, OPT_NO_FORCE_LR,
	   OPT_TIMER, OPT_TIMER_SE, OPT_TIMER_MIN_SE,
	   OPT_VIDEO, OPT_EXTRA_AUDIO,
	   OPT_VCAPTURE_DEV, OPT_VRENDER_DEV, OPT_PLAY_AVI, OPT_AUTO_PLAY_AVI,
	   OPT_USE_CLI, OPT_CLI_TELNET_PORT, OPT_DISABLE_CLI_CONSOLE
    };
    struct pj_getopt_option long_options[] = {
	{ "config-file",1, 0, OPT_CONFIG_FILE},
	{ "log-file",	1, 0, OPT_LOG_FILE},
	{ "log-level",	1, 0, OPT_LOG_LEVEL},
	{ "app-log-level",1,0,OPT_APP_LOG_LEVEL},
	{ "log-append", 0, 0, OPT_LOG_APPEND},
	{ "color",	0, 0, OPT_COLOR},
	{ "no-color",	0, 0, OPT_NO_COLOR},
	{ "light-bg",		0, 0, OPT_LIGHT_BG},
	{ "no-stderr",  0, 0, OPT_NO_STDERR},
	{ "help",	0, 0, OPT_HELP},
	{ "version",	0, 0, OPT_VERSION},
	{ "clock-rate",	1, 0, OPT_CLOCK_RATE},
	{ "snd-clock-rate",	1, 0, OPT_SND_CLOCK_RATE},
	{ "stereo",	0, 0, OPT_STEREO},
	{ "null-audio", 0, 0, OPT_NULL_AUDIO},
	{ "local-port", 1, 0, OPT_LOCAL_PORT},
	{ "ip-addr",	1, 0, OPT_IP_ADDR},
	{ "bound-addr", 1, 0, OPT_BOUND_ADDR},
	{ "no-tcp",     0, 0, OPT_NO_TCP},
	{ "no-udp",     0, 0, OPT_NO_UDP},
	{ "norefersub", 0, 0, OPT_NOREFERSUB},
	{ "proxy",	1, 0, OPT_PROXY},
	{ "outbound",	1, 0, OPT_OUTBOUND_PROXY},
	{ "registrar",	1, 0, OPT_REGISTRAR},
	{ "reg-timeout",1, 0, OPT_REG_TIMEOUT},
	{ "publish",    0, 0, OPT_PUBLISH},
	{ "mwi",	0, 0, OPT_MWI},
	{ "use-100rel", 0, 0, OPT_100REL},
	{ "use-ims",    0, 0, OPT_USE_IMS},
	{ "id",		1, 0, OPT_ID},
	{ "contact",	1, 0, OPT_CONTACT},
	{ "contact-params",1,0, OPT_CONTACT_PARAMS},
	{ "contact-uri-params",1,0, OPT_CONTACT_URI_PARAMS},
	{ "auto-update-nat",	1, 0, OPT_AUTO_UPDATE_NAT},
	{ "disable-stun",0,0, OPT_DISABLE_STUN},
        { "use-compact-form",	0, 0, OPT_USE_COMPACT_FORM},
	{ "accept-redirect", 1, 0, OPT_ACCEPT_REDIRECT},
	{ "no-force-lr",0, 0, OPT_NO_FORCE_LR},
	{ "realm",	1, 0, OPT_REALM},
	{ "username",	1, 0, OPT_USERNAME},
	{ "password",	1, 0, OPT_PASSWORD},
	{ "rereg-delay",1, 0, OPT_REG_RETRY_INTERVAL},
	{ "reg-use-proxy", 1, 0, OPT_REG_USE_PROXY},
	{ "nameserver", 1, 0, OPT_NAMESERVER},
	{ "stun-srv",   1, 0, OPT_STUN_SRV},
	{ "add-buddy",  1, 0, OPT_ADD_BUDDY},
	{ "offer-x-ms-msg",0,0,OPT_OFFER_X_MS_MSG},
	{ "no-presence", 0, 0, OPT_NO_PRESENCE},
	{ "auto-answer",1, 0, OPT_AUTO_ANSWER},
	{ "auto-play",  0, 0, OPT_AUTO_PLAY},
	{ "auto-play-hangup",0, 0, OPT_AUTO_PLAY_HANGUP},
	{ "auto-rec",   0, 0, OPT_AUTO_REC},
	{ "auto-loop",  0, 0, OPT_AUTO_LOOP},
	{ "auto-conf",  0, 0, OPT_AUTO_CONF},
	{ "play-file",  1, 0, OPT_PLAY_FILE},
	{ "play-tone",  1, 0, OPT_PLAY_TONE},
	{ "rec-file",   1, 0, OPT_REC_FILE},
	{ "rtp-port",	1, 0, OPT_RTP_PORT},

	{ "use-ice",    0, 0, OPT_USE_ICE},
	{ "ice-regular",0, 0, OPT_ICE_REGULAR},
	{ "use-turn",	0, 0, OPT_USE_TURN},
	{ "ice-max-hosts",1, 0, OPT_ICE_MAX_HOSTS},
	{ "ice-no-rtcp",0, 0, OPT_ICE_NO_RTCP},
	{ "turn-srv",	1, 0, OPT_TURN_SRV},
	{ "turn-tcp",	0, 0, OPT_TURN_TCP},
	{ "turn-user",	1, 0, OPT_TURN_USER},
	{ "turn-passwd",1, 0, OPT_TURN_PASSWD},

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	{ "use-srtp",   1, 0, OPT_USE_SRTP},
	{ "srtp-secure",1, 0, OPT_SRTP_SECURE},
#endif
	{ "add-codec",  1, 0, OPT_ADD_CODEC},
	{ "dis-codec",  1, 0, OPT_DIS_CODEC},
	{ "complexity",	1, 0, OPT_COMPLEXITY},
	{ "quality",	1, 0, OPT_QUALITY},
	{ "ptime",      1, 0, OPT_PTIME},
	{ "no-vad",     0, 0, OPT_NO_VAD},
	{ "ec-tail",    1, 0, OPT_EC_TAIL},
	{ "ec-opt",	1, 0, OPT_EC_OPT},
	{ "ilbc-mode",	1, 0, OPT_ILBC_MODE},
	{ "rx-drop-pct",1, 0, OPT_RX_DROP_PCT},
	{ "tx-drop-pct",1, 0, OPT_TX_DROP_PCT},
	{ "next-account",0,0, OPT_NEXT_ACCOUNT},
	{ "next-cred",	0, 0, OPT_NEXT_CRED},
	{ "max-calls",	1, 0, OPT_MAX_CALLS},
	{ "duration",	1, 0, OPT_DURATION},
	{ "thread-cnt",	1, 0, OPT_THREAD_CNT},
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	{ "use-tls",	0, 0, OPT_USE_TLS},
	{ "tls-ca-file",1, 0, OPT_TLS_CA_FILE},
	{ "tls-cert-file",1,0, OPT_TLS_CERT_FILE},
	{ "tls-privkey-file",1,0, OPT_TLS_PRIV_FILE},
	{ "tls-password",1,0, OPT_TLS_PASSWORD},
	{ "tls-verify-server", 0, 0, OPT_TLS_VERIFY_SERVER},
	{ "tls-verify-client", 0, 0, OPT_TLS_VERIFY_CLIENT},
	{ "tls-neg-timeout", 1, 0, OPT_TLS_NEG_TIMEOUT},
	{ "tls-cipher", 1, 0, OPT_TLS_CIPHER},
#endif
	{ "capture-dev",    1, 0, OPT_CAPTURE_DEV},
	{ "playback-dev",   1, 0, OPT_PLAYBACK_DEV},
	{ "capture-lat",    1, 0, OPT_CAPTURE_LAT},
	{ "playback-lat",   1, 0, OPT_PLAYBACK_LAT},
	{ "stdout-refresh", 1, 0, OPT_STDOUT_REFRESH},
	{ "stdout-refresh-text", 1, 0, OPT_STDOUT_REFRESH_TEXT},
#ifdef _IONBF
	{ "stdout-no-buf",  0, 0, OPT_STDOUT_NO_BUF },
#endif
	{ "snd-auto-close", 1, 0, OPT_SND_AUTO_CLOSE},
	{ "no-tones",    0, 0, OPT_NO_TONES},
	{ "jb-max-size", 1, 0, OPT_JB_MAX_SIZE},
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	{ "ipv6",	 0, 0, OPT_IPV6},
#endif
	{ "set-qos",	 0, 0, OPT_QOS},
	{ "use-timer",  1, 0, OPT_TIMER},
	{ "timer-se",   1, 0, OPT_TIMER_SE},
	{ "timer-min-se", 1, 0, OPT_TIMER_MIN_SE},
	{ "outb-rid",	1, 0, OPT_OUTB_RID},
	{ "video",	0, 0, OPT_VIDEO},
	{ "extra-audio",0, 0, OPT_EXTRA_AUDIO},
	{ "vcapture-dev", 1, 0, OPT_VCAPTURE_DEV},
	{ "vrender-dev",  1, 0, OPT_VRENDER_DEV},
	{ "play-avi",	1, 0, OPT_PLAY_AVI},
	{ "auto-play-avi", 0, 0, OPT_AUTO_PLAY_AVI},
	{ "use-cli",	0, 0, OPT_USE_CLI},
	{ "cli-telnet-port", 1, 0, OPT_CLI_TELNET_PORT},
	{ "no-cli-console", 0, 0, OPT_DISABLE_CLI_CONSOLE},
	{ NULL, 0, 0, 0}
    };
    pj_status_t status;
    pjsua_acc_config *cur_acc;
    char *config_file = NULL;
    unsigned i;

    /* Run pj_getopt once to see if user specifies config file to read. */
    pj_optind = 0;
    while ((c=pj_getopt_long(argc, argv, "", long_options,
			     &option_index)) != -1)
    {
	switch (c) {
	case OPT_CONFIG_FILE:
	    config_file = pj_optarg;
	    break;
	}
	if (config_file)
	    break;
    }

    if (config_file) {
	status = read_config_file(cfg->pool, config_file, &argc, &argv);
	if (status != 0)
	    return status;
    }

    cfg->acc_cnt = 0;
    cur_acc = cfg->acc_cfg;


    /* Reinitialize and re-run pj_getopt again, possibly with new arguments
     * read from config file.
     */
    pj_optind = 0;
    while((c=pj_getopt_long(argc,argv, "", long_options,&option_index))!=-1) {
	pj_str_t tmp;
	long lval;

	switch (c) {

	case OPT_CONFIG_FILE:
	    /* Ignore as this has been processed before */
	    break;

	case OPT_LOG_FILE:
	    cfg->log_cfg.log_filename = pj_str(pj_optarg);
	    break;

	case OPT_LOG_LEVEL:
	    c = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (c < 0 || c > 6) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: expecting integer value 0-6 "
			  "for --log-level"));
		return PJ_EINVAL;
	    }
	    cfg->log_cfg.level = c;
	    pj_log_set_level( c );
	    break;

	case OPT_APP_LOG_LEVEL:
	    cfg->log_cfg.console_level = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (cfg->log_cfg.console_level > 6) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: expecting integer value 0-6 "
			  "for --app-log-level"));
		return PJ_EINVAL;
	    }
	    break;

	case OPT_LOG_APPEND:
	    cfg->log_cfg.log_file_flags |= PJ_O_APPEND;
	    break;

	case OPT_COLOR:
	    cfg->log_cfg.decor |= PJ_LOG_HAS_COLOR;
	    break;

	case OPT_NO_COLOR:
	    cfg->log_cfg.decor &= ~PJ_LOG_HAS_COLOR;
	    break;

	case OPT_LIGHT_BG:
	    pj_log_set_color(1, PJ_TERM_COLOR_R);
	    pj_log_set_color(2, PJ_TERM_COLOR_R | PJ_TERM_COLOR_G);
	    pj_log_set_color(3, PJ_TERM_COLOR_B | PJ_TERM_COLOR_G);
	    pj_log_set_color(4, 0);
	    pj_log_set_color(5, 0);
	    pj_log_set_color(77, 0);
	    break;

	case OPT_NO_STDERR:
#if !defined(PJ_WIN32_WINCE) || PJ_WIN32_WINCE==0
	    freopen("/dev/null", "w", stderr);
#endif
	    break;

	case OPT_HELP:
	    return PJ_EINVAL;

	case OPT_VERSION:   /* version */
	    pj_dump_config();
	    return PJ_EINVAL;

	case OPT_NULL_AUDIO:
	    cfg->null_audio = PJ_TRUE;
	    break;

	case OPT_CLOCK_RATE:
	    lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (lval < 8000 || lval > 192000) {
		PJ_LOG(1,(THIS_FILE, "Error: expecting value between "
				     "8000-192000 for conference clock rate"));
		return PJ_EINVAL;
	    }
	    cfg->media_cfg.clock_rate = lval;
	    break;

	case OPT_SND_CLOCK_RATE:
	    lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (lval < 8000 || lval > 192000) {
		PJ_LOG(1,(THIS_FILE, "Error: expecting value between "
				     "8000-192000 for sound device clock rate"));
		return PJ_EINVAL;
	    }
	    cfg->media_cfg.snd_clock_rate = lval;
	    break;

	case OPT_STEREO:
	    cfg->media_cfg.channel_count = 2;
	    break;

	case OPT_LOCAL_PORT:   /* local-port */
	    lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (lval < 0 || lval > 65535) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: expecting integer value for "
			  "--local-port"));
		return PJ_EINVAL;
	    }
	    cfg->udp_cfg.port = (pj_uint16_t)lval;
	    break;

	case OPT_IP_ADDR: /* ip-addr */
	    cfg->udp_cfg.public_addr = pj_str(pj_optarg);
	    cfg->rtp_cfg.public_addr = pj_str(pj_optarg);
	    break;

	case OPT_BOUND_ADDR: /* bound-addr */
	    cfg->udp_cfg.bound_addr = pj_str(pj_optarg);
	    cfg->rtp_cfg.bound_addr = pj_str(pj_optarg);
	    break;

	case OPT_NO_UDP: /* no-udp */
	    if (cfg->no_tcp && !cfg->use_tls) {
	      PJ_LOG(1,(THIS_FILE,"Error: cannot disable both TCP and UDP"));
	      return PJ_EINVAL;
	    }

	    cfg->no_udp = PJ_TRUE;
	    break;

	case OPT_NOREFERSUB: /* norefersub */
	    cfg->no_refersub = PJ_TRUE;
	    break;

	case OPT_NO_TCP: /* no-tcp */
	    if (cfg->no_udp && !cfg->use_tls) {
	      PJ_LOG(1,(THIS_FILE,"Error: cannot disable both TCP and UDP"));
	      return PJ_EINVAL;
	    }

	    cfg->no_tcp = PJ_TRUE;
	    break;

	case OPT_PROXY:   /* proxy */
	    if (pjsua_verify_sip_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid SIP URL '%s' "
			  "in proxy argument", pj_optarg));
		return PJ_EINVAL;
	    }
	    cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	    break;

	case OPT_OUTBOUND_PROXY:   /* outbound proxy */
	    if (pjsua_verify_sip_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid SIP URL '%s' "
			  "in outbound proxy argument", pj_optarg));
		return PJ_EINVAL;
	    }
	    cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt++] = pj_str(pj_optarg);
	    break;

	case OPT_REGISTRAR:   /* registrar */
	    if (pjsua_verify_sip_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid SIP URL '%s' in "
			  "registrar argument", pj_optarg));
		return PJ_EINVAL;
	    }
	    cur_acc->reg_uri = pj_str(pj_optarg);
	    break;

	case OPT_REG_TIMEOUT:   /* reg-timeout */
	    cur_acc->reg_timeout = pj_strtoul(pj_cstr(&tmp,pj_optarg));
	    if (cur_acc->reg_timeout < 1 || cur_acc->reg_timeout > 3600) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid value for --reg-timeout "
			  "(expecting 1-3600)"));
		return PJ_EINVAL;
	    }
	    break;

	case OPT_PUBLISH:   /* publish */
	    cur_acc->publish_enabled = PJ_TRUE;
	    break;

	case OPT_MWI:	/* mwi */
	    cur_acc->mwi_enabled = PJ_TRUE;
	    break;

	case OPT_100REL: /** 100rel */
	    cur_acc->require_100rel = PJSUA_100REL_MANDATORY;
	    cfg->cfg.require_100rel = PJSUA_100REL_MANDATORY;
	    break;

	case OPT_TIMER: /** session timer */
	    lval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (lval < 0 || lval > 3) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: expecting integer value 0-3 for --use-timer"));
		return PJ_EINVAL;
	    }
	    cur_acc->use_timer = lval;
	    cfg->cfg.use_timer = lval;
	    break;

	case OPT_TIMER_SE: /** session timer session expiration */
	    cur_acc->timer_setting.sess_expires = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (cur_acc->timer_setting.sess_expires < 90) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid value for --timer-se "
			  "(expecting higher than 90)"));
		return PJ_EINVAL;
	    }
	    cfg->cfg.timer_setting.sess_expires = cur_acc->timer_setting.sess_expires;
	    break;

	case OPT_TIMER_MIN_SE: /** session timer minimum session expiration */
	    cur_acc->timer_setting.min_se = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (cur_acc->timer_setting.min_se < 90) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid value for --timer-min-se "
			  "(expecting higher than 90)"));
		return PJ_EINVAL;
	    }
	    cfg->cfg.timer_setting.min_se = cur_acc->timer_setting.min_se;
	    break;

	case OPT_OUTB_RID: /* Outbound reg-id */
	    cur_acc->rfc5626_reg_id = pj_str(pj_optarg);
	    break;

	case OPT_USE_IMS: /* Activate IMS settings */
	    cur_acc->auth_pref.initial_auth = PJ_TRUE;
	    break;

	case OPT_ID:   /* id */
	    if (pjsua_verify_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid SIP URL '%s' "
			  "in local id argument", pj_optarg));
		return PJ_EINVAL;
	    }
	    cur_acc->id = pj_str(pj_optarg);
	    break;

	case OPT_CONTACT:   /* contact */
	    if (pjsua_verify_sip_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid SIP URL '%s' "
			  "in contact argument", pj_optarg));
		return PJ_EINVAL;
	    }
	    cur_acc->force_contact = pj_str(pj_optarg);
	    break;

	case OPT_CONTACT_PARAMS:
	    cur_acc->contact_params = pj_str(pj_optarg);
	    break;

	case OPT_CONTACT_URI_PARAMS:
	    cur_acc->contact_uri_params = pj_str(pj_optarg);
	    break;

	case OPT_AUTO_UPDATE_NAT:   /* OPT_AUTO_UPDATE_NAT */
            cur_acc->allow_contact_rewrite  = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    break;

	case OPT_DISABLE_STUN:
	    cur_acc->sip_stun_use = PJSUA_STUN_USE_DISABLED;
	    cur_acc->media_stun_use = PJSUA_STUN_USE_DISABLED;
	    break;

	case OPT_USE_COMPACT_FORM:
	    /* enable compact form - from Ticket #342 */
            {
		extern pj_bool_t pjsip_use_compact_form;
		extern pj_bool_t pjsip_include_allow_hdr_in_dlg;
		extern pj_bool_t pjmedia_add_rtpmap_for_static_pt;

		pjsip_use_compact_form = PJ_TRUE;
		/* do not transmit Allow header */
		pjsip_include_allow_hdr_in_dlg = PJ_FALSE;
		/* Do not include rtpmap for static payload types (<96) */
		pjmedia_add_rtpmap_for_static_pt = PJ_FALSE;
            }
	    break;

	case OPT_ACCEPT_REDIRECT:
	    cfg->redir_op = my_atoi(pj_optarg);
	    if (cfg->redir_op>PJSIP_REDIRECT_STOP) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: accept-redirect value '%s' ", pj_optarg));
		return PJ_EINVAL;
	    }
	    break;

	case OPT_NO_FORCE_LR:
	    cfg->cfg.force_lr = PJ_FALSE;
	    break;

	case OPT_NEXT_ACCOUNT: /* Add more account. */
	    cfg->acc_cnt++;
	    cur_acc = &cfg->acc_cfg[cfg->acc_cnt];
	    break;

	case OPT_USERNAME:   /* Default authentication user */
	    cur_acc->cred_info[cur_acc->cred_count].username = pj_str(pj_optarg);
	    cur_acc->cred_info[cur_acc->cred_count].scheme = pj_str("Digest");
	    break;

	case OPT_REALM:	    /* Default authentication realm. */
	    cur_acc->cred_info[cur_acc->cred_count].realm = pj_str(pj_optarg);
	    break;

	case OPT_PASSWORD:   /* authentication password */
	    cur_acc->cred_info[cur_acc->cred_count].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	    cur_acc->cred_info[cur_acc->cred_count].data = pj_str(pj_optarg);
#if PJSIP_HAS_DIGEST_AKA_AUTH
	    cur_acc->cred_info[cur_acc->cred_count].data_type |= PJSIP_CRED_DATA_EXT_AKA;
	    cur_acc->cred_info[cur_acc->cred_count].ext.aka.k = pj_str(pj_optarg);
	    cur_acc->cred_info[cur_acc->cred_count].ext.aka.cb = &pjsip_auth_create_aka_response;
#endif
	    break;

	case OPT_REG_RETRY_INTERVAL:
	    cur_acc->reg_retry_interval = pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    break;

	case OPT_REG_USE_PROXY:
	    cur_acc->reg_use_proxy = (unsigned)pj_strtoul(pj_cstr(&tmp, pj_optarg));
	    if (cur_acc->reg_use_proxy > 3) {
		PJ_LOG(1,(THIS_FILE, "Error: invalid --reg-use-proxy value '%s'",
			  pj_optarg));
		return PJ_EINVAL;
	    }
	    break;

	case OPT_NEXT_CRED: /* next credential */
	    cur_acc->cred_count++;
	    break;

	case OPT_NAMESERVER: /* nameserver */
	    cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
	    if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver)) {
		PJ_LOG(1,(THIS_FILE, "Error: too many nameservers"));
		return PJ_ETOOMANY;
	    }
	    break;

	case OPT_STUN_SRV:   /* STUN server */
	    cfg->cfg.stun_host = pj_str(pj_optarg);
	    if (cfg->cfg.stun_srv_cnt==PJ_ARRAY_SIZE(cfg->cfg.stun_srv)) {
		PJ_LOG(1,(THIS_FILE, "Error: too many STUN servers"));
		return PJ_ETOOMANY;
	    }
	    cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt++] = pj_str(pj_optarg);
	    break;

	case OPT_ADD_BUDDY: /* Add to buddy list. */
	    if (pjsua_verify_url(pj_optarg) != 0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid URL '%s' in "
			  "--add-buddy option", pj_optarg));
		return -1;
	    }
	    if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg)) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: too many buddies in buddy list."));
		return -1;
	    }
	    cfg->buddy_cfg[cfg->buddy_cnt].uri = pj_str(pj_optarg);
	    cfg->buddy_cnt++;
	    break;

	case OPT_AUTO_PLAY:
	    cfg->auto_play = 1;
	    break;

	case OPT_AUTO_PLAY_HANGUP:
	    cfg->auto_play_hangup = 1;
	    break;

	case OPT_AUTO_REC:
	    cfg->auto_rec = 1;
	    break;

	case OPT_AUTO_LOOP:
	    cfg->auto_loop = 1;
	    break;

	case OPT_AUTO_CONF:
	    cfg->auto_conf = 1;
	    break;

	case OPT_PLAY_FILE:
	    cfg->wav_files[cfg->wav_count++] = pj_str(pj_optarg);
	    break;

	case OPT_PLAY_TONE:
	    {
		int f1, f2, on, off;
		int n;

		n = sscanf(pj_optarg, "%d,%d,%d,%d", &f1, &f2, &on, &off);
		if (n != 4) {
		    puts("Expecting f1,f2,on,off in --play-tone");
		    return -1;
		}

		cfg->tones[cfg->tone_count].freq1 = (short)f1;
		cfg->tones[cfg->tone_count].freq2 = (short)f2;
		cfg->tones[cfg->tone_count].on_msec = (short)on;
		cfg->tones[cfg->tone_count].off_msec = (short)off;
		++cfg->tone_count;
	    }
	    break;

	case OPT_REC_FILE:
	    cfg->rec_file = pj_str(pj_optarg);
	    break;

	case OPT_USE_ICE:
	    cfg->media_cfg.enable_ice =
		    cur_acc->ice_cfg.enable_ice = PJ_TRUE;
	    break;

	case OPT_ICE_REGULAR:
	    cfg->media_cfg.ice_opt.aggressive =
		    cur_acc->ice_cfg.ice_opt.aggressive = PJ_FALSE;
	    break;

	case OPT_USE_TURN:
	    cfg->media_cfg.enable_turn =
		    cur_acc->turn_cfg.enable_turn = PJ_TRUE;
	    break;

	case OPT_ICE_MAX_HOSTS:
	    cfg->media_cfg.ice_max_host_cands =
		    cur_acc->ice_cfg.ice_max_host_cands = my_atoi(pj_optarg);
	    break;

	case OPT_ICE_NO_RTCP:
	    cfg->media_cfg.ice_no_rtcp =
		    cur_acc->ice_cfg.ice_no_rtcp = PJ_TRUE;
	    break;

	case OPT_TURN_SRV:
	    cfg->media_cfg.turn_server =
		    cur_acc->turn_cfg.turn_server = pj_str(pj_optarg);
	    break;

	case OPT_TURN_TCP:
	    cfg->media_cfg.turn_conn_type =
		    cur_acc->turn_cfg.turn_conn_type = PJ_TURN_TP_TCP;
	    break;

	case OPT_TURN_USER:
	    cfg->media_cfg.turn_auth_cred.type =
		    cur_acc->turn_cfg.turn_auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;
	    cfg->media_cfg.turn_auth_cred.data.static_cred.realm =
		    cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm = pj_str("*");
	    cfg->media_cfg.turn_auth_cred.data.static_cred.username =
		    cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username = pj_str(pj_optarg);
	    break;

	case OPT_TURN_PASSWD:
	    cfg->media_cfg.turn_auth_cred.data.static_cred.data_type =
		    cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data_type = PJ_STUN_PASSWD_PLAIN;
	    cfg->media_cfg.turn_auth_cred.data.static_cred.data =
		    cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data = pj_str(pj_optarg);
	    break;

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	case OPT_USE_SRTP:
	    app_config.cfg.use_srtp = my_atoi(pj_optarg);
	    if (!pj_isdigit(*pj_optarg) || app_config.cfg.use_srtp > 3) {
		PJ_LOG(1,(THIS_FILE, "Invalid value for --use-srtp option"));
		return -1;
	    }
	    if ((int)app_config.cfg.use_srtp == 3) {
		/* SRTP optional mode with duplicated media offer */
		app_config.cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
		app_config.cfg.srtp_optional_dup_offer = PJ_TRUE;
		cur_acc->srtp_optional_dup_offer = PJ_TRUE;
	    }
	    cur_acc->use_srtp = app_config.cfg.use_srtp;
	    break;
	case OPT_SRTP_SECURE:
	    app_config.cfg.srtp_secure_signaling = my_atoi(pj_optarg);
	    if (!pj_isdigit(*pj_optarg) ||
		app_config.cfg.srtp_secure_signaling > 2)
	    {
		PJ_LOG(1,(THIS_FILE, "Invalid value for --srtp-secure option"));
		return -1;
	    }
	    cur_acc->srtp_secure_signaling = app_config.cfg.srtp_secure_signaling;
	    break;
#endif

	case OPT_RTP_PORT:
	    cfg->rtp_cfg.port = my_atoi(pj_optarg);
	    if (cfg->rtp_cfg.port == 0) {
		enum { START_PORT=4000 };
		unsigned range;

		range = (65535-START_PORT-PJSUA_MAX_CALLS*2);
		cfg->rtp_cfg.port = START_PORT +
				    ((pj_rand() % range) & 0xFFFE);
	    }

	    if (cfg->rtp_cfg.port < 1 || cfg->rtp_cfg.port > 65535) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: rtp-port argument value "
			  "(expecting 1-65535"));
		return -1;
	    }
	    break;

	case OPT_DIS_CODEC:
            cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str(pj_optarg);
	    break;

	case OPT_ADD_CODEC:
	    cfg->codec_arg[cfg->codec_cnt++] = pj_str(pj_optarg);
	    break;

	case OPT_DURATION:
	    cfg->duration = my_atoi(pj_optarg);
	    break;

	case OPT_THREAD_CNT:
	    cfg->cfg.thread_cnt = my_atoi(pj_optarg);
	    if (cfg->cfg.thread_cnt > 128) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --thread-cnt option"));
		return -1;
	    }
	    break;

	case OPT_PTIME:
	    cfg->media_cfg.ptime = my_atoi(pj_optarg);
	    if (cfg->media_cfg.ptime < 10 || cfg->media_cfg.ptime > 1000) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --ptime option"));
		return -1;
	    }
	    break;

	case OPT_NO_VAD:
	    cfg->media_cfg.no_vad = PJ_TRUE;
	    break;

	case OPT_EC_TAIL:
	    cfg->media_cfg.ec_tail_len = my_atoi(pj_optarg);
	    if (cfg->media_cfg.ec_tail_len > 1000) {
		PJ_LOG(1,(THIS_FILE, "I think the ec-tail length setting "
			  "is too big"));
		return -1;
	    }
	    break;

	case OPT_EC_OPT:
	    cfg->media_cfg.ec_options = my_atoi(pj_optarg);
	    break;

	case OPT_QUALITY:
	    cfg->media_cfg.quality = my_atoi(pj_optarg);
	    if (cfg->media_cfg.quality > 10) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --quality (expecting 0-10"));
		return -1;
	    }
	    break;

	case OPT_ILBC_MODE:
	    cfg->media_cfg.ilbc_mode = my_atoi(pj_optarg);
	    if (cfg->media_cfg.ilbc_mode!=20 && cfg->media_cfg.ilbc_mode!=30) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --ilbc-mode (expecting 20 or 30"));
		return -1;
	    }
	    break;

	case OPT_RX_DROP_PCT:
	    cfg->media_cfg.rx_drop_pct = my_atoi(pj_optarg);
	    if (cfg->media_cfg.rx_drop_pct > 100) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --rx-drop-pct (expecting <= 100"));
		return -1;
	    }
	    break;

	case OPT_TX_DROP_PCT:
	    cfg->media_cfg.tx_drop_pct = my_atoi(pj_optarg);
	    if (cfg->media_cfg.tx_drop_pct > 100) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid --tx-drop-pct (expecting <= 100"));
		return -1;
	    }
	    break;

	case OPT_AUTO_ANSWER:
	    cfg->auto_answer = my_atoi(pj_optarg);
	    if (cfg->auto_answer < 100 || cfg->auto_answer > 699) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: invalid code in --auto-answer "
			  "(expecting 100-699"));
		return -1;
	    }
	    break;

	case OPT_MAX_CALLS:
	    cfg->cfg.max_calls = my_atoi(pj_optarg);
	    if (cfg->cfg.max_calls < 1 || cfg->cfg.max_calls > PJSUA_MAX_CALLS) {
		PJ_LOG(1,(THIS_FILE,"Error: maximum call setting exceeds "
				    "compile time limit (PJSUA_MAX_CALLS=%d)",
			  PJSUA_MAX_CALLS));
		return -1;
	    }
	    break;

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	case OPT_USE_TLS:
	    cfg->use_tls = PJ_TRUE;
	    break;

	case OPT_TLS_CA_FILE:
	    cfg->udp_cfg.tls_setting.ca_list_file = pj_str(pj_optarg);
	    break;

	case OPT_TLS_CERT_FILE:
	    cfg->udp_cfg.tls_setting.cert_file = pj_str(pj_optarg);
	    break;

	case OPT_TLS_PRIV_FILE:
	    cfg->udp_cfg.tls_setting.privkey_file = pj_str(pj_optarg);
	    break;

	case OPT_TLS_PASSWORD:
	    cfg->udp_cfg.tls_setting.password = pj_str(pj_optarg);
	    break;

	case OPT_TLS_VERIFY_SERVER:
	    cfg->udp_cfg.tls_setting.verify_server = PJ_TRUE;
	    break;

	case OPT_TLS_VERIFY_CLIENT:
	    cfg->udp_cfg.tls_setting.verify_client = PJ_TRUE;
	    cfg->udp_cfg.tls_setting.require_client_cert = PJ_TRUE;
	    break;

	case OPT_TLS_NEG_TIMEOUT:
	    cfg->udp_cfg.tls_setting.timeout.sec = atoi(pj_optarg);
	    break;

	case OPT_TLS_CIPHER:
	    {
		pj_ssl_cipher cipher;

		if (pj_ansi_strnicmp(pj_optarg, "0x", 2) == 0) {
		    pj_str_t cipher_st = pj_str(pj_optarg + 2);
		    cipher = pj_strtoul2(&cipher_st, NULL, 16);
		} else {
		    cipher = atoi(pj_optarg);
		}

		if (pj_ssl_cipher_is_supported(cipher)) {
		    static pj_ssl_cipher tls_ciphers[128];

		    tls_ciphers[cfg->udp_cfg.tls_setting.ciphers_num++] = cipher;
		    cfg->udp_cfg.tls_setting.ciphers = tls_ciphers;
		} else {
		    pj_ssl_cipher ciphers[128];
		    unsigned j, ciphers_cnt;

		    ciphers_cnt = PJ_ARRAY_SIZE(ciphers);
		    pj_ssl_cipher_get_availables(ciphers, &ciphers_cnt);

		    PJ_LOG(1,(THIS_FILE, "Cipher \"%s\" is not supported by "
					 "TLS/SSL backend.", pj_optarg));
		    printf("Available TLS/SSL ciphers (%d):\n", ciphers_cnt);
		    for (j=0; j<ciphers_cnt; ++j)
			printf("- 0x%06X: %s\n", ciphers[j], pj_ssl_cipher_name(ciphers[j]));
		    return -1;
		}
	    }
 	    break;
#endif /* PJSIP_HAS_TLS_TRANSPORT */

	case OPT_CAPTURE_DEV:
	    cfg->capture_dev = atoi(pj_optarg);
	    break;

	case OPT_PLAYBACK_DEV:
	    cfg->playback_dev = atoi(pj_optarg);
	    break;

	case OPT_STDOUT_REFRESH:
	    break;

	case OPT_STDOUT_REFRESH_TEXT:
	    break;

#ifdef _IONBF
	case OPT_STDOUT_NO_BUF:
	    setvbuf(stdout, NULL, _IONBF, 0);
	    break;
#endif

	case OPT_CAPTURE_LAT:
	    cfg->capture_lat = atoi(pj_optarg);
	    break;

	case OPT_PLAYBACK_LAT:
	    cfg->playback_lat = atoi(pj_optarg);
	    break;

	case OPT_SND_AUTO_CLOSE:
	    cfg->media_cfg.snd_auto_close_time = atoi(pj_optarg);
	    break;

	case OPT_NO_TONES:
	    cfg->no_tones = PJ_TRUE;
	    break;

	case OPT_JB_MAX_SIZE:
	    cfg->media_cfg.jb_max = atoi(pj_optarg);
	    break;

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	case OPT_IPV6:
	    cfg->ipv6 = PJ_TRUE;
	    break;
#endif
	case OPT_QOS:
	    cfg->enable_qos = PJ_TRUE;
	    /* Set RTP traffic type to Voice */
	    cfg->rtp_cfg.qos_type = PJ_QOS_TYPE_VOICE;
	    /* Directly apply DSCP value to SIP traffic. Say lets
	     * set it to CS3 (DSCP 011000). Note that this will not
	     * work on all platforms.
	     */
	    cfg->udp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	    cfg->udp_cfg.qos_params.dscp_val = 0x18;
	    break;
	case OPT_VIDEO:
	    cfg->vid.vid_cnt = 1;
	    cfg->vid.in_auto_show = PJ_TRUE;
	    cfg->vid.out_auto_transmit = PJ_TRUE;
	    break;
	case OPT_EXTRA_AUDIO:
	    cfg->aud_cnt++;
	    break;

	case OPT_VCAPTURE_DEV:
	    cfg->vid.vcapture_dev = atoi(pj_optarg);
	    cur_acc->vid_cap_dev = cfg->vid.vcapture_dev;
	    break;

	case OPT_VRENDER_DEV:
	    cfg->vid.vrender_dev = atoi(pj_optarg);
	    cur_acc->vid_rend_dev = cfg->vid.vrender_dev;
	    break;

	case OPT_PLAY_AVI:
	    if (app_config.avi_cnt >= PJSUA_APP_MAX_AVI) {
		PJ_LOG(1,(THIS_FILE, "Too many AVIs"));
		return -1;
	    }
	    app_config.avi[app_config.avi_cnt++].path = pj_str(pj_optarg);
	    break;

	case OPT_AUTO_PLAY_AVI:
	    app_config.avi_auto_play = PJ_TRUE;
	    break;

	case OPT_USE_CLI:
	case OPT_CLI_TELNET_PORT:
	case OPT_DISABLE_CLI_CONSOLE:
	    break;

	default:
	    PJ_LOG(1,(THIS_FILE,
		      "Argument \"%s\" is not valid. Use --help to see help",
		      argv[pj_optind-1]));
	    return -1;
	}
    }

    if (pj_optind != argc) {
	pj_str_t uri_arg;

	if (pjsua_verify_url(argv[pj_optind]) != PJ_SUCCESS) {
	    PJ_LOG(1,(THIS_FILE, "Invalid SIP URI %s", argv[pj_optind]));
	    return -1;
	}
	uri_arg = pj_str(argv[pj_optind]);
	if (uri_to_call)
	    *uri_to_call = uri_arg;
	pj_optind++;

	/* Add URI to call to buddy list if it's not already there */
	for (i=0; i<cfg->buddy_cnt; ++i) {
	    if (pj_stricmp(&cfg->buddy_cfg[i].uri, &uri_arg)==0)
		break;
	}
	if (i == cfg->buddy_cnt && cfg->buddy_cnt < PJSUA_MAX_BUDDIES) {
	    cfg->buddy_cfg[cfg->buddy_cnt++].uri = uri_arg;
	}

    } else {
	if (uri_to_call)
	    uri_to_call->slen = 0;
    }

    if (pj_optind != argc) {
	PJ_LOG(1,(THIS_FILE, "Error: unknown options %s", argv[pj_optind]));
	return PJ_EINVAL;
    }

    if (cfg->acc_cfg[cfg->acc_cnt].id.slen)
	cfg->acc_cnt++;

    for (i=0; i<cfg->acc_cnt; ++i) {
	pjsua_acc_config *acfg = &cfg->acc_cfg[i];

	if (acfg->cred_info[acfg->cred_count].username.slen)
	{
	    acfg->cred_count++;
	}

	if (acfg->ice_cfg.enable_ice) {
	    acfg->ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
	}
	if (acfg->turn_cfg.enable_turn) {
	    acfg->turn_cfg_use = PJSUA_TURN_CONFIG_USE_CUSTOM;
	}

	/* When IMS mode is enabled for the account, verify that settings
	 * are okay.
	 */
	/* For now we check if IMS mode is activated by looking if
	 * initial_auth is set.
	 */
	if (acfg->auth_pref.initial_auth && acfg->cred_count) {
	    /* Realm must point to the real domain */
	    if (*acfg->cred_info[0].realm.ptr=='*') {
		PJ_LOG(1,(THIS_FILE,
			  "Error: cannot use '*' as realm with IMS"));
		return PJ_EINVAL;
	    }

	    /* Username for authentication must be in a@b format */
	    if (strchr(acfg->cred_info[0].username.ptr, '@')==0) {
		PJ_LOG(1,(THIS_FILE,
			  "Error: Username for authentication must "
			  "be in user@domain format with IMS"));
		return PJ_EINVAL;
	    }
	}
    }
    return PJ_SUCCESS;
}

/* Set default config. */
static void default_config()
{
    char tmp[80];
    unsigned i;
    pjsua_app_config *cfg = &app_config;

    pjsua_config_default(&cfg->cfg);
    pj_ansi_sprintf(tmp, "PJSUA v%s %s", pj_get_version(),
		    pj_get_sys_info()->info.ptr);
    pj_strdup2_with_null(app_config.pool, &cfg->cfg.user_agent, tmp);

    pjsua_logging_config_default(&cfg->log_cfg);
    pjsua_media_config_default(&cfg->media_cfg);
    pjsua_transport_config_default(&cfg->udp_cfg);
    cfg->udp_cfg.port = 5060;
    pjsua_transport_config_default(&cfg->rtp_cfg);
    cfg->rtp_cfg.port = 4000;
    cfg->redir_op = PJSIP_REDIRECT_ACCEPT_REPLACE;
    cfg->duration = PJSUA_APP_NO_LIMIT_DURATION;
    cfg->wav_id = PJSUA_INVALID_ID;
    cfg->rec_id = PJSUA_INVALID_ID;
    cfg->wav_port = PJSUA_INVALID_ID;
    cfg->rec_port = PJSUA_INVALID_ID;
    cfg->mic_level = cfg->speaker_level = 1.0;
    cfg->capture_dev = PJSUA_INVALID_ID;
    cfg->playback_dev = PJSUA_INVALID_ID;
    cfg->capture_lat = PJMEDIA_SND_DEFAULT_REC_LATENCY;
    cfg->playback_lat = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    cfg->ringback_slot = PJSUA_INVALID_ID;
    cfg->ring_slot = PJSUA_INVALID_ID;

    for (i=0; i<PJ_ARRAY_SIZE(cfg->acc_cfg); ++i)
	pjsua_acc_config_default(&cfg->acc_cfg[i]);

    for (i=0; i<PJ_ARRAY_SIZE(cfg->buddy_cfg); ++i)
	pjsua_buddy_config_default(&cfg->buddy_cfg[i]);

    cfg->vid.vcapture_dev = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
    cfg->vid.vrender_dev = PJMEDIA_VID_DEFAULT_RENDER_DEV;
    cfg->aud_cnt = 1;

    cfg->avi_def_idx = PJSUA_INVALID_ID;

}

static pj_status_t parse_config(int argc, char *argv[], pj_str_t *uri_arg)
{
    pj_status_t status;

    /* Initialize default config */
    default_config();

    /* Parse the arguments */
    status = parse_args(argc, argv, uri_arg);
    return status;
}

pj_status_t load_config(int argc,
			char **argv,
			pj_str_t *uri_arg)
{
    pj_status_t status;
    int cli_fe = 0;
    pj_uint16_t cli_telnet_port = 0;

    status = parse_config(argc, argv, uri_arg);
    if (status != PJ_SUCCESS)
		return status;

    return status;
}

