//
// Created by 97366 on 2024/10/4.
//

#ifndef CHECKROM_MEMCRC_H
#define CHECKROM_MEMCRC_H

#include "../util/helper/helper.h"

//记录so中可执行段的结构体，一个是 plt 段一个是 text 段
typedef struct stExecSection {
    int execSectionCount;
    unsigned long offset[2];
    unsigned long addr[2];
    unsigned long dataSize[2];
    unsigned long checkSum[2];
    unsigned long memLibBase;
} execSection;

class memCrc {
public:
    static int check_sum_for_libart();
    static int check_sum_for_libc();
    static int check_sum_for_linker();
    static int chekc_sum_for_customlib(const char *libPath);

};


#endif //CHECKROM_MEMCRC_H
