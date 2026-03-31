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
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    volatile int result = 0;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile limit to prevent optimization */
    
    /* Outer loop with volatile bound - forces scheduler context management */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (iter % 5) + 10;  /* Another volatile bound */
        
        /* Complex inner loop with multiple dependency chains */
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain 1 */
            a = arr1[(i + j) % ARRAY_SIZE];
            b = arr2[(i * j) % ARRAY_SIZE];
            c = a * b + iter;
            
            /* Memory barrier to split scheduling region */
            __asm__ volatile ("" : : : "memory");
            
            d = c ^ (a >> 2);
            e = d * 0x5A827999;
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Long dependency chain 2 (independent) */
            f = arr1[(i * 3 + j) % ARRAY_SIZE];
            int g = f * 0x6ED9EBA1;
            int h = g ^ (f << 3);
            
            /* Complex control flow creating multiple basic blocks */
            if ((mode + i + j) & 1) {
                /* Branch 1: Arithmetic operations */
                a = (c + d) * e;
                b = (f ^ g) | h;
                
                /* Memory access pattern 1 */
                arr2[(i + j * 2) % ARRAY_SIZE] = a + b;
                
                /* Another barrier */
                __asm__ volatile ("" : : : "memory");
            } else if ((mode + i + j) & 2) {
                /* Branch 2: Different operations */
                a = c - d + e;
                b = (g & h) * 0x9E3779B9;
                
                /* Memory access pattern 2 */
                arr1[(i * j + 5) % ARRAY_SIZE] = a ^ b;
                
                /* Potential function call in one branch */
                if ((j % 13) == 0) {
                    volatile int pid = getpid();
                    a ^= (pid & 0xFF);
                }
            } else if ((mode + i + j) & 4) {
                /* Branch 3: More complex operations */
                a = (c * d) >> (e & 0xF);
                b = (g + h) * 0x243F6A88;
                
                /* Memory access pattern 3 */
                arr2[(i + j * 3) % ARRAY_SIZE] = a | b;
            } else {
                /* Branch 4: Mixed operations */
                a = (c ^ d) + (e & 0xFFFF);
                b = (g - h) * 0xB7E15162;
                
                /* Memory access pattern 4 */
                arr1[(i * 4 + j) % ARRAY_SIZE] = a & b;
            }
            
            /* Switch-like multi-way branch using bit tests */
            volatile int branch_selector = (i * 17 + j * 13) % 16;
            for (k = 0; k < 4; k++) {
                if (branch_selector & (1 << k)) {
                    /* Different operations in each bit-tested branch */
                    switch (k) {
                        case 0:
                            a = b * c + d;
                            __asm__ volatile ("" : : : "memory");
                            break;
                        case 1:
                            e = a ^ f;
                            __asm__ volatile ("" : : : "memory");
                            break;
                        case 2:
                            g = e >> (h & 0x7);
                            break;
                        case 3:
                            arr1[(i + k) % ARRAY_SIZE] = g * 0x3C6EF372;
                            break;
                    }
                }
            }
            
            /* Final dependency chain mixing all values */
            result += a + b + c + d + e + f + g + h;
            result ^= (arr1[j % ARRAY_SIZE] * arr2[(j + 1) % ARRAY_SIZE]);
            
            /* Periodic memory barrier */
            if (j % 5 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Cross-iteration dependency to prevent loop unrolling elimination */
        arr1[i % ARRAY_SIZE] = result & 0x7FFFFFFF;
        arr2[(i + 1) % ARRAY_SIZE] = (result >> 16) & 0x7FFF;
    }
    
    return result;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
another_scheduling_context(volatile int *arr, volatile int seed) {
    volatile int i, j;
    volatile int x = 0, y = 0, z = 0;
    volatile int limit = (seed % 8) + 4;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (seed + i) % 6 + 3;
        
        for (j = 0; j < inner; j++) {
            /* Complex arithmetic with memory barriers */
            x = arr[(i * 3 + j * 7) % ARRAY_SIZE];
            y = x * 0x9E3779B9 + seed;
            
            __asm__ volatile ("" : : : "memory");
            
            z = (y ^ (x << 5)) + (i * j);
            
            /* Multi-way conditional */
            if ((i + j) & 1) {
                x = z * 0x6A09E667;
                arr[(i + j * 2) % ARRAY_SIZE] = x;
            } else {
                y = z ^ 0xBB67AE85;
                arr[(i * j + 3) % ARRAY_SIZE] = y;
            }
            
            /* Another barrier */
            if (j % 3 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return x + y + z;
}

int main() {
    /* Seed for deterministic but complex behavior */
    srand(0x12345678);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int mode = 0;
    volatile int checksum = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        volatile int bound1 = (rand() % 5) + 2;
        volatile int bound2 = (rand() % 7) + 1;
        
        /* Call core scheduling function multiple times with different modes */
        for (mode = 0; mode < bound1; mode++) {
            volatile int result = complex_schedule_loop(array1, array2, mode, iter);
            checksum ^= result;
            
            /* Call secondary function to create additional scheduling contexts */
            if (mode & 1) {
                volatile int res2 = another_scheduling_context(array1, iter + mode);
                checksum += res2;
            }
        }
        
        /* Alternate between different array access patterns */
        if (iter & 1) {
            for (int i = 0; i < ARRAY_SIZE / 2; i++) {
                volatile int temp = array1[i];
                array1[i] = array2[ARRAY_SIZE - 1 - i];
                array2[ARRAY_SIZE - 1 - i] = temp;
            }
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
