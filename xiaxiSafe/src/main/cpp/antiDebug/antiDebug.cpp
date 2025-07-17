//
// Created by 97366 on 2024/7/17.
//

#include "antiDebug.h"

static ANTIDEBUG anti = {
        guardPorcess::check_process_stopped,
        guardPorcess::scan_process_threads
};

bool antiDebug::check_process_and_threads(pid_t pid) {
    if (anti.guardPorcess_check_process_stopped(pid) ||
        anti.guardPorcess_scan_process_threads(pid)){
        return true;
    }else{
        return false;
    }
}


