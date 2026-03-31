/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int a = 0, b = 0, c = 0, d = 0;
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Non-constant, volatile limit */
    
    /* Outer loop with volatile limit to prevent compile-time analysis */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (mode + i) % 7 + 3;
        
        /* Inner loop with complex operations */
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain 1 */
            a = arr1[(i * 17 + j) % ARRAY_SIZE];
            b = arr2[(j * 13 + i) % ARRAY_SIZE];
            c = a * b + (i ^ j);
            d = c ^ (a >> 2) ^ (b << 3);
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Multiple independent operations to fill instruction queue */
            arr1[(i + j * 5) % ARRAY_SIZE] = d + (a & b);
            arr2[(j + i * 7) % ARRAY_SIZE] = (d * 3) ^ (c % 17);
            
            /* Complex if-else chain creating multiple basic blocks */
            volatile int branch_selector = (a + b + c + d) % 5;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic operations */
                int t1 = a * 3 - b / 2;
                int t2 = c ^ d;
                result += t1 * t2;
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector == 1) {
                /* Branch 1: More arithmetic with memory access */
                arr1[(i * 11) % ARRAY_SIZE] = (b << 2) | (c >> 3);
                result -= arr2[(j * 19) % ARRAY_SIZE] * 7;
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 2) {
                /* Branch 2: Function call to add complexity */
                if ((iter + j) % 7 == 0) {
                    volatile int pid = getpid();
                    result ^= pid & 0xFF;
                }
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 3) {
                /* Branch 3: Nested operations */
                for (k = 0; k < 3; k++) {
                    int temp = (a << k) ^ (b >> k);
                    result += temp * (k + 1);
                }
                __asm__ volatile ("" : : : "memory");
            }
            else {
                /* Branch 4: Mixed operations */
                result = (result * 31) ^ d;
                arr2[(i + j) % ARRAY_SIZE] = result;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Another dependency chain */
            a = (a + 1) * 1103515245 + 12345;
            b = (b ^ a) * 1664525 + 1013904223;
            c = c + (a >> 16) - (b & 0xFFFF);
            
            /* Final memory barrier */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Switch-like structure using bit tests */
        volatile int switch_val = result & 0xF;
        for (int bit = 0; bit < 4; bit++) {
            if (switch_val & (1 << bit)) {
                /* Each bit test creates a basic block */
                volatile int idx = (i * 13 + bit * 7) % ARRAY_SIZE;
                arr1[idx] = arr1[idx] ^ result;
                arr2[idx] = arr2[idx] + bit;
            }
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int seed) {
    volatile int sum = 0;
    volatile int limit = (seed % 5) + 4;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Complex addressing patterns */
        int idx1 = (i * 19 + seed) % ARRAY_SIZE;
        int idx2 = (i * 23 + seed * 3) % ARRAY_SIZE;
        
        /* Operation chain */
        int val1 = arr[idx1];
        int val2 = arr[idx2];
        int prod = val1 * val2;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Multiple condition checks */
        if (prod & 1) {
            sum += (val1 << 2) | (val2 >> 2);
        } else if (prod & 2) {
            sum -= val1 ^ val2;
        } else {
            sum ^= prod;
        }
        
        /* Memory store with barrier */
        arr[(i * 29) % ARRAY_SIZE] = sum;
        __asm__ volatile ("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (int iter = 0; iter < MAX_LOOP_ITER; iter++) {
        volatile int outer_mode = (iter * 17) % 11;
        
        /* Call core scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, outer_mode, iter);
        
        /* Alternate between different modes */
        if (iter % 2 == 0) {
            volatile int res2 = secondary_schedule_func(array1, iter);
            checksum ^= res1 + res2;
        } else {
            volatile int res3 = secondary_schedule_func(array2, iter * 3);
            checksum ^= res1 * res3;
        }
        
        /* Modify mode for next iteration */
        mode_switch = (mode_switch + iter) % 7;
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= array1[i];
        final_sum ^= array2[i];
    }
    
    final_sum ^= checksum;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_sum);
    
    return 0;
}
