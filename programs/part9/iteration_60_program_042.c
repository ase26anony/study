/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumps
 * when compiled with appropriate flags. The goal is to reach the uncovered
 * lines in sel-sched-dump.cc that handle RTL instruction dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Function 1: Inner loop with conditional branch and memory writes
 * Creates data dependencies that force the scheduler to work harder
 */
void test_inner_loop(int *arr, int n) {
    volatile int counter = 0; /* Prevent dead code elimination */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Create data dependency chain */
        sum += arr[i];
        
        /* Conditional store with side effect */
        if (sum > 1000) {
            arr[i] = sum % 256;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(sum) : "memory");
        } else {
            arr[i] = i;
        }
        
        /* Another dependency */
        counter += arr[i] & 1;
    }
    
    /* Use result to prevent optimization */
    asm volatile ("" : : "r"(sum), "r"(counter));
}

/* Function 2: Nested loops with different iteration patterns
 * Creates outer loop pipelining opportunities
 */
void test_nested_loops(int *matrix, int rows, int cols) {
    int total = 0;
    volatile int checksum = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple operations with dependencies */
            row_sum += val;
            matrix[idx] = val ^ (i * j);
            
            /* Conditional with unpredictable branch */
            if ((i + j) % 7 == 0) {
                row_sum -= val / 2;
                asm volatile ("" : : "r"(row_sum));
            }
        }
        
        total += row_sum;
        checksum ^= row_sum;
        
        /* Function call to break basic block */
        if (i % 3 == 0) {
            /* Small inline asm barrier */
            asm volatile ("nop" : : : "memory");
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(total), "r"(checksum));
}

/* Function 3: Switch statement with computed goto-like pattern
 * Creates complex control flow for the scheduler
 */
int test_switch_complex(int mode, int iterations) {
    static int state = 0;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with multiple cases creates jump table */
        switch ((mode + i) % 5) {
            case 0:
                result += i * 2;
                /* Memory barrier */
                asm volatile ("" : : : "memory");
                break;
            case 1:
                result -= i / 2;
                /* Complex dependency */
                state = (state * 1103515245 + 12345) & 0x7fffffff;
                break;
            case 2:
                result ^= i;
                /* Another asm to force specific RTL */
                asm volatile ("addl $1, %0" : "+r"(result));
                break;
            case 3:
                result = result >> 1;
                /* Function call simulation */
                if (result < 0) result = -result;
                break;
            case 4:
                result = result * 3 + 1;
                /* Memory operation */
                state = result % 100;
                break;
            default:
                result = 0;
        }
        
        /* Additional conditional */
        if (result > 1000) {
            result %= 1000;
            asm volatile ("" : : "r"(result));
        }
    }
    
    return result + state;
}

/* Function 4: Mixed operations with pointer chasing
 * Creates memory dependencies that are hard to schedule
 */
void test_pointer_chasing(int *data, int size, int stride) {
    int *ptr = data;
    int sum = 0;
    volatile int marker = 0;
    
    for (int i = 0; i < size; i++) {
        /* Pointer arithmetic with dependency */
        int idx = (ptr - data) % size;
        int val = data[idx];
        
        /* Multiple dependent operations */
        sum = sum * 13 + val;
        
        /* Conditional pointer update */
        if (sum % 2 == 0) {
            ptr = data + ((idx + stride) % size);
        } else {
            ptr = data + ((idx - stride + size) % size);
        }
        
        /* Store with dependency */
        data[idx] = sum & 0xFF;
        
        /* Periodic barrier */
        if (i % 8 == 0) {
            marker ^= sum;
            asm volatile ("" : : "r"(marker));
        }
    }
    
    /* Ensure results are used */
    asm volatile ("" : : "r"(sum), "r"(marker));
}

/* Function 5: Loop with early exit and multiple exits
 * Creates control flow with multiple basic blocks
 */
int test_early_exit(int *values, int n, int threshold) {
    int product = 1;
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Early exit condition 1 */
        if (values[i] == 0) {
            product = 0;
            /* Inline asm to prevent optimization */
            asm volatile ("" : : "r"(product));
            break;
        }
        
        product *= values[i];
        count++;
        
        /* Early exit condition 2 */
        if (product > threshold) {
            /* Complex calculation before exit */
            product = (product % threshold) + 1;
            asm volatile ("" : : "r"(product));
            break;
        }
        
        /* Another conditional path */
        if (values[i] < 0) {
            product = -product;
            /* Memory operation */
            values[i] = product % 100;
        }
    }
    
    return product + count;
}

/* Main driver function that ensures all code paths are compiled */
int main(int argc, char **argv) {
    /* Initialize test data */
    int data1[100];
    int data2[10][10];
    int data3[50];
    int data4[64];
    int data5[20];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        data1[i] = (i * 37) % 101;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            data2[i][j] = (i * 11 + j * 13) % 47;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        data3[i] = (i * 19) % 73;
    }
    
    for (int i = 0; i < 64; i++) {
        data4[i] = (i * 23) % 89;
    }
    
    for (int i = 0; i < 20; i++) {
        data5[i] = (i * 29) % 31 - 15;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(data1, 100);
    test_nested_loops((int *)data2, 10, 10);
    int r3 = test_switch_complex(argc > 1 ? atoi(argv[1]) : 2, 25);
    test_pointer_chasing(data3, 50, 7);
    int r5 = test_early_exit(data5, 20, 1000000);
    
    /* Use results to prevent dead code elimination */
    volatile int result = r3 + r5;
    asm volatile ("" : : "r"(result));
    
    /* Print something to ensure execution */
    if (argc > 2) {
        printf("Test completed: %d\n", result);
    }
    
    return 0;
}
