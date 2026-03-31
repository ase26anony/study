/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa-trigger haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 50
#define BRANCH_COUNT 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int i, j;
    volatile int outer_limit = (iter % 7) + 3;  /* Volatile-like calculation */
    volatile int inner_limit = (mode % 5) + 10;
    
    /* Volatile array for intermediate results */
    volatile int temp[BRANCH_COUNT * 2];
    
    /* Initialize temp array with pseudo-random values */
    for (i = 0; i < BRANCH_COUNT * 2; i++) {
        temp[i] = (arr1[i % ARRAY_SIZE] ^ arr2[i % ARRAY_SIZE]) + i;
    }
    
    /* Outer loop with volatile limit - forces scheduler context management */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (arr1[i % ARRAY_SIZE] + iter) % BRANCH_COUNT;
        volatile int accum = 0;
        
        /* Inner loop creating instruction pressure */
        for (j = 0; j < inner_limit; j++) {
            volatile int idx = (i * 17 + j * 13) % ARRAY_SIZE;
            volatile int val1 = arr1[idx];
            volatile int val2 = arr2[idx];
            
            /* Long dependency chain with memory barriers */
            volatile int t1 = val1 * val2 + (j << 3);
            __asm__ volatile ("" : : : "memory");
            
            volatile int t2 = t1 ^ (val1 >> 4);
            volatile int t3 = t2 + (val2 & 0xFF);
            __asm__ volatile ("" : : : "memory");
            
            volatile int t4 = t3 * 7 - (t2 >> 1);
            volatile int t5 = (t4 ^ t3) + (val1 % 32);
            
            /* Multi-way branching creating multiple basic blocks */
            switch (branch_selector) {
                case 0:
                    accum += t5 * 3;
                    /* Memory operation with barrier */
                    arr1[(idx + 1) % ARRAY_SIZE] = accum;
                    __asm__ volatile ("" : : : "memory");
                    break;
                    
                case 1:
                    accum ^= t5 >> 2;
                    /* Another dependency chain */
                    volatile int t6 = accum * accum - t4;
                    temp[0] = t6;
                    break;
                    
                case 2:
                    accum = (accum << 3) | (t5 & 0x7);
                    /* Function call in one branch - adds call instruction */
                    if ((iter + j) % 17 == 0) {
                        volatile int pid = getpid();
                        accum ^= pid & 0xFF;
                    }
                    break;
                    
                case 3:
                    accum = accum - t5 + (val2 % 16);
                    /* Complex arithmetic with barrier */
                    __asm__ volatile ("" : : : "memory");
                    for (volatile int k = 0; k < 3; k++) {
                        accum = (accum * 1103515245 + 12345) & 0x7FFFFFFF;
                    }
                    break;
                    
                case 4:
                    /* Nested if-else chain */
                    if (t5 > 1000) {
                        accum += t5 / 3;
                    } else if (t5 > 500) {
                        accum += t5 / 2;
                    } else if (t5 > 100) {
                        accum += t5;
                    } else {
                        accum -= t5;
                    }
                    __asm__ volatile ("" : : : "memory");
                    break;
                    
                case 5:
                    /* Bit manipulation sequence */
                    accum = (accum << 4) ^ t5;
                    accum = (accum >> 1) | (accum << 31);
                    accum ^= 0x9E3779B9;
                    break;
                    
                case 6:
                    /* Memory intensive operations */
                    for (volatile int k = 0; k < 2; k++) {
                        volatile int mem_idx = (idx + k * 7) % ARRAY_SIZE;
                        arr2[mem_idx] = (arr1[mem_idx] + accum) ^ t5;
                        __asm__ volatile ("" : : : "memory");
                    }
                    accum = t5;
                    break;
                    
                case 7:
                    /* Mixed operations with barrier */
                    accum = (t5 * t4) / (val1 % 8 + 1);
                    __asm__ volatile ("" : : : "memory");
                    accum += (val2 << 2) | (val1 >> 6);
                    if (accum & 1) {
                        temp[1] = accum;
                    }
                    break;
                    
                default:
                    accum = t5;
                    break;
            }
            
            /* Additional independent operations to fill instruction queue */
            volatile int indep1 = val1 * val1 - val2 * val2;
            volatile int indep2 = (val1 ^ val2) << (j % 4);
            volatile int indep3 = indep1 + indep2 * 3;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Store results creating more memory dependencies */
            if ((i + j) % 3 == 0) {
                arr1[idx] = accum ^ indep3;
            } else if ((i + j) % 3 == 1) {
                arr2[idx] = accum + indep3;
            } else {
                temp[(i + j) % BRANCH_COUNT] = accum * indep3;
            }
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
        }
        
        result ^= accum;
        
        /* Conditional with volatile check */
        volatile int cond = (iter + i) % 11;
        if (cond == 0) {
            /* Additional scheduling region */
            for (volatile int k = 0; k < 2; k++) {
                volatile int tmp = result * (k + 1);
                temp[k] = tmp;
                result ^= tmp;
            }
        } else if (cond == 3 || cond == 7) {
            /* Different path with more operations */
            result = (result << 1) | (result >> 31);
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final computation with barriers */
    __asm__ volatile ("" : : : "memory");
    for (i = 0; i < BRANCH_COUNT; i++) {
        result += temp[i] * (i + 1);
    }
    __asm__ volatile ("" : : : "memory");
    
    return result;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int final_result = 0;
    
    /* Seed for deterministic behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create scheduling contexts across different invocations */
    for (i = 0; i < 8; i++) {
        volatile int mode = (i * 13) % 5;
        volatile int iter_result = 0;
        
        /* Call core function multiple times with different parameters */
        for (j = 0; j < 3; j++) {
            volatile int call_param = (i * 17 + j * 23) % 11;
            iter_result ^= complex_schedule_loop(array1, array2, mode + j, call_param);
            
            /* Modify arrays between calls to prevent optimization */
            if ((i + j) % 4 == 0) {
                for (volatile int k = 0; k < 10; k++) {
                    int idx = (i * 19 + j * 7 + k) % ARRAY_SIZE;
                    array1[idx] = (array1[idx] + iter_result) ^ 0x5A5A5A5A;
                }
            }
        }
        
        final_result ^= iter_result;
        
        /* Additional operations to maintain scheduling pressure */
        if (i % 3 == 0) {
            volatile int tmp_sum = 0;
            for (j = 0; j < 20; j++) {
                int idx = (i * 23 + j * 7) % ARRAY_SIZE;
                tmp_sum += array1[idx] - array2[idx];
                __asm__ volatile ("" : : : "memory");
            }
            final_result += tmp_sum;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= final_result;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}
