/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa-trigger haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 128

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int i, j, k;
    volatile int a, b, c, d, e, f, g, h;
    volatile int temp_results[8];
    volatile int branch_selector;
    volatile int memory_barrier_counter = 0;
    
    /* Outer loop with volatile bound to prevent compile-time optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Create scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Initialize some volatile working variables */
        a = arr1[i] ^ arr2[i];
        b = arr1[(i + 1) % ARRAY_SIZE];
        c = arr2[(i + 2) % ARRAY_SIZE];
        
        /* Inner loop with complex dependency chains */
        for (j = 0; j < (mode_flag & 0x3F); j++) {
            /* Long dependency chain 1 */
            d = a * b + c;
            e = d ^ (arr1[j % ARRAY_SIZE]);
            f = e >> (j & 0x7);
            g = f * arr2[(i + j) % ARRAY_SIZE];
            h = g - arr1[(i * j) % ARRAY_SIZE];
            
            /* Memory barrier to split scheduling regions */
            if ((j & 0x7) == 0) {
                __asm__ volatile ("" : : : "memory");
                memory_barrier_counter++;
            }
            
            /* Multiple independent operations to fill instruction queue */
            temp_results[0] = arr1[j] + arr2[255 - j];
            temp_results[1] = arr1[j] * arr2[j];
            temp_results[2] = arr1[j] ^ arr2[j];
            temp_results[3] = arr1[j] - arr2[j];
            
            /* Complex multi-way branch structure */
            branch_selector = (i * j + mode_flag) & 0xF;
            
            /* Branch 0: Arithmetic intensive */
            if (branch_selector == 0) {
                for (k = 0; k < 4; k++) {
                    temp_results[k] = temp_results[k] * 3 + 7;
                    temp_results[k] = temp_results[k] ^ 0x55AA55AA;
                    temp_results[k] = temp_results[k] >> 1;
                }
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 1: Memory intensive */
            else if (branch_selector == 1) {
                arr1[(i + j) % ARRAY_SIZE] = temp_results[0];
                arr2[(i + j) % ARRAY_SIZE] = temp_results[1];
                arr1[(i + j + 1) % ARRAY_SIZE] = temp_results[2];
                arr2[(i + j + 1) % ARRAY_SIZE] = temp_results[3];
            }
            /* Branch 2: Function call */
            else if (branch_selector == 2) {
                if ((memory_barrier_counter & 1) == 0) {
                    volatile int pid = getpid();
                    temp_results[4] = pid & 0xFF;
                }
            }
            /* Branch 3: More arithmetic with barriers */
            else if (branch_selector == 3) {
                temp_results[0] = (temp_results[0] << 3) | (temp_results[1] >> 5);
                __asm__ volatile ("" : : : "memory");
                temp_results[1] = (temp_results[1] << 3) | (temp_results[2] >> 5);
                temp_results[2] = (temp_results[2] << 3) | (temp_results[3] >> 5);
                __asm__ volatile ("" : : : "memory");
            }
            /* Default branch: Mixed operations */
            else {
                temp_results[5] = arr1[branch_selector % ARRAY_SIZE] + 
                                 arr2[(branch_selector * 3) % ARRAY_SIZE];
                temp_results[6] = temp_results[5] * arr1[(branch_selector + 7) % ARRAY_SIZE];
                temp_results[7] = temp_results[6] ^ arr2[(branch_selector + 11) % ARRAY_SIZE];
                
                if ((branch_selector & 1) == 0) {
                    __asm__ volatile ("" : : : "memory");
                }
            }
            
            /* Update array elements with computed values */
            arr1[i] = (arr1[i] + h) & 0x7FFFFFFF;
            arr2[i] = (arr2[i] ^ temp_results[branch_selector & 0x7]) & 0x7FFFFFFF;
            
            /* Another scheduling barrier */
            if ((j & 0x3) == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Switch-like structure using bitwise conditions */
        volatile int switch_var = arr1[i] & 0x7;
        if (switch_var & 0x1) {
            arr1[i] = arr1[i] * 3 + 1;
            __asm__ volatile ("" : : : "memory");
        }
        if (switch_var & 0x2) {
            arr2[i] = arr2[i] ^ arr1[(i + 1) % ARRAY_SIZE];
        }
        if (switch_var & 0x4) {
            volatile int clock_val = clock() & 0xFF;
            arr1[i] = arr1[i] + clock_val;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Compute and return a volatile result */
    volatile int result = 0;
    for (i = 0; i < 8; i++) {
        result ^= temp_results[i];
    }
    return result ^ memory_barrier_counter;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int size, volatile int iterations) {
    volatile int i, j;
    volatile int acc = 0;
    
    for (i = 0; i < iterations; i++) {
        volatile int limit = (size * (i + 1)) / iterations;
        
        for (j = 0; j < limit; j++) {
            /* Complex dependency web */
            volatile int x = arr[j];
            volatile int y = arr[(j + 1) % size];
            volatile int z = arr[(j + 2) % size];
            
            /* Multiple independent chains */
            volatile int chain1 = (x * y) + z;
            volatile int chain2 = (x ^ y) | z;
            volatile int chain3 = (x + y) * z;
            volatile int chain4 = (x - y) ^ z;
            
            /* Interleave with barriers */
            __asm__ volatile ("" : : : "memory");
            
            /* Use results in conditional updates */
            if ((chain1 & 1) == 0) {
                arr[j] = chain2;
                __asm__ volatile ("" : : : "memory");
            } else {
                arr[j] = chain3;
            }
            
            if ((chain4 & 2) == 0) {
                arr[(j + 1) % size] = chain1;
            } else {
                arr[(j + 1) % size] = chain4;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Accumulate for return value */
            acc ^= chain1 ^ chain2 ^ chain3 ^ chain4;
        }
        
        /* Force state save possibility with varying inner loop */
        if ((i & 0x3) == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

int main() {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Volatile loop bounds and mode flags */
    volatile int outer_bound = MAX_LOOP_BOUND;
    volatile int mode1 = 1, mode2 = 2, mode3 = 3;
    volatile int iter_count = 8;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() & 0x7FFF;
        array2[i] = rand() & 0x7FFF;
    }
    
    volatile int total_result = 0;
    
    /* Multiple calls to create different scheduling contexts */
    for (int call_num = 0; call_num < iter_count; call_num++) {
        volatile int current_mode;
        
        /* Vary parameters to create different scheduling scenarios */
        switch (call_num & 0x3) {
            case 0:
                current_mode = mode1;
                outer_bound = MAX_LOOP_BOUND / 2;
                break;
            case 1:
                current_mode = mode2;
                outer_bound = MAX_LOOP_BOUND;
                break;
            case 2:
                current_mode = mode3;
                outer_bound = MAX_LOOP_BOUND * 3 / 4;
                break;
            default:
                current_mode = (mode1 ^ mode2 ^ mode3);
                outer_bound = MAX_LOOP_BOUND - call_num;
                break;
        }
        
        /* Call primary scheduling function */
        volatile int result1 = complex_schedule_loop(array1, array2, 
                                                    outer_bound, current_mode);
        
        /* Call secondary function */
        volatile int size = ARRAY_SIZE - call_num * 16;
        if (size < 64) size = 64;
        volatile int result2 = secondary_schedule_func(array1, size, 
                                                      (call_num + 1) * 2);
        
        total_result ^= result1 ^ result2;
        
        /* Modify arrays between calls */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            array1[i] = (array1[i] * 1103515245 + 12345) & 0x7FFFFFFF;
            array2[i] = (array2[i] * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= total_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
