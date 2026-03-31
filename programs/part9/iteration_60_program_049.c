/* test_sel_sched_debug.c
 * 
 * This code is designed to trigger GCC's selective scheduler debug output
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Use volatile to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_array[256];

/* Function 1: Complex loop with conditional branch and memory operations
 * This should create scheduling challenges with data dependencies */
void complex_loop_with_branch(int *arr, int n) {
    int sum = 0;
    volatile int temp = 0;
    
    /* Create data dependencies that prevent simple scheduling */
    for (int i = 0; i < n; i++) {
        /* Complex conditional with side effects */
        if (arr[i] > 0) {
            sum += arr[i];
            /* Memory store that creates dependency */
            global_array[i & 255] = sum;
            
            /* Inline assembly to create unschedulable dependency */
            asm volatile ("" : : "r"(sum) : "memory");
        } else {
            sum -= arr[i];
            /* Another memory operation with dependency */
            temp = global_array[(i + 1) & 255];
            
            /* More inline assembly for dependencies */
            asm volatile ("" : : "r"(temp) : "memory");
        }
        
        /* Additional computation to increase scheduling complexity */
        arr[i] = (sum * 3 + i) / 2;
    }
    
    /* Final store with dependency chain */
    global_counter = sum;
    asm volatile ("" : : "r"(global_counter) : "memory");
}

/* Function 2: Nested loops with different iteration patterns
 * Outer loop pipelining should be triggered here */
void nested_loops_pipelining(int rows, int cols, int *matrix) {
    volatile int acc = 0;
    
    /* Outer loop - should trigger outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with data dependencies */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex addressing with conditional */
            if ((i + j) % 3 == 0) {
                row_sum += matrix[idx] * 2;
                /* Memory barrier via inline assembly */
                asm volatile ("" : : "r"(row_sum) : "memory");
            } else {
                row_sum -= matrix[idx];
                /* Another dependency */
                asm volatile ("" : : "r"(matrix[idx]) : "memory");
            }
            
            /* Modify matrix element creating write-after-read dependency */
            matrix[idx] = (matrix[idx] + row_sum) % 1000;
        }
        
        /* Accumulate with global variable */
        acc += row_sum;
        global_array[i & 255] = row_sum;
    }
    
    /* Final result stored */
    global_counter = acc;
}

/* Function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow for the scheduler */
void switch_complex_control_flow(int mode, int iterations) {
    volatile int state = 0;
    int *dyn_arr = (int*)malloc(iterations * sizeof(int));
    
    if (!dyn_arr) return;
    
    /* Initialize dynamic array */
    for (int i = 0; i < iterations; i++) {
        dyn_arr[i] = i * 3 + 1;
    }
    
    /* Complex switch with loops inside cases */
    for (int i = 0; i < iterations; i++) {
        switch (mode) {
            case 0:
                /* Simple arithmetic */
                state += dyn_arr[i];
                asm volatile ("" : : "r"(state) : "memory");
                break;
                
            case 1:
                /* More complex with condition */
                if (dyn_arr[i] % 2 == 0) {
                    state *= dyn_arr[i];
                } else {
                    state /= (dyn_arr[i] | 1); /* Avoid division by zero */
                }
                asm volatile ("" : : "r"(dyn_arr[i]) : "memory");
                break;
                
            case 2:
                /* Nested loop inside switch case */
                for (int j = 0; j < 5; j++) {
                    state += dyn_arr[i] * j;
                    /* Memory dependency */
                    global_array[(i + j) & 255] = state;
                }
                asm volatile ("" : : "r"(state) : "memory");
                break;
                
            default:
                /* Complex default case */
                state = (state << 3) | (dyn_arr[i] & 7);
                asm volatile ("" : : "r"(state) : "memory");
                break;
        }
        
        /* Modify array based on state */
        dyn_arr[i] = (dyn_arr[i] + state) % 100;
    }
    
    global_counter = state;
    free(dyn_arr);
}

/* Function 4: Mixed operations with function calls
 * Creates additional scheduling complexity */
int helper_func(int x, int y) {
    volatile int result = 0;
    
    /* Small loop in helper */
    for (int i = 0; i < 3; i++) {
        result += (x * y) >> i;
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    return result;
}

void mixed_operations_with_calls(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    volatile int total = 0;
    
    if (!data) return;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        data[i] = i * 7 - 3;
    }
    
    /* Loop with function calls and memory ops */
    for (int i = 1; i < n - 1; i++) {
        /* Call helper function - creates scheduling barrier */
        int temp = helper_func(data[i-1], data[i+1]);
        
        /* Complex update with multiple dependencies */
        data[i] = (data[i] * 2 + temp) % 1000;
        
        /* Accumulate with conditional */
        if (data[i] > 500) {
            total += data[i];
            /* Memory store with dependency */
            global_array[i & 255] = total;
        } else {
            total -= data[i];
        }
        
        /* Inline assembly for dependency */
        asm volatile ("" : : "r"(data[i]), "r"(total) : "memory");
    }
    
    global_counter = total;
    free(data);
}

/* Main function to ensure all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Test data */
    int test_arr[100];
    int test_matrix[10][20];
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        test_arr[i] = (i * 13) % 97;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            test_matrix[i][j] = (i * 20 + j) % 123;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    complex_loop_with_branch(test_arr, 100);
    
    nested_loops_pipelining(10, 20, (int*)test_matrix);
    
    switch_complex_control_flow(argc > 1 ? atoi(argv[1]) % 4 : 0, 50);
    
    mixed_operations_with_calls(80);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", global_counter);
    
    return 0;
}
