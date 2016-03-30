/* $Id: opensl_dev.c 4960 2014-11-13 01:48:34Z ming $ */
/* 
 * Copyright (C) 2012-2012 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2010-2012 Regis Montoya (aka r3gis - www.r3gis.fr)
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
/* This file is the implementation of Android OpenSL ES audio device.
 * The original code was originally part of CSipSimple
 * (http://code.google.com/p/csipsimple/) and was kindly donated
 * by Regis Montoya.
 */

#include <pjmedia-audiodev/audiodev_imp.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/string.h>
#include <pjmedia/errno.h>
#include <pjmedia-audiodev/app_callback.h>
 
#if defined(PJMEDIA_AUDIO_DEV_HAS_OPENSL) && PJMEDIA_AUDIO_DEV_HAS_OPENSL != 0

#include <SLES/OpenSLES.h>

#ifdef __ANDROID__
    #include <SLES/OpenSLES_Android.h>
    #include <SLES/OpenSLES_AndroidConfiguration.h>
    #include <sys/system_properties.h>
    #include <android/api-level.h>

    #define W_SLBufferQueueItf SLAndroidSimpleBufferQueueItf
    #define W_SLBufferQueueState SLAndroidSimpleBufferQueueState
    #define W_SL_IID_BUFFERQUEUE SL_IID_ANDROIDSIMPLEBUFFERQUEUE
#else
    #define W_SLBufferQueueItf SLBufferQueueItf
    #define W_SLBufferQueueState SLBufferQueueState
    #define W_SL_IID_BUFFERQUEUE SL_IID_BUFFERQUEUE
#endif

#define THIS_FILE	"opensl_dev.c"
#define DRIVER_NAME	"OpenSL"

#define NUM_BUFFERS 2
struct opensl_aud_stream *stream;

callback_func callback_player_func;
callback_func recorder_func;
callback_func player_func;

struct opensl_aud_factory
{
    pjmedia_aud_dev_factory  base;
    pj_pool_factory         *pf;
    pj_pool_t               *pool;
    
    SLObjectItf              engineObject;
    SLEngineItf              engineEngine;
    SLObjectItf              outputMixObject;
};

/*
 * Sound stream descriptor.
 * This struct may be used for both unidirectional or bidirectional sound
 * streams.
 */
struct opensl_aud_stream
{
    pjmedia_aud_stream  base;
    pj_pool_t          *pool;
    pj_str_t            name;
    pjmedia_dir         dir;
    pjmedia_aud_param   param;
    
    void               *user_data;
    pj_bool_t           quit_flag;
    pjmedia_aud_rec_cb  rec_cb;
    pjmedia_aud_play_cb play_cb;

    pj_timestamp	play_timestamp;
    pj_timestamp	rec_timestamp;
    
    pj_bool_t		rec_thread_initialized;
    pj_thread_desc	rec_thread_desc;
    pj_thread_t        *rec_thread;
    
    pj_bool_t		play_thread_initialized;
    pj_thread_desc	play_thread_desc;
    pj_thread_t        *play_thread;
    
    /* Player */
    SLObjectItf         playerObj;
    SLPlayItf           playerPlay;
    SLVolumeItf         playerVol;
    unsigned            playerBufferSize;
    char               *playerBuffer[NUM_BUFFERS];
    int                 playerBufIdx;
    
    /* Recorder */
    SLObjectItf         recordObj;
    SLRecordItf         recordRecord;
    unsigned            recordBufferSize;
    char               *recordBuffer[NUM_BUFFERS];
    int                 recordBufIdx;

    W_SLBufferQueueItf  playerBufQ;
    W_SLBufferQueueItf  recordBufQ;
};

/* Factory prototypes */
static pj_status_t opensl_init(pjmedia_aud_dev_factory *f);
static pj_status_t opensl_destroy(pjmedia_aud_dev_factory *f);
static pj_status_t opensl_refresh(pjmedia_aud_dev_factory *f);
static unsigned opensl_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t opensl_get_dev_info(pjmedia_aud_dev_factory *f,
                                       unsigned index,
                                       pjmedia_aud_dev_info *info);
static pj_status_t opensl_default_param(pjmedia_aud_dev_factory *f,
                                        unsigned index,
                                        pjmedia_aud_param *param);
static pj_status_t opensl_create_stream(pjmedia_aud_dev_factory *f,
                                        const pjmedia_aud_param *param,
                                        pjmedia_aud_rec_cb rec_cb,
                                        pjmedia_aud_play_cb play_cb,
                                        void *user_data,
                                        pjmedia_aud_stream **p_aud_strm);

/* Stream prototypes */
static pj_status_t strm_get_param(pjmedia_aud_stream *strm,
                                  pjmedia_aud_param *param);
static pj_status_t strm_get_cap(pjmedia_aud_stream *strm,
                                pjmedia_aud_dev_cap cap,
                                void *value);
static pj_status_t strm_set_cap(pjmedia_aud_stream *strm,
                                pjmedia_aud_dev_cap cap,
                                const void *value);
static pj_status_t strm_start(pjmedia_aud_stream *strm);
static pj_status_t strm_stop(pjmedia_aud_stream *strm);
static pj_status_t strm_destroy(pjmedia_aud_stream *strm);

static pjmedia_aud_dev_factory_op opensl_op =
{
    &opensl_init,
    &opensl_destroy,
    &opensl_get_dev_count,
    &opensl_get_dev_info,
    &opensl_default_param,
    &opensl_create_stream,
    &opensl_refresh
};

static pjmedia_aud_stream_op opensl_strm_op =
{
    &strm_get_param,
    &strm_get_cap,
    &strm_set_cap,
    &strm_start,
    &strm_stop,
    &strm_destroy
};

/* This callback is called every time a buffer finishes playing. */
void cubicPlayerCallback(void *buf,int size)
{
    int status;
	PJ_UNUSED_ARG(buf);
	PJ_UNUSED_ARG(size);
    if (stream->play_thread_initialized == 0 || !pj_thread_is_registered())
    {
	pj_bzero(stream->play_thread_desc, sizeof(pj_thread_desc));
	status = pj_thread_register("opensl_play", stream->play_thread_desc,
				    &stream->play_thread);
	stream->play_thread_initialized = 1;
	PJ_LOG(5, (THIS_FILE, "Player thread started"));
    }

    if (!stream->quit_flag) {
        pjmedia_frame frame;
        char * buf;
		char buf_test[BUFF_SIZE];
		memset(buf_test,0,sizeof(buf_test));
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = buf = stream->playerBuffer[stream->playerBufIdx++];
        frame.size = stream->playerBufferSize;
        frame.timestamp.u64 = stream->play_timestamp.u64;
        frame.bit_info = 0;

        status = (*stream->play_cb)(stream->user_data, &frame);
        if (status != PJ_SUCCESS || frame.type != PJMEDIA_FRAME_TYPE_AUDIO)
            pj_bzero(buf, stream->playerBufferSize);
		
        if(frame.size == sizeof(buf_test))
		{
			if(memcmp(frame.buf,(void *)buf_test,BUFF_SIZE) != 0)
			{
			    app_player_callback(frame.buf,frame.size);
			}
		}
        stream->play_timestamp.u64 += stream->param.samples_per_frame /
                                      stream->param.channel_count;
									  
        stream->playerBufIdx %= NUM_BUFFERS;
    }
}

/* This callback handler is called every time a buffer finishes recording */
void cubicRecorderCallback(void *buf,int size)
{
    SLresult result;
    int status;
    if (stream->rec_thread_initialized == 0 || !pj_thread_is_registered())
    {
	pj_bzero(stream->rec_thread_desc, sizeof(pj_thread_desc));
	status = pj_thread_register("cubic_rec", stream->rec_thread_desc,
				    &stream->rec_thread);
	PJ_UNUSED_ARG(status);  /* Unused for now.. */
	stream->rec_thread_initialized = 1;
	PJ_LOG(5, (THIS_FILE, "Recorder thread started")); 
    }
    if (!stream->quit_flag) {
        pjmedia_frame frame;
        char buf_tmp[BUFF_SIZE];
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = buf;
        frame.size = stream->recordBufferSize;
        frame.timestamp.u64 = stream->rec_timestamp.u64;
        frame.bit_info = 0;

		status = (*stream->rec_cb)(stream->user_data, &frame);
        
        stream->rec_timestamp.u64 += stream->param.samples_per_frame /
                                     stream->param.channel_count;
		
        stream->recordBufIdx %= NUM_BUFFERS;
    }
}

pj_status_t opensl_to_pj_error(SLresult code)
{
    switch(code) {
	case SL_RESULT_SUCCESS:
            return PJ_SUCCESS;
	case SL_RESULT_PRECONDITIONS_VIOLATED:
	case SL_RESULT_PARAMETER_INVALID:
	case SL_RESULT_CONTENT_CORRUPTED:
	case SL_RESULT_FEATURE_UNSUPPORTED:
            return PJMEDIA_EAUD_INVOP;
	case SL_RESULT_MEMORY_FAILURE:
	case SL_RESULT_BUFFER_INSUFFICIENT:
            return PJ_ENOMEM;
	case SL_RESULT_RESOURCE_ERROR:
	case SL_RESULT_RESOURCE_LOST:
	case SL_RESULT_CONTROL_LOST:
            return PJMEDIA_EAUD_NOTREADY;
	case SL_RESULT_CONTENT_UNSUPPORTED:
            return PJ_ENOTSUP;
	default:
            return PJMEDIA_EAUD_ERR;
    }
}

/* Init Android audio driver. */
pjmedia_aud_dev_factory* pjmedia_opensl_factory(pj_pool_factory *pf)
{
    struct opensl_aud_factory *f;
    pj_pool_t *pool;
    
    pool = pj_pool_create(pf, "opensles", 256, 256, NULL);
    f = PJ_POOL_ZALLOC_T(pool, struct opensl_aud_factory);
    f->pf = pf;
    f->pool = pool;
    f->base.op = &opensl_op;
    
    return &f->base;
}

/* API: Init factory */
static pj_status_t opensl_init(pjmedia_aud_dev_factory *f)
{
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    SLresult result;    
    
    /* Create engine */
    result = slCreateEngine(&pa->engineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Cannot create engine %d ", result));
        return opensl_to_pj_error(result);
    }
    
    /* Realize the engine */
    result = (*pa->engineObject)->Realize(pa->engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Cannot realize engine"));
        opensl_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    /* Get the engine interface, which is needed in order to create
     * other objects.
     */
    result = (*pa->engineObject)->GetInterface(pa->engineObject,
                                               SL_IID_ENGINE,
                                               &pa->engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Cannot get engine interface"));
        opensl_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    /* Create output mix */
    result = (*pa->engineEngine)->CreateOutputMix(pa->engineEngine,
                                                  &pa->outputMixObject,
                                                  0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Cannot create output mix"));
        opensl_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    /* Realize the output mix */
    result = (*pa->outputMixObject)->Realize(pa->outputMixObject,
                                             SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "Cannot realize output mix"));
        opensl_destroy(f);
        return opensl_to_pj_error(result);
    }
    
    PJ_LOG(4,(THIS_FILE, "OpenSL sound library initialized"));
    return PJ_SUCCESS;
}

/* API: Destroy factory */
static pj_status_t opensl_destroy(pjmedia_aud_dev_factory *f)
{
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    pj_pool_t *pool;
    
    PJ_LOG(4,(THIS_FILE, "OpenSL sound library shutting down.."));
    
    /* Destroy Output Mix object */
    if (pa->outputMixObject) {
        (*pa->outputMixObject)->Destroy(pa->outputMixObject);
        pa->outputMixObject = NULL;
    }
    
    /* Destroy engine object, and invalidate all associated interfaces */
    if (pa->engineObject) {
        (*pa->engineObject)->Destroy(pa->engineObject);
        pa->engineObject = NULL;
        pa->engineEngine = NULL;
    }
    
    pool = pa->pool;
    pa->pool = NULL;
    pj_pool_release(pool);
    
    return PJ_SUCCESS;
}

/* API: refresh the list of devices */
static pj_status_t opensl_refresh(pjmedia_aud_dev_factory *f)
{
    PJ_UNUSED_ARG(f);
    return PJ_SUCCESS;
}

/* API: Get device count. */
static unsigned opensl_get_dev_count(pjmedia_aud_dev_factory *f)
{
    PJ_UNUSED_ARG(f);
    return 1;
}

/* API: Get device info. */
static pj_status_t opensl_get_dev_info(pjmedia_aud_dev_factory *f,
                                       unsigned index,
                                       pjmedia_aud_dev_info *info)
{
    PJ_UNUSED_ARG(f);

    pj_bzero(info, sizeof(*info));
    
    pj_ansi_strcpy(info->name, "OpenSL ES Audio");
    info->default_samples_per_sec = 8000;
    info->caps = PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING;
    info->input_count = 1;
    info->output_count = 1;
    
    return PJ_SUCCESS;
}

/* API: fill in with default parameter. */
static pj_status_t opensl_default_param(pjmedia_aud_dev_factory *f,
                                        unsigned index,
                                        pjmedia_aud_param *param)
{
    
    pjmedia_aud_dev_info adi;
    pj_status_t status;
    
    status = opensl_get_dev_info(f, index, &adi);
    if (status != PJ_SUCCESS)
	return status;
    
    pj_bzero(param, sizeof(*param));
    if (adi.input_count && adi.output_count) {
        param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
        param->rec_id = index;
        param->play_id = index;
    } else if (adi.input_count) {
        param->dir = PJMEDIA_DIR_CAPTURE;
        param->rec_id = index;
        param->play_id = PJMEDIA_AUD_INVALID_DEV;
    } else if (adi.output_count) {
        param->dir = PJMEDIA_DIR_PLAYBACK;
        param->play_id = index;
        param->rec_id = PJMEDIA_AUD_INVALID_DEV;
    } else {
        return PJMEDIA_EAUD_INVDEV;
    }
    
    param->clock_rate = adi.default_samples_per_sec;
    param->channel_count = 1;
    param->samples_per_frame = adi.default_samples_per_sec * 20 / 1000;
    param->bits_per_sample = 16;
    param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
    param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    
    return PJ_SUCCESS;
}


/* API: create stream */
static pj_status_t opensl_create_stream(pjmedia_aud_dev_factory *f,
                                        const pjmedia_aud_param *param,
                                        pjmedia_aud_rec_cb rec_cb,
                                        pjmedia_aud_play_cb play_cb,
                                        void *user_data,
                                        pjmedia_aud_stream **p_aud_strm)
{
    /* Audio sink for recorder and audio source for player */
    struct opensl_aud_factory *pa = (struct opensl_aud_factory*)f;
    pj_pool_t *pool;
    pj_status_t status = PJ_SUCCESS;
    int i, bufferSize;
    
    /* Only supports for mono channel for now */
    PJ_ASSERT_RETURN(param->channel_count == 1, PJ_EINVAL);
    PJ_ASSERT_RETURN(play_cb && rec_cb && p_aud_strm, PJ_EINVAL);

    PJ_LOG(4,(THIS_FILE, "Creating OpenSL stream"));
    
    pool = pj_pool_create(pa->pf, "openslstrm", 1024, 1024, NULL);
    if (!pool)
        return PJ_ENOMEM;
    
    stream = PJ_POOL_ZALLOC_T(pool, struct opensl_aud_stream);
    stream->pool = pool;
    pj_strdup2_with_null(pool, &stream->name, "OpenSL");
    stream->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
    pj_memcpy(&stream->param, param, sizeof(*param));
    stream->user_data = user_data;
    stream->rec_cb = rec_cb;
    stream->play_cb = play_cb;
    bufferSize = param->samples_per_frame * param->bits_per_sample / 8;
	
    if (stream->dir & PJMEDIA_DIR_PLAYBACK) {

        stream->playerBufferSize = bufferSize;
        for (i = 0; i < NUM_BUFFERS; i++) {
            stream->playerBuffer[i] = (char *)
                                      pj_pool_alloc(stream->pool,
                                                    stream->playerBufferSize);
        }

		player_func = &cubicPlayerCallback;
    }

    if (stream->dir & PJMEDIA_DIR_CAPTURE) {
        stream->recordBufferSize = bufferSize;
		
        for (i = 0; i < NUM_BUFFERS; i++) {
            stream->recordBuffer[i] = (char *)
                                      pj_pool_alloc(stream->pool,
                                                    stream->recordBufferSize);
        }
		
		recorder_func = &cubicRecorderCallback;

    }
    
    if (param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING) {
	strm_set_cap(&stream->base, PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
                     &param->output_vol);
    }
    
    /* Done */
    stream->base.op = &opensl_strm_op;
    *p_aud_strm = &stream->base;
    return PJ_SUCCESS;
    
on_error:
    strm_destroy(&stream->base);
    return status;
}

/* API: Get stream parameters */
static pj_status_t strm_get_param(pjmedia_aud_stream *s,
                                  pjmedia_aud_param *pi)
{
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;
    PJ_ASSERT_RETURN(strm && pi, PJ_EINVAL);
    pj_memcpy(pi, &strm->param, sizeof(*pi));

    if (strm_get_cap(s, PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
                     &pi->output_vol) == PJ_SUCCESS)
    {
        pi->flags |= PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING;
    }    
    
    return PJ_SUCCESS;
}

/* API: get capability */
static pj_status_t strm_get_cap(pjmedia_aud_stream *s,
                                pjmedia_aud_dev_cap cap,
                                void *pval)
{
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;    
    pj_status_t status = PJMEDIA_EAUD_INVCAP;
    
    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);
    
    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING &&
	(strm->param.dir & PJMEDIA_DIR_PLAYBACK))
    {
        if (strm->playerVol) {
            SLresult res;
            SLmillibel vol, mvol;
            
            res = (*strm->playerVol)->GetMaxVolumeLevel(strm->playerVol,
                                                        &mvol);
            if (res == SL_RESULT_SUCCESS) {
                res = (*strm->playerVol)->GetVolumeLevel(strm->playerVol,
                                                         &vol);
                if (res == SL_RESULT_SUCCESS) {
                    *(int *)pval = ((int)vol - SL_MILLIBEL_MIN) * 100 /
                                   ((int)mvol - SL_MILLIBEL_MIN);
                    return PJ_SUCCESS;
                }
            }
        }
    }
    
    return status;
}

/* API: set capability */
static pj_status_t strm_set_cap(pjmedia_aud_stream *s,
                                pjmedia_aud_dev_cap cap,
                                const void *value)
{
    struct opensl_aud_stream *strm = (struct opensl_aud_stream*)s;
    
    PJ_ASSERT_RETURN(s && value, PJ_EINVAL);

    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING &&
	(strm->param.dir & PJMEDIA_DIR_PLAYBACK))
    {
        if (strm->playerVol) {
            SLresult res;
            SLmillibel vol, mvol;
            
            res = (*strm->playerVol)->GetMaxVolumeLevel(strm->playerVol,
                                                        &mvol);
            if (res == SL_RESULT_SUCCESS) {
                vol = (SLmillibel)(*(int *)value *
                      ((int)mvol - SL_MILLIBEL_MIN) / 100 + SL_MILLIBEL_MIN);
                res = (*strm->playerVol)->SetVolumeLevel(strm->playerVol,
                                                         vol);
                if (res == SL_RESULT_SUCCESS)
                    return PJ_SUCCESS;
            }
        }
    }

    return PJMEDIA_EAUD_INVCAP;
}

/* API: start stream. */
static pj_status_t strm_start(pjmedia_aud_stream *s)
{
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    int i;
    SLresult result = SL_RESULT_SUCCESS;
    
    PJ_LOG(4, (THIS_FILE, "Starting %s stream..", stream->name.ptr));
    stream->quit_flag = 0;

    if (stream->recordBufQ && stream->recordRecord) {
        /* Enqueue an empty buffer to be filled by the recorder
         * (for streaming recording, we need to enqueue at least 2 empty
         * buffers to start things off)
         */
        for (i = 0; i < NUM_BUFFERS; i++) {
            result = (*stream->recordBufQ)->Enqueue(stream->recordBufQ,
                                                stream->recordBuffer[i],
                                                stream->recordBufferSize);
            /* The most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
             * which for this code would indicate a programming error
             */
            pj_assert(result == SL_RESULT_SUCCESS);
        }
        
        result = (*stream->recordRecord)->SetRecordState(
                     stream->recordRecord, SL_RECORDSTATE_RECORDING);
        if (result != SL_RESULT_SUCCESS) {
            PJ_LOG(3, (THIS_FILE, "Cannot start recorder"));
            goto on_error;
        }
    }
    
    if (stream->playerPlay && stream->playerBufQ) {
        /* Set the player's state to playing */
        result = (*stream->playerPlay)->SetPlayState(stream->playerPlay,
                                                     SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            PJ_LOG(3, (THIS_FILE, "Cannot start player"));
            goto on_error;
        }
        
        for (i = 0; i < NUM_BUFFERS; i++) {
            pj_bzero(stream->playerBuffer[i], stream->playerBufferSize/100);
            result = (*stream->playerBufQ)->Enqueue(stream->playerBufQ,
                                                stream->playerBuffer[i],
                                                stream->playerBufferSize/100);
            pj_assert(result == SL_RESULT_SUCCESS);
        }
    }
    
    PJ_LOG(4, (THIS_FILE, "%s stream started", stream->name.ptr));
    return PJ_SUCCESS;
    
on_error:
    if (result != SL_RESULT_SUCCESS)
        strm_stop(&stream->base);
    return opensl_to_pj_error(result);
}

/* API: stop stream. */
static pj_status_t strm_stop(pjmedia_aud_stream *s)
{
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    
    if (stream->quit_flag)
        return PJ_SUCCESS;
    
    PJ_LOG(4, (THIS_FILE, "Stopping stream"));
    
    stream->quit_flag = 1;    
    
    if (stream->recordBufQ && stream->recordRecord) {
        /* Stop recording and clear buffer queue */
        (*stream->recordRecord)->SetRecordState(stream->recordRecord,
                                                  SL_RECORDSTATE_STOPPED);
        (*stream->recordBufQ)->Clear(stream->recordBufQ);
    }

    if (stream->playerBufQ && stream->playerPlay) {
        /* Wait until the PCM data is done playing, the buffer queue callback
         * will continue to queue buffers until the entire PCM data has been
         * played. This is indicated by waiting for the count member of the
         * SLBufferQueueState to go to zero.
         */
/*      
        SLresult result;
        W_SLBufferQueueState state;

        result = (*stream->playerBufQ)->GetState(stream->playerBufQ, &state);
        while (state.count) {
            (*stream->playerBufQ)->GetState(stream->playerBufQ, &state);
        } */
        /* Stop player */
        (*stream->playerPlay)->SetPlayState(stream->playerPlay,
                                            SL_PLAYSTATE_STOPPED);
    }

    PJ_LOG(4,(THIS_FILE, "OpenSL stream stopped"));
    
    return PJ_SUCCESS;
    
}

/* API: destroy stream. */
static pj_status_t strm_destroy(pjmedia_aud_stream *s)
{    
    struct opensl_aud_stream *stream = (struct opensl_aud_stream*)s;
    
    /* Stop the stream */
    strm_stop(s);
    
    if (stream->playerObj) {
        /* Destroy the player */
        (*stream->playerObj)->Destroy(stream->playerObj);
        /* Invalidate all associated interfaces */
        stream->playerObj = NULL;
        stream->playerPlay = NULL;
        stream->playerBufQ = NULL;
        stream->playerVol = NULL;
    }
    
    if (stream->recordObj) {
        /* Destroy the recorder */
        (*stream->recordObj)->Destroy(stream->recordObj);
        /* Invalidate all associated interfaces */
        stream->recordObj = NULL;
        stream->recordRecord = NULL;
        stream->recordBufQ = NULL;
    }
    
    pj_pool_release(stream->pool);
    PJ_LOG(4, (THIS_FILE, "OpenSL stream destroyed"));
    
    return PJ_SUCCESS;
}

#endif	/* PJMEDIA_AUDIO_DEV_HAS_OPENSL */
