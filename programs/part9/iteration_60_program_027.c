/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps,
 * specifically the dump_insn_rtx function in sel-sched-dump.cc.
 * The goal is to reach the uncovered lines that handle RTL instruction
 * dumping when scheduler debugging is enabled.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[256];

/* Function 1: Inner loop with conditional branch and memory write
 * This creates a scheduling region with data dependencies */
int test_inner_loop(int n, int *arr) {
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Loop with data dependency chain */
    for (int i = 0; i < n; i++) {
        /* Create anti-dependency with volatile */
        local_volatile = i;
        
        /* Conditional store to create control flow */
        if (i % 3 == 0) {
            arr[i] = i * 2;
            /* Inline asm to create specific RTL patterns */
            asm volatile ("# Inner loop conditional store" : : "r"(i), "r"(arr[i]));
        } else if (i % 3 == 1) {
            arr[i] = i * 3;
            /* Another asm to create scheduling barrier */
            asm volatile ("# Alternative path" : : "r"(i));
        } else {
            arr[i] = i;
            /* Complex asm with multiple constraints */
            asm volatile ("# Default path" : : "r"(i), "r"(arr) : "memory");
        }
        
        sum += arr[i];
        
        /* Volatile access to prevent dead code elimination */
        g_volatile_counter++;
    }
    
    /* Final asm to create scheduling boundary */
    asm volatile ("# Loop end barrier" : : "r"(sum) : "memory");
    return sum;
}

/* Function 2: Nested loops with different iteration counts
 * Creates outer loop pipelining opportunities */
int test_nested_loops(int rows, int cols, int *matrix) {
    int total = 0;
    volatile int row_sum = 0;
    
    /* Outer loop - target for outer loop pipelining */
    for (int i = 0; i < rows; i++) {
        row_sum = 0;
        
        /* Inner loop with stride access */
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;
            
            /* Complex addressing mode */
            matrix[index] = (i * j) + (i ^ j);
            
            /* Inline asm with memory constraint */
            asm volatile ("# Nested loop store" 
                         : "+m"(matrix[index]) 
                         : "r"(i), "r"(j));
            
            row_sum += matrix[index];
            
            /* Conditional with multiple branches */
            if (j % 4 == 0) {
                g_volatile_array[j] = i;
            } else if (j % 4 == 1) {
                g_volatile_array[j] = j;
            } else if (j % 4 == 2) {
                g_volatile_array[j] = i + j;
            }
        }
        
        total += row_sum;
        
        /* Function call to create scheduling boundary */
        asm volatile ("# Outer loop iteration end" : : "r"(total));
    }
    
    return total;
}

/* Function 3: Switch statement with computed goto
 * Creates complex control flow for selective scheduling */
int test_switch_complex(int x, int *results) {
    int ret = 0;
    
    /* Labels for computed goto */
    void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&case4 };
    
    /* Switch-like structure using computed goto */
    if (x >= 0 && x < 5) {
        goto *labels[x];
    } else {
        goto default_case;
    }
    
case0:
    results[0] = x * 2;
    asm volatile ("# Case 0" : : "r"(x));
    ret = 1;
    goto end;
    
case1:
    results[1] = x * 3;
    /* Memory barrier asm */
    asm volatile ("# Case 1" : : : "memory");
    ret = 2;
    goto end;
    
case2:
    results[2] = x * 4;
    /* Multiple output constraints */
    asm volatile ("# Case 2 with outputs" 
                 : "=r"(results[2]) 
                 : "r"(x));
    ret = 3;
    goto end;
    
case3:
    results[3] = x * 5;
    /* Volatile asm that can't be moved */
    asm volatile ("# Case 3 volatile" : : "r"(x) : "cc", "memory");
    ret = 4;
    goto end;
    
case4:
    results[4] = x * 6;
    /* Complex asm with multiple inputs */
    asm volatile ("# Case 4 complex" 
                 : "+m"(results[4]) 
                 : "r"(x), "r"(g_volatile_counter));
    ret = 5;
    goto end;
    
default_case:
    for (int i = 0; i < 5; i++) {
        results[i] = x + i;
        asm volatile ("# Default loop" : : "r"(i));
    }
    ret = 0;
    
end:
    /* Final scheduling barrier */
    asm volatile ("# Switch end" : : "r"(ret));
    return ret;
}

/* Function 4: Mixed operations with pointer chasing
 * Creates memory dependency chains */
int test_pointer_chasing(int n, int *base) {
    int *ptr = base;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Pointer arithmetic with volatile */
        ptr = base + (i * 2) % 256;
        
        /* Load-store chain */
        int val = *ptr;
        val += g_volatile_counter;
        *ptr = val;
        
        /* Complex asm with memory input/output */
        asm volatile ("# Pointer chase iteration %0" 
                     : "+m"(*ptr) 
                     : "r"(val), "r"(ptr));
        
        sum += val;
        
        /* Conditional that can't be easily predicted */
        if ((i ^ val) % 7 == 0) {
            asm volatile ("# Unpredictable branch taken" : : : "memory");
            g_volatile_array[i % 256] = val;
        }
    }
    
    return sum;
}

/* Function 5: Recursive-like pattern with tail operations */
int test_tail_operations(int n, int *buf) {
    int acc = 0;
    
    while (n > 0) {
        /* Multiple operations in loop tail */
        buf[n] = n * acc;
        
        /* Asm with early clobber */
        asm volatile ("# Tail operation %0" 
                     : "=&r"(acc) 
                     : "r"(n), "r"(buf[n]));
        
        /* Another memory op */
        g_volatile_counter = acc % 256;
        
        /* Complex condition */
        if ((acc ^ n) & 1) {
            asm volatile ("# Odd path" : : "r"(acc));
            n--;
        } else {
            asm volatile ("# Even path" : : "r"(n));
            n -= 2;
        }
    }
    
    return acc;
}

/* Main driver that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int array1[256];
    int matrix[16][16];
    int results[5];
    int buffer[256];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        buffer[i] = 255 - i;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = test_inner_loop(100, array1);
    int result2 = test_nested_loops(8, 8, (int *)matrix);
    int result3 = test_switch_complex(argc > 1 ? atoi(argv[1]) % 6 : 0, results);
    int result4 = test_pointer_chasing(50, array1);
    int result5 = test_tail_operations(30, buffer);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 
        result1 + result2 + result3 + result4 + result5;
    
    /* Final asm to mark end of compilation unit */
    asm volatile ("# Program end %0" : : "r"(final_result));
    
    return final_result != 0 ? 0 : 1;
}
