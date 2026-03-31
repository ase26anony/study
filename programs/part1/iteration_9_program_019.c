/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-sched-trigger
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
    volatile int i, j, k;
    volatile int limit = (iter % 7) + 3;  /* Volatile limit to prevent optimization */
    volatile int result = 0;
    volatile int branch_selector;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < limit; i++) {
        /* Create instruction queue pressure with mixed operations */
        volatile int a = arr1[i] * 3 + arr2[i];
        volatile int b = arr2[i] ^ (arr1[i] << 2);
        volatile int c = a + b;
        
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Complex dependency chain */
        for (j = 0; j < (iter % 5) + 2; j++) {
            volatile int temp = c;
            
            /* Multi-way branching to create multiple basic blocks */
            branch_selector = (temp + j) % BRANCH_COUNT;
            
            if (branch_selector == 0) {
                /* Integer arithmetic chain */
                temp = temp * 7 + 13;
                temp = temp ^ (temp >> 3);
                temp = temp * 0x5A827999;
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 1) {
                /* Memory operations */
                arr1[(i + j) % ARRAY_SIZE] = temp + arr2[j];
                arr2[j] = arr1[i] - temp;
                temp = arr1[(i + j) % ARRAY_SIZE] ^ arr2[j];
            } else if (branch_selector == 2) {
                /* Another arithmetic chain */
                temp = (temp << 4) | (temp >> 28);
                temp = temp + 0xDEADBEEF;
                temp = temp * 0x9E3779B9;
            } else if (branch_selector == 3) {
                /* Function call in one branch - adds call instruction */
                if (mode & 1) {
                    volatile int pid = getpid();
                    temp = temp ^ (pid & 0xFF);
                }
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 4) {
                /* Complex bit manipulation */
                temp = ((temp & 0xAAAAAAAA) >> 1) | ((temp & 0x55555555) << 1);
                temp = temp ^ ((temp << 16) | (temp >> 16));
            } else if (branch_selector == 5) {
                /* Memory barrier intensive block */
                __asm__ volatile ("" : : : "memory");
                temp = arr1[i] * arr2[j];
                __asm__ volatile ("" : : : "memory");
                temp = temp + arr1[(i + 1) % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 6) {
                /* Time-based operation */
                if (mode & 2) {
                    volatile clock_t t = clock();
                    temp = temp ^ (t & 0xFFFF);
                }
            } else { /* branch_selector == 7 */
                /* Nested arithmetic with barriers */
                for (k = 0; k < 3; k++) {
                    temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
                    if (k == 1) {
                        __asm__ volatile ("" : : : "memory");
                    }
                }
            }
            
            /* Store result back with another barrier */
            __asm__ volatile ("" : : : "memory");
            arr1[i] = temp;
            c = temp + j;
            
            /* Additional independent operations to fill instruction queue */
            volatile int x = arr2[(i + j + 1) % ARRAY_SIZE];
            volatile int y = x * x + 17;
            volatile int z = y ^ (x << 3);
            arr2[(i + j) % ARRAY_SIZE] = z;
        }
        
        /* Final computation in the outer loop */
        result = result ^ c;
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int rounds) {
    volatile int i, j;
    volatile int acc = 0;
    
    for (i = 0; i < rounds; i++) {
        volatile int base = arr1[i] + arr2[i];
        
        /* Switch-like structure using bit tests (compiles to multiple branches) */
        for (j = 0; j < 4; j++) {
            if (base & (1 << (j * 2))) {
                /* Branch 1: Multiply-accumulate chain */
                volatile int t = base * (j + 2);
                t = t + (t >> 8);
                t = t ^ (t << 7);
                acc += t;
                __asm__ volatile ("" : : : "memory");
            } else if (base & (1 << (j * 2 + 1))) {
                /* Branch 2: Memory intensive */
                arr1[(i + j) % ARRAY_SIZE] = acc;
                arr2[j] = base - acc;
                acc = arr1[(i + j) % ARRAY_SIZE] * 3;
            } else {
                /* Branch 3: Simple arithmetic */
                acc = acc ^ base;
            }
            
            /* Insert barrier every few iterations */
            if (j % 2 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Long dependency chain */
        volatile int chain = base;
        chain = chain * 0xCCCCCCCD;
        chain = chain ^ (chain >> 16);
        chain = chain + acc;
        chain = chain * 0xAAAAAAAB;
        chain = chain ^ arr2[i];
        
        arr1[i] = chain;
        acc = chain;
    }
    
    return acc;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create scheduling contexts across function boundaries */
    for (i = 0; i < 8; i++) {
        volatile int mode = i % 4;
        volatile int iter_limit = (i * 17) % MAX_LOOP_ITER;
        
        /* Call first scheduling pattern */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter_limit);
        
        /* Modify arrays between calls */
        for (j = 0; j < ARRAY_SIZE / 4; j++) {
            array1[(i * 7 + j) % ARRAY_SIZE] = res1 ^ j;
        }
        
        /* Call alternate pattern with different parameters */
        volatile int res2 = alternate_schedule_pattern(array1, array2, 
                                                      (iter_limit % 10) + 5);
        
        /* Mix results */
        checksum = checksum ^ res1 ^ res2;
        
        /* Memory barrier between major iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to use all array elements */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum = checksum ^ array1[i] ^ array2[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
