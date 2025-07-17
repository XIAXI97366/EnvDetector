//
// Created by 97366 on 2024/7/19.
//

#include "memScan.h"

bool memScan::check_maps_segment_have_magisk(pid_t pid, const char *segmentFlag) {
    bool flag = false;
    char formatStr[BUFFER_LEN] = {0};
    char buffer[BUFFER_LEN] = {0};
    int fd = 0;
    char path[256] = {0};
    char perm[5]= {0};
    unsigned long offset = 0;
    unsigned long base = 0;
    unsigned long end = 0;
    void *checkSegment = nullptr;
    const char *MAGISK = ".magisk";
    const char *magisk = "/.magisk/";

    snprintf(formatStr, BUFFER_LEN, "/proc/%d/maps", pid);
    fd = sub_openat(AT_FDCWD, formatStr, O_RDONLY, 0);
    if (fd > 0) {
        while ((read_line(fd, buffer, BUFFER_LEN)) > 0) {
            if (sub_strstr(buffer, segmentFlag)){
                sscanf(buffer, "%lx-%lx %4s %lx %*s %*s %255[^\n]", &base, &end, perm, &offset, path);
                checkSegment = (void*)malloc(end - base + strlen(magisk));
                if ('r' == perm[0] && NULL != checkSegment){
                    LOGD("正在检测->%s", buffer);
                    memmove(checkSegment, (void*)base, end - base);
                    for (unsigned long i = 0; i < (end - base); i += strlen(MAGISK)) {
                        if (0 == memcmp((void*)((unsigned long)checkSegment + i), (void*)MAGISK, strlen(MAGISK))){
                            //在tls内存段中检测出 magisk 的情况
                            LOGD("在 %s 内存段中找到 magisk 关键字", segmentFlag);
                            flag = true;
                            goto CHECK_MAPS_SEGMENT_EXIT;
                        }
                    }
                    for (unsigned long i = 0; i < (end - base); i += strlen(magisk)) {
                        if (0 == memcmp((void*)((unsigned long)checkSegment + i), (void*)magisk, strlen(magisk))){
                            //在tls内存段中检测出 magisk 的情况
                            LOGD("在 %s 内存段中找到 magisk 关键字", segmentFlag);
                            flag = true;
                            goto CHECK_MAPS_SEGMENT_EXIT;
                        }
                    }
                }
                if (NULL != checkSegment){
                    free(checkSegment);
                }
            }
            memset(path, 0, sizeof(path));
        }
    }

CHECK_MAPS_SEGMENT_EXIT:
    if (nullptr != checkSegment){
        free(checkSegment);
        checkSegment = nullptr;
    }
    if (0 < fd){
        sub_close(fd);
        fd = 0;
    }
    return flag;
}

bool memScan::check_process_mem(pid_t pid) {
    if (check_maps_segment_have_magisk(pid, "tls") ||
        check_maps_segment_have_magisk(pid, "[stack]") ||
        check_maps_segment_have_magisk(pid, "/memfd:") ||
        check_maps_segment_have_magisk(pid, "/system/bin/app_process")){
        return true;
    }else{
        return false;
    }
}