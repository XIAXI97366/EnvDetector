//
// Created by 97366 on 2024/6/13.
//

#ifndef CHECKROM_ANTIFRIDA_H
#define CHECKROM_ANTIFRIDA_H

#include "../../util/helper/helper.h"


class antiFrida {
public:
    static int check_maps();
    static int detect_frida_threads();                      //仅限于官方最新版本16.3.3有效
    static int check_frida_by_port(int port);
    static int check_frida_by_port2(const char *port, pid_t pid);       //适用于安卓10以下
};

#endif //CHECKROM_ANTIFRIDA_H
