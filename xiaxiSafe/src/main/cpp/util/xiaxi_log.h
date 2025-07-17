//
// Created by XIAXI on 2024/12/5.
//

#ifndef CHECKROM_XIAXI_LOG_H
#define CHECKROM_XIAXI_LOG_H

#define XIAXI_DEBUG 1
#if XIAXI_DEBUG
#define TAG "XIAXI"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#else
#define LOGD(...) ((void)0)
    #define LOGI(...) (void)0)
    #define LOGV(...) (void)0)
    #define LOGW(...) (void)0)
    #define LOGE(...) (void)0)
#endif

#endif //CHECKROM_XIAXI_LOG_H
