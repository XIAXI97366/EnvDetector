//
// Created by 97366 on 2024/10/8.
//

#include "linkerInfo.h"

typedef void *(*solist_get_headv)();

// 暂时用于安卓13
void *linkerInfo::get_linker_solistHead() {
    void *handle = nullptr;
    solist_get_headv solistHeadv = nullptr;
    void *solistHead = nullptr;

    handle = xdl_open(get_linker_path(), XDL_TRY_FORCE_LOAD);
    solistHeadv = (solist_get_headv)xdl_dsym(handle, "__dl__Z15solist_get_headv", NULL);
    if (nullptr == solistHeadv){
        LOGE("[-] get_linker_solist %s %d xdl_dsym is error", __FUNCTION__, __LINE__);
        return nullptr;
    }
    LOGE("[+] get_linker_solist %s %d xdl_dsym is successfull solistHeadv -> %p", __FUNCTION__, __LINE__, solistHeadv);

    solistHead = solistHeadv();
    if (nullptr == solistHead){
        LOGE("[-] get_linker_solist %s %d solistHeadv is error", __FUNCTION__, __LINE__);
        return nullptr;
    }
    LOGE("[+] get_linker_solist %s %d solistHeadv is successfull solistHead -> %p", __FUNCTION__, __LINE__, solistHead);

    return solistHead;
}

//
void linkerInfo::enum_solist(void *solistHead) {
    void *soNextAddr = nullptr;
    const char *soPath = nullptr;

    if (nullptr == solistHead){
        LOGE("[-] enum_solist %s %d solistHead is nullptr", __FUNCTION__, __LINE__);
        return;
    }

    soNextAddr = *(void **)((unsigned long)solistHead + solist_next_offset);
    LOGE("[+] enum_solist %s %d soNextAddr -> %p ", __FUNCTION__, __LINE__, soNextAddr);

    while(true){
        if (nullptr == soNextAddr){
            break;
        }
        soPath = ((std::string *) ( (uintptr_t) soNextAddr + solist_realpath_offset))->c_str();
        soNextAddr = *(void **)((unsigned long)soNextAddr + solist_next_offset);
        LOGE("[+] enum_solist %s %d soNextAddr -> %p soPath -> %s ", __FUNCTION__, __LINE__, soNextAddr, soPath);
    }
}