#include "xxGuard.h"

jobjectArray get_detections(JNIEnv *env, jobject thiz,
                            jobject context, jobject packageManager);
void xx_protect(JNIEnv *env, jclass clazz, jobject application, jobject process, jobject manager);

extern "C" void _init(void){
    // 修复 so 格式并读取SM4配置文件中的密钥和IV设置对应加固选项
    // 后续添加修复so格式的函数并配置SM4配置文件的二进制数据记录在so的elf文件格式中的函数
};

__attribute__((constructor)) void init_global_obj(){
    /** init checkMaps global object（获取 maps info 和 basefd） **/
    g_cm.get_map_seg_info();
    g_cm.get_base_fd();
}

__attribute__((constructor)) void init_array_2(){}

__attribute__((constructor)) void init_array_3(){}

__attribute__((destructor)) void finit_array_1(){}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env= nullptr;
    jclass clsXxSafe = nullptr;
    jclass clsNativeDetections = nullptr;
    jclass clsEnv = nullptr;
    std::string romInfo = "";
    g_vm = vm;

    if (JNI_OK != vm->GetEnv((void **) &env, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }else{
        g_env = env;
    }

    clsXxSafe = env->FindClass("com/xiaxi/safe/XxSafe");
    static JNINativeMethod methodsXxSafe[] = {
            {"protect",
             "(Landroid/app/Application;Lcom/xiaxi/safe/XxEventProcess;Landroid/content/res/AssetManager;)V",
             (void *)&xx_protect},
    };
    env->RegisterNatives(clsXxSafe, methodsXxSafe,
                         sizeof(methodsXxSafe) / sizeof(methodsXxSafe[0]));

    clsNativeDetections = env->FindClass("com/xiaxi/safety/app/NativeDetections");
    static JNINativeMethod methodsNativeDetections[] = {
            {"getDetections",
             "(Landroid/content/Context;Landroid/content/pm/PackageManager;)[Lcom/xiaxi/safety/app/DetectionResult;",
             (void *) get_detections},
    };
    env->RegisterNatives(clsNativeDetections, methodsNativeDetections,
                         sizeof(methodsNativeDetections) / sizeof(methodsNativeDetections[0]));

    clsEnv = env->FindClass("com/xiaxi/safe/util/EnvDetector");
    static JNINativeMethod methodsEnvDetector[] = {
            {"getDeviceBrand",  "()Ljava/lang/String;", (void *)&get_device_brand},
            {"getDeviceDevice",       "()Ljava/lang/String;", (void *)&get_device_device},
            {"getDeviceManufacturer", "()Ljava/lang/String;", (void *)&get_device_manufacturer},
            {"getDeviceModel",        "()Ljava/lang/String;", (void *)&get_device_model},
            {"getDeviceProduct",         "()Ljava/lang/String;", (void *)&get_device_product},
            {"isBootLoaderEnabled", "(Lcom/xiaxi/safe/util/KeyAttestation/KeyAttestation;)Z", (void *)&check_bootloader_enabled},
            {"checkSignatureV2", "()Z", (void *)&check_signature_V2},
    };
    env->RegisterNatives(clsEnv, methodsEnvDetector,
                         sizeof(methodsEnvDetector) / sizeof(methodsEnvDetector[0]));




    //memCrc::check_sum_for_libart();

    //void *solistHead = hideMapsItem::get_linker_solistHead();
    //hideMapsItem::enum_solist(solistHead);



//    char libPath[BUFFER_LEN] = {0};
//    strcpy(libPath, get_app_native_lib(env));
//    strcat(libPath, "/libmx-guard.so");
//    LOGD("libPath -> %s ", libPath);
//    void *handle = dlopen("/data/local/tmp/libart.so", RTLD_NOW);
//    LOGD("error -> %s ", dlerror());
//    void *handle1 = xdl_open("/apex/com.android.art/lib64/libart.so", XDL_TRY_FORCE_LOAD);
//    LOGD("handle1 -> %p ", handle1);
//    void *sym = xdl_dsym(handle1, "_ZN3artL15GetMethodShortyEP7_JNIEnvP10_jmethodID", NULL);
//    LOGD("sym -> %p ", sym);


//    checkMaps ck;
//    ck.get_map_seg_info();
//    ck.get_base_fd();
//    ck.check_maps_valid();
//    ck.is_zygote_injected();
//    ck.is_map_segment_compliance();
//    if (!check_baseapk_valid(ck.basefd, ck.basePath, sub_strlen(ck.basePath), ck.inode)){
//        LOGD("该文件不符合预期");
//    }else{
//        check_certificate_2_V2(ck.basefd, 0x36a, cert_V2_sha256);
//    }

    return JNI_VERSION_1_6;
}

jobjectArray get_detections(JNIEnv *env, jobject thiz, jobject context, jobject packageManager){
    return nullptr;
};

void xx_protect(JNIEnv *env, jclass clazz, jobject application, jobject process, jobject manager) {
    jobject objAppContext = nullptr;
    DR2J result = {0};

    // 反射调用 EnvDetector 中的检测方法
    invoke_func()->getStaticObject(env, &objAppContext, "com/xx/shell/RiskCheckApplication",
                                   "appContext", "Landroid/content/Context;");
    if (nullptr != objAppContext){
        invoke_func()->callStaticBooleanMethod(env, &result.isAdbEnable, "com/xiaxi/safe/util/EnvDetector",
                                               "(Landroid/content/Context;)Z", "isAdbEnable", objAppContext);
        invoke_func()->callStaticBooleanMethod(env, &result.isDeveloperModeEnable, "com/xiaxi/safe/util/EnvDetector",
                                               "(Landroid/content/Context;)Z", "isDeveloperModeEnable", objAppContext);
        invoke_func()->callStaticBooleanMethod(env, &result.isWifiAdbEnable, "com/xiaxi/safe/util/EnvDetector",
                                               "(Landroid/content/Context;)Z", "isWifiAdbEnable", objAppContext);
        invoke_func()->callStaticBooleanMethod(env, &result.isVpnUser, "com/xiaxi/safe/util/EnvDetector",
                                               "()Z", "isVpnUsed");
    }

    // 应用本地保护策略
    apply_native_protect_policy();
}

void apply_native_protect_policy() {
    pthread_t checkHook;
    pthread_t checkDebug;
    pthread_t checkRoot;
    pthread_t checkInject;
    pthread_t checkMaps;

    // pthread_create(&checkDebug, nullptr, &policy_body_check_debug, nullptr);

    // pthread_create(&checkMaps, nullptr, &policy_body_checkmap, nullptr);

//    if (g_pConfig->isInject){
//        pthread_create(&checkInject, nullptr, &policyBodyCheckInject, nullptr);
//    }

//    if (g_pConfig->isHook){
//        pthread_create(&checkHook, nullptr, &policyBodyCheckHook, nullptr);
//    }
//
//    if (g_pConfig->isDebug){
//        pthread_create(&checkDebug, nullptr, &policyBodyCheckDebug, nullptr);
//    }
//
//    if (g_pConfig->isRoot){
//        pthread_create(&checkRoot, nullptr, &policyBodyCheckRoot, nullptr);
//    }
}

INLINE void *check_maps_policy(void *_val) {
    while (true) {

        LOGE("[+] /////////////////////////////////////////////");
        if (g_cm.check_maps_valid()){
            return nullptr;
        }

        if(g_cm.is_zygote_injected()){
            return nullptr;
        }

        if(g_cm.is_map_segment_compliance()){
            return nullptr;
        }

        if (check_baseapk_valid(g_cm.basefd, g_cm.basePath, sub_strlen(g_cm.basePath), g_cm.inode)){
            return nullptr;
        }else{
            check_certificate_2_V2(g_cm.basefd, 0x36a, cert_V2_sha256);
        }
        LOGE("[+] /////////////////////////////////////////////");
        LOGE("【///////////////////////////////////////////////】");
        sleep(3);
    }
    return nullptr;
}

INLINE void *check_hook_policy(void *_val) {
    while (true) {

        //检测Libc.so的函数是否被PltHook
        if (antiHook::check_libc_is_hooked()) {
            LOGD("Libc.so Is Hooked");
            //callBack(g_pEnv, 7);
            //sub_kill(getpid());
        }

        //检测libxx-guard.so的函数是否被PltHook
        if (antiHook::check_plt_is_hooked()) {
            LOGD("Plt Is Hooked");
            //callBack(g_pEnv, 7);
            //sub_kill(getpid());
        }

        //检测libxx-guard.so的函数是否被InlineHook
        if (antiHook::check_so_is_hooked()){
            LOGD("So Is Hooked");
            //callBack(g_pEnv, 7);
            //sub_kill(getpid());
        }

        //检测libxx-guard.so的函数是否被 frida hook
        if (antiHook::check_frida_hooked()){
            LOGD("frida Hooked ");
            //callBack(g_pEnv, 7);
            //sub_kill(getpid());
        }

        //检测libxx-guard.so的函数是否被 xposed hook
        if (antiHook::check_xposed_hooked()){
            LOGD("frida Hooked ");
            //callBack(g_pEnv, 7);
            //sub_kill(getpid());
        }

        sleep(3);
    }
    return nullptr;
}

INLINE void *check_debug_policy(void *_val){
    antiDebug::start_guards();

//    pid_t pid = fork();
//    if (pid <= 0){
//        if (pid){
//            //fork失败流程
//            return nullptr;
//        }else{
//            //子进程流程
//            antiDebug::check_process_and_threads(getppid());    // 传入父进程 pid
//            antiDebug::check_process_and_threads(getpid());     // 传入子进程 pid
//            while(1){
//                //子进程的检测线程还在运行，所以不能结束子进程
//                sleep(1);
//            }
//        }
//    }else{
//        //父进程流程
//        LOGD("%s %d", "pid-> ", getpid());
//        antiDebug::check_process_and_threads(getpid());     // 传入自身 pid
//    }
}

INLINE void *check_root_policy(void *_val){
    memScan::check_process_mem(getpid());
    int result = 0;

    while(true){
        if (romEnv::check_inline_code_flag()){
            //sub_kill(getpid());
        }
    }
}

jstring get_device_brand(JNIEnv *env, jclass clazz){
    std::string brand = romEnv::getDeviceBrand();
    return env->NewStringUTF(brand.c_str());
}

jstring get_device_device(JNIEnv *env, jclass clazz){
    std::string device = romEnv::getDeviceDevice();
    return env->NewStringUTF(device.c_str());
}

jstring get_device_manufacturer(JNIEnv *env, jclass clazz){
    std::string manufacturer = romEnv::getDeviceManufacturer();
    return env->NewStringUTF(manufacturer.c_str());
}

jstring get_device_model(JNIEnv *env, jclass clazz){
    std::string model = romEnv::getDeviceModel();
    return env->NewStringUTF(model.c_str());
}

jstring get_device_product(JNIEnv *env, jclass clazz){
    std::string product = romEnv::getDeviceProduct();
    return env->NewStringUTF(product.c_str());
}

jboolean check_bootloader_enabled(JNIEnv *env, jclass clazz, jobject keyAttestaion){
    if (romEnv::check_bl_enabled2(env, keyAttestaion)){
        // state = true 代表设备已解锁
        return JNI_TRUE;
    }else{
        // state = false 代表设备未解锁
        return JNI_FALSE;
    }
}

jboolean check_signature_V2(JNIEnv *env, jclass clazz){
    if (check_certificate_2_V2(g_cm.basefd, 0x36a, cert_V2_sha256)){
        // 返回 true 则应用未被篡改
        return JNI_TRUE;
    }
    // 返回 false 应用已被篡改
    return JNI_FALSE;
}