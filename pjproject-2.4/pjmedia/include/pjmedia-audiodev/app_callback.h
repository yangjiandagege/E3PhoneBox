#ifndef _APP_CALLBACK_H_
#define _APP_CALLBACK_H_

#define BUFF_SIZE 1280

typedef void (*callback_func)(void *buf, int size);

extern void app_player_callback(void *buf, int size);

#endif