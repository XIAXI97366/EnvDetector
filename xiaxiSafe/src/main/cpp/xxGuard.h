//
// Created by 97366 on 2024/7/8.
//

#ifndef CHECKROM_XXGUARD_H
#define CHECKROM_XXGUARD_H

#include "romEnv/romEnv.h"
#include "antiHook/antiHook.h"
#include "antiDebug/antiDebug.h"
#include "antiInject/checkMaps.h"
#include "mem/memCrc.h"
#include "mem/memScan.h"
#include "linkerInfo/linkerInfo.h"


void xx_protect(JNIEnv *env, jclass clazz, jobject application, jobject process, jobject manager);
void apply_native_protect_policy();
void *check_hook_policy(void *_val);
void *check_debug_policy(void *_val);
void *check_root_policy(void *_val);
void *check_maps_policy(void *_val);

jstring get_device_brand(JNIEnv *env, jclass clazz);  // 获取当前rom的品牌名
jstring get_device_device(JNIEnv *env, jclass clazz); // 获取当前rom的设备名
jstring get_device_manufacturer(JNIEnv *env, jclass clazz);   // 获取当前rom的制造商名
jstring get_device_model(JNIEnv *env, jclass clazz);  // 获取当前rom的机型名
jstring get_device_product(JNIEnv *env, jclass clazz);    // 获取当前rom的产品名
jboolean check_bootloader_enabled(JNIEnv *env, jclass clazz, jobject keyAttestaion); // 判断当前设备是否解锁BL
jboolean check_signature_V2(JNIEnv *env, jclass clazz);    // 判断当前应用是否二次打包

JavaVM *g_vm = nullptr;
JNIEnv *g_env = nullptr;

/** 全局类对象 **/
checkMaps g_cm;

/** 全局类对象 **/

/** 结果以 native 为准，但 Detect_Result_2_Java 对象中的字段的值与 native 一致，则同样具有准确性  **/
typedef struct Detect_Result_2_Java{
    jboolean isVpnUser;
    jboolean isAdbEnable;
    jboolean isWifiAdbEnable;
    jboolean isDeveloperModeEnable;
}DR2J;


#endif //CHECKROM_XXGUARD_H
