#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <pjmedia-audiodev/app_callback.h>

#define BUFF_SIZE 1280

extern callback_func callback_player_func;

void app_player_callback(void *buf, int size)
{
	if(callback_player_func != NULL){
		callback_player_func(buf,size);
	}else
		return;
}
