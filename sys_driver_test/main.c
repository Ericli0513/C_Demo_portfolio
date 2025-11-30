#include <stdio.h>
#include "sys_test.h"

int main(void) {
    puts("=== System Driver Test (C) ===");
    if (sys_init() != 0) {
        fprintf(stderr, "init failed\n");
        return 1;
    }
    if (sys_run() != 0) {
        fprintf(stderr, "run failed\n");
    }
    sys_shutdown();
    return 0;
}
