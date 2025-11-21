//
// Created by 97366 on 2024/7/17.
//

#include "antiDebug.h"

static ANTIDEBUG anti = {
        guardPorcess::check_process_stopped,
        guardPorcess::scan_process_threads
};

bool antiDebug::check_process_and_threads(pid_t pid, pid_t tracerPid) {
    if (anti.guardPorcess_check_process_stopped(pid, tracerPid) ||
        anti.guardPorcess_scan_process_threads(pid, tracerPid)){
        return true;
    }else{
        return false;
    }
}


static int write_int(int fd, int v) {
    int n = write(fd, &v, sizeof(v));
    if (n != sizeof(v)){
        return -1;
    }
    return 0;
}

static int read_int(int fd) {
    int v = -1;
    if (read(fd, &v, sizeof(v)) != sizeof(v)) {
        return -1;
    }
    return v;
}

static size_t write_str(int fd, const char *s) {
    size_t len = strlen(s) + 1;
    if (write(fd, (void *)s, len) != len){
        return -1;
    }
    return len;
}

static ssize_t read_str(int fd, char *buf, size_t bufsize) {
    ssize_t n = 0;
    do {
        n = read(fd, buf, bufsize);
    } while (n == -1 && errno == EINTR);
    if (n <= 0) {
        return -1;
    }
    buf[bufsize - 1] = 0;
    return n;
}

// 附加目标进程（PTRACE_ATTACH），并接收其 SIGSTOP 信号，通过 PTRACE_CONT 让其继续运行
int attach_and_continue(pid_t pid) {
    int state = 0;

    // 使用 PTRACE_ATTACH，内核会向目标发送 SIGSTOP 令其停止
    if (-1 == ptrace(PTRACE_ATTACH, pid, 0, 0)){
        LOGE("[-] %s %d PTRACE_ATTACH fail errno = %d(%s) pid=%d", __FUNCTION__ , __LINE__, errno,
             strerror(errno), pid);
        return -1;
    }

    // 等待目标进入 ptrace 停止（收到 SIGSTOP）；__WALL 支持等待任意子/线程
    if (-1 == waitpid(pid, &state, __WALL)){
        LOGE("[-] %s %d waitpid fail errno = %d(%s) pid = %d", __FUNCTION__ , __LINE__, errno,
             strerror(errno), pid);
        return -1;
    }

    // 让目标继续运行；后续可按需使用 PTRACE_INTERRUPT 定点打断
    if (-1 == ptrace(PTRACE_CONT, pid, 0, 0)){
        LOGE("[-] %s %d PTRACE_CONT fail errno = %d(%s) pid = %d", __FUNCTION__ , __LINE__, errno,
             strerror(errno), pid);
        return -1;
    }

    // LOGE("[+] %s %d pid %d PTRACE_CONT ok", __FUNCTION__ , __LINE__, pid);
    return 0;
}

void *antiDebug::check_loop(void *arg){
    TRACE trace = *(TRACE *)arg;

    for (;;) {
        if (antiDebug::check_process_and_threads(trace.Stalked, trace.Stalker)){
            LOGE("[-] %s %d 存在被调试 ", __FUNCTION__ , __LINE__);
            break;
        }
        sleep(1);
    }

    return nullptr;
}

void *antiDebug::monitor_child_loop(void *arg) {
    int status = 0;
    pid_t dead_pid = 0;
    pid_t child1 = ((MONITOR *)arg)->child1;
    pid_t child2 = ((MONITOR *)arg)->child2;

    LOGE("[+] %s %d 开始监视 child1 -> %d child2 -> %d", __FUNCTION__ , __LINE__, child1, child2);
    while (true) {
        // 使用WNOHANG非阻塞等待任一子进程
        dead_pid = waitpid(-1, &status, WNOHANG);
        if (dead_pid > 0) {
            // 有子进程退出了
            if (dead_pid == child1) {
                LOGE("[-] %s %d child1 has died!", __FUNCTION__ , __LINE__);
            } else if (dead_pid == child2) {
                LOGE("[-] %s %d child2 has died!", __FUNCTION__ , __LINE__);
            } else {
                LOGE("[-] %s %d Unknown child has died (PID %d) !", __FUNCTION__ , __LINE__, dead_pid);
            }

            // 父进程自杀
            LOGE("[-] %s %d arent process will kill itself now", __FUNCTION__ , __LINE__);
            sleep(1);  // 给日志输出一点时间
            kill(getpid(), SIGKILL);
            break;
        } else if (dead_pid == -1 && errno != ECHILD) {
            // waitpid 出错但不是"没有子进程"错误
            LOGE("[-] %s %d waitpid error: %s", __FUNCTION__ , __LINE__, strerror(errno));
            sleep(1);
            continue;
        }

        // 没有子进程退出，短暂休眠后继续检查
        sleep(1);
    }

    return nullptr;
}

void *antiDebug::monitor_parent_loop(void *arg) {
    pid_t ori_ppid = getppid();
    pid_t cur_ppid = 0;
    LOGE("[+] %s %d pid -> %d 开始监视 ppid -> %d", __FUNCTION__ , __LINE__, getpid(), getppid());

    while (true) {
        cur_ppid = getppid();
        if (cur_ppid == 1) {
            // 父进程已死，被init进程(PID=1)接管
            LOGE("[-] %s %d kill child pid -> %d", __FUNCTION__ , __LINE__, getpid());
            sleep(1);  // 给日志输出一点时间
            kill(getpid(), SIGKILL);
            break;
        } else if (cur_ppid != ori_ppid) {
            // 父进程PID发生了非预期的变化
            LOGE("[-] %s %d Parent PID changed ori_ppid -> %d cur_ppid -> %d", __FUNCTION__ , __LINE__,
                 ori_ppid, cur_ppid);
            sleep(1);
            kill(getpid(), SIGKILL);
            break;
        }

        // 父进程仍然存在，继续监控
        sleep(1);  // 1秒检查一次
    }

    return nullptr;
}

int antiDebug::start_guards(){
    int p2c1[2] = {0};
    int p2c2[2] = {0};
    int c12p[2] = {0};
    int c22p[2] = {0};
    pid_t child1 = 0;
    pid_t child2 = 0;
    char ack[32] = {0};
    pthread_t stalker_parent;
    TRACE trace_parent;
    pthread_t monitor_child;
    MONITOR monitor;

    // [0] → 读端 (read end) [1] → 写端 (write end)
    if(-1 == pipe(p2c1) || -1 == pipe(p2c2) || -1 == pipe(c12p) || -1 == pipe(c22p)){
        LOGE("[-] %s %d pipe faild ", __FUNCTION__ , __LINE__);
        return 0;
    }

    child1 = fork();
    if(0 == child1){    // child1子进程执行流程
        char buf[36] = {0};
        pid_t ppid = 0;
        pid_t tid = 0;
        int status = 0;
        int sig = 0;
        pthread_t stalker_child1;
        TRACE trace_child1;
        pthread_t monitor_parent;

        if (-1 == read_str(p2c1[0], buf, sizeof(buf))){
            LOGE("[-] %s %d child1 read_str parent failed ", __FUNCTION__ , __LINE__);
            return 0;
        }
        // LOGE("[+] %s %d child1 in parent read_str %s successful ", __FUNCTION__ , __LINE__, buf);

        // child1 对 parent 进行附加
        ppid = getppid();
        if (-1 == attach_and_continue(ppid)){
            LOGE("[-] %s %d child1 attach_and_continue parent failed ", __FUNCTION__ , __LINE__);
            return 0;
        }

        // 向父进程的管道中输出 C1_ATTACHED_OK
        if (-1 == write_str(c12p[1], "C1_ATTACHED_OK")){
            LOGE("[-] %s %d child1 2 parent write_str C1_ATTACHED_OK failed ", __FUNCTION__ , __LINE__);
            return 0;
        }
        // LOGE("[+] %s %d child1 2 parent write_str C1_ATTACHED_OK successful ", __FUNCTION__ , __LINE__);

        // 创建监视线程
        pthread_create(&monitor_parent, nullptr, monitor_parent_loop, nullptr);
        pthread_detach(monitor_parent);

         // 创建用于检测 parent 的 task 和 status 文件，传递的参数为parent的pid和child1的pid
         trace_child1.Stalked = getppid();
         trace_child1.Stalker = getpid();
         pthread_create(&stalker_child1, nullptr, check_loop, (void *)&trace_child1);
         pthread_detach(stalker_child1);

        for (;;) {
            tid = waitpid(-1, &status, __WALL); // 返回具体被停住的 tid
            if (tid <= 0) {
                if (errno == EINTR) {
                    LOGE("[-] %s %d tid <= 0 errno = %d(%s) continue", __FUNCTION__ , __LINE__, errno, strerror(errno));
                    continue;
                }
                LOGE("[-] %s %d tid <= 0 errno = %d(%s) break", __FUNCTION__ , __LINE__, errno, strerror(errno));
                break;
            }
            if (WIFSTOPPED(status)) {
                sig = WSTOPSIG(status); // 可根据需要选择传递或屏蔽
                ptrace(PTRACE_CONT, tid, 0, sig); // 让该线程继续
            }
        }
    }

    /************ parent -> child1 ************/
    // 通知 child1 可占坑 parent
    if (-1 == write_str(p2c1[1], "GO1")){
        LOGE("[-] %s %d parent write_str child1 GO1 failed ", __FUNCTION__ , __LINE__);
        return 0;
    }
    // LOGE("[+] %s %d parent 2 child1 write_str GO1 successful ", __FUNCTION__ , __LINE__);

    // 等 child1 附加到 parent，接收 child1 向管道传递过来的 "C1_ATTACHED_OK" 字符串
    if (-1 == read_str(c12p[0], ack, sizeof(ack))){
        LOGE("[-] %s %d parent in child1 read_str failed ", __FUNCTION__ , __LINE__);
        return 0;
    }
    // LOGE("[+] %s %d parent in child1 read_str %s successful ", __FUNCTION__ , __LINE__, ack);
    /************ parent -> child1 ************/

    child2 = fork();
    if (0 == child2){   // child2子进程执行流程
        pid_t child1 = 0;
        pid_t tid = 0;
        int status = 0;
        int sig = 0;
        pthread_t stalker_child2;
        TRACE trace_child2;
        pthread_t monitor_parent;

        // 等待 parent 下发 child1 的 pid 后再开始 attach child1
        child1 = read_int(p2c2[0]);
        if (-1 == child1){
            LOGE("[-] %s %d child2 in parent read_int child1 pid failed ", __FUNCTION__ , __LINE__);
            return 0;
        }
        // LOGE("[+] %s %d child2 in parent read_int child1 pid %d ", __FUNCTION__ , __LINE__, child1);

        // child2 对 child1 进行附加
        if (-1 == attach_and_continue(child1)){
            LOGE("[-] %s %d child2 attach_and_continue child1 failed ", __FUNCTION__ , __LINE__);
            return 0;
        }

        // 向管道中输出 C2_ATTACHED_OK
        if (-1 == write_str(c22p[1], "C2_ATTACHED_OK")){
            LOGE("[-] %s %d child1 2 parent write_str C2_ATTACHED_OK failed ", __FUNCTION__ , __LINE__);
            return 0;
        }
        // LOGE("[+] %s %d child1 2 parent write_str C2_ATTACHED_OK successful ", __FUNCTION__ , __LINE__);

        // 创建监视线程
        pthread_create(&monitor_parent, nullptr, monitor_parent_loop, nullptr);
        pthread_detach(monitor_parent);

        // 创建用于检测 child1 的 task 和 status 文件，传递的参数为child1的pid和child2的pid
        trace_child2.Stalked = child1;
        trace_child2.Stalker = getpid();
        pthread_create(&stalker_child2, nullptr, check_loop, (void *)&trace_child2);
        pthread_detach(stalker_child2);

        for (;;) {
            tid = waitpid(-1, &status, __WALL); // 返回具体被停住的 tid
            if (tid <= 0) {
                if (errno == EINTR) {
                    LOGE("[-] %s %d tid <= 0 errno = %d(%s) continue", __FUNCTION__ , __LINE__, errno, strerror(errno));
                    continue;
                }
                LOGE("[-] %s %d tid <= 0 errno = %d(%s) break", __FUNCTION__ , __LINE__, errno, strerror(errno));
                break;
            }
            if (WIFSTOPPED(status)) {
                sig = WSTOPSIG(status); // 可根据需要选择传递或屏蔽
                ptrace(PTRACE_CONT, tid, 0, sig); // 让该线程继续
            }
        }
    }

    /************ parent -> child2 ************/
    // 通知 child2 可占坑 child1
    if (-1 == write_int(p2c2[1], child1)){
        LOGE("[-] %s %d parent 2 child2 write_int child1 pid failed ", __FUNCTION__ , __LINE__);
        return 0;
    }
    // LOGE("[+] %s %d parent 2 child2 write_int child1 pid %d ", __FUNCTION__ , __LINE__, child1);

    // 等 child2 附加到 child2，接收 child2 向管道传递过来的 "C2_ATTACHED_OK" 字符串
    if (-1 == read_str(c22p[0], ack, sizeof(ack))){
        LOGE("[-] %s %d parent in child2 read_str failed ", __FUNCTION__ , __LINE__);
        return 0;
    }
    // LOGE("[+] %s %d parent in child2 read_str %s successful ", __FUNCTION__ , __LINE__, ack);
    /************ parent -> child2 ************/

    // 创建监视线程
    monitor.child1 = child1;
    monitor.child2 = child2;
    pthread_create(&monitor_child, nullptr, monitor_child_loop, (void *)&monitor);
    pthread_detach(monitor_child);

    // 创建用于检测 child2 的 task 和 status 文件，传递的参数为child2的pid和parent的pid
    trace_parent.Stalked = child2;
    trace_parent.Stalker = getpid();
    pthread_create(&stalker_parent, nullptr, check_loop, (void *)&trace_parent);
    pthread_detach(stalker_parent);

    /************ parent ************/
    // 检查 child1 和 child2 是否存活，如长时间未收到子进程心跳包，则父进程退出
    for (;;) {
        // LOGE("[+] %s %d parent is run ", __FUNCTION__ , __LINE__);
        sleep(1);
    }
    /************ parent ************/
}