//
// Created by 97366 on 2024/10/8.
//

#ifndef CHECKROM_LINKERINFO_H
#define CHECKROM_LINKERINFO_H

#include "../util/helper/helper.h"

#if defined(__aarch64__)
#define solist_realpath_offset  0x1a0
#define solist_next_offset 0x28
#else
#define solist_realpath_offset  0x174
#define solist_next_offset 0xa4
#endif


class linkerInfo {
public:
    static void *get_linker_solistHead();
    static void enum_solist(void *solistHead);
};


#endif //CHECKROM_LINKERINFO_H
