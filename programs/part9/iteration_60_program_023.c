/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies and control flow
 */
int test_inner_loop(int n, int* arr) {
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_volatile = i;
        
        /* Conditional branch inside loop */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2 + local_volatile;
        } else if (i % 3 == 1) {
            arr[i] = arr[i] / 2 - local_volatile;
        } else {
            arr[i] = arr[i] + local_volatile * 3;
        }
        
        /* Memory write with dependency */
        sum += arr[i];
        
        /* Inline assembly to create specific RTL patterns */
        asm volatile ("" : : "r"(arr[i]), "r"(sum));
    }
    
    /* Force side effect */
    g_volatile_counter += sum;
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop scheduling opportunities
 */
void test_nested_loops(int rows, int cols, int matrix[][64]) {
    volatile int temp = 0;
    
    /* Outer loop with inner loop dependency */
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = (i * 64 + j) % 256;
            
            /* Memory access with potential stall */
            matrix[i][j] = g_volatile_array[idx] + i * j;
            
            /* Conditional store */
            if (matrix[i][j] > 1000) {
                matrix[i][j] = matrix[i][j] % 1000;
            }
            
            row_sum += matrix[i][j];
            
            /* Another inline asm to prevent reordering */
            asm volatile ("" : "+r"(row_sum) : : "memory");
        }
        
        /* Store result with volatile side effect */
        temp = row_sum;
        g_volatile_array[i % 256] = temp;
    }
}

/* Function 3: Switch statement with computed goto-like pattern
 * Creates complex control flow for the scheduler
 */
int test_switch_complex(int x, int* results) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3, &&case_default
    };
    
    int result = 0;
    volatile int selector = x % 5;
    
    /* Indirect jump simulation */
    switch (selector) {
        case 0:
            /* Array processing in case block */
            for (int i = 0; i < 16; i++) {
                results[i] = i * x + g_volatile_counter;
                asm volatile ("" : : "r"(results[i]));
            }
            result = 1;
            break;
            
        case 1:
            /* Nested conditionals */
            if (x > 100) {
                results[0] = x / 2;
            } else if (x > 50) {
                results[0] = x * 2;
            } else {
                results[0] = x + 100;
            }
            result = 2;
            break;
            
        case 2:
            /* Small loop with break */
            for (int i = 0; i < 8; i++) {
                if (i == x % 8) break;
                results[i] = x - i;
            }
            result = 3;
            break;
            
        case 3:
            /* Memory intensive */
            for (int i = 0; i < 32; i++) {
                g_volatile_array[(x + i) % 256] = i;
                results[i % 16] += g_volatile_array[i % 256];
            }
            result = 4;
            break;
            
        default:
            /* Default with function call */
            result = test_inner_loop(8, results);
            break;
    }
    
    return result;
}

/* Function 4: Mixed operations with pointer aliasing
 * Creates scheduling challenges due to potential aliasing
 */
int test_mixed_ops(int* a, int* b, int* c, int n) {
    int total = 0;
    volatile int barrier = 0;
    
    /* Loop with pointer accesses that may alias */
    for (int i = 0; i < n; i++) {
        /* Operations that create dependencies */
        int t1 = a[i] * 3;
        int t2 = b[i] + t1;
        int t3 = c[i] - t2;
        
        /* Conditional store with side effect */
        if (t3 > 0) {
            a[i] = t3;
            barrier = t3;  /* Volatile write creates barrier */
        } else {
            b[i] = -t3;
        }
        
        /* Complex expression with multiple uses */
        total += (a[i] * b[i]) / (c[i] + 1);
        
        /* Memory barrier via asm */
        asm volatile ("" : : "r"(a[i]), "r"(b[i]), "r"(c[i]) : "memory");
    }
    
    return total;
}

/* Function 5: Recursive-like pattern using iteration
 * Creates back-edge dependencies
 */
int test_backedge_deps(int n) {
    int prev = 1, curr = 1, next;
    volatile int store[128];
    
    /* Fibonacci-like calculation with stores */
    for (int i = 0; i < n && i < 128; i++) {
        next = prev + curr + g_volatile_counter;
        
        /* Store with dependency chain */
        store[i] = next;
        
        /* Update with dependency */
        prev = curr;
        curr = next % 1024;
        
        /* Periodic conditional */
        if (i % 16 == 0) {
            g_volatile_array[i % 256] = curr;
            asm volatile ("" : : "r"(curr));
        }
    }
    
    return curr;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char** argv) {
    /* Initialize test data */
    int array1[256];
    int array2[256];
    int results[64];
    int matrix[8][64];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 - 2;
        g_volatile_array[i] = i;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    /* Vary parameters to hit different paths */
    for (int i = 0; i < 4; i++) {
        result += test_inner_loop(32 + i * 8, array1);
        test_nested_loops(4 + i, 32, matrix);
        result += test_switch_complex(20 + i * 15, results);
        result += test_mixed_ops(array1, array2, results, 48);
        result += test_backedge_deps(40 + i * 4);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
