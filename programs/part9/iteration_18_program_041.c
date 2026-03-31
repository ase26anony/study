/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compiler directives to control instrumentation */
#define HOT_LOOP_COUNT 10000
#define COLD_LOOP_COUNT 5

/* Function attributes to preserve structure */
__attribute__((noinline)) static void cold_function_1(int seed);
__attribute__((noinline)) static void cold_function_2(int seed);
__attribute__((hot)) void hot_function_a(int iterations);
__attribute__((hot)) void hot_function_b(int iterations);
__attribute__((cold)) void rarely_called_function(void);
__attribute__((noinline)) int complex_branching(int x, int mode);

/* Global variables for object-level separation */
int global_counter = 0;
static int module_local = 0;

/* ========== MODULE 1 FUNCTIONS ========== */

/* Function with same name as in module 2 (for -F fullname testing) */
__attribute__((noinline)) 
static void process_data(int* arr, int size, int threshold)
{
    int i;
    int local_sum = 0;
    
    /* Hot loop - will have high counts */
    for (i = 0; i < size; i++) {
        if (arr[i] > threshold) {
            local_sum += arr[i] * 2;
            if (arr[i] > threshold * 2) {
                local_sum -= arr[i] / 2;
            }
        } else {
            local_sum += arr[i];
            /* Nested cold branch */
            if (arr[i] < 0 && (i % 7 == 0)) {
                local_sum += 100;
            }
        }
        
        /* Switch for additional branching */
        switch (i % 4) {
            case 0: local_sum += 1; break;
            case 1: local_sum += 2; break;
            case 2: local_sum += 3; break;
            case 3: local_sum += 4; break;
        }
    }
    
    global_counter += local_sum;
}

/* Cold function 1 - rarely executed deeply */
__attribute__((noinline)) 
static void cold_function_1(int seed)
{
    int i;
    int temp = 0;
    
    /* Small loop - cold */
    for (i = 0; i < COLD_LOOP_COUNT; i++) {
        if ((seed + i) % 13 == 0) {
            temp += i * 3;
            if (i % 2 == 0) {
                temp -= i;
            }
        }
    }
    
    module_local += temp;
    rarely_called_function();
}

/* ========== MODULE 2 FUNCTIONS (in separate file) ========== */
/* Note: In actual use, this would be in a separate .c file */

/* Another function with same name (for -F testing) */
__attribute__((noinline)) 
void process_data_alt(int* arr, int size, int threshold)
{
    int i, j;
    int result = 0;
    
    /* Different hot loop structure */
    for (i = 0; i < size; i += 2) {
        if (arr[i] % 2 == 0) {
            result += arr[i];
            /* Nested loop - creates more arcs */
            for (j = 0; j < 3; j++) {
                if ((arr[i] + j) > threshold) {
                    result += j;
                }
            }
        } else {
            result -= arr[i];
            if (arr[i] < threshold / 2) {
                result *= 2;
            }
        }
    }
    
    global_counter += result;
}

/* Hot function A - high execution count */
__attribute__((hot)) 
void hot_function_a(int iterations)
{
    int i, j;
    int sum = 0;
    
    /* Very hot loop */
    #pragma GCC unroll 0  /* Prevent unrolling */
    for (i = 0; i < iterations; i++) {
        sum += i;
        
        /* Inner hot loop */
        for (j = 0; j < 100; j++) {
            if (j % 7 == 0) {
                sum += complex_branching(j, i % 3);
            } else if (j % 5 == 0) {
                sum -= j / 2;
            } else {
                sum += j * 3;
            }
        }
        
        /* Branch with varying probability */
        if (i % 1000 == 0) {
            sum /= 2;
        }
    }
    
    if (sum > 1000000) {
        global_counter += sum % 1000;
    }
}

/* Hot function B - different hot pattern */
__attribute__((hot)) 
void hot_function_b(int iterations)
{
    int i;
    long product = 1;
    
    for (i = 1; i < iterations; i++) {
        if (i % 3 == 0) {
            product *= i;
            if (product > 1000000) {
                product = product % 1000000 + 1;
            }
        } else if (i % 7 == 0) {
            product += i * 2;
        } else {
            product -= i / 2;
        }
        
        /* Switch with multiple cases */
        switch (i % 8) {
            case 0: product += 10; break;
            case 1: product += 20; break;
            case 2: product += 30; break;
            case 3: product += 40; break;
            case 4: product += 50; break;
            case 5: product += 60; break;
            case 6: product += 70; break;
            case 7: product += 80; break;
        }
    }
    
    global_counter += (int)(product % 10000);
}

/* Cold function 2 */
__attribute__((noinline)) 
static void cold_function_2(int seed)
{
    int i;
    
    /* Very cold - minimal execution */
    for (i = 0; i < 2; i++) {
        if (seed % (i + 2) == 0) {
            global_counter += i;
        }
    }
    
    /* Dead code section - never executed in normal runs */
    #ifdef ENABLE_DEAD_CODE
    for (i = 0; i < 100; i++) {
        global_counter += i * 1000;
    }
    #endif
}

/* Rarely called function */
__attribute__((cold)) 
void rarely_called_function(void)
{
    static int call_count = 0;
    call_count++;
    
    if (call_count % 100 == 0) {
        global_counter += 999;
    }
}

/* Complex branching function */
__attribute__((noinline)) 
int complex_branching(int x, int mode)
{
    int result = x;
    
    if (mode == 0) {
        if (x < 50) {
            result = x * 2;
            if (x % 2 == 0) {
                result += 10;
                if (x % 4 == 0) {
                    result -= 5;
                }
            }
        } else {
            result = x / 2;
            if (x > 75) {
                result += 25;
            }
        }
    } else if (mode == 1) {
        switch (x % 6) {
            case 0: result = x + 100; break;
            case 1: result = x - 50; break;
            case 2: result = x * 3; break;
            case 3: result = x / 3; break;
            case 4: result = x + 200; break;
            case 5: result = x - 100; break;
        }
    } else {
        /* Mode 2 - complex nested conditionals */
        result = 0;
        for (int i = 0; i < 5; i++) {
            if (x > i * 20) {
                result += i * 10;
                if (x > i * 30) {
                    result += i * 5;
                }
            }
        }
    }
    
    return result;
}

/* Additional function for more coverage */
void medium_function(int limit)
{
    int i;
    int array[50];
    
    /* Initialize array */
    for (i = 0; i < 50; i++) {
        array[i] = (i * 7) % 113;
    }
    
    /* Process with varying intensity */
    for (i = 0; i < limit; i++) {
        int idx = i % 50;
        if (array[idx] > 50) {
            array[idx] = complex_branching(array[idx], i % 3);
        } else {
            array[idx] += i;
        }
    }
    
    /* Final pass */
    for (i = 0; i < 50; i++) {
        global_counter += array[i];
    }
}

/* ========== MAIN PROGRAM ========== */

int main(int argc, char *argv[])
{
    int mode = 0;
    int seed = 1;
    int iterations = 1000;
    int algorithm = 0;
    int data_size = 1000;
    int *data;
    int i;
    
    /* Parse command line arguments for different run modes */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        }
    }
    
    /* Seed RNG for reproducible but varied profiles */
    srand(seed);
    
    /* Allocate and initialize data array */
    data = (int*)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Reset global counter */
    global_counter = 0;
    module_local = 0;
    
    /* Execute different code paths based on mode and algorithm */
    switch (mode) {
        case 0:
            /* Mode 0: Heavy hot function usage */
            hot_function_a(HOT_LOOP_COUNT);
            hot_function_b(HOT_LOOP_COUNT / 2);
            process_data(data, data_size, 500);
            medium_function(iterations);
            break;
            
        case 1:
            /* Mode 1: More cold functions, less hot */
            cold_function_1(seed);
            cold_function_2(seed);
            process_data_alt(data, data_size, 300);
            medium_function(iterations / 10);
            hot_function_a(HOT_LOOP_COUNT / 10);
            break;
            
        case 2:
            /* Mode 2: Mixed execution */
            for (i = 0; i < 3; i++) {
                hot_function_a(HOT_LOOP_COUNT / 5);
                cold_function_1(seed + i);
                process_data(data + i * 100, 100, 200 + i * 100);
            }
            break;
            
        case 3:
            /* Mode 3: Algorithm-specific path */
            if (algorithm == 0) {
                hot_function_a(iterations * 2);
                process_data(data, data_size, 700);
            } else if (algorithm == 1) {
                hot_function_b(iterations * 3);
                process_data_alt(data, data_size, 400);
            } else {
                for (i = 0; i < iterations / 100; i++) {
                    hot_function_a(100);
                    hot_function_b(50);
                }
            }
            medium_function(iterations);
            break;
            
        default:
            /* Default: Balanced execution */
            hot_function_a(HOT_LOOP_COUNT / 4);
            hot_function_b(HOT_LOOP_COUNT / 4);
            process_data(data, data_size, 600);
            process_data_alt(data, data_size, 200);
            cold_function_1(seed);
            medium_function(iterations);
            break;
    }
    
    /* Additional conditional execution based on random seed */
    if (seed % 7 == 0) {
        rarely_called_function();
    }
    
    if (seed % 13 == 0) {
        /* Extra cold path */
        for (i = 0; i < 5; i++) {
            cold_function_2(seed + i);
        }
    }
    
    /* Final computation for deterministic output */
    int checksum = global_counter + module_local;
    for (i = 0; i < data_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    /* Output deterministic result to prevent dead code elimination */
    printf("Result: %d (Mode: %d, Seed: %d, Algorithm: %d)\n", 
           checksum, mode, seed, algorithm);
    
    free(data);
    return 0;
}
