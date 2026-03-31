/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compiler directives to control instrumentation */
#define HOT_LOOP_ITERATIONS 10000
#define COLD_LOOP_ITERATIONS 5

/* Function prototypes with attributes for coverage control */
__attribute__((noinline)) void hot_function_a(int iterations, int mode);
__attribute__((noinline)) void hot_function_b(int iterations, int seed);
__attribute__((noinline)) void medium_function(int size, int threshold);
__attribute__((noinline)) void cold_function(int value, int *result);
__attribute__((noinline, cold)) void rarely_called(int mode);
__attribute__((noinline, hot)) void frequently_called(int count);
static void helper_in_file1(int x);  /* For -F (fullname) testing */

/* Global configuration */
static int global_mode = 0;
static int use_algorithm_a = 1;

/* ========== FILE 1 FUNCTIONS ========== */
/* These will be compiled separately for -o (object-level) testing */

__attribute__((noinline)) 
void hot_function_a(int iterations, int mode) {
    int sum = 0;
    volatile int temp; /* Prevent optimization */
    
    /* Hot loop - runs many times */
    for (int i = 0; i < iterations; i++) {
        temp = i * mode;
        
        /* Branch with varying probability based on mode */
        if (i % (mode + 2) == 0) {
            sum += i * 2;
        } else if (i % (mode + 3) == 0 && mode > 1) {
            sum += i * 3;
        } else {
            sum += i;
        }
        
        /* Nested condition for deeper branch coverage */
        if (mode == 3 && i > iterations / 2) {
            for (int j = 0; j < 10; j++) {
                sum += (j % 2 == 0) ? j : -j;
            }
        }
    }
    
    /* Another hot section */
    for (int i = iterations - 1; i >= 0; i -= 2) {
        if (sum > 1000000) {
            sum /= 2;  /* Prevent overflow */
        }
        sum += (i % 7 == 0) ? i * 2 : i;
    }
    
    printf("Hot A result: %d\n", sum % 1000);
}

__attribute__((noinline, cold))
void rarely_called(int mode) {
    /* This function should have low counts */
    static int call_count = 0;
    call_count++;
    
    switch (mode) {
        case 0:
            printf("Rare mode 0\n");
            break;
        case 1:
            printf("Rare mode 1\n");
            break;
        case 2:
            printf("Rare mode 2\n");
            break;
        default:
            printf("Rare default\n");
            for (int i = 0; i < COLD_LOOP_ITERATIONS; i++) {
                if (i == mode) break;
            }
    }
}

/* ========== FILE 2 FUNCTIONS ========== */
/* To be placed in separate source file for multi-object compilation */

__attribute__((noinline)) 
void hot_function_b(int iterations, int seed) {
    int result = seed;
    srand(seed);
    
    /* Different hot loop pattern */
    for (int i = 0; i < iterations; i++) {
        int r = rand() % 100;
        
        /* Complex branching with varying probabilities */
        if (r < 30) {
            result += r * 2;
        } else if (r < 60) {
            result += r * 3;
            if (r > 45 && r < 55) {
                result -= 10;
            }
        } else if (r < 80) {
            result += r;
            for (int j = 0; j < 3; j++) {
                result += (j % 2) * 5;
            }
        } else {
            result += r / 2;
        }
        
        /* Threshold check for hot-only filtering */
        if (result > 1000000) {
            result = result % 1000000;
        }
    }
    
    printf("Hot B result: %d\n", result % 1000);
}

__attribute__((noinline))
void medium_function(int size, int threshold) {
    int data[100];
    if (size > 100) size = 100;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = (i * 7) % 19;
    }
    
    /* Process with threshold-based branching */
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            count++;
            data[i] = -data[i];
        } else if (data[i] < -threshold) {
            count--;
            data[i] = 0;
        } else {
            data[i] += threshold;
        }
    }
    
    /* Additional loop with different characteristics */
    int sum = 0;
    for (int i = 0; i < size; i += 2) {
        sum += data[i];
        if (i % 3 == 0) {
            sum -= data[i] / 2;
        }
    }
    
    printf("Medium result: %d (count: %d)\n", sum, count);
}

/* ========== COMMON FUNCTIONS ========== */

__attribute__((noinline))
void cold_function(int value, int *result) {
    /* Should have low execution counts */
    switch (value % 4) {
        case 0:
            *result = value + 1;
            break;
        case 1:
            *result = value * 2;
            break;
        case 2:
            *result = value / 2;
            for (int i = 0; i < 2; i++) {
                *result += i;
            }
            break;
        case 3:
            *result = value - 1;
            if (*result < 0) *result = 0;
            break;
    }
    
    /* Rarely executed path */
    if (value > 1000 && global_mode == 2) {
        *result *= 2;
    }
}

__attribute__((noinline, hot))
void frequently_called(int count) {
    int local = 0;
    
    #pragma GCC unroll 0  /* Prevent unrolling for stable coverage */
    for (int i = 0; i < count; i++) {
        local += (i % 10 == 0) ? i * 2 : i;
        
        /* Nested condition with varying probability */
        if (i % 20 == 0 && count > 100) {
            local -= i / 2;
        }
    }
    
    /* Multiple exit paths */
    if (local > 5000) {
        printf("Frequent: high %d\n", local % 100);
    } else if (local < 100) {
        printf("Frequent: low %d\n", local);
    }
}

static void helper_in_file1(int x) {
    /* Static function for -F (fullname) testing */
    int y = x * 2;
    if (y > 100) {
        y -= 50;
    }
}

/* ========== MAIN PROGRAM ========== */

int main(int argc, char *argv[]) {
    int seed = 42;
    int iterations = 1000;
    int mode = 0;
    int use_full = 0;
    
    /* Parse command-line arguments for different execution paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--full") == 0) {
            use_full = 1;
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            use_algorithm_a = (strcmp(argv[++i], "A") == 0);
        }
    }
    
    global_mode = mode;
    srand(seed);
    
    printf("Starting run: seed=%d, iterations=%d, mode=%d\n", 
           seed, iterations, mode);
    
    /* Vary execution based on mode to create different profiles */
    if (mode == 0) {
        /* Mode 0: Focus on hot functions */
        hot_function_a(HOT_LOOP_ITERATIONS, 1);
        hot_function_b(iterations * 2, seed);
        frequently_called(iterations);
        
        if (use_full) {
            helper_in_file1(seed);
        }
        
    } else if (mode == 1) {
        /* Mode 1: Balanced execution */
        hot_function_a(iterations, 2);
        medium_function(50, 10);
        frequently_called(iterations / 2);
        
        int cold_result;
        cold_function(seed, &cold_result);
        printf("Cold result: %d\n", cold_result);
        
    } else if (mode == 2) {
        /* Mode 2: Many cold paths */
        hot_function_a(iterations / 10, 3);
        
        for (int i = 0; i < 5; i++) {
            rarely_called(i % 3);
        }
        
        int results[5];
        for (int i = 0; i < 5; i++) {
            cold_function(seed + i, &results[i]);
        }
        
        medium_function(30, 5);
        
    } else {
        /* Default mode: Mixed execution */
        hot_function_b(iterations * 3, seed + mode);
        
        if (use_algorithm_a) {
            hot_function_a(iterations, mode);
        } else {
            hot_function_b(iterations, seed * 2);
        }
        
        frequently_called(iterations);
        rarely_called(mode);
    }
    
    /* Additional variability based on seed */
    if (seed % 3 == 0) {
        medium_function(40, 15);
    }
    
    if (seed % 5 == 0 && mode != 2) {
        hot_function_a(iterations / 2, 4);
    }
    
    /* Final computation for deterministic output */
    int final_result = seed;
    for (int i = 0; i < iterations % 100; i++) {
        final_result = (final_result * 31 + i) % 10007;
    }
    
    printf("Final checksum: %d\n", final_result);
    return final_result % 100;
}
