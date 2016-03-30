/**
 * @file: FskDetector.hpp
 * @auth: lishujie
 * @brif: FSK detector
 */
#ifndef _FSK_DETECTOR_HPP_
#define _FSK_DETECTOR_HPP_ 1

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h> 
#include <unistd.h>

using namespace std;

#define TAG "FskDetector"
#include "ThirAndroidLog.h"

class FskDetector
{
public:
	static const int PHONE_NUMBER_LEN_MAX = 64;

private:
	int              m_fd;
	unsigned int     m_msg_type;
	unsigned int     m_msg_len;
	unsigned int     m_month;
	unsigned int     m_day;
	unsigned int     m_hour;
	unsigned int     m_minute;
	char             m_phone_number[PHONE_NUMBER_LEN_MAX];

	static const int CHAIN_OCCUPY_FLAG = 0x55;
	static const int CHAIN_OCCUPY_FLAG_COUNT = 20;

	enum
	{
		FSK_MSG_TYPE_SINGLE = 0x4,
		FSK_MSG_TYPE_COMPLEX = 0x80,
	};

	enum
	{
		FSK_PARAM_CALLING_TIME = 1,
		FSK_PARAM_CALLING_PHONE = 2,
		FSK_PARAM_NO_CALLING_PHONE = 4,
		FSK_PARAM_CALLING_NAME = 7,
		FSK_PARAM_NO_CALLING_NAME = 8,
	};


	int msg_bbc_code_check(unsigned char msg[])
	{
		int i, data_sum = 0;
		int data_len = msg[1];
		int bbc_check_code;
		for (i = 0; i < data_len + 2; i++)
		{
			printf("msg[%d] = 0x%x\n", i, msg[i]);
			data_sum += msg[i];
			data_sum %= 256;
		}

		bbc_check_code = msg[i];
		printf("bbc_check_code = %d\n", bbc_check_code);

		return (256 - data_sum) != bbc_check_code;
	};

	int fsk_msg_set_single(unsigned char msg[])
	{
		int i = 0;
		int temp;
		m_msg_type = msg[i];
		m_msg_len = msg[++i];
		if (m_msg_len < 9)
		{
			printf("m_msg_len is %d\n", m_msg_len);
			return -1;
		}

		//copy time 8 byte to fsk msg
		m_month = (msg[i + 1] - 0x30) * 10 + (msg[i + 2] - 0x30);
		m_day = (msg[i + 3] - 0x30) * 10 + (msg[i + 4] - 0x30);
		m_hour = (msg[i + 5] - 0x30) * 10 + (msg[i + 6] - 0x30);
		m_minute = (msg[i + 7] - 0x30) * 10 + (msg[i + 8] - 0x30);
		i += 9;  //1 byte type + 1 byte len + 8 byte time

		memset(m_phone_number, sizeof(m_phone_number), 0);
		snprintf(m_phone_number, m_msg_len - 8 + 1, "%s", msg + i);

		return 0;
	};

	int fsk_msg_set_complex(unsigned char msg[])
	{
		int i = 0;
		int temp;

		m_msg_type = msg[i];
		m_msg_len = msg[++i];
		while (i < m_msg_len)
		{
			switch (msg[++i])
			{
			case FSK_PARAM_CALLING_TIME:  //month/day
				if(msg[i] != 8)
				{
					i += msg[i];
					break;
				}

				m_month = (msg[i + 1] - 0x30) * 10 + (msg[i + 2] - 0x30);
				m_day = (msg[i + 3] - 0x30) * 10 + (msg[i + 4] - 0x30);
				m_hour = (msg[i + 5] - 0x30) * 10 + (msg[i + 6] - 0x30);
				m_minute = (msg[i + 7] - 0x30) * 10 + (msg[i + 8] - 0x30);
				i += 8;
				break;
			case FSK_PARAM_CALLING_PHONE:
				i++;
				snprintf(m_phone_number, msg[i] + 1, "%s", msg + i + 1);
				i += msg[i];
				break;
			default:
				break;
			}
		}

		return i != m_msg_len;

	};

	int fsk_flag_dec()
	{
		const char expect_buf[16] = {0xFF, 0xFD, 0xF5, 0xD5, 0x4, 0x80, 0x0};
		int con_flag = 0; //detect continuous flag: 0x55, number of 0x55 at lease 50
		unsigned char s_buf[1] = {0}; //store one buf
		unsigned int num = 0;
		struct termios tty_tio;

		if (m_fd < 0)
		{
			return -1;
		}

		tcgetattr(m_fd, &tty_tio);

		//set baud
		cfsetispeed(&tty_tio, B1200);
		//no check 8 data bit
		tty_tio.c_cflag &= ~PARENB;
		tty_tio.c_cflag &= ~CSTOPB; //1 bit stop bit
		tty_tio.c_cflag &= ~CSIZE;
		tty_tio.c_cflag |= CS8;

		//do nothing after handle
		tty_tio.c_oflag  &= ~OPOST;
		tty_tio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
		tty_tio.c_iflag &= (~INLCR | ~ICRNL | IGNCR);
		tty_tio.c_oflag &= ~(ONLCR | OCRNL);
		//tty_tio.c_cflag &= ~CRTSCTS;  //no hardware flow

		tcsetattr(m_fd, TCSANOW, &tty_tio);
		tcflush(m_fd, TCIOFLUSH);
		while(1)
		{
			*s_buf = 0;
			num = read(m_fd, s_buf, 1);
            //LOGD("FskDetector:fsk_flag_dec:read:0x%X", s_buf[0]);
			if (num != 1)
			{
				printf("read fsk err num = %d\n", num);
				perror("read fail");
				break;
			}

			usleep(10);
            printf("read *s_buf = 0x%X\n", *s_buf);
			if (*s_buf == CHAIN_OCCUPY_FLAG)
			{
			    printf("con_flag++ = %d\n", con_flag);
				con_flag++;
				continue;
			}

			if(con_flag < CHAIN_OCCUPY_FLAG_COUNT)
			{
				con_flag = 0;
				continue;
			}

			printf("con_flag = %d\n", con_flag);
			if (strchr(expect_buf, s_buf[0]) != NULL)
			{
			    LOGD("FskDetector:fsk_flag_dec:break by:0x%X", s_buf[0]);
				return *s_buf;  //success to detect flag
			}
			else
			{
				con_flag = 0;
				continue;
			}
		}
        return -1;
	};

	int fsk_msg_detect(unsigned int data)
	{
		unsigned char msg[64] = {0}, s_buf[1] = {0}; //store one buf
		int num = 0;
		int msg_len = 0;

		if (data == 0x4 || data == 0x80)
		{
			msg[0] = data;
		}
		else
		{
			num = read(m_fd, s_buf, 1);
            LOGD("FskDetector:fsk_msg_detect:read:0x%X", s_buf[0]);
			if (*s_buf == 0)  //0 caused by 180 bit 1
			{
				num = read(m_fd, s_buf, 1);
			}

			if (*s_buf == 0x4 || *s_buf == 0x80)
			{
				msg[0] = *s_buf;
			}
			else
			{
				printf("msg type %d error, it must be 0x4 or 0x80\n", *s_buf);
				return -1;
			}
		}

		printf("fsk_msg_detect msg_type = %d\n", msg[0]);
		num = read(m_fd, s_buf, 1);
		msg_len = *s_buf;
		msg[1] = *s_buf;
		printf("fsk_msg_detect msg_len = %d\n", msg[1]);

		num = read(m_fd, msg + 2, msg_len + 1); //include bbc check code
		printf("expect to read %d, in fact read %d\n", msg_len + 1, num);
		if (num != -1 && num < msg_len + 1)
		{
			printf("read msg again %d\n", msg_len + 1 - num);
			num = read(m_fd, msg + 2 + num, msg_len + 1 - num);
		}

		num = msg_bbc_code_check(msg);
		if (num != 0)
		{
			printf("fsk_msg_detect bbc check err\n");
			return -1;
		}

		switch (msg[0])
		{
		case FSK_MSG_TYPE_SINGLE:
			num = fsk_msg_set_single(msg);
			break;

		case FSK_MSG_TYPE_COMPLEX:
			num = fsk_msg_set_complex(msg);
			break;

		default:
			break;
		}
		if (num == -1)
		{
			printf("fsk msg set fail\n");
			return -1;
		}

		return 0;
	};

public:
	FskDetector( const char* path )
	{
	    m_fd = open(path, O_RDONLY);
        if( m_fd < 0 )
        {
            perror("open FSK file");
        }
	};

	~FskDetector()
	{};

	int detect()
	{
		int data = 0;
		int ret = -1;

		if(m_fd<=0) return -1;

		memset(m_phone_number,0,PHONE_NUMBER_LEN_MAX);

		data = fsk_flag_dec();
		if (data == -1)
		{
			printf("fsk flag dec fail\n");
			return -2;
		}

		ret = fsk_msg_detect(data);
		if (ret == -1)
		{
			printf("fsk msg detect fail\n");
			return -3;
		}
		return 0;
	};
	
	inline const char* getPhoneNumber()
	{
		return m_phone_number;
	};
};


#endif //_FSK_DETECTOR_HPP_
