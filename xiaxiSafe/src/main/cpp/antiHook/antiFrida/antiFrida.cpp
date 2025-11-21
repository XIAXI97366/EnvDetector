//
// Created by 97366 on 2024/6/6.
//

#include "antiFrida.h"

const char *FRIDA_THREAD_GUM_JS_LOOP = "gum-js-loop";
const char *FRIDA_THREAD_GMAIN = "gmain";
const char *FRIDA_THREAD_POOL = "pool-frida";
const char *PROC_STATUS = "/proc/self/task/%s/status";
const char *PROC_TASK = "/proc/self/task";

int antiFrida::check_maps(){
    int fd = 0;
    char path[256] = {0};
    char perm[5]= {0};
    unsigned long offset = 0;
    unsigned int base = 0;
    unsigned long end = 0;
    char buffer[BUFFER_LEN] = {0};
    int ret = 0;

    fd = sub_openat(AT_FDCWD, "/proc/self/maps", O_RDONLY, 0);
    if (fd > 0) {
        while (0 < (read_line(fd, buffer, BUFFER_LEN)) ) {
            if (sscanf(buffer, "%x-%lx %4s %lx %*s %*s %s", &base, &end, perm, &offset, path) != 5) {
                continue;
            }

            if(sub_strstr(path,"frida") || sub_strstr(path,"linjector") ||
                sub_strstr(path,"gum-js-loop") || sub_strstr(path,"gmain") ||
                sub_strstr(path,"pool-frida")){
                LOGD("frida 存在与 maps 中");
                ret = 1;
                goto CHECK_LOOP_EXIT;
            }

            if(sub_strstr(path, "libc.so")){
                if (perm[0] == 'r' && perm[1] == 'w' && perm[2] == 'x' && perm[3] == 'p'){
                    LOGD("加载 libc.so 的权限不对 %s", buffer);
                    ret = 1;
                    goto CHECK_LOOP_EXIT;
                }
            }

            if (perm[0] != 'r') continue;
            if (perm[3] != 'p') continue;
            if (0 == sub_strlen(path)) continue;
            if ('[' == path[0]) continue;
            if (end - base <= 1000000) continue;
        }
    }else {
        LOGD("打开 maps 失败");
        ret = -1;
    }

CHECK_LOOP_EXIT:
    if (0 != fd){
        sub_close(fd);
        fd = 0;
    }
    return ret;
}

//检测frida线程所具有的特征（非虫的frida server无效，官方的frida server有效）
int antiFrida::detect_frida_threads() {
    DIR *dir = nullptr;
    struct dirent *entry = nullptr;
    char filePath[MAX_LENGTH] = "";
    int fd = 0;
    int ret = 0;
    char buf[MAX_LENGTH] = "";

    dir = opendir(PROC_TASK);
    if (NULL != dir) {
        entry = nullptr;
        while (NULL != (entry = readdir(dir))) {
            if (0 == sub_strcmp(entry->d_name, ".") || 0 == sub_strcmp(entry->d_name, "..")) {
                continue;
            }
            snprintf(filePath, sizeof(filePath), PROC_STATUS, entry->d_name);

            fd = sub_openat(AT_FDCWD, filePath, O_RDONLY | O_CLOEXEC, 0);
            if (0 != fd) {
                memset(buf, 0, sizeof(buf));
                read_line(fd, buf, MAX_LENGTH);
                if (sub_strstr(buf, FRIDA_THREAD_GUM_JS_LOOP) ||
                    sub_strstr(buf, FRIDA_THREAD_GMAIN) ||
                    sub_strstr(buf, FRIDA_THREAD_POOL)) {
                    LOGD("找到 frida 专用线程");
                    ret = 1;
                    goto DETECT_FRIDA_THREADS_EXIT;
                }
                sub_close(fd);
            }
        }
        closedir(dir);
    }

DETECT_FRIDA_THREADS_EXIT:
    if (0 != fd){
        sub_close(fd);
        fd = 0;
    }
    if (nullptr != entry){
        entry = nullptr;
    }
    if (nullptr != dir){
        closedir(dir);
        dir = nullptr;
    }
    return ret;
}

//安卓10以上（包括安卓10）不让访问显示权限不够
int antiFrida::check_frida_by_port2(const char *port, pid_t pid){
    std::string tcp6 = "cat /proc/net/tcp6 | grep ";
    std::string tcp = "cat /proc/net/tcp | grep ";
    char buffer[BUFFER_LEN] = {0};
    FILE *fp = NULL;
    int sdk = 0;
    int ret = 0;

    sdk = get_rom_sdk();
    if (sdk > 28){
        return 0;
    }else{
        tcp6 += port;
        fp = popen(tcp6.c_str(), "r");
        if(NULL != fp){
            while(NULL != fgets(buffer, sizeof(buffer), fp)){
                // 如果fgets到达文件末尾或者没有读取到任何字符，则buf的内容保持不变，并返回一个空指针
                // 执行到这里，判定为调试状态
                LOGD("存在 frida 的端口");
                ret = 1;
                goto CHECK_FRIDA_BY_PORT2_EXIT;
            }
            LOGD("%s", strerror(errno));
            pclose(fp);
            fp = NULL;
        }

        tcp += port;
        fp = popen(tcp.c_str(), "r");
        if(NULL != fp){
            while(NULL != fgets(buffer, sizeof(buffer), fp)){//
                // 如果fgets到达文件末尾或者没有读取到任何字符，则buf的内容保持不变，并返回一个空指针
                // 执行到这里，判定为调试状态
                LOGD("存在 frida 的端口");
                ret = 1;
                goto CHECK_FRIDA_BY_PORT2_EXIT;
            }
            LOGD("%s", strerror(errno));
            pclose(fp);
            fp = NULL;
        }
    }

CHECK_FRIDA_BY_PORT2_EXIT:
    if (nullptr != fp){
        pclose(fp);
        fp = nullptr;
    }
    return ret;
}

//检测frida常用端口
int antiFrida::check_frida_by_port(int port) {
    int sock = 0;
    struct sockaddr_in sa = {0};

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_aton("127.0.0.1", &sa.sin_addr);
    sock = sub_socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sock){
        LOGD("socket error: %s\n", strerror(errno));
        return -1;
    }else{
        if (connect(sock, (struct sockaddr *) &sa, sizeof(sa)) == 0) {
            sub_close(sock);
            LOGD("frida 服务器端口正在运行");
            return 1;
        }else{
            LOGD("connect frida port 失败");
            sub_close(sock);
            return 0;
        }
    }
}

