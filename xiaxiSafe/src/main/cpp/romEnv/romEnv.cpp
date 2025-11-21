//
// Created by 97366 on 2024/6/18.
//

#include "romEnv.h"
#include "../rootOfTrust/rootOfTrust.h"

char magisk[32] = {'/', 'd', 'a', 't', 'a', '/', 'd', 'a', 't',
                   'a', '/', 'c', 'o', 'm', '.', 't', 'o',
                   'p', 'j', 'o', 'h', 'n', 'w', 'u', '.',
                   'm', 'a', 'g', 'i', 's', 'k', '\0'};
char kernelsu[30] = {'/', 'd', 'a', 't', 'a', '/', 'd', 'a', 't',
                     'a', '/', 'm', 'e', '.', 'w', 'e', 'i',
                     's', 'h', 'u', '.', 'k', 'e', 'r', 'n',
                     'e', 'l', 's', 'u', '\0'};

char *dangerApp[2] = {magisk, kernelsu};

bool romEnv::check_inline_code_flag() {
    char buffer[BUFFER_LEN] = {0};

    // dalvik-vm-dex2oat-flags --inline-max-code-uits=0 为了hook更多的函数所以要取消函数优化（如：内联）
    get_rom_property("dalvik.vm.dex2oat-flags", buffer);
    if (!sub_strlen(buffer)){
        return false;
    }else{
        if (!sub_strncmp(buffer, "--inline-max-code-units=0", sub_strlen("--inline-max-code-units=0"))){
            LOGD("rom flag check -> %s ", buffer);
            LOGD("已被Inline Hook");
            return true;
        }else{
            return false;
        }
    }
}

//使用 stat64 和 stat 可以在安卓10下（包括安卓10）检测/data/data/下的目录，
bool romEnv::check_danger_app() {
    int sdk = get_rom_sdk();
    if (sdk > 29){
        return false;
    }

    for (int i = 0; i < 2; ++i) {
#if defined(__LP64__)
        struct stat64 dirStat;
        if (0 == stat64(dangerApp[i], &dirStat)){
            return true;
        }else{
            LOGD("%s", strerror(errno));
        }
#else
    struct stat dirStat;
    if (0 == stat("appNames[i]", &dirStat)){
        return true;
    }else{
        LOGD("%s", strerror(errno));
    }
#endif
    }
    return false;
}

// 通过属性判断当前设备是否开启adb
bool romEnv::check_usb_debug1() {
    //    getprop | grep init.svc.adbd
    //    [init.svc.adbd]: [running]
    //    [init.svc.adbd]: [stopped]
    char buffer[BUFFER_LEN] = {0};
    get_rom_property("init.svc.adbd", buffer);
    if (!sub_strlen(buffer)){
        return false;
    }else{
        if (!sub_strncmp(buffer, "running", sub_strlen("running"))){
            LOGD("init.svc.adbd -> %s ", buffer);
            return true;
        }
    }

    //    getprop | grep usb
    //    [persist.sys.usb.config]: [adb]   // 该值为adb，目前只发现为pixel设备
    //    [persist.sys.usb.config]: []  // 该属性在一加设备上无效

    return false;
}

// sys.oem_unlock_allowed
bool romEnv::check_bl_enabled1() {
    //getprop ro.boot.vbmeta.device_state 如果未解锁的，那么内容就是 locked，已解锁则是 unlocked
    char buffer[BUFFER_LEN] = {0};
    get_rom_property("ro.boot.vbmeta.device_state", buffer);
    if (!sub_strncmp(buffer, "unlocked", sub_strlen("unlocked"))){
        LOGD("bl is -> %s ", buffer);
        return true;
    }else{
        return false;
    }
}

// 通过密钥认证来验证是否开启引导程序
bool romEnv::check_bl_enabled2(JNIEnv *env, jobject keyAttestaion) {
    RootOfTrust rootOfTrust;
    return rootOfTrust.ParseCertificateChain(env, rootOfTrust.GetCertificateChain(env, keyAttestaion));
}

bool romEnv::check_rom_userdebug() {
    // 通过 maps 中的 /dev/__properties__/property_info 获取 prop_info，再获取下方设备指纹
    // /system/build.prop
    /*
        [ro.build.tags]: [release-keys]
        [ro.product.build.tags]: [release-keys]
        [ro.system.build.tags]: [release-keys]
        [ro.system_ext.build.tags]: [release-keys]
        [ro.vendor.build.tags]: [release-keys]
        [ro.build.fingerprint]: [google/oriole/oriole:13/TQ2A.230505.002/9891397:user/release-keys]
        检测如上属性，查看当前的rom是否为userdebug（test-keys）
     */
}

bool romEnv::checkr_rom_customize() {


}

std::string romEnv::getDeviceBrand() {
    char buf[MAX_LENGTH] = {0};
    std::string brand = "";

    get_rom_property("ro.product.vendor.brand", buf);
    if (0 == sub_strlen(buf)){
        LOGD("get ro.product.vendor.brand value is null ");
    }else{
        brand = brand + buf;
    }

    return brand;
}

std::string romEnv::getDeviceDevice() {
    char buf[MAX_LENGTH] = {0};
    std::string device = "";

    get_rom_property("ro.product.vendor.device", buf);
    if (0 == sub_strlen(buf)){
        LOGD("ro.product.vendor.device value is null ");
    }else{
        device = device + buf;
    }

    return device;
}

std::string romEnv::getDeviceManufacturer() {
    char buf[MAX_LENGTH] = {0};
    std::string manufacturer = "";

    get_rom_property("ro.product.vendor.manufacturer", buf);
    if (0 == sub_strlen(buf)){
        LOGD("ro.product.vendor.manufacturer value is null ");
    }else{
        manufacturer = manufacturer + buf;
    }

    return manufacturer;
}

std::string romEnv::getDeviceModel() {
    char buf[MAX_LENGTH] = {0};
    std::string model = "";

    get_rom_property("ro.product.vendor.model", buf);
    if (0 == sub_strlen(buf)){
        LOGD("ro.product.vendor.model value is null ");
    }else{
        model = model + buf;
    }

    return model;
}

std::string romEnv::getDeviceProduct() {
    char buf[MAX_LENGTH] = {0};
    std::string name = "";

    get_rom_property("ro.product.vendor.name", buf);
    if (0 == sub_strlen(buf)){
        LOGD("ro.product.vendor.name value is null ");
    }else{
        name = name + buf;
    }

    return name;
}
