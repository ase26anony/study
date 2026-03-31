/* test_sel_sched_dump.c
 * Designed to trigger GCC's selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-sched2 -c test_sel_sched_dump.c
 * Or: gcc -O3 -fsel-sched-pipelining-outer-loops -dS -fdump-rtl-all -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int func_inner_loop(int *arr, int n) {
    int sum = 0;
    volatile int local_vol = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_vol = arr[i];
        
        /* Conditional store with side effect */
        if (arr[i] > 0) {
            sum += arr[i];
            /* Memory write that can't be moved */
            g_volatile_array[i & 255] = sum;
        } else {
            sum -= arr[i];
            /* Different memory write path */
            g_volatile_array[(i + 128) & 255] = sum;
        }
        
        /* Inline asm to create unschedulable dependency */
        asm volatile ("# Dependency barrier %0" : : "r" (sum));
    }
    
    /* Another asm to prevent tail optimization */
    asm volatile ("# Result %0" : : "r" (sum));
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Outer loop pipelining candidate */
int func_nested_loops(int rows, int cols, int *matrix) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - candidate for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        
        /* Inner loop with complex addressing */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Multiple uses of idx create dependencies */
            int val = matrix[idx];
            
            /* Conditional with both paths used */
            if (val & 1) {
                row_sum += val * 3;
            } else {
                row_sum -= val / 2;
            }
            
            /* Store with anti-dependency */
            matrix[idx] = row_sum;
            
            /* Periodic asm barrier */
            if ((j & 7) == 0) {
                asm volatile ("# Inner loop barrier %0, %1" : : "r" (i), "r" (j));
            }
        }
        
        total += row_sum;
        /* Outer loop barrier */
        asm volatile ("# Outer loop total %0" : : "r" (total));
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto-like behavior
 * Creates complex control flow for selective scheduling */
int func_switch_complex(int x, int *results) {
    int result = 0;
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    /* Use volatile to force all cases to be compiled */
    volatile int selector = x % 5;
    
    /* Indirect goto through switch to create complex CFG */
    switch (selector) {
        case 0:
            L0:
            result = x * 2;
            /* Memory op with dependency */
            results[0] = result;
            asm volatile ("# Case 0: %0" : : "r" (result));
            break;
            
        case 1:
            L1:
            result = x + x;
            for (int i = 0; i < 3; i++) {
                result += results[i];
                asm volatile ("# Case 1 loop: %0" : : "r" (result));
            }
            break;
            
        case 2:
            L2:
            result = x * x;
            /* Multiple memory accesses */
            results[1] = result;
            results[2] = result + 1;
            asm volatile ("# Case 2: %0" : : "r" (result));
            break;
            
        case 3:
            L3:
            result = 0;
            /* Small unrolled loop */
            for (int i = 0; i < 4; i++) {
                result += x << i;
                asm volatile ("# Case 3 shift: %0" : : "r" (result));
            }
            break;
            
        case 4:
            L4:
            result = x;
            /* Conditional nested loop */
            if (x > 10) {
                for (int i = 0; i < x % 8; i++) {
                    result += i * results[i & 3];
                }
            }
            asm volatile ("# Case 4: %0" : : "r" (result));
            break;
    }
    
    /* Final computation with volatile side effect */
    g_volatile_counter = result;
    return result;
}

/* Function 4: Mixed control flow with function calls
 * Creates scheduling boundaries */
int func_mixed_flow(int a, int b, int c) {
    int temp;
    
    /* Data-dependent if-else chain */
    if (a > b) {
        temp = a - b;
        /* Inline asm prevents reordering */
        asm volatile ("# Path a>b: %0" : : "r" (temp));
    } else if (b > c) {
        temp = b - c;
        asm volatile ("# Path b>c: %0" : : "r" (temp));
    } else {
        temp = c - a;
        /* Loop with variable bound */
        for (int i = 0; i < (temp & 7); i++) {
            temp += g_volatile_array[i];
            asm volatile ("# Else loop: %0" : : "r" (temp));
        }
    }
    
    /* Another loop with early exit */
    int accum = 0;
    for (int i = 0; i < 16; i++) {
        accum += temp * i;
        if (accum > 1000) {
            /* Early exit creates control flow */
            asm volatile ("# Early exit: %0" : : "r" (accum));
            break;
        }
        /* Memory store in loop */
        g_volatile_array[i] = accum;
    }
    
    return accum;
}

/* Function 5: Pointer chasing loop
 * Creates memory dependency chain */
int func_pointer_chase(int *base, int steps) {
    int *ptr = base;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Load with dependency on previous load */
        int val = *ptr;
        sum += val;
        
        /* Next pointer depends on current value */
        ptr = base + (val & 0xF);
        
        /* Volatile store creates side effect */
        g_volatile_counter = (int)(ptr - base);
        
        /* Asm with memory clobber */
        asm volatile ("# Chase step %0, sum %1" : : "r" (i), "r" (sum) : "memory");
    }
    
    return sum;
}

/* Main driver that ensures all functions are compiled and used */
int main(int argc, char **argv) {
    /* Initialize test data */
    int data[256];
    int matrix[10][20];
    int results[8] = {0};
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * 37 + 13) & 0xFF;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 20 + j;
        }
    }
    
    /* Call all test functions with different parameters
     * to ensure they're all compiled and potentially scheduled */
    int result1 = func_inner_loop(data, 100);
    int result2 = func_nested_loops(5, 10, &matrix[0][0]);
    int result3 = func_switch_complex(argc, results);
    int result4 = func_mixed_flow(result1, result2, result3);
    int result5 = func_pointer_chase(data, 50);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 
        result1 + result2 + result3 + result4 + result5;
    
    /* Print to ensure code isn't optimized away entirely */
    printf("Results: %d %d %d %d %d\n", 
           result1, result2, result3, result4, result5);
    
    return final_result > 0 ? 0 : 1;
}
