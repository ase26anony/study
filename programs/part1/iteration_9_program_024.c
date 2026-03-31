/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa-test haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 50

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0;
    volatile int result = 0;
    volatile int outer_limit = (mode % 7) + 3;  /* Non-constant volatile limit */
    
    /* Outer loop with volatile bound - forces scheduler state management */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (iter % 5) + 10;  /* Varying inner bounds */
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Create long dependency chain */
            a = arr1[j] * arr2[j] + i;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            b = a ^ (j * 3);
            c = b >> (j % 4);
            
            /* Multiple independent memory operations */
            d = arr1[(j + 1) % ARRAY_SIZE] + arr2[(j + 2) % ARRAY_SIZE];
            e = arr1[(j + 3) % ARRAY_SIZE] - arr2[(j + 4) % ARRAY_SIZE];
            
            __asm__ volatile ("" : : : "memory");  /* Another barrier */
            
            /* Complex multi-way branch creating multiple basic blocks */
            volatile int branch_selector = (a + b + c) % 8;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic operations */
                result = a * b + c * d - e;
                arr1[j] = result ^ 0x55AA55AA;
            } else if (branch_selector == 1) {
                /* Branch 1: Bit manipulation */
                result = (a << 3) | (b >> 2);
                result ^= (c & d) | e;
                arr2[j] = result;
            } else if (branch_selector == 2) {
                /* Branch 2: Memory intensive */
                for (k = 0; k < 3; k++) {
                    arr1[(j + k) % ARRAY_SIZE] += arr2[(j + k + 1) % ARRAY_SIZE];
                }
                result = arr1[j] + arr2[j];
            } else if (branch_selector == 3) {
                /* Branch 3: Function call - adds call instruction to schedule */
                if ((iter + j) % 17 == 0) {
                    volatile int pid = getpid();
                    result = pid & 0xFF;
                } else {
                    result = clock() & 0xFF;
                }
                arr1[j] ^= result;
            } else if (branch_selector == 4) {
                /* Branch 4: Nested conditionals */
                if (a > b) {
                    result = a - b;
                    if (c > d) {
                        result += c - d;
                    }
                } else {
                    result = b - a;
                    if (e != 0) {
                        result /= (e & 0xF) + 1;
                    }
                }
                arr2[j] = result;
            } else if (branch_selector == 5) {
                /* Branch 5: More complex arithmetic */
                result = (a * a + b * b) % 1024;
                result = (result * c) / ((d & 0x1F) + 1);
                arr1[j] = result;
            } else if (branch_selector == 6) {
                /* Branch 6: Mixed operations with barrier */
                result = arr1[j] * arr2[j];
                __asm__ volatile ("" : : : "memory");
                result += arr1[(j + 5) % ARRAY_SIZE];
                result -= arr2[(j + 6) % ARRAY_SIZE];
                arr2[j] = result;
            } else { /* branch_selector == 7 */
                /* Branch 7: Switch-like behavior using bit tests */
                result = 0;
                for (k = 0; k < 4; k++) {
                    if (a & (1 << k)) {
                        result += arr1[(j + k) % ARRAY_SIZE];
                    }
                    if (b & (1 << k)) {
                        result -= arr2[(j + k) % ARRAY_SIZE];
                    }
                }
                arr1[j] = result;
            }
            
            /* Another memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Additional independent operations to fill instruction queue */
            volatile int tmp1 = arr1[(j + 7) % ARRAY_SIZE] ^ 0x12345678;
            volatile int tmp2 = arr2[(j + 8) % ARRAY_SIZE] & 0x87654321;
            volatile int tmp3 = tmp1 * tmp2 + (j % 256);
            
            /* Store results back with varying patterns */
            if ((i + j) % 3 == 0) {
                arr1[(j + 9) % ARRAY_SIZE] = tmp3;
            } else if ((i + j) % 3 == 1) {
                arr2[(j + 10) % ARRAY_SIZE] = tmp3 ^ arr1[j];
            } else {
                arr1[(j + 11) % ARRAY_SIZE] = tmp3 + arr2[j];
            }
        }
        
        /* Occasionally add another barrier between outer loop iterations */
        if (i % 2 == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final computation mixing all accumulated values */
    volatile int final_result = 0;
    for (i = 0; i < 16; i++) {
        final_result ^= arr1[i] + arr2[i];
    }
    
    return final_result;
}

/* Secondary function to create different calling contexts */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                          volatile int seed) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (seed % 15) + 5;
    
    for (i = 0; i < limit; i++) {
        /* Different pattern to create alternative scheduling context */
        for (j = 0; j < ARRAY_SIZE / 4; j++) {
            volatile int idx = (i * 17 + j * 3) % ARRAY_SIZE;
            
            /* Complex expression with many intermediate values */
            volatile int x = arr1[idx];
            volatile int y = arr2[idx];
            volatile int z = arr1[(idx + 13) % ARRAY_SIZE];
            
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-operation dependency chain */
            volatile int t1 = x * y + z;
            volatile int t2 = (x ^ y) | (z & 0xFF);
            volatile int t3 = t1 << (t2 % 8);
            volatile int t4 = t3 - (y * 2);
            
            /* Conditional store with barrier */
            if (t4 > 0) {
                arr1[idx] = t4;
                __asm__ volatile ("" : : : "memory");
                arr2[idx] = t4 ^ 0xAA55AA55;
            } else {
                arr1[idx] = -t4;
                arr2[idx] = t4 & 0x55AA55AA;
            }
            
            acc += t4;
        }
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
    
    /* Initialize arrays with pseudo-random but complex patterns */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        
        /* Create some inter-dependencies */
        if (i > 0) {
            array1[i] ^= array2[i-1];
            array2[i] += array1[i-1] % 256;
        }
    }
    
    /* Main loop: repeatedly call scheduling functions to create multiple contexts */
    for (i = 0; i < 8; i++) {
        volatile int mode = (i * 7) % 11;  /* Varying mode */
        volatile int iter = i;
        
        /* Call core scheduling function - may create scheduling context */
        volatile int result1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Occasionally call alternate pattern */
        if (i % 3 == 0) {
            volatile int result2 = alternate_schedule_pattern(array1, array2, i);
            checksum ^= result2;
        }
        
        checksum ^= result1;
        
        /* Modify arrays between calls to change scheduling context */
        for (j = 0; j < ARRAY_SIZE; j += 8) {
            array1[j] ^= checksum;
            array2[j] += (i * j) % 256;
        }
        
        /* Memory barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final checksum computation to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
