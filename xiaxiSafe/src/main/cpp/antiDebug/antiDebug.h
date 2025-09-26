//
// Created by 97366 on 2024/7/17.
//

#ifndef CHECKROM_ANTIDEBUG_H
#define CHECKROM_ANTIDEBUG_H

#include "guardPorcess/guardPorcess.h"

typedef struct {
    bool (*guardPorcess_check_process_stopped)(pid_t pid, pid_t tracerPid);
    bool (*guardPorcess_scan_process_threads)(pid_t pid, pid_t tracerPid);
} ANTIDEBUG;

typedef struct{
    pid_t Stalked;  // 被跟踪进程
    pid_t Stalker;  // 跟踪进程
}TRACE;

typedef struct {
    pid_t child1;   // 子进程1
    pid_t child2;   // 子进程2
} MONITOR;

class antiDebug {
public:
    static bool check_process_and_threads(pid_t pid, pid_t tracerPid);
    static void *check_loop(void *arg);
    static void *monitor_child_loop(void *arg);
    static void *monitor_parent_loop(void *arg);
    static int start_guards();
};


#endif //CHECKROM_ANTIDEBUG_H
