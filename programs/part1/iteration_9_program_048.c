/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize haifa-sched-trigger.c -o haifa-sched-trigger
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
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0;
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Volatile-like variation */
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (mode + i) % 7 + 10;
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Create instruction pressure with long dependency chains */
            a = arr1[j] * arr2[j] + i;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            b = a ^ arr1[(j + 1) % ARRAY_SIZE];
            c = b >> (j % 8);
            
            /* Multiple independent memory operations */
            d = arr2[(j + 3) % ARRAY_SIZE] - arr1[(j + 2) % ARRAY_SIZE];
            e = d * c + arr1[j];
            
            __asm__ volatile ("" : : : "memory");  /* Another barrier */
            
            /* Complex conditional structure creating multiple basic blocks */
            if (j & 1) {
                /* Branch 1: Arithmetic operations */
                arr1[j] = a + b - c;
                arr2[j] = (d * e) ^ (a << 2);
                
                /* Nested condition */
                if (j & 2) {
                    arr1[j] += (e >> 3);
                    __asm__ volatile ("" : : : "memory");
                } else {
                    arr2[j] -= (c & 0xFF);
                }
            } else if (j & 4) {
                /* Branch 2: Different operations */
                arr1[j] = b * c / (d != 0 ? d : 1);
                arr2[j] = (a ^ b) | (c & d);
                
                /* Memory barrier to split scheduling regions */
                __asm__ volatile ("" : : : "memory");
                
                /* Additional computation */
                if (j & 8) {
                    volatile int temp = arr1[(j + 4) % ARRAY_SIZE];
                    arr1[j] += temp * 3;
                }
            } else if (j & 16) {
                /* Branch 3: More complex operations */
                arr1[j] = (arr1[j] << 3) | (arr2[j] >> 2);
                arr2[j] = arr1[j] ^ arr2[j];
                
                /* Function call in one branch (adds call instruction) */
                if ((j & 32) && (mode & 1)) {
                    volatile pid_t pid = getpid();
                    arr1[j] ^= (pid & 0xFF);
                }
            } else {
                /* Default branch */
                arr1[j] = (a + b + c + d + e) & 0xFFFF;
                arr2[j] = arr1[j] * arr1[j];
            }
            
            /* Switch-like structure using bit checks */
            volatile int selector = arr1[j] & 7;
            for (k = 0; k < 4; k++) {
                if (selector & (1 << k)) {
                    /* Different operations based on bit position */
                    switch (k) {
                        case 0:
                            arr2[j] += (arr1[j] << 1);
                            break;
                        case 1:
                            arr2[j] ^= 0xAAAAAAAA;
                            __asm__ volatile ("" : : : "memory");
                            break;
                        case 2:
                            arr2[j] = (arr2[j] * 3) / 2;
                            break;
                        case 3:
                            arr2[j] |= 0x55555555;
                            /* Another memory barrier */
                            __asm__ volatile ("" : : : "memory");
                            break;
                    }
                }
            }
            
            /* Final computation with memory access pattern */
            if (j % 3 == 0) {
                volatile int idx = (j * 17) % ARRAY_SIZE;
                arr1[idx] = (arr1[idx] + arr2[j]) & 0xFFFFFFFF;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Cross-iteration dependency */
        result += arr1[i % ARRAY_SIZE] - arr2[i % ARRAY_SIZE];
        
        /* Additional scheduling complexity between outer loop iterations */
        if (i % 2 == 0) {
            volatile int temp_sum = 0;
            for (k = 0; k < 4; k++) {
                temp_sum += arr1[(i + k) % ARRAY_SIZE];
            }
            result ^= temp_sum;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
another_scheduling_function(volatile int *arr, volatile int size, volatile int seed) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (seed % 5) + 3;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (size + i) % 20 + 5;
        
        for (j = 0; j < inner; j++) {
            /* Mix of operations */
            volatile int idx = (i * 19 + j * 7) % size;
            volatile int val = arr[idx];
            
            if (j & 1) {
                val = (val << 3) | (val >> 5);  /* Rotate */
                __asm__ volatile ("" : : : "memory");
            } else {
                val = val ^ (val * 3);
            }
            
            /* Complex conditional */
            if ((i + j) & 2) {
                volatile int other_idx = (j * 13) % size;
                val += arr[other_idx];
                arr[other_idx] = val & 0xFF;
            }
            
            arr[idx] = val;
            acc += val;
            
            /* Periodic barrier */
            if (j % 4 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Function call occasionally */
        if (i % 3 == 0) {
            volatile clock_t clk = clock();
            acc ^= (clk & 0xFF);
        }
    }
    
    return acc;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile unsigned long checksum = 0;
    
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create different scheduling contexts */
    for (i = 0; i < MAX_LOOP_ITER; i++) {
        volatile int mode = i % 4;
        volatile int result;
        
        /* Call main scheduling function */
        result = complex_schedule_loop(array1, array2, mode, i);
        
        /* Process result to prevent elimination */
        checksum ^= (unsigned long)result;
        
        /* Call secondary function */
        if (i % 2 == 0) {
            volatile int size = ARRAY_SIZE - (i * 7) % 50;
            volatile int sec_result = another_scheduling_function(array1, size, i);
            checksum += (unsigned long)sec_result;
        }
        
        /* Modify arrays between calls */
        for (j = 0; j < ARRAY_SIZE / 8; j++) {
            volatile int idx = (i * 11 + j * 5) % ARRAY_SIZE;
            array1[idx] = (array1[idx] + i) & 0x7FF;
            array2[idx] = (array2[idx] ^ j) & 0x7FF;
        }
        
        __asm__ volatile ("" : : : "memory");  /* Barrier between iterations */
    }
    
    /* Final computation to use all results */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= (unsigned long)array1[i];
        checksum += (unsigned long)array2[i];
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
