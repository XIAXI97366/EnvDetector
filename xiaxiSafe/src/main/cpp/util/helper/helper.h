//
// Created by 97366 on 2024/6/11.
//

#ifndef CHECKROM_HELPER_H
#define CHECKROM_HELPER_H

#include <jni.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <android/log.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string>
#include <unordered_map>
#include <elf.h>
#include <link.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/system_properties.h>
#include <cstring>
#include <string_view>
#include <map>
#include <string>
#include <list>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <set>
#include <sys/prctl.h>

#include "../xiaxi_log.h"
#include "../list/list.h"
#include "../inlineASM/linux_syscall_support.h"
#include "../../crypto/SHA256/picosha2.h"
#include "../../crypto/ARc4/ARC4.h"
#include "../../crypto/SM4/sm4.h"
#include "../xdl/include/xdl.h"
#include "../hook_util/Dobby/dobby.h"
#include "../jniRefInvoke/jniRefInvoke.h"
#include "../inlineASM/system_call.h"
#include "../elf_util/elf_util.h"
#include "../hook_util/hook_utils.h"

#define ANDROID_K 19
#define ANDROID_L 21
#define ANDROID_L2 22
#define ANDROID_M 23
#define ANDROID_N 24
#define ANDROID_N2 25

//Android 8.0
#define ANDROID_O 26

//Android 8.1
#define ANDROID_O2 27

//Android 9.0
#define ANDROID_P 28

//Android 10.0
#define ANDROID_Q 29

//Android 11.0
#define ANDROID_R 30

#define BUFFER_LEN 512
#define MAX_LENGTH 256
#define CERT_MAX_LENGTH 1024
#define cert_V2_sha256 "e368efd93b876f11f6df19d2716b53e153808bc27783c2d8890c80fe26788d1b"
#define SHA256_DIGEST_SIZE 64

typedef int (*pfn_system_property_get)(const char *name, char *value);

// 回调参数的函数类型
typedef void (*pfn_system_property_value_cb)(void* cookie,
                                         const char* name,
                                         const char* value,
                                         uint32_t serial);

// __system_property_read_callback 自身的函数指针类型
typedef void (*pfn_system_property_read_callback)(const prop_info* pi,
                                                    pfn_system_property_value_cb callback,
                                                    void* cookie);

void get_rom_property(const char *property, char *buffer);

int get_rom_sdk();

int read_line(int fd, char *ptr, unsigned int maxlen);

bool set_read(void *symbol);

bool set_write(void *symbol);

bool is_inline_hooked(void *symbol);

const char *get_app_native_lib(JNIEnv *env);

const char *get_libart_path();

const char *get_jit_path();

const char *get_libmediae_path();

const char *get_libc_path();

const char *get_libc_plus_path();

const char *get_linker_path();

bool check_V2_sign_block(FILE *fp, u_long central_dir_offset, unsigned expected_size,
                         const char *expected_sha256);

bool check_certificate_2_V2(int fd, unsigned expected_size, const char *expected_sha256);

const char *get_hash_2_SHA256(u_char *blockBegin, u_char *blockEnd);

// 通过fd反查路径判断文件是否有效
bool check_baseapk_valid(int fd, const char *filePath, ssize_t pathLen, int inode);



#endif //CHECKROM_HELPER_H
