//
// Created by 97366 on 2024/6/6.
//

#ifndef CHECKROM_ANTIXPOSED_H
#define CHECKROM_ANTIXPOSED_H

#include "../../util/helper/helper.h"

class antiXPosed{
public:
    static int check_model();       //根据该函数返回决定是否调用后续XPosed检测, 返回值-1则是获取属性失败
    static bool check_maps();
    static int search_xposed(void *buffer, const char *xposed , int fileSize);
    static bool is_process_have_xposed(const char *processPath);
    static bool check_process();
    static bool check_stack(JNIEnv* env);
    static bool check_class(JNIEnv* env);
    static bool check_service(JNIEnv* env);
    static int check_manufacturer();
};

#endif //CHECKROM_ANTIXPOSED_H
