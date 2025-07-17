//
// Created by 97366 on 2024/7/12.
//

#ifndef CHECKROM_ANTIHOOK_H
#define CHECKROM_ANTIHOOK_H

#include "../util/helper/helper.h"
#include "antiFrida/antiFrida.h"
#include "antiXPosed/antiXPosed.h"
#include "antiPlt/antiPlt.h"

typedef struct {
    int (*antiFrida_check_maps)();
    int (*antiFrida_detect_frida_threads)();
    int (*antiFrida_check_frida_by_port)(int port);
    int (*antiFrida_check_frida_by_port2)(const char *port, pid_t pid);

    int (*antiXPosed_check_model)();
    bool (*antiXPosed_check_maps)();
    bool (*antiXPosed_check_stack)(JNIEnv *env);
    bool (*antiXPosed_check_service)(JNIEnv *env);
    bool (*antiXPosed_check_class)(JNIEnv *env);
    bool (*antiXPosed_check_process)();
} ANTIHOOK;

static bool check_hook_function(void *handle, const char *name);

class antiHook {
public:
    static bool check_frida_hooked();
    static bool check_xposed_hooked();
    static bool check_libc_is_hooked();
    static bool check_plt_is_hooked();
    static bool check_so_is_hooked();
public:
    static std::unordered_map<std::string, void*> protectFunc;
    static JNIEnv *env;
};


#endif //CHECKROM_ANTIHOOK_H
