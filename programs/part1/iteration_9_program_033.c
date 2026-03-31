/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-sched-trigger
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
    volatile int a, b, c, d, e, f, g, h;
    volatile int result = 0;
    volatile int outer_limit = (iter % 7) + 3;  /* Volatile limit to prevent optimization */
    
    /* Volatile array indices to prevent compile-time analysis */
    volatile int idx1 = iter % ARRAY_SIZE;
    volatile int idx2 = (iter * 17) % ARRAY_SIZE;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < outer_limit; i++) {
        /* Create multiple basic blocks with complex control flow */
        if (mode & 0x1) {
            /* Branch 1: Arithmetic dependency chain */
            a = arr1[idx1] * 3 + 7;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            b = a ^ arr2[idx2];
            c = b >> (i & 0x3);
            d = c * 11 - 5;
            
            /* Memory operations mixed with computation */
            arr1[(idx1 + i) % ARRAY_SIZE] = d;
            __asm__ volatile ("" : : : "memory");
            
            e = arr2[(idx2 + i) % ARRAY_SIZE] + d;
            f = e * e - 2 * e + 1;
        } else {
            /* Branch 2: Different arithmetic pattern */
            a = arr1[idx2] / 2 + 1;
            b = arr2[idx1] * a;
            __asm__ volatile ("" : : : "memory");
            
            /* Nested if-else to create more basic blocks */
            if (i & 0x2) {
                c = b << 1;
                d = c ^ 0xAAAAAAAA;
            } else {
                c = b >> 1;
                d = c | 0x55555555;
            }
            
            arr2[(idx2 + i * 3) % ARRAY_SIZE] = d;
            f = d + i;
        }
        
        /* Another scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with pseudo-random iterations */
        volatile int inner_limit = (iter + i) % 5 + 2;
        for (j = 0; j < inner_limit; j++) {
            /* Complex multi-way branch using bitwise conditions */
            int branch_selector = (iter + i + j) & 0x7;
            
            /* Create 8 different basic blocks */
            if (branch_selector & 0x1) {
                g = arr1[(idx1 + j) % ARRAY_SIZE] * 3;
                h = g + arr2[(idx2 + j) % ARRAY_SIZE];
            } else if (branch_selector & 0x2) {
                g = arr1[(idx2 + j) % ARRAY_SIZE] ^ 0xFF;
                h = g - j;
            } else if (branch_selector & 0x4) {
                g = arr2[(idx1 + j) % ARRAY_SIZE] | 0x3C;
                h = g * j;
            } else {
                g = (arr1[idx1] + arr2[idx2]) % 256;
                h = g << (j & 0x3);
            }
            
            /* Mix in a function call in some branches */
            if ((branch_selector == 3) && (mode & 0x2)) {
                /* Dummy system call to add call instruction to scheduling mix */
                volatile int pid = getpid();
                h ^= (pid & 0xFF);
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Long dependency chain */
            volatile int x = h;
            for (k = 0; k < 3; k++) {
                x = x * 97 + 13;
                x = x ^ (x >> 3);
                x = x + k;
            }
            
            /* Store result with memory barrier */
            arr1[(idx1 + i + j) % ARRAY_SIZE] = x;
            __asm__ volatile ("" : : : "memory");
            
            /* More independent operations to fill instruction queue */
            volatile int y = arr2[(idx2 + j * 2) % ARRAY_SIZE];
            volatile int z = y * y - 2 * y + x;
            arr2[(idx2 + i * 2 + j) % ARRAY_SIZE] = z;
            
            result += x + z;
        }
        
        /* Switch-like structure compiled to jump table */
        switch (i & 0x3) {
            case 0:
                arr1[idx1] = result & 0xFF;
                break;
            case 1:
                arr2[idx2] = (result >> 8) & 0xFF;
                break;
            case 2:
                arr1[idx2] = (result >> 16) & 0xFF;
                break;
            case 3:
                arr2[idx1] = (result >> 24) & 0xFF;
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                break;
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
another_scheduling_function(volatile int *arr, volatile int seed) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (seed % 10) + 5;
    
    for (i = 0; i < limit; i++) {
        /* Create register pressure with many live variables */
        volatile int r1 = arr[i % ARRAY_SIZE];
        volatile int r2 = arr[(i + 1) % ARRAY_SIZE];
        volatile int r3 = arr[(i + 2) % ARRAY_SIZE];
        volatile int r4 = arr[(i + 3) % ARRAY_SIZE];
        volatile int r5 = arr[(i + 4) % ARRAY_SIZE];
        
        /* Complex computation with dependencies */
        for (j = 0; j < 4; j++) {
            r1 = r1 * r2 + r3;
            r2 = r2 ^ r4;
            r3 = r3 + r5 * j;
            r4 = r4 - r1;
            r5 = r5 | r2;
            
            /* Scheduling barrier every few operations */
            if (j & 0x1) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Store results back */
        arr[i % ARRAY_SIZE] = r1 + r2 + r3 + r4 + r5;
        acc += arr[i % ARRAY_SIZE];
    }
    
    return acc;
}

int main() {
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
    
    volatile int total_result = 0;
    volatile int mode = 1;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        /* Vary mode to trigger different scheduling paths */
        mode = (mode * 3 + 1) & 0x3;
        
        /* Call complex scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        
        /* Alternate between different volatile conditions */
        if (iter & 0x1) {
            volatile int res2 = another_scheduling_function(array1, iter);
            total_result += res1 + res2;
        } else {
            volatile int res2 = another_scheduling_function(array2, iter);
            total_result += res1 - res2;
        }
        
        /* Modify arrays between calls to create new scheduling scenarios */
        for (int i = 0; i < ARRAY_SIZE / 4; i++) {
            int idx = (iter * 31 + i * 7) % ARRAY_SIZE;
            array1[idx] = (array1[idx] * 3 + 1) & 0x7FF;
            array2[idx] = (array2[idx] * 5 - 2) & 0x7FF;
        }
        
        /* Memory barrier between iterations */
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
