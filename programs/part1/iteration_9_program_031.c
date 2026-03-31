/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int i, j;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Complex inner loop with varying operations */
        for (j = 0; j < 128; j++) {
            /* Create long dependency chains */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");
            e = a ^ f;
            f = e >> (j & 7);
            c = d - a + f;
            
            /* Multiple independent memory operations */
            arr1[j] = arr1[j] * a + b;
            arr2[(j + 1) & 0xFF] = arr2[j] ^ c;
            
            /* Complex conditional structure creating multiple basic blocks */
            if (mode_flag & 0x01) {
                /* Branch 1: Arithmetic operations */
                d = (a * b) / (c + 1);
                e = e << 2;
                __asm__ volatile ("" : : : "memory");
            } else if (mode_flag & 0x02) {
                /* Branch 2: Bit manipulation */
                f = (f ^ 0x55AA) | (a & 0xFF);
                b = b ^ arr1[j];
                __asm__ volatile ("" : : : "memory");
            } else if (mode_flag & 0x04) {
                /* Branch 3: Memory intensive */
                arr1[(j + 64) & 0xFF] = arr2[j] + arr1[(j + 32) & 0xFF];
                arr2[j] = arr1[j] - arr2[(j + 16) & 0xFF];
                __asm__ volatile ("" : : : "memory");
            } else if (mode_flag & 0x08) {
                /* Branch 4: Function call (scheduling barrier) */
                if ((j & 15) == 0) {
                    volatile int pid = getpid();
                    arr1[j] ^= pid & 0xFF;
                }
                __asm__ volatile ("" : : : "memory");
            } else {
                /* Default branch: Mixed operations */
                a = (a + b) * (c - d);
                f = f ^ ((e << 3) | (d >> 2));
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Switch-like structure using bit tests */
            volatile int selector = arr1[j] & 0x07;
            if (selector & 0x01) {
                b = b + arr2[j] * 3;
            }
            if (selector & 0x02) {
                c = c ^ (arr1[j] << 1);
            }
            if (selector & 0x04) {
                d = d - (arr2[j] / 2);
            }
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* More dependency chains */
            result = result + a - b + c * d - e / (f + 1);
            arr1[(j + 128) & 0xFF] = result;
            
            /* Pseudo-random branch to create control flow complexity */
            if ((rand() % 100) < 25) {
                volatile int temp = arr2[j];
                arr2[j] = arr1[j];
                arr1[j] = temp;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Change mode_flag periodically to vary scheduling context */
        if ((i & 3) == 0) {
            mode_flag = (mode_flag + 1) & 0x0F;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function with different patterns */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int iterations) {
    volatile int x = 0, y = 0, z = 0;
    volatile int sum = 0;
    
    for (volatile int k = 0; k < iterations; k++) {
        /* Create instruction pressure with unroll-like pattern */
        for (int m = 0; m < 64; m++) {
            /* Multiple independent chains */
            x = arr1[m] * 3 + k;
            y = arr2[m] ^ x;
            z = (x + y) * (k + 1);
            
            /* Memory barriers at strategic points */
            if ((m & 7) == 0) {
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Complex conditional network */
            switch (m & 3) {
                case 0:
                    arr1[m] = x + y;
                    arr2[m] = z - x;
                    break;
                case 1:
                    arr1[m] = y * z;
                    arr2[m] = x ^ z;
                    break;
                case 2:
                    arr1[m] = (x << 2) | (y & 0xF);
                    arr2[m] = (z >> 1) + y;
                    break;
                case 3:
                    arr1[m] = x - y + z;
                    arr2[m] = y * 2 - z;
                    /* Function call in one case */
                    if ((k & 7) == 0) {
                        volatile int clk = clock();
                        arr1[m] ^= clk & 0xFF;
                    }
                    break;
            }
            
            sum += arr1[m] + arr2[m];
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = sum;
        arr2[63] = sum ^ 0xABCD;
        
        __asm__ volatile ("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic pseudo-randomness */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int outer_limit = 8;  /* Volatile to prevent constant propagation */
    volatile int mode = 0;
    volatile int total_result = 0;
    
    /* Multiple calls to create different scheduling contexts */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary parameters to create different scheduling scenarios */
        outer_limit = 4 + (iter % 5);
        mode = iter & 0x0F;
        
        /* Call the complex scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, outer_limit, mode);
        
        /* Alternate between different scheduling patterns */
        if (iter & 1) {
            volatile int res2 = alternate_schedule_pattern(array1, array2, 3 + (iter % 3));
            total_result ^= res2;
        }
        
        total_result ^= res1;
        
        /* Modify arrays between calls to create new dependencies */
        for (int i = 0; i < 256; i += 8) {
            array1[i] = array1[i] ^ total_result;
            array2[i] = array2[i] + iter;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    checksum ^= total_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
