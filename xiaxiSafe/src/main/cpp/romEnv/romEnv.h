//
// Created by 97366 on 2024/6/18.
//

#ifndef CHECKROM_ROMENV_H
#define CHECKROM_ROMENV_H

#include "../util/helper/helper.h"

namespace MX_SU {
    bool check_by_selinux();
    bool check_by_prop();
    bool check_by_env();
    bool check_file_by_file();
    bool check_by_mount();
    bool check_file_by_which();
}

class romEnv{
public:
    static bool check_inline_code_flag();                                   // 检测是否安装类XPosed框架（但在KERNEL SU的LSPosed中失效）
    static bool check_danger_app();                                         // 检测在安卓10下的环境中（包括安卓10）是否安装危险应用
    static bool checkr_rom_customize();                                     // 检测当前的rom是否为自定义
    static bool check_rom_userdebug();                                      // 检测rom是否为userdebug版本
    static bool check_usb_debug1();                                         // 是否开启adb调试的方式1
    static bool check_bl_enabled1();                                        // bl是否已解锁方式1
    static bool check_bl_enabled2(JNIEnv *env, jobject keyAttestaion);      // bl是否已经解锁方式2
    static std::string getDeviceBrand();                                    // 获取当前rom的品牌名
    static std::string getDeviceDevice();                                   // 获取当前rom的设备名
    static std::string getDeviceManufacturer();                             // 获取当前rom的制造商名
    static std::string getDeviceModel();                                    // 获取当前rom的机型名
    static std::string getDeviceProduct();                                  // 获取当前rom的产品名
};


#endif //CHECKROM_ROMENV_H
