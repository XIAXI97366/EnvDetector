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


bool guardPorcess::check_process_stopped(pid_t pid) {
    char formatStr[256] = {0};
    char buffer[1024] = {0};
    char compBuffer[1024] = {0};
    char tmpBuffer[1024] = {0};
    FILE *fp = nullptr;
    bool flag = false;
    int tracePid = 0;

    snprintf(formatStr, 256LL, "/proc/%d/status", pid);
    fp = fopen(formatStr, "r");
    if (nullptr == fp){
        LOGD("%s", strerror(errno));
        goto CHECK_PROCESS_STOPPED_EXIT;
    }
    while (fgets(buffer, 1024LL, fp)){
        int len = sub_strlen(targetStr1);
        if (!(unsigned int)sub_strncmp(buffer, targetStr1, len)
             && (strcasestr(buffer, targetStr2) || strcasestr(buffer, targetStr5) ||
                strcasestr(buffer, targetStr4)) ){
            while (fgets(compBuffer, 1024LL, fp)){
                len = sub_strlen(targetStr0);
                if (!(unsigned int)sub_strncmp(compBuffer, targetStr0, len) ){
                    sscanf(compBuffer, "%s %d", tmpBuffer, &tracePid);
                    break;
                }
                memset(compBuffer, 0, sizeof(compBuffer));
            }
            if (tracePid){
                //getpid()如果不是子进程 trace 父进程 或者是父进程 trace 自身
                if (getpid() != tracePid){
                    flag = true;
                    goto CHECK_PROCESS_STOPPED_EXIT;
                }
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

bool guardPorcess::scan_process_threads(pid_t pid){
    char formatStr[256] = {0};
    bool flag = false;
    struct dirent64 *dirent64 = nullptr;
    int tid = 0;

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

        if (sub_strcmp(dirent64->d_name, ".") ){
            if (sub_strcmp(dirent64->d_name, "..")){
                tid = atoi(dirent64->d_name);
                if ( pid != tid ){
                    if (check_threads_status(pid, tid)){
                        flag = true;
                        goto SCAN_PROCESS_THREADS_EXIT;
                    }
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

bool guardPorcess::check_threads_status(int pid, int tid) {
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
        LOGD("%s", strerror(errno));
        goto CHECK_THREADS_STATUS_EXIT;
    }

    while (fgets(buffer, 1024LL, fp)){
        len = sub_strlen(targetStr1);
        if ( !sub_strncmp(buffer, targetStr1, len) && (strcasestr(buffer, targetStr2) ||
            strcasestr(buffer, targetStr4)) ){
            while (fgets(targetBuff, 1024LL, fp) ){
                len = sub_strlen(targetStr3);
                if ( !(unsigned int)sub_strncmp(targetBuff, targetStr3, len) ){
                    sscanf(targetBuff, "%s %d", tmpBuffer, &ppid);
                    break;
                }
            }
            memset(targetBuff, 0, sizeof(targetBuff));
            while ( fgets(targetBuff, 1024LL, fp) ){
                len = sub_strlen(targetStr0);
                if ( !(unsigned int)sub_strncmp(targetBuff, targetStr0, len) ){
                    sscanf(targetBuff, "%s %d", tmpBuffer, &tracePid);
                    break;
                }
            }
            //getpid() != tracePid 如果不是当前进程（如子进程）trace父进程，且也不是父进程trace自身，同时tracePid不为0那么处于被调试状态
            if ( tracePid && getpid() != tracePid && tracePid != ppid ){
                flag = true;
                goto CHECK_THREADS_STATUS_EXIT;
            }
        }
    }

CHECK_THREADS_STATUS_EXIT:
    if (nullptr != fp){
        fclose(fp);
    }
    return flag;
}

