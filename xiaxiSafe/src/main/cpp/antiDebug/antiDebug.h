//
// Created by 97366 on 2024/7/17.
//

#ifndef CHECKROM_ANTIDEBUG_H
#define CHECKROM_ANTIDEBUG_H

#include "guardPorcess/guardPorcess.h"

typedef struct {
    bool (*guardPorcess_check_process_stopped)(pid_t pid);
    bool (*guardPorcess_scan_process_threads)(pid_t pid);
} ANTIDEBUG;

class antiDebug {
public:
    static bool check_process_and_threads(pid_t pid);
};


#endif //CHECKROM_ANTIDEBUG_H
