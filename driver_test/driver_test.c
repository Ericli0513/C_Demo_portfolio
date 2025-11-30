#include <stdio.h>
#include <time.h>
#include "driver_test.h"

static FILE *log_file;

void init_driver() {
    log_file = fopen("logs.txt", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    fprintf(log_file, "[INFO] Driver initialized\n");
    printf("Driver initialized.\n");
}

void run_test() {
    if (log_file == NULL) return;
    time_t now = time(NULL);
    fprintf(log_file, "[TEST] Running driver test at %s", ctime(&now));
    printf("Driver test executed.\n");
}

void close_driver() {
    if (log_file != NULL) {
        fprintf(log_file, "[INFO] Driver closed\n");
        fclose(log_file);
    }
    printf("Driver closed.\n");
}
