//
// Created by 97366 on 2024/10/21.
//

#include "checkMaps.h"

checkMaps::~checkMaps() {
    p_map_seg_info tmp = nullptr;
    p_map_seg_info node = nullptr;

    if (nullptr != phdr){
        tmp = phdr->next;
        while (true){
            if (nullptr == tmp){
                break;
            }
            node = tmp;
            tmp = tmp->next;
            free(node);
        }
    }
    free(phdr);
    phdr = nullptr;

    if (0 != mapfd){
        sub_close(mapfd);
    }

    if (0 != basefd){
        sub_close(basefd);
    }
}

p_map_seg_info checkMaps::get_map_seg_info() {
    unsigned long base = 0;
    unsigned long end = 0;
    char perm[5]= {0};
    unsigned long inode = 0;
    unsigned long offset = 0;
    char path[MAX_LENGTH] = {0};
    char buffer[512] = {0};
    p_map_seg_info tmp = nullptr;
    p_map_seg_info node = nullptr;
    char fdPath[MAX_LENGTH] = {0};

    if (nullptr == phdr){
        // 用于处理全局类对象的构造函数的优先级比 __attribute__((constructor)) 初始化函数慢的情况（故删除了类的的初始化函数）
        phdr = (p_map_seg_info)malloc(sizeof(map_seg_info));
        memset(phdr, 0, sizeof(map_seg_info));
    }

    if (nullptr != phdr->next){
        return phdr;
    }

    sprintf(fdPath, "/proc/%d/maps", getpid());
    mapfd = sub_openat(AT_FDCWD, fdPath, O_RDONLY, 0);
    if (mapfd > 0) {
        while (0 < (read_line(mapfd, buffer, BUFFER_LEN)) ) {
            if (sscanf(buffer, "%lx-%lx %4s %lx %*s %ld %255[^\n]", &base, &end, perm, &offset, &inode, path) != 0) {
                // LOGE("base:%lx-end:%lx perm:%4s offset:%lx inode:%ld path:%s", base, end, perm, offset, inode, path);
                tmp = (p_map_seg_info)malloc(sizeof(map_seg_info));
                memset(tmp, 0, sizeof(map_seg_info));
                tmp->start = base;
                tmp->end = end;
                tmp->inode = inode;
                memcpy(tmp->name, path, MAX_LENGTH);
                memcpy(tmp->property, perm, 4);
                memset(path, 0, MAX_LENGTH);
                tmp->next = nullptr;
                if (nullptr == phdr->next){
                    phdr->next = tmp;
                }else{
                    node = phdr->next;
                    while (nullptr != node->next){
                        node = node->next;
                    }
                    node->next = tmp;
                }
            }else{
                continue;
            }
        }
        LOGE("[+] save maps info successfull");
        return phdr;
    }else {
        LOGE("[-] sub_openat map error %s, %d", __FUNCTION__, __LINE__);
        return nullptr;
    }

    return nullptr;
}

// 根据jit内存段的属性判断当前进程是否被zygote模块注入（该方案用于针对 Shamiko 前期方案）
bool checkMaps::is_zygote_injected() {
    const char *jit_zygote_cache = "/memfd:jit-zygote-cache (deleted)";
    const char *jit_cache = "/memfd:jit-cache (deleted)";
    p_map_seg_info tmp = phdr;
    unsigned long zygote_private_inode = -1;
    unsigned long zygote_shared_inode = -1;
    unsigned long jit_private_inode = -1;
    unsigned long jit_shared_inode = -1;
    unsigned long zygote_private_exc_seg_num = 0;
    unsigned long zygote_shared_exc_seg_num = 0;
    unsigned long jit_private_exc_seg_num = 0;
    unsigned long jit_shared_ecx_seg_num = 0;

    if (nullptr != phdr){
        while (true){
            if (nullptr == tmp->next){
                break;
            }

            if (0 != sub_strlen(tmp->name)){
                if (0 == sub_strncmp(jit_zygote_cache, tmp->name, sub_strlen(jit_zygote_cache))){
                    if('s' == tmp->property[3]){
                        if (0 == zygote_shared_inode){
                            // jit_zygote_cache 的 inode 不为 0
                            LOGE("[-] zygote shared inode is 0 ");
                            return true;
                        }else{
                            if (-1 == zygote_shared_inode){
                                zygote_shared_inode = tmp->inode;
                            }else{
                                if (zygote_shared_inode != tmp->inode){
                                    // 如果与之前的共享区 inode 不同则说明被zygote注入
                                    LOGE("[-] zygote shared inode have plural ");
                                    return true;
                                }
                            }
                        }

                        // 记录共享属性的内存段中的可执行段的个数，但如果超过一个可执行段则执行被判定为被注入
                        if ('x' == tmp->property[2]){
                            zygote_shared_exc_seg_num++;
                        }

                        if (1 < zygote_shared_exc_seg_num){
                            LOGE("[-] zygote shared exc mem count > 1 ");
                            return true;
                        }
                    }else if('p' == tmp->property[3]){
                        if (0 == zygote_private_inode){
                            // jit_zygote_cache 的 inode 不为 0
                            LOGE("[-] zygote private inode is 0 ");
                            return true;
                        }else{
                            if (-1 == zygote_private_inode){
                                zygote_private_inode = tmp->inode;
                            }else{
                                if (zygote_private_inode != tmp->inode){
                                    // 如果与之前的私有区 inode 不同则说明被zygote注入
                                    LOGE("[-] zygote_private_inode->%d,  tmp->inode->%d", zygote_private_inode, tmp->inode);
                                    LOGE("[-] zygote private inode have plural ");
                                    return true;
                                }
                            }
                        }

                        if ('x' == tmp->property[2]){
                            zygote_private_exc_seg_num++;
                        }

                        if (1 < zygote_private_exc_seg_num){
                            LOGE("[-] zygote private exc mem count > 1 ");
                            return true;
                        }
                    }
                }

                if(0 == sub_strncmp(jit_cache, tmp->name, sub_strlen(jit_cache))){
                    if('s' == tmp->property[3]){
                        if (0 == jit_shared_inode){
                            // jit_shared_inode 的 inode 不为 0
                            LOGE("[-] jit shared inode is 0 ");
                            return true;
                        }else{
                            if (-1 == jit_shared_inode){
                                jit_shared_inode = tmp->inode;
                            }else{
                                if (jit_shared_inode != tmp->inode){
                                    // 如果与之前的共享区 inode 不同则说明被zygote注入
                                    LOGE("[-] jit_shared_inode->%d,  tmp->inode->%d", jit_shared_inode, tmp->inode);
                                    LOGE("[-] jit shared inode have plural ");
                                    return true;
                                }
                            }
                        }

                        if ('x' == tmp->property[2]){
                            jit_shared_ecx_seg_num++;
                        }

                        if (1 < jit_shared_ecx_seg_num){
                            LOGE("[-] jit shared ecx mem > 1 ");
                            return true;
                        }
                    }else if('p' == tmp->property[3]){
                        if (0 == jit_private_inode){
                            // jit_private_inode 的 inode 不为 0
                            LOGE("[-] jit private inode is 0 ");
                            return true;
                        }else{
                            if (-1 == jit_private_inode){
                                jit_private_inode = tmp->inode;
                            }else{
                                if (jit_private_inode != tmp->inode){
                                    // 如果与之前的私有区 inode 不同则说明被zygote注入
                                    LOGE("[-] jit private inode have plural ");
                                    return true;
                                }
                            }
                        }

                        if ('x' == tmp->property[2]){
                            jit_private_exc_seg_num++;
                        }

                        if (1 < jit_private_exc_seg_num){
                            LOGE("[-] jit private exc mem count > 1 ");
                            return true;
                        }
                    }
                }
            }
            tmp = tmp->next;
        }
    }
    LOGE("[+] no zygote inject ");
    return false;
}

// 该检测用于检查扫描 maps 中所有可执行内存，如果路径既不是以 / 开头，也不是 [vdso]，或者路径以 /dev/zero 开头，则认为存在注入
// 剩余的项如果 maps 中的 inode 和 stat 对应路径的 inode 不一致，或常规路径检查发现端倪，则认为存在注入
bool checkMaps::is_map_segment_compliance() {
    const char *shared_mem_flag = "/dev/zero";
    const char *rename_flag = "[anon:name]";
    const char *vdso_flag = "[vdso]";
    p_map_seg_info tmp = phdr;
    struct kernel_stat buf = {0};

    if (nullptr == phdr){
        return false;
    }

    while(true){
        if (nullptr == tmp->next){
            break;
        }

        // 扫描是否具有匿名内存且存在可执行属性的情况（无内存名称和 inode 为0）
        if ('x' == tmp->property[2] && (0 == sub_strlen(tmp->name)) && 0 == tmp->inode){
            LOGE("[-] 扫描到具有可执行属性的匿名内存 start->%p end->%p 大小->%p",
                 tmp->start, tmp->end, tmp->end - tmp->start);
            return true;
        }
        // 扫描 maps 中所有可执行内存 要是路径以 / 或者 [vdso] 且不是以 /dev/zero 和 [anon:name] 开头 则不认为存在注入
        // 扫描 maps 中所有可执行内存，如果路径既不是以 / 开头，也不是 [vdso]，或者路径以 /dev/zero 和 [anon:name] 开头，则认为存在注入
        if ('x' == tmp->property[2] && ('/' != tmp->name[0])){
            // 排除掉是内存名称是 [vdso] 的情况
            if(0 != sub_strncmp(vdso_flag, tmp->name,sub_strlen(tmp->name))){
                LOGE("[-] 扫描到非法路径 ==> %s start->%p end->%p 大小->%p"
                     , tmp->name[0], tmp->start, tmp->end, tmp->end - tmp->start);
                return true;
            }
        }

        if ('x' == tmp->property[2] && (
                (nullptr != sub_strstr(shared_mem_flag, tmp->name)) ||
                (nullptr != sub_strstr(rename_flag, tmp->name))
                )){
            LOGE("[-] 扫描到具有共享和可执行属性的匿名内存或[anon:name]的存在 start->%p end->%p 大小->%p",
                 tmp->start, tmp->end, tmp->end - tmp->start);
            return true;
        }

        // 剩余的项如果 maps 中的 inode 和 stat 对应路径的 inode 不一致，或常规路径检查发现端倪，则认为存在注入
        if(0 == sub_stat(tmp->name, &buf)){
            if ('x' == tmp->property[2] && (tmp->inode != buf.st_ino)){
                LOGE("[-] inode对比不上 ");
                return true;
            }
        }

        tmp = tmp->next;
    }
    return false;
}

// 通过 fd 反查是否伪造了 maps 文件
bool checkMaps::check_maps_valid() {
    std::string fdPath("/proc/");
    size_t len = 0;
    char mapPath[MAX_LENGTH] = {0};
    int dstFd = 0;
    char dstPath[MAX_LENGTH] = {0};
    char realPath[MAX_LENGTH] = {0};

    if (mapfd <= 0){
        LOGE("[-] mapfd is <= 0 %s, %d", __FUNCTION__, __LINE__);
        return false;
    }

    // 攻击者 是从 Hook open 函数作为起点，修改了 maps 文件的路径（如：在私有目录（/data/data/package）创建了一个新的 maps 文件）
    dstFd = open("/proc/self/maps", O_RDONLY);
    if (dstFd > 0){
        fdPath.append(std::to_string(getpid())).append("/fd/").append(std::to_string(mapfd));
        sub_readlinkat(AT_FDCWD, fdPath.c_str(), mapPath, MAX_LENGTH);
        LOGE("[+] dstFd -> %d mapfd -> %d ", dstFd , mapfd);
        snprintf(dstPath, sizeof(dstPath), "/proc/self/fd/%d", dstFd);
        len = sub_readlinkat(AT_FDCWD, dstPath, realPath, MAX_LENGTH);

        if ((len > 0) && (0 == sub_strncmp(mapPath, realPath, sub_strlen(realPath)))){
            LOGE("[+] %s %d maps path is meeting expectations ", __FUNCTION__ , __LINE__);
            LOGE("[+] mapPath -> %s realPath -> %s", mapPath , realPath);
            return false;
        }else{
            LOGE("[-] %s %d mapPath -> %s realPath -> %s", mapPath , realPath);
            LOGE("[-] %s %d maps path is not meeting expectations ", __FUNCTION__ , __LINE__);
            return true;
        }
    }else{
        LOGE("[-] open map error %s, %d", __FUNCTION__, __LINE__);
        return false;
    }
}

// 获取 base.apk 的 fd 并给 basePath 进行赋值
bool checkMaps::get_base_fd(){
    char path[MAX_LENGTH] = {0};

    if (nullptr == phdr){
        LOGE("[-] %s %d path size ==> 0 phdr == nullptr ", __FUNCTION__ , __LINE__);
        return false;
    }

    // 调用 get_map_segment_path 方法中给 basePath 成员赋值
    get_map_segment_path("/base.apk", path, sizeof(path));
    if (0 == sub_strlen(path)){
        LOGE("[-] %s %d path size ==> 0 ", __FUNCTION__ , __LINE__);
        return false;
    }

    basefd = sub_openat(AT_FDCWD, path, O_RDONLY, 0);
    if (basefd <= 0){
        LOGE("[-] %s %d basefd error is => %s ", __FUNCTION__ , __LINE__, strerror(errno));
        return false;
    }
    return true;
}

void checkMaps::get_map_segment_path(const char *key, char *buf, int size){
    p_map_seg_info tmp = nullptr;

    if (nullptr == phdr){
        memset(buf, 0, size);
        return;
    }

    tmp = phdr;
    while (true){
        if (nullptr == tmp->next){
            break;
        }
        if (0 != sub_strlen(tmp->name) && (nullptr != sub_strstr(tmp->name, key)
            && (0 != tmp->inode))){
            sub_strncpy(buf, tmp->name, sub_strlen(tmp->name) + 1);
            if (0 != sub_strlen(basePath)){
                memset(basePath, 0, MAX_LENGTH);
            }
            sub_strncpy(basePath, tmp->name, sub_strlen(tmp->name) + 1);
            inode = tmp->inode;
            break;
        }
        tmp = tmp->next;
    }
}

bool checkMaps::check_map_injected(){
}

