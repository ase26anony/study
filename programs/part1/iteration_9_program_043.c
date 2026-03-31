/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
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
    volatile int outer_limit = (mode % 7) + 3;  /* Non-constant, volatile bound */
    
    /* Outer loop with volatile limit to force scheduler state management */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (iter % 5) + 10;  /* Varying inner bounds */
        
        /* Complex inner loop with multiple dependency chains */
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain 1 */
            a = arr1[(i + j) % ARRAY_SIZE];
            b = arr2[(i * j) % ARRAY_SIZE];
            c = a * b + iter;
            d = c ^ (a << 2);
            e = d >> (b % 8);
            f = e + (i * j);
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Independent operations to fill instruction queue */
            volatile int x = arr1[(j + 1) % ARRAY_SIZE];
            volatile int y = arr2[(j * 3) % ARRAY_SIZE];
            volatile int z = x * y - d;
            
            /* Complex conditional structure creating multiple basic blocks */
            if ((f & 1) && (mode > 0)) {
                /* Branch 1: Arithmetic operations */
                z = z * 3 + 7;
                arr1[(i + j) % ARRAY_SIZE] = z;
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Nested condition */
                if (z > 1000) {
                    z = z / 2;
                    arr2[j % ARRAY_SIZE] = z + e;
                } else {
                    z = z * 2;
                    arr2[j % ARRAY_SIZE] = z - e;
                }
            } else if ((f & 2) && (mode < 5)) {
                /* Branch 2: Different operations */
                z = z / 2 + 11;
                arr1[(i * j) % ARRAY_SIZE] = z;
                
                /* Function call in one branch to add complexity */
                if ((iter + j) % 13 == 0) {
                    volatile int pid = getpid();
                    z = z ^ (pid & 0xFF);
                }
            } else if ((f & 4) || (mode == 3)) {
                /* Branch 3: More operations */
                z = (z << 3) | (z >> 5);
                arr1[(i + 3) % ARRAY_SIZE] = z;
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Complex expression with multiple dependencies */
                volatile int t1 = arr1[(j + 5) % ARRAY_SIZE];
                volatile int t2 = arr2[(j + 7) % ARRAY_SIZE];
                volatile int t3 = t1 * t2 + t1 - t2;
                arr2[(j + 1) % ARRAY_SIZE] = t3;
            } else {
                /* Default branch */
                z = z + f + i + j;
                arr1[(i + j + 1) % ARRAY_SIZE] = z;
            }
            
            /* Switch-like structure using bit tests (creates jump table) */
            volatile int selector = (z ^ iter) & 0xF;
            volatile int temp_result = 0;
            
            for (k = 0; k < 4; k++) {
                if (selector & (1 << k)) {
                    switch (k) {
                        case 0:
                            temp_result = arr1[(j + k) % ARRAY_SIZE] * 3;
                            break;
                        case 1:
                            temp_result = arr2[(j + k) % ARRAY_SIZE] / 2;
                            break;
                        case 2:
                            temp_result = arr1[(j + k) % ARRAY_SIZE] ^ arr2[(j + k) % ARRAY_SIZE];
                            break;
                        case 3:
                            temp_result = arr1[(j + k) % ARRAY_SIZE] + arr2[(j + k) % ARRAY_SIZE];
                            break;
                    }
                    arr2[(i + k) % ARRAY_SIZE] = temp_result;
                }
            }
            
            /* Final memory operation with barrier */
            result += z;
            __asm__ volatile ("" : : : "memory");
            
            /* Store results back with complex addressing */
            arr1[(result + i + j) % ARRAY_SIZE] = result;
            arr2[(result * j) % ARRAY_SIZE] = f;
        }
        
        /* Additional operations between inner loops */
        if (i % 2 == 0) {
            volatile int tmp = arr1[i % ARRAY_SIZE];
            arr1[i % ARRAY_SIZE] = arr2[(i + 1) % ARRAY_SIZE];
            arr2[(i + 1) % ARRAY_SIZE] = tmp;
            
            /* Barrier to potentially trigger scheduler context save */
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
another_schedule_function(volatile int *arr, volatile int seed) {
    volatile int i, j;
    volatile int sum = 0;
    volatile int limit = (seed % 15) + 5;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (seed + i) % 8 + 4;
        
        for (j = 0; j < inner; j++) {
            /* Complex dependency chain */
            volatile int x = arr[(i * 3 + j) % ARRAY_SIZE];
            volatile int y = arr[(i + j * 7) % ARRAY_SIZE];
            volatile int z = (x * y) + (x << (y % 4)) - (y >> (x % 4));
            
            /* Multiple barriers */
            __asm__ volatile ("" : : : "memory");
            
            /* Conditional with function call */
            if ((z + i + j) % 3 == 0) {
                volatile int clk = clock() & 0xFF;
                z = z ^ clk;
                __asm__ volatile ("" : : : "memory");
            }
            
            arr[(i + j) % ARRAY_SIZE] = z;
            sum += z;
        }
    }
    
    return sum;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create scheduling contexts */
    for (i = 0; i < 8; i++) {
        volatile int mode = i % 4;
        volatile int result;
        
        /* Call main scheduling function */
        result = complex_schedule_loop(array1, array2, mode, i);
        checksum ^= result;
        
        /* Call secondary function */
        result = another_schedule_function(array1, i);
        checksum ^= result;
        
        /* Alternate between arrays */
        result = complex_schedule_loop(array2, array1, (mode + 1) % 4, i + 1);
        checksum ^= result;
        
        /* Additional complexity between iterations */
        if (i % 3 == 0) {
            for (j = 0; j < ARRAY_SIZE / 4; j++) {
                volatile int tmp = array1[j];
                array1[j] = array2[ARRAY_SIZE - j - 1];
                array2[ARRAY_SIZE - j - 1] = tmp;
            }
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
