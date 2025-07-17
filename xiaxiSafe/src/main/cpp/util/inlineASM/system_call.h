//
// Created by XIAXI on 2024/12/5.
//

#ifndef CHECKROM_SYSTEM_CALL_H
#define CHECKROM_SYSTEM_CALL_H

#include "../helper/helper.h"

#define INLINE __attribute__((always_inline)) __inline__

extern "C"{
    INLINE long raw_syscall(long __number, ...);

    INLINE static int sub_openat(int _fd, const void *_path, int _flags, int _mode){
        return sys_openat(_fd, (const char *)_path, _flags, _mode);
    }

    INLINE static ssize_t sub_read(int _fd, void* _buf, size_t _count){
        return sys_read(_fd, _buf, _count);
    }

    INLINE static off_t sub_lseek(int __fd, off_t __offset, int __whence){
        return sys_lseek(__fd, __offset, __whence);
    }

    INLINE static ssize_t sub_write(int _fd, void* _buf, size_t _count){
        return sys_write(_fd, _buf, _count);
    }

    INLINE static int sub_kill(pid_t pid){
        return sys_kill(pid, 9);
    }

    INLINE static int sub_close(int _fd){
        return sys_close(_fd);
    }

    INLINE static int sub_socket(int __af, int __type, int __protocol){
        return sys_socket(__af, __type, __protocol);
    }

    INLINE static ssize_t sub_readlinkat(int __dir_fd, const char* __path, char* __buf, size_t __buf_size){
        return sys_readlinkat(__dir_fd, __path, __buf,__buf_size);
    }

    INLINE static ssize_t sub_readlink(const char* __path, char* __buf, size_t __buf_size){
        return sys_readlink(__path, __buf, __buf_size);
    }

    INLINE static int sub_fstat(int fd, struct stat *buf){
    #if defined(__arm__)
                LOGE("[+] %s %d 当前的指令集是 __arm__ ", __FUNCTION__ , __LINE__);
                //return raw_syscall(__NR_fstat, fd, buf);
                return fstat(fd, buf);
    #elif defined(__arm64__) || defined(__aarch64__)
                LOGE("[+] %s %d 当前的指令集是 __aarch64__ ", __FUNCTION__ , __LINE__);
                return raw_syscall(__NR_fstat, fd, buf);
    #elif defined(_M_IX86) || defined(__i386__)
                LOGE("[+] %s %d 当前的指令集是 __i386__ ", __FUNCTION__ , __LINE__);
                return fstat(fd, buf);
    #elif defined(_M_X64) || defined(__x86_64__)
                LOGE("[+] %s %d 当前的指令集是 __x86_64__ ", __FUNCTION__ , __LINE__);
                return fstat(fd, buf);
    #endif
    }

    INLINE static int sub_stat(const char *path, struct kernel_stat *buf){
        return sys_stat(path, buf);
    }
}

INLINE static size_t sub_strlen(const char *s){
    size_t len = 0;

    while(*s++) len++;
    return len;
}

INLINE static int sub_strncmp(const char *s1, const char *s2, size_t n){
    if (n == 0)
        return (0);
    do {
        if (*s1 != *s2++){
            return (*(unsigned char *)s1 - *(unsigned char *)--s2);
        }
        if (*s1++ == 0){
            break;
        }
    } while (--n != 0);
    return (0);
}

INLINE static char* sub_strstr(const char *s, const char *find){
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = sub_strlen(find);
        do {
            do {
                if ((sc = *s++) == '\0'){
                    return (NULL);
                }
            } while (sc != c);
        } while (sub_strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

INLINE static int sub_strcmp(const char *s1, const char *s2){
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return (0);
    return (*(unsigned char *)s1 - *(unsigned char *)--s2);
}

INLINE static char *sub_strcpy(char *s1, char *s2){
    char *start = s1;

    while ((*s1++ = *s2++));
    return start;
}

// sub_strncpy 拷贝的最后一个字节自动归0
INLINE static size_t sub_strncpy(char *dst, const char *src, size_t len){
    char *d = dst;
    const char *s = src;
    size_t n = len;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }
    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (len != 0)
            *d = '\0';        /* NUL-terminate dst */
        while (*s++)
            ;
    }
    return(s - src - 1);    /* count does not include NUL */
}


#endif //CHECKROM_SYSTEM_CALL_H
