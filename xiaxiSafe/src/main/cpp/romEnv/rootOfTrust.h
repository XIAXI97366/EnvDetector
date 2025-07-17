//
// Created by XIAXI on 2025/7/17.
//

#ifndef CHECKROM_ROOTOFTRUST_H
#define CHECKROM_ROOTOFTRUST_H
#include "../util/helper/helper.h"

#define VERIFIED_BOOT_KEY_INDEX 0
#define DEVICE_LOCKED_INDEX 1
#define VERIFIED_BOOT_STATE_INDEX 2
#define VERIFIED_BOOT_HASH_INDEX 3

#define KM_VERIFIED_BOOT_VERIFIED 0
#define KM_VERIFIED_BOOT_SELF_SIGNED 1
#define KM_VERIFIED_BOOT_UNVERIFIED 2
#define KM_VERIFIED_BOOT_FAILED 3

class rootOfTrust {
public:
    void get_extension_value();
public:
    static JNIEnv *env;
};


#endif //CHECKROM_ROOTOFTRUST_H
