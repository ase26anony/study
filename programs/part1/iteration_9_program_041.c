/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile limit to prevent optimization */
    volatile int inner_limit = (iter % 5) + 10; /* Dynamic inner loop bound */
    
    /* Outer loop with volatile bound - forces scheduler context management */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Memory barrier to split scheduling region */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex dependency chains */
        for (volatile int inner = 0; inner < inner_limit; inner++) {
            /* Create multiple basic blocks with if-else chains */
            int selector = (inner + outer * iter) % 8;
            
            /* Basic block 1: Arithmetic dependency chain */
            if (selector == 0) {
                a = b * c + d;
                __asm__ volatile ("" : : : "memory");
                e = a ^ f;
                g = e >> (inner & 3);
                arr1[inner % ARRAY_SIZE] = g + outer;
            }
            /* Basic block 2: Memory operations */
            else if (selector == 1 || selector == 2) {
                volatile int temp = arr2[(inner + 1) % ARRAY_SIZE];
                b = temp * 31 + 17;
                c = b ^ arr1[inner % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
                d = c - (temp >> 2);
            }
            /* Basic block 3: Complex arithmetic with barriers */
            else if (selector == 3) {
                a = (b * c) | (d & e);
                __asm__ volatile ("" : : : "memory");
                f = a * 1103515245 + 12345;
                g = f % 65536;
                arr2[inner % ARRAY_SIZE] = g ^ iter;
            }
            /* Basic block 4: Function call in one branch */
            else if (selector == 4) {
                /* Guarded function call to add call instruction to scheduling */
                if ((iter & 1) && (outer & 1)) {
                    volatile int pid = getpid();
                    a = pid & 0xFF;
                }
                b = a * 3 + 7;
                c = b ^ 0x5A5A;
            }
            /* Basic block 5: Long dependency chain */
            else if (selector == 5) {
                for (int i = 0; i < 4; i++) {
                    a = a * 1664525 + 1013904223;
                    b = b ^ a;
                    c = c + b * 7;
                }
                __asm__ volatile ("" : : : "memory");
                d = (c >> 16) & 0xFFFF;
            }
            /* Basic block 6: Switch-like behavior */
            else {
                /* Multi-way computation */
                int x = inner & 7;
                switch (x) {
                    case 0: a = b + c; break;
                    case 1: a = b - c; break;
                    case 2: a = b * c; break;
                    case 3: a = b ^ c; break;
                    case 4: a = b & c; break;
                    case 5: a = b | c; break;
                    case 6: a = ~(b ^ c); break;
                    default: a = b % (c + 1); break;
                }
                __asm__ volatile ("" : : : "memory");
                f = a * 214013 + 2531011;
            }
            
            /* Independent memory operations to fill instruction queue */
            arr1[(inner + 5) % ARRAY_SIZE] = a + b;
            arr2[(inner + 3) % ARRAY_SIZE] = c ^ d;
            volatile int temp_load = arr1[(inner + 7) % ARRAY_SIZE];
            arr2[(inner + 11) % ARRAY_SIZE] = temp_load * e;
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* More arithmetic to create scheduling pressure */
            result += (a * b + c * d - e * f) & 0xFF;
            result ^= (inner << 3) | (outer << 8);
        }
        
        /* Pseudo-random branch to vary control flow */
        if ((outer ^ iter) & 1) {
            /* Alternate computation path */
            for (int k = 0; k < 3; k++) {
                a = (a * 13 + b * 17) % 1000;
                b = (b * 29 + c * 31) % 1000;
                c = (c * 47 + a * 53) % 1000;
            }
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
another_scheduling_function(volatile int *arr, volatile int seed) {
    volatile int x = seed, y = seed * 3, z = seed * 7;
    volatile int limit = (seed % 15) + 5;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Complex dependency network */
        int op = (i + seed) % 6;
        if (op == 0) {
            x = y * z + i;
            __asm__ volatile ("" : : : "memory");
            y = x ^ (z << 2);
        } else if (op == 1) {
            z = (x + y) * 3 - i;
        } else if (op == 2) {
            x = (y | z) & (0xFFFF >> (i & 7));
            __asm__ volatile ("" : : : "memory");
        } else if (op == 3) {
            /* Function call in some iterations */
            if ((i & 3) == 0) {
                volatile int clk = clock() & 0xFF;
                y = clk ^ z;
            }
            z = x * y + i * 7;
        } else {
            x = y ^ z;
            y = z ^ x;
            z = x ^ y;  /* XOR swap chain */
        }
        
        /* Memory operations with varying indices */
        arr[(i * 17 + seed) % ARRAY_SIZE] = x + y + z;
        arr[(i * 23 + seed) % ARRAY_SIZE] = (x * y) | z;
        
        /* Barrier to split scheduling regions */
        if (i % 4 == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return x + y + z;
}

int main(void) {
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
    volatile int mode_switch = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int main_iter = 0; main_iter < 8; main_iter++) {
        volatile int outer_bound = (main_iter % 3) + 2;  /* Volatile-like behavior */
        
        /* Call core scheduling function multiple times with varying parameters */
        for (volatile int call_idx = 0; call_idx < outer_bound; call_idx++) {
            volatile int result1 = complex_schedule_loop(array1, array2, 
                                                       mode_switch + call_idx, 
                                                       main_iter * 10 + call_idx);
            
            checksum ^= result1;
            
            /* Alternate between different modes */
            mode_switch = (mode_switch + 1) % 4;
            
            /* Call secondary function to create additional scheduling contexts */
            if (call_idx & 1) {
                volatile int result2 = another_scheduling_function(array1, 
                                                                 main_iter * 100 + call_idx);
                checksum += result2;
            }
        }
        
        /* Modify arrays between iterations to prevent optimization */
        for (int i = 0; i < ARRAY_SIZE; i += 7) {
            array1[i] = (array1[i] * 3 + 1) % 1000;
            array2[i] = (array2[i] * 5 + 2) % 1000;
        }
        
        /* Memory barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to use all results */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array1[i] ^ array2[i];
    }
    final_sum ^= checksum;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", final_sum);
    
    return 0;
}
