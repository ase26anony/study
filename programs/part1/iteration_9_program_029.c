/* gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_QUEUE_PRESSURE 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int outer_limit = (mode % 5) + 3;  /* Volatile to prevent constant propagation */
    volatile int inner_limit = (iter % 7) + 4;
    
    /* Create multiple basic blocks with complex control flow */
    for (volatile int o = 0; o < outer_limit; o++) {
        volatile int branch_selector = (o * 17 + iter) % 11;
        
        /* First basic block: initial computations */
        volatile int a = arr1[o % ARRAY_SIZE];
        volatile int b = arr2[o % ARRAY_SIZE];
        volatile int c = a * b + iter;
        
        /* Scheduling barrier to split regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Multi-way branch creating several basic blocks */
        if (branch_selector < 3) {
            /* Branch 1: Arithmetic chain with memory ops */
            for (volatile int i = 0; i < inner_limit; i++) {
                volatile int idx = (o + i) % ARRAY_SIZE;
                volatile int temp = arr1[idx] ^ arr2[idx];
                arr1[idx] = temp * 3 + c;
                arr2[idx] = (arr2[idx] >> 2) | (temp << 30);
                c = c ^ (arr1[idx] + i);
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Independent operations to fill instruction queue */
                volatile int d = arr1[(idx + 1) % ARRAY_SIZE];
                volatile int e = arr2[(idx + 2) % ARRAY_SIZE];
                volatile int f = d * e - temp;
                arr1[(idx + 3) % ARRAY_SIZE] = f ^ 0x5A5A5A5A;
                arr2[(idx + 4) % ARRAY_SIZE] = f & 0x0F0F0F0F;
            }
        } 
        else if (branch_selector < 6) {
            /* Branch 2: Different arithmetic pattern with barriers */
            volatile int accum = 0;
            for (volatile int i = 0; i < inner_limit * 2; i++) {
                volatile int idx = (o * 7 + i) % ARRAY_SIZE;
                volatile int x = arr1[idx];
                volatile int y = arr2[idx];
                
                /* Long dependency chain */
                volatile int t1 = x * y;
                volatile int t2 = t1 ^ (x + y);
                volatile int t3 = t2 >> (i % 8);
                volatile int t4 = t3 * 0x9E3779B9;
                volatile int t5 = t4 + accum;
                
                arr1[idx] = t5;
                accum = t5 ^ (accum << 1);
                
                /* Strategic barrier to potentially cause state save */
                if (i % 4 == 0) {
                    __asm__ volatile ("" : : : "memory");
                }
            }
            result ^= accum;
        }
        else if (branch_selector < 9) {
            /* Branch 3: Mix of operations with function call */
            volatile int use_call = (o % 3 == 0);
            for (volatile int i = 0; i < inner_limit; i++) {
                volatile int idx = (o + i * 5) % ARRAY_SIZE;
                
                /* Memory load/store chain */
                volatile int val1 = arr1[idx];
                volatile int val2 = arr2[(idx + 8) % ARRAY_SIZE];
                volatile int prod = val1 * val2;
                
                arr1[idx] = prod + i;
                arr2[(idx + 8) % ARRAY_SIZE] = prod - i;
                
                /* Conditional function call to add complexity */
                if (use_call && (i % 5 == 2)) {
                    volatile int pid = getpid();
                    arr1[(idx + 1) % ARRAY_SIZE] ^= pid & 0xFF;
                }
                
                /* Barrier every few iterations */
                if (i % 3 == 1) {
                    __asm__ volatile ("" : : : "memory");
                }
            }
        }
        else {
            /* Branch 4: Bit manipulation intensive */
            for (volatile int i = 0; i < inner_limit; i++) {
                volatile int idx = (o * 11 + i * 13) % ARRAY_SIZE;
                
                /* Multiple independent operations */
                volatile int bits1 = arr1[idx];
                volatile int bits2 = arr2[idx];
                
                volatile int r1 = (bits1 << 3) | (bits1 >> 29);
                volatile int r2 = bits2 ^ 0xAAAAAAAA;
                volatile int r3 = r1 & r2;
                volatile int r4 = (r3 * 0xCCCCCCCD) >> 3;
                volatile int r5 = r4 | (bits1 & bits2);
                
                arr1[idx] = r5;
                arr2[idx] = r5 ^ bits1 ^ bits2;
                
                /* More barriers to create scheduling regions */
                __asm__ volatile ("" : : : "memory");
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Final computation in each outer iteration */
        volatile int final_idx = o % ARRAY_SIZE;
        volatile int mix = arr1[final_idx] + arr2[final_idx];
        result += mix * o;
        
        /* Barrier before loop continuation */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary function to increase call complexity */
static volatile int __attribute__((noinline))
schedule_helper(volatile int *arr1, volatile int *arr2, volatile int depth) {
    volatile int sum = 0;
    for (volatile int d = 0; d < depth; d++) {
        sum += complex_schedule_loop(arr1, arr2, d, depth);
        
        /* Modify arrays between calls */
        arr1[d % ARRAY_SIZE] ^= sum;
        arr2[d % ARRAY_SIZE] += d;
        
        __asm__ volatile ("" : : : "memory");
    }
    return sum;
}

int main(void) {
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() ^ (i * 17);
        array2[i] = rand() ^ (i * 23);
    }
    
    volatile int total_result = 0;
    volatile int mode_selector = 0;
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int main_iter = 0; main_iter < 8; main_iter++) {
        volatile int mode = (main_iter * 7 + mode_selector) % 5;
        mode_selector = (mode_selector * 13 + 11) % 19;
        
        /* Call core scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, main_iter);
        
        /* Call helper with varying depth */
        volatile int depth = (main_iter % 3) + 2;
        volatile int res2 = schedule_helper(array1, array2, depth);
        
        total_result ^= res1 + res2;
        
        /* Modify arrays between major iterations */
        for (volatile int i = 0; i < 16; i++) {
            int idx = (main_iter * 16 + i) % ARRAY_SIZE;
            array1[idx] = (array1[idx] * 3 + i) & 0x7FFFFFFF;
            array2[idx] ^= array1[idx] >> 4;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= total_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
