/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o trigger haifa-sched-trigger.c
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Non-constant, volatile bound */
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with complex operations */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Long dependency chain 1 */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            g = e >> (i & 7);
            
            /* Memory access pattern */
            arr1[i] = arr1[i] * g + arr2[(i + 1) % ARRAY_SIZE];
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way branch creating multiple basic blocks */
            volatile int branch_selector = (arr1[i] + iter) % 5;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic operations */
                b = (a * c) ^ (d << 2);
                c = b - arr2[i];
                arr2[i] = c * 0x5A5A5A5A;
            } 
            else if (branch_selector == 1) {
                /* Branch 1: Different arithmetic */
                d = (e | f) & 0x0F0F0F0F;
                f = d * 3 + arr1[(i + 2) % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 2) {
                /* Branch 2: Memory intensive */
                for (volatile int j = 0; j < 2; j++) {
                    arr1[(i + j) % ARRAY_SIZE] += arr2[(i - j + ARRAY_SIZE) % ARRAY_SIZE];
                }
                a = arr1[i] ^ arr2[i];
            }
            else if (branch_selector == 3) {
                /* Branch 3: Function call (scheduling complexity) */
                if ((iter & 1) && (i % 16 == 0)) {
                    volatile int pid = getpid();
                    arr1[i] ^= pid & 0xFF;
                }
                b = (b << 1) | (c & 1);
            }
            else {
                /* Branch 4: Complex dependency chain */
                volatile int t1 = arr1[i] * 0x9E3779B9;
                volatile int t2 = arr2[i] * 0x6A09E667;
                __asm__ volatile ("" : : : "memory");
                arr1[i] = (t1 ^ t2) + (a * b);
                arr2[i] = (t1 & t2) | (c * d);
                a = b = c = d = e = f = arr1[i] % 256;
            }
            
            /* Additional independent operations to fill instruction queue */
            volatile int x = arr1[i] * 3;
            volatile int y = arr2[i] / 2;
            volatile int z = x ^ y;
            
            /* Switch-like structure compiled to jump table */
            switch (z & 3) {
                case 0:
                    result += x * y;
                    break;
                case 1:
                    result -= x | y;
                    break;
                case 2:
                    result ^= x & y;
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 3:
                    result = (result << 3) | (z & 7);
                    break;
            }
            
            /* More memory operations with barriers */
            if ((i % 8) == 0) {
                arr2[i] = result;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Mode-dependent operations */
        if (mode & 1) {
            /* Additional loop with different characteristics */
            for (volatile int k = 0; k < (iter % 4 + 1); k++) {
                volatile int tmp = arr1[outer % ARRAY_SIZE];
                arr1[outer % ARRAY_SIZE] = arr2[(outer + k) % ARRAY_SIZE];
                arr2[(outer + k) % ARRAY_SIZE] = tmp;
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, volatile int seed) {
    volatile int acc = seed;
    volatile int limit = (seed % 7) + 3;
    
    for (volatile int phase = 0; phase < limit; phase++) {
        /* Bit manipulation loop */
        for (int i = 0; i < ARRAY_SIZE; i += 2) {
            /* Create instruction-level parallelism */
            volatile int p1 = arr1[i] * arr2[i + 1];
            volatile int p2 = arr1[i + 1] * arr2[i];
            __asm__ volatile ("" : : : "memory");
            
            /* Conditional execution paths */
            if (p1 > p2) {
                arr1[i] = p1 - p2;
                for (volatile int j = 0; j < 2; j++) {
                    arr2[i + j] += (phase << j);
                }
            } else {
                arr1[i + 1] = p2 - p1;
                arr2[i] ^= arr1[i];
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Complex condition chain */
            volatile int cond = (i * phase) & 0xF;
            if (cond & 1) acc += arr1[i];
            if (cond & 2) acc -= arr2[i];
            if (cond & 4) acc ^= arr1[i + 1];
            if (cond & 8) acc |= arr2[i + 1];
        }
    }
    
    return acc;
}

int main() {
    /* Seed for deterministic but complex behavior */
    srand(0xDEADBEEF);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int checksum = 0;
    volatile int mode_flags = 1;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (int iter = 0; iter < MAX_LOOP_ITER; iter++) {
        volatile int mode = (iter & 3) | mode_flags;
        
        /* Call complex scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Alternate between patterns */
        if (iter & 1) {
            volatile int res2 = alternate_schedule_pattern(array2, array1, iter);
            checksum ^= res1 + res2;
        } else {
            checksum ^= res1 * 0x12345678;
        }
        
        /* Modify mode flags to change behavior */
        mode_flags = (mode_flags << 1) | (iter & 1);
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum ^= array1[i];
        final_checksum ^= array2[i];
    }
    final_checksum ^= checksum;
    
    printf("Final checksum: %d\n", (int)final_checksum);
    
    return 0;
}
