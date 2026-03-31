/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 100

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int result = 0;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile limit to prevent optimization */
    volatile int inner_limit = (iter % 5) + 5;
    
    /* Create multiple basic blocks with complex control flow */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (arr1[i] ^ arr2[i]) & 0xF;
        
        /* Long dependency chain with memory barriers */
        int a = arr1[i * 3];
        int b = arr2[i * 3 + 1];
        int c = arr1[i * 3 + 2];
        
        /* First dependency chain */
        a = a * b + c;
        __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
        b = (a ^ arr2[i]) >> 2;
        c = b * a - arr1[i];
        
        /* Independent operations to fill instruction queue */
        int d = arr2[i] * arr1[i + 1];
        int e = arr1[i + 2] ^ arr2[i + 2];
        int f = d + e;
        
        __asm__ volatile ("" : : : "memory");  /* Another barrier */
        
        /* Multi-way branch creating multiple basic blocks */
        if (branch_selector & 0x1) {
            /* Branch 1: Arithmetic operations */
            a = a + (b << 2);
            b = b ^ (c * 3);
            __asm__ volatile ("" : : : "memory");
            arr1[i] = a + b;
        } else if (branch_selector & 0x2) {
            /* Branch 2: Memory intensive */
            for (k = 0; k < 3; k++) {
                arr2[i + k] = arr1[i + k] * arr2[i + k];
            }
            __asm__ volatile ("" : : : "memory");
        } else if (branch_selector & 0x4) {
            /* Branch 3: Function call (adds call instruction to schedule) */
            if (mode & 0x1) {
                volatile int pid = getpid();
                arr1[i] ^= pid & 0xFF;
            }
        } else if (branch_selector & 0x8) {
            /* Branch 4: Complex dependency chain */
            int x = arr1[i];
            for (j = 0; j < inner_limit; j++) {
                x = x * 1103515245 + 12345;
                x = (x >> 16) & 0x7FFF;
                __asm__ volatile ("" : : : "memory");
            }
            arr2[i] = x;
        }
        
        /* More independent operations */
        int g = arr1[(i * 7) % ARRAY_SIZE];
        int h = arr2[(i * 11) % ARRAY_SIZE];
        int m = g * h - f;
        int n = (g ^ h) | m;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Switch-like structure compiled to jump table */
        volatile int case_val = arr1[i] % 4;
        switch (case_val) {
            case 0:
                result += a * 2;
                break;
            case 1:
                result += b + c;
                break;
            case 2:
                result += d - e;
                /* Additional dummy operation */
                arr1[i] = result & 0xFF;
                break;
            case 3:
                result += f ^ g;
                if (mode & 0x2) {
                    volatile int clk = clock() & 0xFF;
                    result ^= clk;
                }
                break;
        }
        
        /* Final dependency chain with barrier */
        arr2[i] = result + arr1[i];
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int depth) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (depth % 10) + 5;
    
    for (i = 0; i < limit; i++) {
        /* Create instruction pressure with many independent vars */
        int v1 = arr1[i];
        int v2 = arr2[i];
        int v3 = arr1[i + 1];
        int v4 = arr2[i + 1];
        int v5 = arr1[i + 2];
        int v6 = arr2[i + 2];
        
        /* Parallel computation chains */
        int chain1 = v1 * v2 + v3;
        int chain2 = v4 ^ v5 | v6;
        int chain3 = (v1 + v2) * (v3 - v4);
        int chain4 = v5 << (v6 & 0x3);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Interdependent operations */
        chain1 = chain1 ^ chain2;
        chain2 = chain2 + chain3;
        chain3 = chain3 * chain4;
        chain4 = chain4 - chain1;
        
        /* Conditional store with barrier */
        if ((i ^ depth) & 0x1) {
            arr1[i] = chain1;
            __asm__ volatile ("" : : : "memory");
            arr2[i] = chain2;
        } else {
            arr1[i] = chain3;
            arr2[i] = chain4;
        }
        
        /* Accumulate with varying operations */
        acc += chain1 - chain2 + chain3 ^ chain4;
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 2; j++) {
            volatile int temp = arr1[i + j] * arr2[i + j];
            acc ^= temp;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int checksum = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        volatile int outer_bound = (iter % 3) + 2;  /* Volatile-like behavior */
        
        /* Alternate between different scheduling patterns */
        if (iter & 0x1) {
            volatile int result = complex_schedule_loop(array1, array2, 
                                                       mode_switch, iter);
            checksum ^= result;
            __asm__ volatile ("" : : : "memory");
        } else {
            volatile int result = alternate_schedule_pattern(array1, array2, 
                                                            iter);
            checksum += result;
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Modify mode to change scheduling behavior */
        mode_switch = (mode_switch + iter) % 7;
        
        /* Occasionally introduce function call in main loop */
        if (iter % 3 == 0) {
            volatile int dummy = getpid();
            checksum ^= (dummy & 0xFF);
        }
    }
    
    /* Final computation to use all results */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= array1[i];
        final_sum += array2[i];
    }
    
    final_sum ^= checksum;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_sum);
    
    return 0;
}
