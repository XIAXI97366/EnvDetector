//
// Created by 97366 on 2024/10/4.
//

#include "memCrc.h"

static unsigned long check_sum(void *buffer, size_t len) {
    unsigned long seed = 0;
    auto *buf = (uint8_t *) buffer;

    if (buffer == nullptr || len < 100) {
        return 0;
    }

    for (size_t i = 0; i < len; ++i) {
        uint8_t *ptr = buf++;
        seed += (unsigned long) (*ptr);
    }
    return seed;
}

bool get_file_exec_section_info(const char *filePath, execSection *obj) {
    ElfW(Ehdr) hdr = {0};
    ElfW(Shdr) sectHdr = {0};
    int fd = 0;
    unsigned long dataSize[2] = {0};
    unsigned long offset[2] = {0};
    unsigned long addr[2] = {0};
    int execSectionCount = 0;

    if (nullptr == obj){
        LOGE("[-] get_file_exec_section_info %s %d execSection obj is nullptr", __FUNCTION__, __LINE__);
        return false;
    }

    fd = sub_openat(AT_FDCWD, filePath, O_RDONLY, NULL);
    if (fd == -1) {
        LOGE("[-] get_file_exec_section_info %s %d sub_openat Error", __FUNCTION__, __LINE__);
        return false;
    }
    sub_read(fd, &hdr, sizeof(ElfW(Ehdr)));
    sub_lseek(fd, (off_t) hdr.e_shoff, SEEK_SET);

    for (int i = 0; i < hdr.e_shnum; i++) {
        memset(&sectHdr, 0, sizeof(ElfW(Shdr)));
        sub_read(fd, &sectHdr, sizeof(ElfW(Shdr)));
        //通常 .plt 和 .text 一般都是可执行段，一般也就计算这两个段
        if (sectHdr.sh_flags & SHF_EXECINSTR) {
            offset[execSectionCount] = sectHdr.sh_offset;
            addr[execSectionCount] = sectHdr.sh_addr;
            dataSize[execSectionCount] = sectHdr.sh_size;
            execSectionCount++;
            if (execSectionCount == 2) {
                obj->execSectionCount = execSectionCount;
                break;
            }
        }
    }
    if (execSectionCount == 0) {
        LOGE("[-] get_file_exec_section_info %s %d execSectionCount is zero", __FUNCTION__, __LINE__);
        sub_close(fd);
        return false;
    }

    for (int i = 0; i < execSectionCount; i++) {
        sub_lseek(fd, (off_t) offset[i], SEEK_SET);
        auto buffer = (void *) calloc(1, dataSize[i] * sizeof(uint8_t));
        if (buffer == nullptr) {
            LOGE("[-] get_file_exec_section_info %s %d calloc error", __FUNCTION__, __LINE__);
            sub_close(fd);
            return false;
        }
        sub_read(fd, buffer, dataSize[i]);
        obj->offset[i] = offset[i];
        obj->addr[i] = addr[i];
        obj->dataSize[i] = dataSize[i];
        obj->checkSum[i] = check_sum(buffer, dataSize[i]);
        free(buffer);
    }
    sub_close(fd);
    return true;
}

bool contrast_proc_executable_segment(
        char *mapItem,
        execSection *obj,
        const char *soName) {
    unsigned long start, end = 0;
    char buf[MAX_LENGTH] = {0};
    char path[MAX_LENGTH] = {0};
    char tmp[100] = {0};
    uint8_t *buffer = nullptr;

    sscanf(mapItem, "%lx-%lx %s %s %s %s %s", &start, &end, buf, tmp, tmp, tmp, path);
    if ('r' == buf[0] && 'x' == buf[2]) {
        buffer = (uint8_t *) start;
        for (int i = 0; i < obj->execSectionCount; i++) {
            if (start + obj->addr[i] + obj->dataSize[i] > end) {
                if (0 != obj->memLibBase) {
                    buffer = (uint8_t *) obj->memLibBase;
                    obj->memLibBase = 0;
                    break;
                }
            }
        }
        for (int i = 0; i < obj->execSectionCount; i++) {
            LOGI("%s [%p] size ->[%lu]", soName, (void *) ((unsigned long)buffer + obj->addr[i]),
                 obj->dataSize[i]);
            unsigned long result = check_sum((void *) ((unsigned long)buffer + obj->addr[i]),
                                             obj->dataSize[i]);
            LOGI("%s checksum:[%ld][%ld]", soName, obj->checkSum[i], result);
            if (result != obj->checkSum[i]) {
                return true;
            }
        }
    }else {
        if ('r' == buf[0] && *(unsigned int*)start == 0x464C457F) {
            obj->memLibBase = start;
        }
    }
    return false;
}

bool check_exec_section_for_sum(const char *soPath, execSection *obj) {
    char map[MAX_LENGTH] = {0};
    const char *mapsPath = nullptr;
    int fd = 0;

    if (nullptr == obj) {
        LOGE("[-] check_exec_section_for_sum %s %d execSection obj is nullptr", __FUNCTION__, __LINE__);
        return false;
    }

    mapsPath = std::string("proc/").append(std::to_string(getpid())).append("/maps").c_str();
    fd = sub_openat(AT_FDCWD, mapsPath, O_RDONLY, 0);
    if (fd <= 0) {
        LOGE("[-] check_exec_section_for_sum %s %d sub_openat is erroe", __FUNCTION__, __LINE__);
        return false;
    }

    while ((read_line(fd, map, MAX_LENGTH)) > 0) {
        if (sub_strstr(map, soPath) != nullptr) {
            if (true == contrast_proc_executable_segment(map, obj, soPath)) {
                sub_close(fd);
                return true;
            }
        }
    }

    sub_close(fd);
    return false;
}

int memCrc::chekc_sum_for_customlib(const char *libPath) {
    execSection libSection = {0};

    if (false == get_file_exec_section_info(libPath, &libSection)){
        return -1;
    }
    return check_exec_section_for_sum(libPath, &libSection);
}


int memCrc::check_sum_for_libart() {
    execSection libartSection = {0};

    if (false == get_file_exec_section_info(get_libart_path(), &libartSection)){
        return -1;
    }
    return check_exec_section_for_sum(get_libart_path(), &libartSection);
}

int memCrc::check_sum_for_libc() {
    execSection libcSection = {0};

    if (false == get_file_exec_section_info(get_libc_path(), &libcSection)){
        return -1;
    }
    return check_exec_section_for_sum(get_libc_path(), &libcSection);
}

int memCrc::check_sum_for_linker() {
    execSection linkerSection = {0};

    if (false == get_file_exec_section_info(get_linker_path(), &linkerSection)){
        return -1;
    }
    return check_exec_section_for_sum(get_linker_path(), &linkerSection);
}
