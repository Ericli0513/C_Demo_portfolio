#include <stdio.h>
#include "driver_test.h"

int main() {
    printf("=== Driver Test (C version) ===\n");
    init_driver();
    run_test();
    close_driver();
    return 0;
}
