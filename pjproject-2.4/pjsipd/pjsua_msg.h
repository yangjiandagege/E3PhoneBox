#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#define MSG_MAX_SIZE 1280

typedef struct SipMsgT
{
    char aCmd[8];
    char aMsg[MSG_MAX_SIZE];
}SipMsg;

#endif
