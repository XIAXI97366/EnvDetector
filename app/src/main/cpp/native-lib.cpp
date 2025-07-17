#include <jni.h>
#include <string>
//#include "../../../../mxSafe/src/main/cpp/antiXPosed/antiXPosed.h"
//#include "../../../../mxSafe/src/main/cpp/guardPorcess/guardPorcess.h"
//#include "../../../../mxSafe/src/main/cpp/antiFrida/antiFrida.h"
//#include "../../../../mxSafe/src/main/cpp/elfParse/elfParse.h"
//#include "../../../../mxSafe/src/main/cpp/romEnv/romEnv.h"

//static void* threadBody(void* arg) {
//    elfParse elfParse;
//    elf_dyn_info* memself = elfParse.elfParseMemSo("libcheckrom.so");
//    while (true){
//        elfParse.checkRelPlt(memself);
//        elfParse.checkRelDyn(memself);
//        sleep(1);
//    }
//
//    //sub_kill(pid);
//    return nullptr;
//}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_checkrom_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
//    int result = antiXPosed::checkModel();
//    if (!result && result != -1){
//        antiXPosed::checkMaps();
//        antiXPosed::chekStack(env);
//        antiXPosed::checkService(env);
//        antiXPosed::checkClass(env);
//        antiXPosed::checkProcess();
//    }

//    romEnv rom;
//    if (rom.checkAppIsInstall()){
//        LOGD("已安装危险应用");
//    }else{
//        LOGD("未安装危险应用");
//    }


//    pthread_t pthread;
//    pthread_create(&pthread, NULL,
//                   threadBody,
//                   (void*)NULL);
//
//    antiFrida::start();
//
//    pid_t pid = fork();
//    if (pid <= 0){
//        if (pid){
//            //fork失败流程
//        }else{
//            //子进程流程
//            guardPorcess::checkMemorgFlag(getpid());
//            while(1){
//                sleep(1);
//            }
//        }
//    }else{
//        guardPorcess::checkMemorgFlag(getpid());
//        //父进程流程
//        LOGD("%s %d", "pid-> ", getpid());
//        guardPorcess::antiThreadOfProcess(getpid());
//    }


    return env->NewStringUTF("hello would");
}







