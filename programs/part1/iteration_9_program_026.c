/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
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
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Volatile-like calculation */
    volatile int inner_limit = ARRAY_SIZE / (iter + 1);
    
    /* Create register pressure with many local variables */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Inner loop with complex operations */
        for (volatile int idx = 0; idx < inner_limit; idx++) {
            /* Create long dependency chains */
            a = arr1[idx] * 1103515245 + 12345;
            b = arr2[(idx + 1) % ARRAY_SIZE] ^ a;
            c = b * 1664525 + 1013904223;
            d = c >> (iter % 16);
            e = d * arr1[(idx + 2) % ARRAY_SIZE];
            f = e ^ arr2[(idx + 3) % ARRAY_SIZE];
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way branching to create multiple basic blocks */
            if (mode & 0x01) {
                g = f * 3 + 1;
                /* Another barrier */
                __asm__ volatile ("" : : : "memory");
                h = g ^ 0xAAAAAAAA;
            } else if (mode & 0x02) {
                g = f / 2;
                h = g | 0x55555555;
                /* Call instruction in one branch */
                if ((idx & 0x0F) == 0) {
                    volatile int pid = getpid();
                    h ^= (pid & 0xFF);
                }
            } else if (mode & 0x04) {
                g = f + f;
                h = g & 0x33333333;
                /* Another barrier */
                __asm__ volatile ("" : : : "memory");
            } else if (mode & 0x08) {
                g = f - 100;
                h = g * 7;
                /* Complex calculation chain */
                i = h << 3;
                j = i ^ 0x0F0F0F0F;
                k = j * 13;
                __asm__ volatile ("" : : : "memory");
                h = k % 997;
            } else {
                g = f;
                h = g;
            }
            
            /* More dependency chains */
            i = h * arr1[(idx + 4) % ARRAY_SIZE];
            j = i ^ (arr2[(idx + 5) % ARRAY_SIZE] + iter);
            k = j >> (outer % 8);
            l = k * 214013 + 2531011;
            
            /* Switch-like structure using bit tests */
            volatile int branch_selector = l & 0x07;
            if (branch_selector & 0x01) {
                m = l * 2 + 1;
            } else {
                m = l / 2;
            }
            
            if (branch_selector & 0x02) {
                n = m ^ 0xCCCCCCCC;
            } else {
                n = m | 0x33333333;
            }
            
            if (branch_selector & 0x04) {
                o = n + arr1[(idx + 6) % ARRAY_SIZE];
            } else {
                o = n - arr2[(idx + 7) % ARRAY_SIZE];
            }
            
            /* Final barrier and store */
            __asm__ volatile ("" : : : "memory");
            arr1[idx] = o;
            arr2[idx] = (o ^ iter) + outer;
            
            /* Accumulate result with complex operation */
            p = (o * 1103515245) ^ (arr1[idx] * 1664525);
            result ^= p;
            
            /* Additional memory operations to create scheduling pressure */
            if ((idx % 16) == 0) {
                volatile int temp = arr1[(idx + 8) % ARRAY_SIZE];
                arr2[(idx + 9) % ARRAY_SIZE] = temp ^ result;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Change mode periodically to vary control flow */
        if (outer % 2 == 0) {
            mode ^= (iter & 0x0F);
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int total = 0;
    volatile int limit = (seed % 10) + 3;
    
    for (volatile int cycle = 0; cycle < limit; cycle++) {
        /* Different loop structure */
        for (volatile int i = 0; i < ARRAY_SIZE / 2; i++) {
            /* Create parallel dependency chains */
            volatile int x = arr1[i * 2];
            volatile int y = arr2[i * 2 + 1];
            
            /* Complex arithmetic network */
            volatile int t1 = x * y + seed;
            volatile int t2 = (x ^ y) * 6364136223846793005ULL;
            volatile int t3 = t1 >> (cycle % 16);
            volatile int t4 = t2 << (i % 8);
            
            __asm__ volatile ("" : : : "memory");
            
            /* Conditional execution paths */
            if ((t3 & 0x01) && (t4 & 0x02)) {
                volatile int t5 = t3 * t4;
                arr1[i * 2] = t5 ^ 0xDEADBEEF;
                total += t5;
            } else if (t3 > t4) {
                volatile int t6 = t3 - t4;
                arr2[i * 2 + 1] = t6 | 0xCAFEBABE;
                total ^= t6;
            } else {
                volatile int t7 = t3 + t4;
                arr1[i * 2] = t7 & 0x0F0F0F0F;
                arr2[i * 2 + 1] = t7 >> 4;
                total -= t7;
            }
            
            /* More barriers */
            __asm__ volatile ("" : : : "memory");
            
            /* Occasional system call */
            if ((i % 32) == 0) {
                volatile clock_t clk = clock();
                total ^= (clk & 0xFFFF);
            }
        }
    }
    
    return total;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() ^ (i * 1103515245);
        array2[i] = rand() ^ (i * 1664525);
    }
    
    volatile int final_result = 0;
    volatile int mode_switch = 1;
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int iter = 0; iter < MAX_LOOP_ITER; iter++) {
        /* Vary parameters to create different scheduling contexts */
        volatile int mode = (iter * 1103515245) & 0x0F;
        
        /* Call core scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        final_result ^= res1;
        
        /* Change mode for next iteration */
        mode_switch ^= (iter & 0x03);
        
        /* Call alternate pattern every other iteration */
        if (iter % 2 == 0) {
            volatile int res2 = alternate_schedule_pattern(array1, array2, iter);
            final_result += res2;
        }
        
        /* Modify array contents between calls */
        for (volatile int i = 0; i < ARRAY_SIZE / 8; i++) {
            array1[i * 8] ^= final_result;
            array2[i * 8 + 1] += iter;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= final_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
