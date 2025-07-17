//
// Created by 97366 on 2024/7/12.
//

#include "antiHook.h"

JNIEnv *antiHook::env = nullptr;

static ANTIHOOK anti = {
                    antiFrida::check_maps,
                    antiFrida::detect_frida_threads,
                    antiFrida::check_frida_by_port,
                    antiFrida::check_frida_by_port2,

                    antiXPosed::check_model,
                    antiXPosed::check_maps,
                    antiXPosed::check_stack,
                    antiXPosed::check_service,
                    antiXPosed::check_class,
                    antiXPosed::check_process
};


#define CHECK_INLINEHOOK_SYMBOL(x, y) \
if(check_hook_function(x, #y))        {\
    dlclose(x);                        \
    return true;}

static bool check_hook_function(void *handle, const char *name) {
    void *symbol = nullptr;
    symbol = dlsym(handle, name);
    if (symbol != nullptr && set_read(symbol) && is_inline_hooked(symbol)) {
        LOGD(" %s Func Is Hooked", name);
        return true;
    }
    return false;
}

bool antiHook::check_libc_is_hooked(){
    void *handle = dlopen("libc.so", RTLD_NOW);
    if (handle) {
        CHECK_INLINEHOOK_SYMBOL(handle, popen);
        CHECK_INLINEHOOK_SYMBOL(handle, access);
        CHECK_INLINEHOOK_SYMBOL(handle, __openat);
        CHECK_INLINEHOOK_SYMBOL(handle, read);
        CHECK_INLINEHOOK_SYMBOL(handle, pread);
        CHECK_INLINEHOOK_SYMBOL(handle, pread64);
        CHECK_INLINEHOOK_SYMBOL(handle, pwrite);
        CHECK_INLINEHOOK_SYMBOL(handle, pwrite64);
        CHECK_INLINEHOOK_SYMBOL(handle, close);
        CHECK_INLINEHOOK_SYMBOL(handle, open);
        CHECK_INLINEHOOK_SYMBOL(handle, pthread_create);
        CHECK_INLINEHOOK_SYMBOL(handle, fork);
    }
    return false;
}

std::unordered_map<std::string, void*> antiHook::protectFunc = {
        {"sub_strcmp", (void*)&sub_strcmp},
        {"sub_strlen", (void*)&sub_strlen},
        {"sub_strncmp", (void*)&sub_strncmp},
        {"sub_strstr", (void*)&sub_strstr},
        {"sub_strcpy", (void*)&sub_strcpy},
};

bool antiHook::check_so_is_hooked() {
    bool isInline = false;

    for (auto &it:antiHook::protectFunc) {
        if(it.second){
            isInline = is_inline_hooked(it.second);
            LOGD("name:%s->%lx :%d", it.first.c_str(), it.second, isInline);
            if(isInline){
                LOGD("%s %s", it.first.c_str(), "已被 Inline Hook");
                //sub_kill(pid);
                return true;
            }
        }
    }
    return false;
}

bool antiHook::check_plt_is_hooked() {
    antiPlt antiPlt;

    elf_dyn_info *memself = antiPlt.elf_parse_mem_so("libxx-guard.so");
    if (memself){
        if (antiPlt.check_rel_plt(memself) || antiPlt.check_rel_dyn(memself)) {
            return true;
        }
    }
    return false;
}

bool antiHook::check_frida_hooked(){
    if (1 == anti.antiFrida_check_maps() ||
        1 == anti.antiFrida_check_frida_by_port(27042) ||
        1 == anti.antiFrida_detect_frida_threads() ||
        1 == anti.antiFrida_check_frida_by_port2("69A2", getpid())){
        return true;
    }
    return false;
}

bool antiHook::check_xposed_hooked(){
    int result = 0;

    result = anti.antiXPosed_check_model();
    if (result && result != -1){
        if(anti.antiFrida_check_maps() || anti.antiXPosed_check_stack(env)
            || anti.antiXPosed_check_service(env) || anti.antiXPosed_check_class(env)
            || anti.antiXPosed_check_process()){
            return true;
        }
    }
    return false;
}


