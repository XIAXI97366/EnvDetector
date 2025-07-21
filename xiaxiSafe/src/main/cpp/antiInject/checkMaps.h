//
// Created by 97366 on 2024/10/21.
//

#ifndef CHECKROM_CHECKMAPS_H
#define CHECKROM_CHECKMAPS_H

#include "../util/helper/helper.h"

typedef struct map_seg_info{
    char name[MAX_LENGTH];
    char property[4];
    unsigned long inode;
    unsigned long start;
    unsigned long end;
    struct map_seg_info *next;
}map_seg_info, *p_map_seg_info;

class checkMaps {
public:
    checkMaps();
    ~checkMaps();
    bool check_map_injected();
    p_map_seg_info get_map_seg_info();
    bool is_zygote_injected();
    bool is_map_segment_compliance();
    bool check_maps_valid();
    bool get_base_fd();
    void get_map_segment_path(const char *key, char *buf, int size);
public:
    p_map_seg_info phdr;
    int mapfd = 0;
    int basefd = 0;
    char basePath[MAX_LENGTH] = {0};
    int inode = 0; // 保存 maps 指向 base.apk 那一项内存段的 inode 号
};


#endif //CHECKROM_CHECKMAPS_H
