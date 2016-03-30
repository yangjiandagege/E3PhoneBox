/**
 * @file: PstnCaller.cc
 * @auth: lishujie
 * @brif: PSTN caller work with engine
 */


#ifndef _PSTN_CALLER_CC_
#define _PSTN_CALLER_CC_ 1

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <limits.h>
#include <fcntl.h>
#include <linux/input.h>
#include <map>
#include "CSafeQueue.cc"
#include "CStatMachine.cc"
#include "DtmfGenerator.cc"
#include "FskDetector.hpp"
#include "module_common_types.h"
#include "audio_processing.h"

#define TAG "PstnCaller"
#include "ThirAndroidLog.h"

using namespace std;


#define RETIF( condition ) if( condition ){ return; }
#define RETNIF( condition, n ) if( condition ){ return n; }
#define BREAKIF( condition ) if( condition ){ break; }
#ifndef KEY_CALL
#define KEY_CALL 231
#endif


#define _USE_SOUND_ARRAY 0
#define _RECORD_TEST_SOUND 0
#define _USE_WEBRTC 1

#if _USE_WEBRTC
using namespace webrtc;
#endif //_USE_WEBRTC

#if _USE_SOUND_ARRAY
#include "linked-8k-16b-mono.c"
#endif //_USE_SOUND_ARRAY


class IPstnUser
{
public:
    virtual ~IPstnUser(){};
	virtual bool onIncommingCall( const char *number ) = 0;
    virtual void onSound(const void* data, int size) = 0;
};

class PstnCaller
{
public:
	static const int DEFAULT_LISTEN_TIMEOUT = 1000;
	static const int SOUND_BIT = 16;
	static const int SOUND_DEVICE_SAMPLE_RATE = 8000;
	static const int SOUND_SAMPLE_RATE = 16000;
	static const int SOUND_FRAME_LEN = 40; // ms
	static const int SOUND_SAMPLE_NUM_IN_DEVICE_FRAME = (SOUND_DEVICE_SAMPLE_RATE * SOUND_FRAME_LEN / 1000);
	static const int SOUND_SAMPLE_NUM_IN_FRAME = (SOUND_SAMPLE_RATE * SOUND_FRAME_LEN / 1000);
	static const int SOUND_DEVICE_FRAME_SIZE = (SOUND_BIT >> 3) *SOUND_SAMPLE_NUM_IN_DEVICE_FRAME;
	static const int SOUND_FRAME_SIZE = (SOUND_BIT >> 3) *SOUND_SAMPLE_NUM_IN_FRAME;
	static const int SOUND_BUFF_LEN = 1000; // ms
	static const int SOUND_BUFF_SIZE = SOUND_BUFF_LEN / SOUND_FRAME_LEN;
	static const int DTMF_FRAME_SIZE = SOUND_SAMPLE_NUM_IN_DEVICE_FRAME;
#if _USE_WEBRTC
    static const int WEBRTC_PAYLOAD_SIZE = SOUND_DEVICE_SAMPLE_RATE * 10 / 1000;
#endif // _USE_WEBRTC

private:
	typedef void * (*thread_func)(void *);
	typedef short Sample;
	typedef union {
        unsigned char byte[SOUND_DEVICE_FRAME_SIZE];
        Sample sample[SOUND_SAMPLE_NUM_IN_DEVICE_FRAME];
    }DeviceFrame;
    typedef union {
        unsigned char byte[SOUND_FRAME_SIZE];
        Sample sample[SOUND_SAMPLE_NUM_IN_FRAME];
    }Frame;

    bool                     m_is_inited;
	CStatMachine             m_sm;
	IPstnUser               *m_user;
	CSafeQueue<DeviceFrame>  m_buf_sound_r_pcm;
	CSafeQueue<DeviceFrame>  m_buf_sound_w_pcm;

	int                      m_fd_event_input;
	int                      m_fd_ctrl;
	int                      m_fd_sound_r_pcm;
	int                      m_fd_sound_w_pcm;
#if _RECORD_TEST_SOUND
    int                      m_fd_sound_r_record;
    int                      m_fd_sound_w_record;
#endif //_RECORD_TEST_SOUND

	pthread_t                m_thread_event_input;
	pthread_t                m_thread_sound_r_pcm;
	pthread_t                m_thread_sound_w_pcm;
	pthread_t                m_thread_sound_upload;

	bool                     m_thread_stop_flag_event_input;
	bool                     m_thread_stop_flag_sound_r_pcm;
	bool                     m_thread_stop_flag_sound_w_pcm;
    bool                     m_thread_stop_flag_sound_upload;

#if _USE_WEBRTC
    int                      m_webrtc_analog_level;
    AudioProcessing*         m_webrtc_audio_pstn_processing;
#endif //_USE_WEBRTC

#if _USE_SOUND_ARRAY
    int                      m_sound_r_pos;
    int                      m_sound_w_pos;
#endif //_USE_SOUND_ARRAY

    typedef struct FileCfgT{
        char path[PATH_MAX+4];
        int mode;
        int right;
        FileCfgT()
        {
        	memset(path, 0, PATH_MAX+4);
        	mode = 0;
        	right = 0;
        };
        FileCfgT(const char* s, int m = O_RDWR, int r = 0)
        {
        	memset(path, 0, PATH_MAX+4);
        	strncpy(path, s, PATH_MAX);
        	mode = m;
        	right = r;
        };
    } FileCfg;
    map<int*, FileCfg>       m_file_descriptor_map;

	// ================== control const value for Si3050 ========================
	enum
	{
		VDAA_DIG_LOOPBACK = 1,
		VDAA_ONHOOK = 2,
		VDAA_OFFHOOK = 3,
		VDAA_ONHOOK_MONITOR = 4,
		VDAA_INIT = 5,
		VDAA_FSK_BEGIN = 6,
		VDAA_FSK_END = 7,
	};

	// ===================== control const value for PCM =========================
#define AUD_DRV_IOC_MAGIC 'C'
#define AUD_PCM_ITF_TEST             _IOWR(AUD_DRV_IOC_MAGIC, 0x07, int)
#define AUD_PCM_ITF_TEST2            _IOWR(AUD_DRV_IOC_MAGIC, 0x08, int)
#define AUD_PCM_ITF_SLAVE            _IOWR(AUD_DRV_IOC_MAGIC, 0x09, int)
#define START_MEMIF_TYPE             _IOWR(AUD_DRV_IOC_MAGIC, 0x20,unsigned int)
#define STANDBY_MEMIF_TYPE           _IOWR(AUD_DRV_IOC_MAGIC, 0x21,unsigned int)
#define ALLOCATE_MEMIF_DL2           _IOWR(AUD_DRV_IOC_MAGIC, 0x12,unsigned int)
#define FREE_MEMIF_DL2               _IOWR(AUD_DRV_IOC_MAGIC, 0x13,unsigned int)
#define ALLOCATE_MEMIF_MODDAI       _IOWR(AUD_DRV_IOC_MAGIC, 0x1a,unsigned int)
#define FREE_MEMIF_MODDAI            _IOWR(AUD_DRV_IOC_MAGIC, 0x1b,unsigned int)

	enum
	{
		MEM_DL1,
		MEM_DL2,
		MEM_VUL,
		MEM_DAI,
		MEM_I2S, // Cuurently not used. Add for sync with user space
		MEM_AWB,
		MEM_MOD_DAI,
		NUM_OF_MEM_INTERFACE
	};

	// ============================ status machine ===============================
	static const TStat STAT_IDLE = STAT_BEGIN + 1;
	static const TStat STAT_RINGING = STAT_BEGIN + 2;
	static const TStat STAT_CONVERSATION = STAT_BEGIN + 3;

	static const TEvt EVT_BEGIN = EVT_DEFAULT;
	static const TEvt EVT_DAIL = EVT_DEFAULT + 1; // outgoing call, from User to PSTN
	static const TEvt EVT_RING = EVT_DEFAULT + 2; // incomming call, from PSTN to User, event receive from input
	static const TEvt EVT_USR_HANGUP = EVT_DEFAULT + 3;  // hange up, by User
	static const TEvt EVT_ACCEPT = EVT_DEFAULT + 4; // accept, by User


	class DialPstnAction : public IAction
	{
	private:
		PstnCaller         *m_caller;
		DtmfGenerator       m_dtmf_generator;

	public:
		DialPstnAction(PstnCaller *caller)
			: m_caller(caller)
			, m_dtmf_generator(DTMF_FRAME_SIZE, 100, 30)
		{};

		virtual bool onStatChange(TStat stat_from, TEvt evt, TStat stat_to, void *data)
		{
		    const int DELAY_TIME = SOUND_FRAME_LEN * 1000;
			DeviceFrame buf_dev;
            int ret = 0;

            LOGD("PstnCaller::DialPstnAction::onStatChange");
			RETNIF(!m_caller, false );

            ioctrlFile(m_caller->m_fd_ctrl, VDAA_OFFHOOK, NULL);
			usleep(1000);
			ioctrlFile(m_caller->m_fd_ctrl, VDAA_ONHOOK_MONITOR, NULL);

            sleep(1);
			m_dtmf_generator.dtmfGeneratorReset();
			m_dtmf_generator.transmitNewDialButtonsArray((char *)data, strlen((char *)data));

            usleep(DELAY_TIME);
            while(!m_dtmf_generator.getReadyFlag())
            {
            	memset(buf_dev.byte, 0, SOUND_DEVICE_FRAME_SIZE);
                m_dtmf_generator.dtmfGenerating(buf_dev.sample); // Generating of a 16 bit's little-endian's pcm samples array
                m_caller->m_buf_sound_w_pcm.push(buf_dev);
            }

        #if _USE_WEBRTC
            if( m_caller->m_webrtc_audio_pstn_processing )
            {
                m_caller->m_webrtc_audio_pstn_processing->Initialize();
            }
        #endif //_USE_WEBRTC

			return true;
		}
	};

	class DialUserAction : public IAction
	{
	private:
		PstnCaller         *m_caller;

	public:
		DialUserAction(PstnCaller *caller)
			: m_caller(caller)
		{};

		virtual bool onStatChange(TStat stat_from, TEvt evt, TStat stat_to, void *data)
		{
            LOGD("PstnCaller::DialUserAction::onStatChange");
			RETNIF(!m_caller, false );
			return m_caller->m_user->onIncommingCall( (char*)data ); // dail user;
		}
	};

	class HangPstnAction : public IAction
	{
	private:
		PstnCaller         *m_caller;

	public:
		HangPstnAction(PstnCaller *caller)
			: m_caller(caller)
		{};

		virtual bool onStatChange(TStat stat_from, TEvt evt, TStat stat_to, void *data)
		{
		    LOGD("PstnCaller::HangPstnAction::onStatChange");
			RETNIF(!m_caller, false );

			ioctrlFile(m_caller->m_fd_ctrl, VDAA_ONHOOK_MONITOR, NULL);
			usleep(1000);
			ioctrlFile(m_caller->m_fd_ctrl, VDAA_OFFHOOK, NULL);
			return true;
		}
	};

	class AcceptAction : public IAction
	{
	private:
		PstnCaller         *m_caller;

	public:
		AcceptAction(PstnCaller *caller)
			: m_caller(caller)
		{};

		virtual bool onStatChange(TStat stat_from, TEvt evt, TStat stat_to, void *data)
		{
		    LOGD("PstnCaller::AcceptAction::onStatChange");
			RETNIF(!m_caller, false );

        #if _USE_WEBRTC
            if( m_caller->m_webrtc_audio_pstn_processing )
            {
                m_caller->m_webrtc_audio_pstn_processing->Initialize();
            }
        #endif //_USE_WEBRTC
			ioctrlFile(m_caller->m_fd_ctrl, VDAA_ONHOOK_MONITOR, NULL);
			return true;
		}
	};

	friend class DialPstnAction;
	friend class DialUserAction;
	friend class HangPstnAction;
	friend class AcceptAction;

	DialPstnAction           m_dail_pstn;
	DialUserAction           m_dail_user;
	HangPstnAction           m_hangup_pstn;
	AcceptAction             m_accept_pstn;


	// ========================== private method ===================================
	inline bool openFile( int &n_fd )
	{
		if( n_fd >= 0 || m_file_descriptor_map.find(&n_fd) == m_file_descriptor_map.end() )
		{
			// already opened
			return true;
		}

        if( m_file_descriptor_map[&n_fd].right != 0 )
        {
            n_fd = open(m_file_descriptor_map[&n_fd].path, m_file_descriptor_map[&n_fd].mode, m_file_descriptor_map[&n_fd].right);
        }
        else
        {
            n_fd = open(m_file_descriptor_map[&n_fd].path, m_file_descriptor_map[&n_fd].mode);
        }

		if( n_fd < 0 )
		{
			perror("Open File");
			fprintf(stderr, "Fail to Open: %s \n", m_file_descriptor_map[&n_fd].path);
			n_fd = -1;
			return false;
		}
		return true;
	};

	inline void closeFile( int &n_fd )
	{
		if( n_fd >= 0 )
		{
			close(n_fd);
		}
		n_fd = -1;
	};

	static inline bool ioctrlFile(int &n_fd, int cmd, int32_t val)
	{
		if( 0 != ioctl(n_fd, cmd, val))
		{
			perror("ioctl error!\n");
			return false;
		}
		return true;
	};

	bool startThread(pthread_t &t_thread, bool &b_stop, thread_func p_fn )
	{
		if( t_thread )
		{
			return true;
		}

		b_stop = false;
		if( 0 != pthread_create(&t_thread, NULL, p_fn, this) )
		{
			LOGD("start thread  pstn error");
			return false;
		}
		return true;
	};

	void stopThread(pthread_t &t_thread, bool &b_stop)
	{
		if(!t_thread)
		{
			return;
		}

		b_stop = true;
		pthread_join(t_thread, NULL);
		t_thread = NULL;
	};

	static int waitRead(int &n_fd, int n_timeout)
	{
		int     ret = 0;
		int     maxfd = 0;
		fd_set  fds;
		struct timeval timeout = { n_timeout / 1000, n_timeout % 1000 };

		memset(&fds, 0, sizeof(fd_set));
		if( n_fd <= 0 )
		{
			return -1;
		}

		/* init fds */
		FD_ZERO( &fds );
		FD_SET( n_fd, &fds );
		maxfd = n_fd;
		maxfd ++;

		/* select read*/
		ret = select( maxfd, &fds, 0, 0, &timeout );
		switch( ret )
		{
		case -1:
			perror("Select File");
			return -1;
		case 0:
			return 0;
		default:
			if( FD_ISSET( n_fd, &fds ) )
			{
				return 1;
			}
			break;
		}
		return -1;
	};


    void upgrade_sample( Frame &f, DeviceFrame &df )
    {
    	Sample *dst = f.sample;
    	Sample *src = df.sample;
        Sample last;
        int i = 0;
    	int j = 0;

        // first sample
        last = src[i++];
        dst[j++] = last;
        
        while(i < SOUND_SAMPLE_NUM_IN_DEVICE_FRAME)
        {
            dst[j++] = ((last + src[i]) >> 1);
            last = src[i++];
            dst[j++] = last;
        };

        // last sample
        dst[j] = last;
    };

	void downgrade_sample( DeviceFrame &df, Frame &f )
	{
		Sample *dst = df.sample;
		Sample *src = f.sample;

		for( int i = 0; i < SOUND_SAMPLE_NUM_IN_FRAME; i += 2 )
		{
			dst[i>>1] = src[i];
		}
	};

#if _USE_WEBRTC
    void webrtc_process_frame( DeviceFrame &df, int delay, int analog_level )
    {
        AudioFrame webrtc_frame;

        // webrtc frame setting
        webrtc_frame._frequencyInHz = SOUND_DEVICE_SAMPLE_RATE;
        webrtc_frame._audioChannel = 1;
        webrtc_frame._payloadDataLengthInSamples = WEBRTC_PAYLOAD_SIZE;

	    for( int i = 0; i + WEBRTC_PAYLOAD_SIZE <= SOUND_DEVICE_FRAME_SIZE; i+=WEBRTC_PAYLOAD_SIZE )
        {
            memcpy( webrtc_frame._payloadData, df.byte + i, WEBRTC_PAYLOAD_SIZE);
            m_webrtc_audio_pstn_processing->set_stream_delay_ms(delay);
            m_webrtc_audio_pstn_processing->gain_control()->set_stream_analog_level(analog_level);
            m_webrtc_audio_pstn_processing->ProcessStream(&webrtc_frame);
            memcpy( df.byte + i, webrtc_frame._payloadData, WEBRTC_PAYLOAD_SIZE);
        }
    };

    int webrtc_analysis_frame( DeviceFrame &df )
    {
        AudioFrame webrtc_frame;

        // webrtc frame setting
        webrtc_frame._frequencyInHz = SOUND_DEVICE_SAMPLE_RATE;
        webrtc_frame._audioChannel = 1;
        webrtc_frame._payloadDataLengthInSamples = WEBRTC_PAYLOAD_SIZE;

	    for( int i = 0; i + WEBRTC_PAYLOAD_SIZE <= SOUND_DEVICE_FRAME_SIZE; i += WEBRTC_PAYLOAD_SIZE )
        {
            memcpy( webrtc_frame._payloadData, df.byte + i, WEBRTC_PAYLOAD_SIZE);
            m_webrtc_audio_pstn_processing->AnalyzeReverseStream(&webrtc_frame);
        }
        return m_webrtc_audio_pstn_processing->gain_control()->stream_analog_level();
    };
#endif //_USE_WEBRTC

	// ================================= incomming call event =======================================
	static void *static_event_input_proc( void *data )
	{
		PstnCaller *p_this = (PstnCaller *)data;
		p_this->input_event_proc();
		return NULL;
	};

	void input_event_proc()
	{
		int ret;
		struct input_event evt;
		const int evt_size = sizeof( evt );

        LOGD("PstnCaller:input_event_proc:begin");

		while( !m_thread_stop_flag_event_input )
		{
			ret = waitRead( m_fd_event_input, DEFAULT_LISTEN_TIMEOUT );
			if( 0 == ret ) // timeout, so try again
			{
				continue;
			}
			else if( 0 > ret )// error, stop
			{
				break;
			}

			// read input
			if( read( m_fd_event_input, &evt, evt_size ) != evt_size &&
					evt.type != EV_KEY )
			{
				continue;
			}

			if( evt.code == KEY_CALL && evt.value >= 1 )
			{
			#if 0 // fsk detector
			    FskDetector fsk_detector("/dev/ttyMT1");
                ioctrlFile(m_fd_ctrl, VDAA_FSK_BEGIN, NULL);
			    ret = fsk_detector.detect();
                ioctrlFile(m_fd_ctrl, VDAA_FSK_END, NULL);
                if( 0 > ret )
                {
                    LOGD("PstnCaller::input_event_proc fsk_detector ret=%d", ret);
                    continue;
                }

                char number[FskDetector::PHONE_NUMBER_LEN_MAX+4] = {0};
                strncpy( number, fsk_detector.getPhoneNumber(), FskDetector::PHONE_NUMBER_LEN_MAX);
                LOGD("PstnCaller::input_event_proc number=%s", number);
            #else // fsk detector
                char number[FskDetector::PHONE_NUMBER_LEN_MAX+4] = {0};
                snprintf(number, FskDetector::PHONE_NUMBER_LEN_MAX, "123456789");
            #endif // fsk detector
			    LOGD("PstnCaller::input_event_proc::EVT_RING");
				CStatMachine::ErrCode ret = m_sm.input(EVT_RING, number);
			}
		}
        LOGD("PstnCaller:input_event_proc:end");
	};

	// ================================= read sound =======================================
	static void *static_sound_r_pcm_proc( void *data )
	{
		PstnCaller *p_this = (PstnCaller *)data;
		p_this->sound_r_pcm_proc();
		return NULL;
	};

	void sound_r_pcm_proc()
	{
	    const int MAX_RETRY = 256;
		DeviceFrame buf_dev;
		int ret;
		int offset = 0;
        int failed = 0;

        LOGD("PstnCaller:sound_r_pcm_proc:begin");

        int enable = 1;
        ioctrlFile(m_fd_sound_r_pcm, AUD_PCM_ITF_SLAVE, (int32_t)(&enable));

		while( !m_thread_stop_flag_sound_r_pcm )
		{
		#if _USE_SOUND_ARRAY
            usleep(SOUND_FRAME_LEN*1000);

            memcpy(buf_dev.byte, sound_array+m_sound_r_pos, SOUND_DEVICE_FRAME_SIZE);
            m_buf_sound_r_pcm.push(buf_dev); 
            m_sound_r_pos += SOUND_DEVICE_FRAME_SIZE;
            if( m_sound_r_pos > SOUND_ARRRY_SIZE )
            {
                m_sound_r_pos = 0;
            }
        #else //_USE_SOUND_ARRAY
			ret = waitRead( m_fd_sound_r_pcm, DEFAULT_LISTEN_TIMEOUT );
			if( 0 == ret )
			{
				// timeout, so try again
				continue;
			}
			else if( 0 > ret )
			{
				// error, stop
				perror("Select File");
                failed = 1;
				break;
			}

			ret = read( m_fd_sound_r_pcm, buf_dev.byte + offset, SOUND_DEVICE_FRAME_SIZE - offset );
			if( ret <= 0 )
			{
				perror("Read File");
                failed = 1;
				break;
			}

			offset += ret;
			if( offset >= SOUND_DEVICE_FRAME_SIZE )
			{
			#if _USE_WEBRTC
			    webrtc_process_frame(buf_dev, 1, m_webrtc_analog_level);
            #endif //_USE_WEBRTC

            #if _RECORD_TEST_SOUND
                write(m_fd_sound_r_record, buf_dev.byte, SOUND_DEVICE_FRAME_SIZE);
            #endif //_RECORD_TEST_SOUND

				m_buf_sound_r_pcm.push(buf_dev); // save to buffer

				offset = 0;
				memset(buf_dev.byte, 0, SOUND_DEVICE_FRAME_SIZE);
			}
        #endif //_USE_SOUND_ARRAY
		}

        int disable = 0;
        ioctrlFile(m_fd_sound_r_pcm, AUD_PCM_ITF_SLAVE, (int32_t)(&disable));

        if( failed != 0 )
        {
            LOGE("PstnCaller:sound_r_pcm_proc: reboot for ERROR_PCM_DEVICE_FAILED");
            //system("reboot -p");
        }
        
        LOGD("PstnCaller:sound_r_pcm_proc:end");
	};

	// ================================= write sound =======================================
	static void *static_sound_w_pcm_proc( void *data )
	{
		PstnCaller *p_this = (PstnCaller *)data;
		p_this->sound_w_pcm_proc();
		return NULL;
	};

	void sound_w_pcm_proc()
	{
        const int DELAY_TIME = SOUND_FRAME_LEN * 1000 /2;
		int ret;
		DeviceFrame buf_dev;
		int on_off = 0;
        int failed = 0;


        LOGD("PstnCaller:sound_w_pcm_proc:begin:%d", SOUND_DEVICE_FRAME_SIZE);

        int enable = 1;
        ioctrlFile(m_fd_sound_w_pcm, AUD_PCM_ITF_SLAVE, (int32_t)(&enable));

		while( !m_thread_stop_flag_sound_w_pcm )
		{
		#if _USE_SOUND_ARRAY
            if( m_sm.current() != STAT_CONVERSATION ) {
                usleep(10000);
                continue;
            }
        
            usleep(SOUND_FRAME_LEN*1000);
            memcpy(buf_dev.byte, sound_array+m_sound_w_pos, SOUND_DEVICE_FRAME_SIZE);
            ret = write(m_fd_sound_w_pcm, buf_dev.byte, SOUND_DEVICE_FRAME_SIZE);
            if( ret != SOUND_DEVICE_FRAME_SIZE )
            {
                perror("write buffer");
                continue;
            }
            m_sound_w_pos += SOUND_DEVICE_FRAME_SIZE;  
            if( m_sound_w_pos >= SOUND_ARRRY_SIZE )
            {
                m_sound_w_pos = 0;
            }
        #else //_USE_SOUND_ARRAY
			if( CSafeQueue<DeviceFrame>::ERR_NO_ERROR != m_buf_sound_w_pcm.pop(buf_dev, DEFAULT_LISTEN_TIMEOUT) )
			{
				continue;
			}

        #if _USE_WEBRTC
            m_webrtc_analog_level = webrtc_analysis_frame(buf_dev);
        #endif //_USE_WEBRTC

        #if _RECORD_TEST_SOUND
            write(m_fd_sound_w_record, buf_dev.byte, SOUND_DEVICE_FRAME_SIZE);
        #endif //_RECORD_TEST_SOUND
			ret = write(m_fd_sound_w_pcm, buf_dev.byte, SOUND_DEVICE_FRAME_SIZE);
			if( ret != SOUND_DEVICE_FRAME_SIZE )
			{
				perror("write buffer");
                failed = 1;
				break;
			}
            usleep(DELAY_TIME);
        #endif //_USE_SOUND_ARRAY
		}

        int disable = 0;
        ioctrlFile(m_fd_sound_w_pcm, AUD_PCM_ITF_SLAVE, (int32_t)(&disable));

        if( failed != 0 )
        {
            LOGE("PstnCaller:sound_w_pcm_proc: reboot for ERROR_PCM_DEVICE_FAILED");
            system("reboot -p");
        }
        
        LOGD("PstnCaller:sound_w_pcm_proc:end");
	};

    // ================================= upload sound =======================================
	static void *static_sound_upload_proc( void *data )
	{
		PstnCaller *p_this = (PstnCaller *)data;
		p_this->sound_upload_proc();
		return NULL;
	};

	void sound_upload_proc()
	{
		int ret;
		Frame buf;
        DeviceFrame buf_dev;

        LOGD("PstnCaller:sound_upload_proc:begin");

		while( !m_thread_stop_flag_sound_upload )
		{
			if( CSafeQueue<DeviceFrame>::ERR_NO_ERROR != m_buf_sound_r_pcm.pop(buf_dev, DEFAULT_LISTEN_TIMEOUT) )
			{
				continue;
			}

            upgrade_sample(buf, buf_dev);
			m_user->onSound(buf.byte, SOUND_FRAME_SIZE);
		}

        LOGD("PstnCaller:sound_upload_proc:end");
	};

public:
	PstnCaller( IPstnUser *p_user )
		: m_is_inited(false)
		, m_sm()
		, m_user(p_user)
		, m_buf_sound_r_pcm(SOUND_BUFF_SIZE)
		, m_buf_sound_w_pcm(SOUND_BUFF_SIZE)
		, m_fd_event_input(-1)
		, m_fd_ctrl(-1)
		, m_fd_sound_r_pcm(-1)
		, m_fd_sound_w_pcm(-1)
	#if _RECORD_TEST_SOUND
        , m_fd_sound_r_record(-1)
        , m_fd_sound_w_record(-1)
    #endif // _RECORD_TEST_SOUND
		, m_thread_event_input(NULL)
		, m_thread_sound_r_pcm(NULL)
		, m_thread_sound_w_pcm(NULL)
		, m_thread_stop_flag_event_input(false)
		, m_thread_stop_flag_sound_r_pcm(false)
		, m_thread_stop_flag_sound_w_pcm(false)
		, m_dail_pstn(this)
		, m_dail_user(this)
		, m_hangup_pstn(this)
		, m_accept_pstn(this)
    #if _USE_WEBRTC
		, m_webrtc_audio_pstn_processing(NULL)
	#endif //_USE_WEBRTC
    #if _USE_SOUND_ARRAY
        , m_sound_r_pos(0)
        , m_sound_w_pos(0)
    #endif //_USE_SOUND_ARRAY
	{
        // ************************** fd map ************************************
        m_file_descriptor_map[&m_fd_event_input] = FileCfg("/dev/input/event1");
        m_file_descriptor_map[&m_fd_ctrl] = FileCfg("/dev/slic");
        m_file_descriptor_map[&m_fd_sound_r_pcm] = FileCfg("/dev/eac");
        m_file_descriptor_map[&m_fd_sound_w_pcm] = FileCfg("/dev/eac");
    #if _RECORD_TEST_SOUND
        m_file_descriptor_map[&m_fd_sound_r_record] = FileCfg("/mnt/shell/emulated/0/sound_r_record.pcm", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        m_file_descriptor_map[&m_fd_sound_w_record] = FileCfg("/mnt/shell/emulated/0/sound_w_record.pcm", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    #endif //_RECORD_TEST_SOUND

		// *********************** Stat Machine *********************************
		m_sm.insertAction(STAT_BEGIN,       EVT_BEGIN,        STAT_IDLE,         NULL);

		// idle stat
		m_sm.insertAction(STAT_IDLE,          EVT_DAIL,         STAT_CONVERSATION, &m_dail_pstn);
		m_sm.insertAction(STAT_IDLE,          EVT_RING,         STAT_RINGING,      &m_dail_user);

		// ring
		m_sm.insertAction(STAT_RINGING,       EVT_USR_HANGUP,   STAT_IDLE,         &m_hangup_pstn);
		m_sm.insertAction(STAT_RINGING,       EVT_ACCEPT,       STAT_CONVERSATION, &m_accept_pstn);
		m_sm.insertAction(STAT_RINGING,       EVT_RING,         STAT_RINGING,      &m_dail_user);

		// conversation
		m_sm.insertAction(STAT_CONVERSATION,  EVT_USR_HANGUP,   STAT_IDLE,         &m_hangup_pstn);
		m_sm.insertAction(STAT_CONVERSATION,  EVT_DAIL,         STAT_CONVERSATION, &m_dail_pstn);

    #if _USE_WEBRTC
        m_webrtc_audio_pstn_processing = AudioProcessing::Create(0);
    #endif //_USE_WEBRTC
	};

	~PstnCaller()
	{
	#if _USE_WEBRTC
	    AudioProcessing::Destroy(m_webrtc_audio_pstn_processing);
        m_webrtc_audio_pstn_processing = NULL;
    #endif //_USE_WEBRTC
	};

	bool init()
	{
	    LOGD("PstnCaller:init");
		if(m_is_inited)
	    {
	        return true;
	    }

		do
		{
			// open device file
			BREAKIF( !openFile(m_fd_event_input) );
			BREAKIF( !openFile(m_fd_ctrl) );
			BREAKIF( !openFile(m_fd_sound_r_pcm) );
			BREAKIF( !openFile(m_fd_sound_w_pcm) );

        #if _RECORD_TEST_SOUND
            BREAKIF( !openFile(m_fd_sound_r_record) );
			BREAKIF( !openFile(m_fd_sound_w_record) );
        #endif //_RECORD_TEST_SOUND

			// set device configuration
			BREAKIF( !ioctrlFile(m_fd_ctrl, VDAA_INIT, 0) );
			BREAKIF( !ioctrlFile(m_fd_sound_r_pcm, START_MEMIF_TYPE, MEM_MOD_DAI) ); // set as read
			BREAKIF( !ioctrlFile(m_fd_sound_w_pcm, START_MEMIF_TYPE, MEM_DL2) ); // set as write

            // setup webrtc audio processing
        #if _USE_WEBRTC 
            // setup general setting
            BREAKIF( !m_webrtc_audio_pstn_processing );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->set_sample_rate_hz(SOUND_DEVICE_SAMPLE_RATE) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->set_num_channels(1, 1) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->set_num_reverse_channels(1) );

            // high pass filter
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->high_pass_filter()->Enable(true) );

            // echo cancellation (AEC)
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->echo_cancellation()->set_device_sample_rate_hz(SOUND_DEVICE_SAMPLE_RATE) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->echo_cancellation()->enable_drift_compensation(false) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->echo_cancellation()->enable_metrics(true) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->echo_cancellation()->set_suppression_level(EchoCancellation::kLowSuppression) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->echo_cancellation()->Enable(true) );
            
            // gain control (AGC)
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->gain_control()->set_analog_level_limits( 0x7F, 0x7FFF) );
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->gain_control()->enable_limiter(true) );
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->gain_control()->set_mode(GainControl::kAdaptiveDigital) );
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->gain_control()->Enable(true) );

            // noise suppression (NS) 
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->noise_suppression()->set_level(NoiseSuppression::kLow) );
            BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->noise_suppression()->Enable(true) );

            // voice activity detection (VAD)
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->voice_detection()->set_likelihood(VoiceDetection::kLowLikelihood) );
            //BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->voice_detection()->Enable(true) );

            // done then initialize
            // BREAKIF( AudioProcessing::kNoError != m_webrtc_audio_pstn_processing->Initialize() );
        #endif //_USE_WEBRTC

        	// start listen thread
    	    BREAKIF( !startThread(m_thread_event_input, m_thread_stop_flag_event_input, static_event_input_proc) );
    	    BREAKIF( !startSound() );
            
            m_sm.input(EVT_BEGIN);
			m_is_inited = true;
		}
		while(0);

		if( !m_is_inited )
		{
			deinit();
		}
		return m_is_inited;
	};

	void deinit()
	{
	    if(!m_is_inited)
        {
            return;
        }

        m_is_inited = false;

	    LOGD("PstnCaller:deinit begain ...");

        // stop listen thread
		stopThread(m_thread_event_input, m_thread_stop_flag_event_input);
        stopSound();

		// set device configuration
		ioctrlFile(m_fd_sound_r_pcm, STANDBY_MEMIF_TYPE, MEM_MOD_DAI);
		ioctrlFile(m_fd_sound_w_pcm, STANDBY_MEMIF_TYPE, MEM_DL2);

		// close device file
		closeFile(m_fd_event_input);
		closeFile(m_fd_ctrl);
		closeFile(m_fd_sound_r_pcm);
		closeFile(m_fd_sound_w_pcm);
    #if _RECORD_TEST_SOUND
        closeFile(m_fd_sound_r_record);
        closeFile(m_fd_sound_w_record);
    #endif //_RECORD_TEST_SOUND
	    LOGD("PstnCaller:deinit end");
	};

    bool startSound()
    {
        bool ret = false;

        do
        {
			BREAKIF( !startThread(m_thread_sound_upload, m_thread_stop_flag_sound_upload, static_sound_upload_proc) );
			BREAKIF( !startThread(m_thread_sound_r_pcm, m_thread_stop_flag_sound_r_pcm, static_sound_r_pcm_proc) );
			BREAKIF( !startThread(m_thread_sound_w_pcm, m_thread_stop_flag_sound_w_pcm, static_sound_w_pcm_proc) );
            ret = true;
        }while(0);

        if( !ret )
        {
            stopSound();
        }
        return ret;
    };

    void stopSound()
    {
		stopThread(m_thread_sound_upload, m_thread_stop_flag_sound_upload);
		stopThread(m_thread_sound_r_pcm, m_thread_stop_flag_sound_r_pcm);
		stopThread(m_thread_sound_w_pcm, m_thread_stop_flag_sound_w_pcm);
    };

    bool dail( const char* number )
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::dail, service not ready !");
            return false;
        }

        LOGD("PstnCaller::dail::EVT_DAIL number:%s", number);
        static const int NUM_LEN_MAX = 128;
        char num[NUM_LEN_MAX] = {0};
        strncpy(num, number, NUM_LEN_MAX);
        if( CStatMachine::ERR_NO_ERROR != m_sm.input(EVT_DAIL, num) )
        {
            return false;
        }
        return true;
    };

    void hangup()
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::hangup, service not ready !");
            return;
        }

        LOGD("PstnCaller::hangup::EVT_USR_HANGUP");
        m_sm.input(EVT_USR_HANGUP);
    };

    bool accept()
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::accept, service not ready !");
            return false;
        }

        LOGD("PstnCaller::accept::EVT_ACCEPT");
        if( CStatMachine::ERR_NO_ERROR != m_sm.input(EVT_ACCEPT) )
        {
            return false;
        }
        return true;
    };

    bool dtmf( const char* number )
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::dtmf, service not ready !");
            return false;
        }

        LOGD("PstnCaller::dtmf::EVT_DTMF number:%s", number);
        static const int NUM_LEN_MAX = 128;
        char num[NUM_LEN_MAX] = {0};
        strncpy(num, number, NUM_LEN_MAX);
        if( CStatMachine::ERR_NO_ERROR != m_sm.input(EVT_DAIL, num) )
        {
            return false;
        }
        return true;        
    };

    bool writeSound( const void*  data, int data_sz )
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::writeSound, service not ready !");
            return false;
        }

    #if _USE_SOUND_ARRAY
        return true;
    #else
        if(data_sz != SOUND_FRAME_SIZE)
        {
            LOGE("PstnCaller::writeSound:data_sz=%d", data_sz);
            return false;
        }

        Frame buf;
        DeviceFrame buf_dev;

        memcpy(buf.byte, data, SOUND_FRAME_SIZE);
        downgrade_sample(buf_dev, buf);
        m_buf_sound_w_pcm.push(buf_dev);
        return true;
    #endif
    };

    int getStat()
    {
        if(!m_is_inited)
        {
            LOGD("PstnCaller::readSound, service not ready !");
            return STAT_ERR;
        }

        LOGD("PstnCaller::getStat:%d", m_sm.current());
        return m_sm.current();
    };
};

#endif //_PSTN_CALLER_CC_
