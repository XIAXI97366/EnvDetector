//
// Created by 97366 on 2024/6/11.
//

#ifndef CHECKROM_GUARDPORCESS_H
#define CHECKROM_GUARDPORCESS_H

#include "../../util/helper/helper.h"

class guardPorcess {
public:
    static const char *targetStr0;
    static const char *targetStr1;
    static const char *targetStr2;
    static const char *targetStr3;
    static const char *targetStr4;
    static const char *targetStr5;
    static const char *testStr6;
public:
    //检测父进程或当前进程的 /proc/pid/status 和 /proc/pid/task/tid/status 文件中的标志
    bool static check_process_stopped(pid_t pid, pid_t tracerPid);
    bool static scan_process_threads(pid_t pid, pid_t tracerPid);
    bool static check_threads_status(pid_t pid, pid_t tid, pid_t tracerPid);
};


#endif //CHECKROM_GUARDPORCESS_H
