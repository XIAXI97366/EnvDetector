//
// Created by 97366 on 2024/7/19.
//

#ifndef CHECKROM_MEMSCAN_H
#define CHECKROM_MEMSCAN_H

#include "../util/helper/helper.h"

class memScan {
public:
    //检查maps文件中的含有tls、[stack]、/memfd:、/system/bin/app_process、的内存段中是否存在".magisk"和"/.magisk/"
    static bool check_maps_segment_have_magisk(pid_t pid, const char *segmentFlag);
    static bool check_process_mem(pid_t pid);
};


#endif //CHECKROM_MEMSCAN_H
