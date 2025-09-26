#include "xxGuard.h"


void xxProtect(JNIEnv *env, jclass clazz, jobject application, jobject process, jobject manager);

extern "C" void _init(void){
    //修复so文件格式并读取SM4配置文件中的密钥和IV设置对应加固选项
    //后续添加修复so格式的函数并配置SM4配置文件的二进制数据记录在so的elf文件格式中的函数
};

__attribute__((constructor)) void init_array_1(){}

__attribute__((constructor)) void init_array_2(){}

__attribute__((constructor)) void init_array_3(){}

__attribute__((destructor)) void finit_array_1(){
    if (nullptr != g_config){
        free(g_config);
        g_config = nullptr;
    }
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env= nullptr;
    jclass clsXxSafe = nullptr;
    jclass clsEnv = nullptr;
    g_vm = vm;
    std::string romInfo = "";

    if (JNI_OK != vm->GetEnv((void **) &env, JNI_VERSION_1_6)) {
        return JNI_ERR;
    }

    // java:com.mx.safe.XxSafe.protect ==> xx-guard.cpp.a
    clsXxSafe = env->FindClass("com/xiaxi/safe/XxSafe");
    static JNINativeMethod methodsXxSafe[] = {
            {"protect",
             "(Landroid/app/Application;Lcom/xiaxi/safe/XxEventProcess;Landroid/content/res/AssetManager;)V",
             (void *)&xxProtect},
    };
    env->RegisterNatives(clsXxSafe, methodsXxSafe, sizeof(methodsXxSafe) / sizeof(methodsXxSafe[0]));

    clsEnv = env->FindClass("com/xiaxi/safe/util/EnvDetector");
    static JNINativeMethod methodsEnvDetector[] = {
            {"getDeviceBrand",  "()Ljava/lang/String;", (void *)&getDeviceBrand},
            {"getDeviceDevice",       "()Ljava/lang/String;", (void *)&getDeviceDevice},
            {"getDeviceManufacturer", "()Ljava/lang/String;", (void *)&getDeviceManufacturer},
            {"getDeviceModel",        "()Ljava/lang/String;", (void *)&getDeviceModel},
            {"getDeviceProduct",         "()Ljava/lang/String;", (void *)&getDeviceProduct},
            {"isBootLoaderEnabled", "(Lcom/xiaxi/safe/util/KeyAttestation/KeyAttestation;)Z", (void *)&isBootLoaderEnabled},
    };
    env->RegisterNatives(clsEnv, methodsEnvDetector, sizeof(methodsEnvDetector) / sizeof(methodsEnvDetector[0]));






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

void xxProtect(JNIEnv *env, jclass clazz, jobject application, jobject process, jobject manager) {
    jclass clsMxsafe = nullptr;
    jmethodID midcheckActivitySafe = nullptr;
    g_env = env;
    g_ref = invoke_func();
    antiHook::env = env;


//    if (process != nullptr) {
//        if (g_event != nullptr) {
//            env->DeleteGlobalRef(g_event);
//        }
//        g_event = env->NewGlobalRef(process);
//    }
//    g_xxsafe = env->NewGlobalRef(clazz);
    //g_midIsVpnUsed = env->GetStaticMethodID(clazz, "isVpnUsed", "()Z");

    //jclass clsMxEventProcess = env->FindClass("com/xiaxi/safe/XxEventProcess");
    //g_midOnMessage = env->GetMethodID(clsMxEventProcess, "onMessage", "(I)V");

//    if (!sm4DecryptNormal(env, manager)){
//        LOGD("Get SM4 Config Error");
//        //sub_kill(getpid());
//    }

//    if (g_config->isScreen || g_config->isHijack) {
//        clsMxsafe = env->FindClass("com/mx/safe/MxSafe");
//        midcheckActivitySafe = env->GetStaticMethodID(clsMxsafe, "checkActivitySafe",
//                                                   "(Landroid/app/Application;ZZ)V");
////        env->CallStaticVoidMethod(clsMxsafe, midcheckActivitySafe, application, g_config->isScreen,
////                                  g_config->isHijack);
//        env->CallStaticVoidMethod(clsMxsafe, midcheckActivitySafe, application, 1,
//                                  1);
//    }

    //应用保护策略
    apply_protect_policy();
}

void apply_protect_policy() {
    pthread_t checkHook;
    pthread_t checkDebug;
    pthread_t checkRoot;
    pthread_t checkInject;
    pthread_t checkMaps;

      pthread_create(&checkDebug, nullptr, &policy_body_check_debug, nullptr);

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

//INLINE void callBack(JNIEnv *env, int code)  {
//    if (nullptr != g_event) {
//        env->CallVoidMethod(g_event, g_midOnMessage, code);
//    } else {
//        sleep(3);
//        exit(0);
//    }
//}

INLINE void *policy_body_checkmap(void *_val) {
    while (true) {
        checkMaps ck;
        ck.get_map_seg_info();
        ck.get_base_fd();

        LOGE("[+] /////////////////////////////////////////////");
        if (ck.check_maps_valid()){
            return nullptr;
        }

        if(ck.is_zygote_injected()){
            return nullptr;
        }

        if(ck.is_map_segment_compliance()){
            return nullptr;
        }

        if (check_baseapk_valid(ck.basefd, ck.basePath, sub_strlen(ck.basePath), ck.inode)){
            return nullptr;
        }else{
            check_certificate_2_V2(ck.basefd, 0x36a, cert_V2_sha256);
        }
        LOGE("[+] /////////////////////////////////////////////");
        LOGE("【///////////////////////////////////////////////】");
        sleep(3);
    }
    return nullptr;
}

INLINE void *policy_body_check_hook(void *_val) {
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

INLINE void *policy_body_check_debug(void *_val){
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

INLINE void *policy_body_check_root(void *_val){
    memScan::check_process_mem(getpid());
    int result = 0;

    while(true){
        if (romEnv::check_inline_code_flag()){
            //sub_kill(getpid());
        }
    }
}

//直接从 assets 目录下直接读取 225ea18d3fbdc836f2c28600171e5817.png 按照 SM4 算法获取对应的加密选项
//int sm4DecryptNormal(JNIEnv *env, jobject manager){
//    AAssetManager *pAssetManager = nullptr;
//    AAsset *pFile = nullptr;
//    u_char *pEncrypt = nullptr;
//    u_char *pDecrypt = nullptr;
//    SM4_KEY_st *pSm4 = nullptr;
//
//    pAssetManager = AAssetManager_fromJava(env, manager);
//    pFile = AAssetManager_open(pAssetManager, "225ea18d3fbdc836f2c28600171e5817.png",
//                               AASSET_MODE_BUFFER);
//    size_t fileLength = AAsset_getLength(pFile);
//    if (0 != (fileLength % 16)){
//        //如果读取出来的数据长度不是16的倍数，说明当前ROM环境不可靠
//        return 0;
//    }
//    if (nullptr != g_config){
//        free(g_config);
//        g_config = nullptr;
//    }
//
//    g_config = (MX_CONFIG *) malloc(sizeof(struct mxConfig));
//    pEncrypt = (u_char *) malloc(fileLength);
//    pDecrypt = (u_char *) malloc(fileLength);
//    pSm4 = (SM4_KEY_st *) malloc(sizeof(struct SM4_KEY_st));
//    memset(g_config, 0, sizeof(struct mxConfig));
//    memset(pEncrypt, 0, fileLength);
//    memset(pDecrypt, 0, fileLength);
//    memset(pSm4, 0, sizeof(struct SM4_KEY_st));
//
//    AAsset_read(pFile, pEncrypt, fileLength);
//    AAsset_close(pFile);
//
//    for (int i = 0; i < sizeof(struct mxConfig); i++) {
//        pEncrypt[i] = (char) (pEncrypt[i] ^ 0x8);
//    }
//    sm4_set_key((u_char *) "mx-safe-jhjxiaxi", pSm4);
//    sm4_decrypt(pEncrypt, pDecrypt, pSm4);
//    memcpy(g_config, pDecrypt, sizeof(struct mxConfig));
//
//    if (nullptr != pSm4){
//        free(pSm4);
//        pSm4 = nullptr;
//    }
//    if (nullptr != pEncrypt){
//        free(pEncrypt);
//        pEncrypt = nullptr;
//    }
//    if (nullptr != pDecrypt){
//        free(pDecrypt);
//        pDecrypt = nullptr;
//    }
//
//    return 1;
//}

//void getSM4Config(){
//    //后续添加 遍历动态段获取第一个不被系统承认的init_proc函数的二进制，通过SM4进行解密，key为"mx-safe-jhjxiaxi"
//}


jstring getDeviceBrand(JNIEnv *env, jclass clazz){
    std::string brand = romEnv::getDeviceBrand();
    return env->NewStringUTF(brand.c_str());
}

jstring getDeviceDevice(JNIEnv *env, jclass clazz){
    std::string device = romEnv::getDeviceDevice();
    return env->NewStringUTF(device.c_str());
}

jstring getDeviceManufacturer(JNIEnv *env, jclass clazz){
    std::string manufacturer = romEnv::getDeviceManufacturer();
    return env->NewStringUTF(manufacturer.c_str());
}

jstring getDeviceModel(JNIEnv *env, jclass clazz){
    std::string model = romEnv::getDeviceModel();
    return env->NewStringUTF(model.c_str());
}

jstring getDeviceProduct(JNIEnv *env, jclass clazz){
    std::string product = romEnv::getDeviceProduct();
    return env->NewStringUTF(product.c_str());
}

jboolean isBootLoaderEnabled(JNIEnv *env, jclass clazz, jobject keyAttestaion){
    bool state = false;

    state = romEnv::check_bl_enabled2(env, keyAttestaion);
    if (state){
        // state = true 代表设备已解锁
        return JNI_TRUE;
    }else{
        // state = false 代表设备未解锁
        return JNI_FALSE;
    }
}