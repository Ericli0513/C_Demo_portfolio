#define _GNU_SOURCE
#include "sys_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#define DEVICE_PATH "/dev/null"   // 可改成 /dev/i2c-1 或其他設備
#define MAX_RETRY 3

static int log_fd = -1;
static volatile sig_atomic_t running = 1;
static pthread_mutex_t log_mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_t logger_thr;

/* ---------------- Utility ---------------- */
static void ts_now(char *buf, size_t n) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", &tm);
}

static void log_line(const char *level, const char *msg) {
    char ts[32];
    ts_now(ts, sizeof(ts));
    pthread_mutex_lock(&log_mu);
    dprintf(log_fd, "[%s] [%s] %s\n", ts, level, msg);
    fsync(log_fd);
    pthread_mutex_unlock(&log_mu);
}

/* ---------------- System Services ---------------- */
static void on_signal(int sig) {
    running = 0;
    char buf[64];
    snprintf(buf, sizeof(buf), "Received signal %d, requesting shutdown", sig);
    log_line("SIGNAL", buf);
}

static void* logger_thread(void *arg) {
    (void)arg;
    while (running) {
        log_line("HEARTBEAT", "logger alive");
        usleep(200 * 1000);
    }
    log_line("INFO", "logger exiting");
    return NULL;
}

static int check_device() {
    for (int i = 0; i < MAX_RETRY; i++) {
        if (access(DEVICE_PATH, F_OK) == 0) {
            log_line("INFO", "Device node found, proceeding with real test");
            return 0; // success
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "Device not found, retry %d/%d", i+1, MAX_RETRY);
            log_line("WARN", buf);
            sleep(1);
        }
    }
    log_line("ERROR", "Device not available, switching to fallback mode");
    return -1; // failure
}

/* ---------------- Public API ---------------- */
int sys_init(void) {
    // 1. 註冊 signal handler
    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGTERM, &sa, NULL) < 0) {
        fprintf(stderr, "sigaction failed: %s\n", strerror(errno));
        return -1;
    }

    // 2. 開啟 log
    log_fd = open("logs.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd < 0) {
        fprintf(stderr, "open logs.txt failed: %s\n", strerror(errno));
        return -1;
    }

    // 3. 檢查 device
    if (check_device() != 0) {
        log_line("SIMULATION", "Running in fallback mode (no device)");
    }

    // 4. 啟動 logger thread
    if (pthread_create(&logger_thr, NULL, logger_thread, NULL) != 0) {
        fprintf(stderr, "pthread_create failed\n");
        return -1;
    }

    log_line("INFO", "system init complete");
    return 0;
}

int sys_run(void) {
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        log_line("ERROR", "pipe creation failed");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        log_line("ERROR", "fork failed");
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // child
        close(pipefd[1]);
        char cmd[64] = {0};
        ssize_t n = read(pipefd[0], cmd, sizeof(cmd)-1);
        if (n <= 0) _exit(127);

        if (dup2(log_fd, STDOUT_FILENO) < 0) _exit(127);

        execlp(cmd, cmd, "-a", (char*)NULL);
        dprintf(STDOUT_FILENO, "[EXEC-ERROR] %s\n", strerror(errno));
        _exit(127);
    } else {
        // parent
        close(pipefd[0]);
        const char *command = "uname";
        if (write(pipefd[1], command, strlen(command)) < 0) {
            log_line("ERROR", "write to pipe failed");
        }
        close(pipefd[1]);

        log_line("INFO", "parent waiting for child");
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            log_line("ERROR", "waitpid failed");
            return -1;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            log_line("INFO", "child exec success");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "child exit status=%d", status);
            log_line("WARN", buf);
        }
    }
    return 0;
}

void sys_shutdown(void) {
    running = 0;
    log_line("INFO", "system shutdown");
    pthread_join(logger_thr, NULL);
    if (log_fd >= 0) {
        close(log_fd);
        log_fd = -1;
    }
}
