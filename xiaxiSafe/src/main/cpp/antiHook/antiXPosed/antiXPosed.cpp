//
// Created by 97366 on 2024/6/6.
//

#include "antiXPosed.h"

int antiXPosed::check_model() {
    char buffer[128] = {0};

    get_rom_property("ro.build.version.sdk", buffer);
    if (0 == sub_strlen(buffer)){
        return -1;
    }
    if (23 != atoi(buffer)){
        return 0;
    }
    memset(buffer, 0, sizeof(buffer));
    get_rom_property("ro.product.model", buffer);
    if (0 == sub_strlen(buffer)){
        return -1;
    }
    return sub_strcmp(buffer, "HUAWEI eH880") == 0;
}

bool antiXPosed::check_stack(JNIEnv *env) {
    jclass clsThread = env->FindClass("java/lang/Thread");
    jmethodID midCurrentThread = env->GetStaticMethodID(clsThread, "currentThread",
                                                         "()Ljava/lang/Thread;");
    jobject objThread = env->CallStaticObjectMethod(clsThread, midCurrentThread);
    jmethodID midGetStackTrace = env->GetMethodID(clsThread, "getStackTrace",
                                                  "()[Ljava/lang/StackTraceElement;");
    jobjectArray objStackTraceElementArray = static_cast<jobjectArray>(env->CallObjectMethod(objThread,
                                                                                        midGetStackTrace));
    jsize arrayLength = env->GetArrayLength(objStackTraceElementArray);
    jobject objStackTraceElement0 = env->GetObjectArrayElement(objStackTraceElementArray,
                                                              (unsigned int)(arrayLength - 1));
    jclass clsStackTraceElement = env->FindClass("java/lang/StackTraceElement");
    jmethodID midGetClassName = env->GetMethodID(clsStackTraceElement, "getClassName",
                                                 "()Ljava/lang/String;");
    jstring targetString0 = (jstring)env->CallObjectMethod(objStackTraceElement0, midGetClassName);
    const char* c_targetString0 = env->GetStringUTFChars(targetString0, 0LL);
    jobject objStackTraceElement1 = env->GetObjectArrayElement(objStackTraceElementArray, (arrayLength - 2));
    jstring targetString1 = (jstring)env->CallObjectMethod(objStackTraceElement1, midGetClassName);
    const char* c_targetString2 = env->GetStringUTFChars(targetString1, 0LL);
    if((strcasestr((char*)(c_targetString0), "xposed") ||
        strcasestr((char*)(c_targetString2), "xposed")) != 0){
        return true;
    }
    return false;
}

bool antiXPosed::check_class(JNIEnv *env) {
    jclass clsClassLoader = env->FindClass("java/lang/ClassLoader");
    jmethodID midGetSystemClassLoader = env->GetStaticMethodID(clsClassLoader, "getSystemClassLoader",
                                                               "()Ljava/lang/ClassLoader;");
    jobject objSystemClassLoader = env->CallStaticObjectMethod(clsClassLoader, midGetSystemClassLoader);
    jmethodID midLoadClass = env->GetMethodID(clsClassLoader, "loadClass",
                                              "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring checkClass = env->NewStringUTF("de.robv.android.xposed.XposedHelpers");
    if (env->CallObjectMethod(objSystemClassLoader, midLoadClass, checkClass) &&
        !env->ExceptionCheck()){
        return true;
    }
    env->ExceptionClear();
    return false;
}

bool antiXPosed::check_service(JNIEnv *env) {
    jclass clsServiceManager = env->FindClass("android/os/ServiceManager");
    jmethodID midGetService = env->GetStaticMethodID(clsServiceManager, "getService",
                                                     "(Ljava/lang/String;)Landroid/os/IBinder;");
    jstring serviceName= env->NewStringUTF("user.xposed.system");
    if ( env->CallStaticObjectMethod(clsServiceManager, midGetService, serviceName)
        && !env->ExceptionCheck()){
        return true;
    }
    env->ExceptionDescribe();
    env->ExceptionClear();
    return false;
}

bool antiXPosed::check_maps() {
    char buffer[1024] = {0};
    FILE *fp = nullptr;
    int bufferLen = 0;
    char *tmp = nullptr;
    bool flag = false;

    snprintf(buffer, sizeof(buffer), "/proc/%d/status", getpid());
    fp = fopen(buffer, "r");
    if (nullptr == fp){
        goto CHECK_MAPS_EXIT;
    }

    memset(buffer, 0, sizeof(buffer));
    while (1 == fscanf(fp, "%*p-%*p %*c%*c%*c%*c %*s %*s %*d%1023[^\n]", buffer)){
        bufferLen = sub_strlen(buffer);
        for (tmp = buffer; (*tmp) && isspace((unsigned char)(*tmp)); tmp++){
            --bufferLen;
        }

        if (buffer != tmp){
            memcpy(buffer, tmp, bufferLen + 1);
        }

        for (tmp = (char *)buffer + sub_strlen(buffer) - 1; tmp != buffer && isspace(*(unsigned char *)tmp);
            tmp = (char *)tmp - 1 ){}
        *((unsigned char *)tmp + (isspace(*(unsigned char *)tmp) == 0)) = 0;

        if(strcasestr(buffer, "xposedbridge") || strcasestr(buffer, ".xposed.") ||
            strcasestr(buffer, "xposed_art")){
            flag = true;
            goto CHECK_MAPS_EXIT;
        }
        memset(buffer, 0, sizeof(buffer));
    }

CHECK_MAPS_EXIT:
    if (nullptr != fp){
        fclose(fp);
        fp = nullptr;
    }
    return flag;
}

int antiXPosed::check_manufacturer() {
    char buffer[128] = {0};
    int sdkVersion = 0;
    int ret = 0;

    get_rom_property("ro.build.version.sdk", buffer);
    if (0 == sub_strlen(buffer)){
        goto CHECK_MANUFACTURER_EXIT;
    }

    sdkVersion = atoi(buffer);
    if ((21 != sdkVersion) && (22 != sdkVersion)){
        goto CHECK_MANUFACTURER_EXIT;
    }
    memset(buffer, 0, sizeof(buffer));
    get_rom_property("ro.product.manufacturer", buffer);
    if (0 == sub_strlen(buffer)){
        ret = -1;
        goto CHECK_MANUFACTURER_EXIT;
    }

    ret = (!strcasecmp(buffer, "OPPO") || !strcasecmp(buffer, "VIVO"));

CHECK_MANUFACTURER_EXIT:
    return ret;
}

bool antiXPosed::check_process(){
    char buffer[1024] = {0};
    FILE *fp = NULL;
    int bufferLen = 0;
    char *tmp = NULL;
    int result = 0;
    u_char byte = 0;
    bool flag = false;
    int len = 0;

    result = check_manufacturer();
    if ((!result) && (-1 != result)){
        snprintf(buffer, sizeof(buffer), "/proc/%d/maps", getpid());
        fp = fopen(buffer, "r");
        if (nullptr == fp){
            goto CHECK_PROCESS_EXIT;
        }

        memset(buffer, 0, sizeof(buffer));
        while (fscanf(fp, "%*p-%*p %*c%*c%c%*c %*s %*s %*d%1023[^\n]", &byte, buffer) == 2){
            bufferLen = sub_strlen(buffer);
            for (tmp = buffer; (*tmp) && isspace((unsigned char)(*tmp)); tmp++){
                --bufferLen;
            }

            if (buffer != tmp){
                memcpy(buffer, tmp, bufferLen + 1);
            }

            for (tmp = (char *)buffer + sub_strlen(buffer) - 1; tmp != buffer && isspace(*(unsigned char *)tmp);
                 tmp = (char *)tmp - 1 ){}
            *((unsigned char *)tmp + (isspace(*(unsigned char *)tmp) == 0)) = 0;

            if (0x78 == byte){
                len = sub_strlen("/system/bin/app_process");
                if (!sub_strncmp(buffer, "/system/bin/app_process", len) ){
                    if (is_process_have_xposed("/system/bin/app_process")){
                        flag = true;
                        goto CHECK_PROCESS_EXIT;
                    }
                    break;
                }
            }
        }
    }

CHECK_PROCESS_EXIT:
    if (nullptr != fp){
        fclose(fp);
        fp = nullptr;
    }
    return flag;
};

bool antiXPosed:: is_process_have_xposed(const char *processPath){
    FILE *fp = NULL;
    unsigned long fileSize = 0;
    void *buffer = nullptr;
    bool flag = false;

    fp = (FILE *)fopen(processPath, "rb");               // a1 == "/system/bin/app_process"
    if (nullptr == fp){
        goto IS_PROCESS_HAVE_XPOSED_EXIT;
    }

    fseek(fp, 0, 2);
    fileSize = ftell(fp);
    rewind(fp);
    buffer = (void *)malloc(fileSize);
    memset(buffer, 0, fileSize);
    if (fileSize == fread(buffer, 1u, fileSize, fp)){
        flag = (-1 != search_xposed(buffer, "xposed", fileSize));
    }

IS_PROCESS_HAVE_XPOSED_EXIT:
    if (nullptr != buffer){
        free(buffer);
        buffer = nullptr;
    }
    if (nullptr != fp){
        fclose(fp);
        fp = nullptr;
    }
    return flag;
}

int antiXPosed:: search_xposed(void* buffer, const char* xposed , int fileSize){
    int len = sub_strlen("xposed");
    for (int i = 0; i < fileSize; ++i){
        if (!sub_strncmp((const char *)((unsigned long)buffer + i), "xposed", len)) {
            return (unsigned int) i;
        }
    }
    return 0xFFFFFFFFLL;
}


