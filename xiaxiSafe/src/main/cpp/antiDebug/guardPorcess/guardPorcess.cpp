//
// Created by 97366 on 2024/6/11.
//

#include "guardPorcess.h"

const char *guardPorcess::targetStr0 = "TracerPid:";
const char *guardPorcess::targetStr1 = "State:";
const char *guardPorcess::targetStr2 = "T (stopped)";
const char *guardPorcess::targetStr3 = "PPid:";
const char *guardPorcess::targetStr4 = "t (tracing stop)";
const char *guardPorcess::targetStr5 = "(zombie)";
const char *guardPorcess::testStr6 = "S (sleeping)";

bool guardPorcess::check_process_stopped(pid_t pid, pid_t tracerPid) {
    char formatStr[256] = {0};
    char buffer[1024] = {0};
    char compBuffer[1024] = {0};
    char tmpBuffer[1024] = {0};
    FILE *fp = nullptr;
    bool flag = false;
    int tracePid = 0;
    int len = 0;
    pid_t ppid = 0;

    // 该函数位于子进程的检测线程中，如果传入的是parent的pid，那么就是parent的status == child1 -> parent
    // 该函数位于子进程的检测线程中，如果传入的是child1的pid，那么就是child1的status == child2 -> child1
    // 该函数位于子进程的检测线程中，如果传入的是child2的pid，那么就是child2的status == parent -> child2
    snprintf(formatStr, 256LL, "/proc/%d/status", pid);
    fp = fopen(formatStr, "r");
    if (nullptr == fp){
        LOGE("[-] %s %d %s ", __FUNCTION__ , __LINE__, strerror(errno));
        goto CHECK_PROCESS_STOPPED_EXIT;
    }

    while (fgets(buffer, 1024LL, fp)){
        len = sub_strlen(targetStr1);
        // 如果比对上 targetStr1（State:）取反返回1且能够找到 "T (stopped)"、"t (tracing stop)"、"(zombie)"
        // 上述三种状态中的其中一个就进入if内
        if (!(unsigned int)sub_strncmp(buffer, targetStr1, len)
             && (strcasestr(buffer, targetStr2) || strcasestr(buffer, targetStr5) ||
                strcasestr(buffer, targetStr4) || strcasestr(buffer, testStr6))){

            // 获取 status 文件中的 ppid
            while (fgets(compBuffer, 1024LL, fp) ){
                len = sub_strlen(targetStr3);
                if ( !(unsigned int)sub_strncmp(compBuffer, targetStr3, len) ){
                    sscanf(compBuffer, "%s %d", tmpBuffer, &ppid);
                    break;
                }
            }
            memset(compBuffer, 0, sizeof(compBuffer));

            while (fgets(compBuffer, 1024LL, fp)){
                len = sub_strlen(targetStr0);
                if (!(unsigned int)sub_strncmp(compBuffer, targetStr0, len) ){
                    sscanf(compBuffer, "%s %d", tmpBuffer, &tracePid);
                    break;
                }
                memset(compBuffer, 0, sizeof(compBuffer));
            }

            // 针对于传入的是child2的pid，但child2的tracePid是在父进程的检测线程中调用
            // 所以 tracerPid == /proc/%d/status 中的 ppid
            // 所以获取从该文件中获取的ppid就等于tracerPid
            if (ppid == tracerPid){
                // 用于parent的检测线程，传入的是child2的pid，/proc/child2/status/tracePid != 0 则判定为被调试
                if (0 != tracePid){
                    LOGE("[-] %s %d child2 tracePid -> %d != 0", __FUNCTION__ , __LINE__, tracePid);
                    flag = true;
                    goto CHECK_PROCESS_STOPPED_EXIT;
                }else{
                    LOGE("[+] %s %d child2 0 == tracePid-> %d", __FUNCTION__ , __LINE__, tracePid);
                    continue;
                }
            }

            if (tracePid != tracerPid){
                // 用于子进程child1的检测线程，传入的是parent的pid，/proc/parent/status/tracePid != child1 则判定为被调试
                // 用于子进程child2的检测线程，传入的是child1的pid，/proc/child1/status/tracePid != child2 则判定为被调试
                LOGE("[-] %s %d tracePid-> %d != tracerPid -> %d", __FUNCTION__ , __LINE__, tracePid, tracerPid);
                flag = true;
                goto CHECK_PROCESS_STOPPED_EXIT;
            }else{
                LOGE("[+] %s %d parent or child tracePid-> %d == tracerPid -> %d", __FUNCTION__ ,
                     __LINE__, tracePid, tracerPid);
            }

            break;
        }
    }

CHECK_PROCESS_STOPPED_EXIT:
    if(nullptr != fp){
        fclose(fp);
    }
    return flag;
}

bool guardPorcess::scan_process_threads(pid_t pid, pid_t tracerPid){
    char formatStr[256] = {0};
    bool flag = false;
    struct dirent64 *dirent64 = nullptr;
    int tid = 0;

    // child1的检测线程中，传入parent的pid，/proc/parent/task/pid/status->TracerPid != child1 && 其余线程的 TracerPid 不为0，则判定为被调试
    // child2的检测线程中，传入child1的pid，/proc/child1/task/pid/status->TracerPid != child2 && 其余线程的 TracerPid 不为0，则判定为被调试
    // parent的检测线程中，传入child2的pid，/proc/child2/task/所有线程/status->TracerPid != 0 则判定为被调试
    sprintf(formatStr, "/proc/%d/task/", pid);
    DIR *dir = (DIR *)opendir(formatStr);
    if (!dir){
        LOGD("%s", strerror(errno));
        goto SCAN_PROCESS_THREADS_EXIT;
    }

    while (true){
        dirent64 = (struct dirent64 *)readdir64(dir);
        if (nullptr == dirent64){
            goto SCAN_PROCESS_THREADS_EXIT;
        }

        // 遍历 task 的目录去除
        if (sub_strcmp(dirent64->d_name, ".") ){
            if (sub_strcmp(dirent64->d_name, "..")){
                tid = atoi(dirent64->d_name);
                //（针对于参数pid是parent和child1的进程号）如果是主线程，那么 tracePid = tracerPid，其他线程的 tracePid = 0
                // （针对于参数pid是child2的进程号），那么所有的线程都的 tracePid 只能为0
                if (check_threads_status(pid, tid, tracerPid)){
                    flag = true;
                    goto SCAN_PROCESS_THREADS_EXIT;
                }
            }
        }
    }

SCAN_PROCESS_THREADS_EXIT:
    if (nullptr != dir){
        closedir(dir);
    }
    return flag;
}

bool guardPorcess::check_threads_status(pid_t pid, pid_t tid, pid_t tracerPid) {
    char formatStr[256] = {0};
    char buffer[1024] = {0};
    char targetBuff[1024] = {0};
    char tmpBuffer[1024] = {0};
    bool flag = false;
    FILE *fp = nullptr;
    int ppid = 0;
    int tracePid = 0;
    int len = 0;

    snprintf(formatStr, 256LL, "/proc/%d/task/%d/status", pid, tid);
    fp = fopen(formatStr, "r");
    if (nullptr == fp){
        LOGE("[-] %s %d %s pid %d tid %d", __FUNCTION__ , __LINE__, strerror(errno), pid, tid);
        goto CHECK_THREADS_STATUS_EXIT;
    }

    while (fgets(buffer, 1024LL, fp)){
        len = sub_strlen(targetStr1);
        if (!sub_strncmp(buffer, targetStr1, len) && (strcasestr(buffer, targetStr2) ||
            strcasestr(buffer, targetStr4) || strcasestr(buffer, targetStr5) || strcasestr(buffer, testStr6))){

            // 获取 status 文件中的 ppid
            while (fgets(targetBuff, 1024LL, fp) ){
                len = sub_strlen(targetStr3);
                if ( !(unsigned int)sub_strncmp(targetBuff, targetStr3, len) ){
                    sscanf(targetBuff, "%s %d", tmpBuffer, &ppid);
                    break;
                }
            }
            memset(targetBuff, 0, sizeof(targetBuff));

            // 获取 status 文件中的 tracePid
            while ( fgets(targetBuff, 1024LL, fp) ){
                len = sub_strlen(targetStr0);
                if ( !(unsigned int)sub_strncmp(targetBuff, targetStr0, len) ){
                    // 拿到每个线程 status 记录的 tracePid
                    sscanf(targetBuff, "%s %d", tmpBuffer, &tracePid);
                    break;
                }
            }

            if (pid == tid){
                // 针对于传入的是child2的pid，但child2的tracePid是在父进程的检测线程中调用，
                // 所以 tracerPid == /proc/pid/task/tid/status 中的 ppid
                // 所以获取从该文件中获取的ppid就等于tracerPid
                if (ppid == tracerPid){
                    if (0 != tracePid){
                        LOGE("[-] %s %d child2 main task 0 != tracePid -> %d", __FUNCTION__ , __LINE__, tracePid);
                        flag = true;
                        goto CHECK_THREADS_STATUS_EXIT;
                    }else{
                        LOGE("[+] %s %d child2 main task 0 == tracePid -> %d", __FUNCTION__ , __LINE__, tracePid);
                        continue;
                    }
                }

                // （针对于传入的是parent和child1的pid）如果是主线程，那么 tracePid = tracerPid
                if (tracePid != tracerPid){
                    LOGE("[-] %s %d pid -> %d main task tracePid-> %d != tracerPid -> %d",
                         __FUNCTION__ , __LINE__, pid, tracePid, tracerPid);
                    flag = true;
                    goto CHECK_THREADS_STATUS_EXIT;
                }else{
                    LOGE("[+] %s %d pid -> %d main task tracePid-> %d == tracerPid -> %d",
                         __FUNCTION__ , __LINE__, pid, tracePid, tracerPid);
                }
            }else{
                // 其他线程的 tracePid = 0
                if(0 != tracePid){
                    LOGE("[-] %s %d pid -> %d Other task 0 != tracerPid -> %d", __FUNCTION__ , __LINE__, pid, tracePid);
                    flag = true;
                    goto CHECK_THREADS_STATUS_EXIT;
                }else{
                    LOGE("[+] %s %d pid -> %d Other task 0 == tracerPid -> %d", __FUNCTION__ , __LINE__, pid, tracePid);
                }
            }
        }
    }

CHECK_THREADS_STATUS_EXIT:
    if (nullptr != fp){
        fclose(fp);
    }
    return flag;
}

