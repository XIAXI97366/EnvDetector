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


typedef struct mxConfig {
    u_char isRoot;
    u_char isEmu;
    u_char isVA;
    u_char isDebug;
    u_char isInject;
    u_char isVpn;
    u_char isHook;
    u_char isScreen;
    u_char isHijack;
} MX_CONFIG;

MX_CONFIG *g_config = nullptr;

JavaVM *g_vm = nullptr;
JNIEnv *g_env = nullptr;
INVOKE *g_ref = nullptr;
jobject g_event = nullptr;
jobject g_xxsafe = nullptr;
jmethodID g_midIsVpnUsed = nullptr;
jmethodID g_midOnMessage = nullptr;

//int sm4DecryptNormal(JNIEnv *env, jobject manager);
//int sm4DecryptHard(JNIEnv *env, jobject manager);
void xxProtect(JNIEnv *env, jclass clazz, jobject application,
               jobject process, jobject manager);
void apply_protect_policy();

void *policy_body_check_hook(void *_val);
void *policy_body_check_debug(void *_val);
void *policy_body_check_root(void *_val);
void *policy_body_checkmap(void *_val);

#endif //CHECKROM_XXGUARD_H
