#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declarations for functions in lib.c
void hot_function(int iterations);
void medium_function(int iterations);
void cold_function(void);
void rarely_called(void);
void path_a(void);
void path_b(void);
void mixed_workload(int mode);

// Local functions
static void local_hot_loop(int count) {
    for (int i = 0; i < count; i++) {
        // Some work
        volatile int x = i * 2;
        (void)x;
    }
}

static void local_conditional(int value) {
    if (value > 100) {
        printf("High value: %d\n", value);
    } else if (value > 50) {
        printf("Medium value: %d\n", value);
    } else {
        printf("Low value: %d\n", value);
    }
}

static void recursive_function(int depth) {
    if (depth <= 0) return;
    volatile int x = depth * 3;
    (void)x;
    recursive_function(depth - 1);
}

void process_mode_1(void) {
    printf("Mode 1: Hot execution path\n");
    // Execute hot paths many times
    for (int i = 0; i < 10000; i++) {
        hot_function(10);
        local_hot_loop(100);
        if (i % 100 == 0) {
            medium_function(5);
        }
    }
    path_a();
}

void process_mode_2(void) {
    printf("Mode 2: Balanced execution\n");
    // More balanced execution
    for (int i = 0; i < 5000; i++) {
        hot_function(5);
        medium_function(10);
        if (i % 500 == 0) {
            cold_function();
        }
        local_conditional(i);
    }
    path_b();
    rarely_called();
}

void process_mode_3(void) {
    printf("Mode 3: Cold execution path\n");
    // Mostly cold paths
    for (int i = 0; i < 1000; i++) {
        if (i % 100 == 0) {
            medium_function(2);
        }
        if (i % 200 == 0) {
            hot_function(1);
        }
    }
    cold_function();
    rarely_called();
    recursive_function(5);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            process_mode_1();
            break;
        case 2:
            process_mode_2();
            break;
        case 3:
            process_mode_3();
            break;
        default:
            printf("Unknown mode, using mode 1\n");
            process_mode_1();
            break;
    }
    
    // Call mixed workload from lib.c
    mixed_workload(mode);
    
    printf("Execution complete for mode %d\n", mode);
    return 0;
}
