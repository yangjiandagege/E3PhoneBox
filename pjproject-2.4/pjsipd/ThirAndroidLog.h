#ifndef _THIR_ANDOIR_LOG_H_
#define _THIR_ANDOIR_LOG_H_

#include <android/log.h>

#ifndef TAG
#define TAG "THIRCHINA"
#endif // TAG

#if 0
#define LOGX(TYPE, ...)
#elif 0
#define LOGX(TYPE, ...) ((void)fprintf(stderr, __VA_ARGS__)); \
						((void)fprintf(stderr, "\n"));
#else
#define LOGX(TYPE, ...) ((void)__android_log_print(TYPE, TAG, __VA_ARGS__)); \
						((void)fprintf(stderr, __VA_ARGS__)); \
						((void)fprintf(stderr, "\n"));
#endif

#define LOGV(...) LOGX(ANDROID_LOG_VERBOSE, __VA_ARGS__)
#define LOGI(...) LOGX(ANDROID_LOG_INFO, __VA_ARGS__)
#define LOGD(...) LOGX(ANDROID_LOG_DEBUG, __VA_ARGS__)
#define LOGW(...) LOGX(ANDROID_LOG_WARN, __VA_ARGS__)
#define LOGE(...) LOGX(ANDROID_LOG_ERROR, __VA_ARGS__)


#endif //_THIR_ANDOIR_LOG_H_
