#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

#define HOT_LOOP_COUNT 1000
#define COLD_LOOP_COUNT 10
#define VERY_COLD_LOOP_COUNT 2

/* Function declarations */
void hot_function_1(int iterations);
void hot_function_2(int iterations);
void cold_function_1(int iterations);
void cold_function_2(int iterations);
void rarely_called_function(int mode);
void conditional_path_function(int mode, int threshold);
void nested_function_calls(int depth, int iterations);
void file_io_simulation(int mode);
void memory_operations(int size, int iterations);

/* Global variables for varied execution paths */
static int global_counter = 0;
static char* dynamic_buffer = NULL;

int main(int argc, char** argv) {
    int mode = 1;  /* Default mode */
    int custom_threshold = 50;
    
    /* Parse command line arguments to vary execution */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            custom_threshold = atoi(argv[2]);
        }
    }
    
    printf("Running in mode %d with threshold %d\n", mode, custom_threshold);
    
    /* Vary execution based on mode to create different profiles */
    switch (mode) {
        case 1:
            /* Mode 1: Heavy execution of hot functions */
            hot_function_1(HOT_LOOP_COUNT);
            hot_function_2(HOT_LOOP_COUNT);
            cold_function_1(COLD_LOOP_COUNT);
            rarely_called_function(1);
            break;
            
        case 2:
            /* Mode 2: More balanced execution */
            hot_function_1(HOT_LOOP_COUNT / 2);
            hot_function_2(HOT_LOOP_COUNT / 4);
            cold_function_1(COLD_LOOP_COUNT * 2);
            cold_function_2(COLD_LOOP_COUNT * 3);
            rarely_called_function(2);
            break;
            
        case 3:
            /* Mode 3: Focus on cold paths */
            hot_function_1(VERY_COLD_LOOP_COUNT);
            cold_function_1(COLD_LOOP_COUNT);
            cold_function_2(COLD_LOOP_COUNT);
            rarely_called_function(3);
            break;
            
        default:
            /* Default: Mixed execution */
            hot_function_1(HOT_LOOP_COUNT / 10);
            hot_function_2(HOT_LOOP_COUNT / 5);
            cold_function_1(COLD_LOOP_COUNT);
            rarely_called_function(0);
    }
    
    /* Call functions with conditionals */
    conditional_path_function(mode, custom_threshold);
    
    /* Nested function calls */
    nested_function_calls(mode % 3 + 1, COLD_LOOP_COUNT);
    
    /* File operations simulation */
    file_io_simulation(mode);
    
    /* Memory operations */
    memory_operations(mode * 100, COLD_LOOP_COUNT);
    
    /* Call library functions */
    lib_function_a(mode * 10);
    lib_function_b(mode * 5);
    lib_function_c(mode);
    
    /* Conditional call to library hot function */
    if (mode % 2 == 0) {
        lib_hot_function(HOT_LOOP_COUNT / mode);
    }
    
    /* Rare library call */
    if (mode == 3) {
        lib_rare_function();
    }
    
    /* Cleanup */
    if (dynamic_buffer) {
        free(dynamic_buffer);
        dynamic_buffer = NULL;
    }
    
    printf("Execution complete. Global counter: %d\n", global_counter);
    return 0;
}

void hot_function_1(int iterations) {
    int i, j;
    int local_sum = 0;
    
    for (i = 0; i < iterations; i++) {
        local_sum += i;
        global_counter++;
        
        /* Inner loop for more execution density */
        for (j = 0; j < 10; j++) {
            local_sum += j * i;
        }
        
        /* Conditional inside hot loop */
        if (i % 100 == 0) {
            local_sum *= 2;
        }
    }
    
    /* Another conditional */
    if (local_sum > 1000000) {
        printf("Hot function 1: Large sum computed\n");
    }
}

void hot_function_2(int iterations) {
    int i;
    double result = 1.0;
    
    for (i = 1; i <= iterations; i++) {
        result *= 1.01;
        global_counter += 2;
        
        /* Different conditional pattern */
        if (i % 50 == 0) {
            result /= 1.005;
        }
        
        /* Nested conditional */
        if (i > iterations / 2) {
            result += 0.1;
            if (i % 33 == 0) {
                result -= 0.05;
            }
        }
    }
    
    /* Final calculation */
    if (result > 100.0) {
        printf("Hot function 2: Result = %.2f\n", result);
    }
}

void cold_function_1(int iterations) {
    int i;
    
    if (iterations <= 0) {
        return;
    }
    
    for (i = 0; i < iterations; i++) {
        global_counter--;
        
        /* Rarely executed condition */
        if (i == iterations - 1) {
            printf("Cold function 1: Last iteration\n");
        }
    }
}

void cold_function_2(int iterations) {
    int i;
    static int call_count = 0;
    
    call_count++;
    
    /* Only execute loop on odd call counts */
    if (call_count % 2 == 1) {
        for (i = 0; i < iterations; i++) {
            global_counter += i;
        }
    } else {
        /* Alternative path */
        global_counter -= iterations;
    }
    
    /* Complex conditional */
    if (call_count > 5 && iterations > 20) {
        printf("Cold function 2: Special case\n");
    }
}

void rarely_called_function(int mode) {
    /* This function is called rarely with different modes */
    switch (mode) {
        case 0:
            printf("Rare function: Mode 0 (never?)\n");
            break;
        case 1:
            printf("Rare function: Mode 1\n");
            global_counter += 1000;
            break;
        case 2:
            printf("Rare function: Mode 2\n");
            global_counter += 500;
            break;
        case 3:
            printf("Rare function: Mode 3\n");
            global_counter += 250;
            break;
        default:
            /* Unreachable in normal execution */
            printf("Rare function: Unexpected mode\n");
    }
    
    /* Allocate memory only in specific mode */
    if (mode == 2 && !dynamic_buffer) {
        dynamic_buffer = malloc(1024);
        if (dynamic_buffer) {
            strcpy(dynamic_buffer, "Rare allocation");
        }
    }
}

void conditional_path_function(int mode, int threshold) {
    int i;
    int limit = threshold * 2;
    
    /* Different execution paths based on mode and threshold */
    if (mode < threshold) {
        /* Path A */
        for (i = 0; i < limit; i++) {
            if (i % 3 == 0) {
                global_counter += 3;
            } else if (i % 3 == 1) {
                global_counter += 1;
            } else {
                global_counter += 2;
            }
        }
    } else {
        /* Path B - less frequent */
        for (i = 0; i < threshold; i++) {
            global_counter -= i;
        }
    }
    
    /* Common code */
    if (global_counter < 0) {
        global_counter = 0;
    }
}

void nested_function_calls(int depth, int iterations) {
    int i;
    
    if (depth <= 0) {
        return;
    }
    
    for (i = 0; i < iterations; i++) {
        global_counter += depth;
        
        /* Recursive call with reduced depth */
        if (depth > 1 && i % 2 == 0) {
            nested_function_calls(depth - 1, iterations / 2);
        }
    }
}

void file_io_simulation(int mode) {
    /* Simulate file operations without actual I/O */
    static int file_counter = 0;
    int i;
    
    file_counter += mode;
    
    for (i = 0; i < mode * 5; i++) {
        /* Simulate write operation */
        global_counter++;
        
        /* Simulate read operation every 3rd iteration */
        if (i % 3 == 0) {
            global_counter += file_counter;
        }
        
        /* Error simulation (rare) */
        if (i == mode * 5 - 1 && mode > 2) {
            global_counter -= 100;
        }
    }
    
    if (file_counter > 100) {
        file_counter = 0;  /* Reset simulation */
    }
}

void memory_operations(int size, int iterations) {
    int i, j;
    int* temp_buffer = NULL;
    
    if (size <= 0 || iterations <= 0) {
        return;
    }
    
    temp_buffer = malloc(size * sizeof(int));
    if (!temp_buffer) {
        return;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Write to buffer */
        for (j = 0; j < size && j < 100; j++) {
            temp_buffer[j] = i * j;
        }
        
        /* Read from buffer */
        for (j = 0; j < size && j < 50; j++) {
            global_counter += temp_buffer[j] % 7;
        }
        
        /* Reallocation simulation (every 5th iteration) */
        if (i % 5 == 0 && size > 50) {
            int* new_buffer = realloc(temp_buffer, (size + 10) * sizeof(int));
            if (new_buffer) {
                temp_buffer = new_buffer;
                size += 10;
            }
        }
    }
    
    free(temp_buffer);
}
