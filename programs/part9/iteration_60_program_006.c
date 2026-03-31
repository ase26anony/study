/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug output
 * when compiled with specific flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that dump RTL instructions with debug enabled.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a data-dependent loop that selective scheduler should process
 */
int test_inner_loop(int n, int* arr) {
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Loop with data dependency and conditional */
    for (int i = 0; i < n; i++) {
        /* Create artificial dependency chain */
        sum += arr[i];
        
        /* Conditional store to prevent simple scheduling */
        if (sum > 1000) {
            arr[i] = sum % 256;
            /* Inline asm to create specific RTL patterns */
            asm volatile ("" : : "r"(sum) : "memory");
        }
        
        /* Volatile access to prevent dead code elimination */
        local_volatile = i;
        g_volatile_array[i & 255] = local_volatile;
    }
    
    /* Another asm to create scheduling barrier */
    asm volatile ("# Scheduling barrier" : : : "memory");
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Outer loop pipelining should be attempted here
 */
void test_nested_loops(int rows, int cols, int matrix[rows][cols]) {
    volatile int temp = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        /* Inner loop with computation */
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = (i * cols + j) % 256;
            
            /* Data-dependent computation */
            matrix[i][j] = (matrix[i][j] * 3 + 7) & 0xFF;
            
            /* Volatile store to force memory operations */
            g_volatile_array[idx] = matrix[i][j];
            
            /* Inline asm with register constraint */
            asm volatile ("# Nested loop computation %0" : : "r"(idx));
        }
        
        /* Conditional update between loop iterations */
        if (i % 3 == 0) {
            temp += matrix[i][0];
            asm volatile ("" : : "r"(temp));
        }
        
        g_volatile_counter++;
    }
}

/* Function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow for the scheduler
 */
int test_switch_complex(int x, int* results) {
    static void* labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    volatile int ret = 0;
    
    /* Indirect jump simulation */
    if (x >= 0 && x < 4) {
        goto *labels[x];
    } else {
        goto *labels[4];
    }
    
case0:
    for (int i = 0; i < 10; i++) {
        results[i] = i * x;
        asm volatile ("# Case0 loop" : : "r"(i));
    }
    ret = 1;
    goto end;
    
case1:
    /* Different loop structure */
    {
        int acc = x;
        for (int i = 5; i > 0; i--) {
            acc = (acc * 17 + i) % 100;
            results[i] = acc;
        }
        ret = 2;
    }
    goto end;
    
case2:
    /* Nested conditionals */
    if (x > 10) {
        for (int i = 0; i < 8; i += 2) {
            results[i] = x << i;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            results[i] = x >> i;
        }
    }
    ret = 3;
    goto end;
    
case3:
    /* Mixed operations */
    for (int i = 0; i < 6; i++) {
        results[i] = (x + i) * (x - i);
        /* Memory barrier asm */
        asm volatile ("" : : "r"(results[i]) : "memory");
    }
    ret = 4;
    goto end;
    
default_case:
    for (int i = 0; i < 12; i++) {
        results[i] = x % (i + 1);
    }
    ret = 0;
    
end:
    /* Final asm to create scheduling point */
    asm volatile ("# Switch end %0" : : "r"(ret));
    return ret;
}

/* Function 4: Mixed control flow with function calls
 * Creates more scheduling opportunities
 */
int test_mixed_control_flow(int limit, int* out) {
    volatile int state = 0;
    int i = 0;
    
    while (i < limit) {
        /* Multiple basic blocks within loop */
        if (i % 2 == 0) {
            out[i] = test_inner_loop(i % 10 + 1, out);
            asm volatile ("# Even iteration" : : "r"(i));
        } else {
            out[i] = i * i - 1;
            /* Dependency chain */
            for (int j = 0; j < (i % 5); j++) {
                out[i] += j;
                asm volatile ("" : : "r"(j));
            }
        }
        
        /* Another conditional */
        if (out[i] > 50) {
            state++;
            g_volatile_array[i & 255] = state;
        }
        
        i++;
        
        /* Loop-carried dependency through volatile */
        asm volatile ("" : : "r"(state));
    }
    
    return state;
}

/* Function 5: Array processing with pointer arithmetic
 * Creates memory access patterns for scheduling
 */
void test_pointer_arithmetic(int* data, int size) {
    volatile int checksum = 0;
    int* end = data + size;
    int* ptr = data;
    
    while (ptr < end) {
        /* Complex pointer arithmetic */
        int offset = ptr - data;
        *ptr = (*ptr * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Conditional based on computed value */
        if (*ptr % 7 == 0) {
            checksum += *ptr;
            /* Inline asm with memory clobber */
            asm volatile ("# Pointer mod 7" : : "r"(checksum) : "memory");
        } else if (*ptr % 13 == 0) {
            checksum -= *ptr;
        }
        
        /* Volatile update */
        g_volatile_counter += offset;
        
        ptr++;
    }
    
    /* Final asm barrier */
    asm volatile ("# Pointer loop done %0" : : "r"(checksum));
}

/* Main driver function that calls all test functions
 * Ensures all code paths are compiled
 */
int main(int argc, char** argv) {
    /* Initialize test data */
    int array1[100];
    int array2[100];
    int matrix[10][10];
    int results[20];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        array1[i] = (i * 37 + 13) % 100;
        array2[i] = (i * 19 + 7) % 100;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 10 + j) % 50;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = test_inner_loop(50, array1);
    
    test_nested_loops(10, 10, matrix);
    
    int result2 = test_switch_complex(argc > 1 ? atoi(argv[1]) % 5 : 2, results);
    
    int result3 = test_mixed_control_flow(30, array2);
    
    test_pointer_arithmetic(array1, 100);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3 + g_volatile_counter;
    
    printf("Test completed (result hint: %d)\n", final_result);
    
    return 0;
}
