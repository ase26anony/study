/* 
 * Complex scheduling test to trigger haifa-sched.cc free_sched_context logic
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
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
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int e = 5, f = 6, g = 7, h = 8;
    volatile int result = 0;
    volatile int loop_limit = (iter % 10) + 15;  /* Volatile-like calculation */
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < loop_limit; outer++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner complex computation with multiple basic blocks */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Create long dependency chains */
            a = arr1[i] * b + c;
            __asm__ volatile ("" : : : "memory");
            e = a ^ arr2[i];
            f = e >> (i & 0x7);
            g = f * d - h;
            
            /* Multiple independent memory operations */
            arr2[(i + 1) % ARRAY_SIZE] = g;
            arr1[(i + 2) % ARRAY_SIZE] = arr1[i] + arr2[i];
            
            /* Complex conditional branching creating multiple basic blocks */
            if (mode & 0x01) {
                /* Branch 1: Arithmetic operations */
                h = (h * 1103515245 + 12345) & 0x7fffffff;
                a = b + c * d;
                __asm__ volatile ("" : : : "memory");
            } 
            else if (mode & 0x02) {
                /* Branch 2: Bit manipulation */
                b = (b << 3) | (b >> 29);
                c = c ^ d ^ e;
                arr1[i] = arr1[i] ^ arr2[i];
            }
            else if (mode & 0x04) {
                /* Branch 3: Memory intensive */
                volatile int temp = arr1[(i + 3) % ARRAY_SIZE];
                arr2[(i + 4) % ARRAY_SIZE] = temp * 3;
                d = arr1[i] - arr2[i];
                __asm__ volatile ("" : : : "memory");
            }
            else if (mode & 0x08) {
                /* Branch 4: Function call (scheduling barrier) */
                if ((i & 0x0F) == 0) {
                    volatile int pid = getpid();
                    arr1[i] = arr1[i] ^ (pid & 0xFF);
                }
            }
            else {
                /* Default branch: Mixed operations */
                g = (g * 7 + 11) % 1024;
                h = h ^ g ^ i;
            }
            
            /* Switch-like behavior using bit tests */
            int selector = (i + outer) & 0x07;
            switch (selector) {
                case 0:
                    a = b + c;
                    arr1[i] = arr1[i] * 2;
                    break;
                case 1:
                    b = c - d;
                    arr2[i] = arr2[i] / 3;
                    break;
                case 2:
                    c = d * e;
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 3:
                    d = e ^ f;
                    arr1[i] = arr1[i] | arr2[i];
                    break;
                case 4:
                    e = f & g;
                    /* Another memory barrier */
                    __asm__ volatile ("" : : : "memory");
                    break;
                case 5:
                    f = g | h;
                    arr2[i] = arr2[i] << 2;
                    break;
                case 6:
                    g = h + a;
                    /* Potential function call */
                    if ((i % 16) == 0) {
                        volatile clock_t clk = clock();
                        arr1[i] = arr1[i] ^ (clk & 0xFF);
                    }
                    break;
                case 7:
                    h = a * b;
                    arr1[i] = arr1[i] - arr2[i];
                    break;
            }
            
            /* Final computation with memory barrier */
            result = result + arr1[i] - arr2[i];
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Change mode pseudo-randomly to vary scheduling patterns */
        mode = (mode * 13 + 7) & 0xFF;
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int seed) {
    volatile int x = seed, y = seed * 2, z = seed * 3;
    volatile int sum = 0;
    
    for (volatile int i = 0; i < MAX_LOOP_ITER; i++) {
        /* Create instruction pressure with many operations */
        for (int j = 0; j < ARRAY_SIZE / 4; j++) {
            x = (x * 1664525 + 1013904223) & 0x7fffffff;
            y = (y * 1103515245 + 12345) & 0x7fffffff;
            z = x ^ y ^ z;
            
            /* Multiple memory operations */
            arr[j] = arr[j] + x;
            arr[j + 64] = arr[j + 64] - y;
            arr[j + 128] = arr[j + 128] * z;
            arr[j + 192] = arr[j + 192] ^ (x + y);
            
            /* Dependency chain */
            volatile int t1 = arr[j] >> 3;
            volatile int t2 = t1 * arr[j + 64];
            volatile int t3 = t2 + arr[j + 128];
            volatile int t4 = t3 ^ arr[j + 192];
            
            sum = sum + t4;
            
            /* Memory barrier every 8 iterations */
            if ((j & 0x07) == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Complex conditional with function call */
        if ((i % 7) == 0) {
            volatile int r = rand() % 100;
            if (r > 50) {
                arr[i % ARRAY_SIZE] = getpid() & 0xFF;
            }
        }
    }
    
    return sum;
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
    
    volatile int checksum = 0;
    volatile int mode = 1;
    
    /* Multiple calls to create scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        /* Vary parameters to create different scheduling patterns */
        volatile int result1 = complex_schedule_loop(array1, array2, mode, iter);
        volatile int result2 = secondary_schedule_func(array1, iter * 17 + 5);
        
        checksum = checksum ^ result1 ^ result2;
        
        /* Change mode for next iteration */
        mode = (mode * 3 + 1) & 0x0F;
        
        /* Occasionally reset arrays */
        if (iter % 3 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i += 7) {
                array1[i] = rand() % 500;
                array2[i] = rand() % 500;
            }
        }
    }
    
    /* Final computation to use all results */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum = final_sum + array1[i] - array2[i];
    }
    
    checksum = checksum ^ final_sum;
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
