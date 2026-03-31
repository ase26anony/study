/* haifa-sched-trigger.c
 * Program to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_ITER 50

/* Dummy function to prevent optimization */
static void use_result(volatile int val) {
    __asm__ volatile ("" : : "r"(val) : "memory");
}

/* Core scheduling-intensive function with complex control flow */
static __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
volatile int complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                                   volatile int mode, volatile int iter) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int m = 13, n = 14, o = 15, p = 16;
    volatile int result = 0;
    volatile int outer_limit = (iter % 5) + 3;  /* Non-constant limit */
    
    /* Outer loop with volatile limit */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        volatile int inner_limit = (mode + outer) % 8 + 4;
        
        /* Inner loop with complex operations */
        for (volatile int idx = 0; idx < inner_limit; idx++) {
            /* Create long dependency chains */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            e = a ^ f;
            g = e >> (h & 0x7);
            
            /* Memory accesses with volatile arrays */
            arr1[(a + idx) % ARRAY_SIZE] = g;
            arr2[(b + idx) % ARRAY_SIZE] = e;
            
            /* Complex multi-way branching */
            volatile int branch_selector = (a + b + c + idx) % 10;
            
            if (branch_selector < 3) {
                /* Branch 1: Arithmetic chain */
                m = n * o - p;
                n = m ^ j;
                o = n << (k % 4);
                __asm__ volatile ("" : : : "memory");
                result += m + n + o;
            } 
            else if (branch_selector < 6) {
                /* Branch 2: Memory-intensive */
                volatile int temp1 = arr1[(c + idx) % ARRAY_SIZE];
                volatile int temp2 = arr2[(d + idx) % ARRAY_SIZE];
                p = temp1 * temp2 + l;
                arr1[(e + idx) % ARRAY_SIZE] = p;
                __asm__ volatile ("" : : : "memory");
                result ^= p;
            }
            else if (branch_selector < 8) {
                /* Branch 3: Function call (creates call instruction) */
                if ((mode + iter + idx) % 7 == 0) {
                    volatile int pid = getpid();
                    result |= (pid & 0xFF);
                }
                /* More arithmetic */
                i = j * k - l;
                j = i ^ m;
                __asm__ volatile ("" : : : "memory");
                result += i * j;
            }
            else {
                /* Branch 4: Complex bit operations */
                volatile int x = (a << 3) | (b >> 2);
                volatile int y = (c ^ d) & 0x7F;
                volatile int z = x * y + (e & 0xF);
                
                for (volatile int bit = 0; bit < 4; bit++) {
                    if (z & (1 << bit)) {
                        result += (1 << (bit * 2));
                    }
                }
                __asm__ volatile ("" : : : "memory");
            }
            
            /* More independent operations to fill instruction queue */
            volatile int t1 = a + b;
            volatile int t2 = c - d;
            volatile int t3 = e * f;
            volatile int t4 = g ^ h;
            
            arr1[(t1 + idx) % ARRAY_SIZE] += t2;
            arr2[(t3 + idx) % ARRAY_SIZE] ^= t4;
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Switch-like behavior using bit masks */
            volatile int mask = (1 << (idx % 4));
            if (result & mask) {
                a = b + c;
                b = c - d;
            } else {
                c = d * e;
                d = e ^ f;
            }
            
            /* Final dependency chain */
            volatile int chain1 = a * b + c;
            volatile int chain2 = chain1 ^ d;
            volatile int chain3 = chain2 >> (e % 8);
            volatile int chain4 = chain3 * f + g;
            
            result += chain4;
        }
        
        /* Inter-loop operations */
        if (outer % 2 == 0) {
            volatile int temp = arr1[outer % ARRAY_SIZE];
            arr2[(outer + 1) % ARRAY_SIZE] = temp * result;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
volatile int alternate_schedule_pattern(volatile int *arr1, volatile int *arr2,
                                        volatile int seed) {
    volatile int x = seed, y = seed * 2, z = seed * 3;
    volatile int acc = 0;
    volatile int limit = (seed % 10) + 5;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Multiple independent chains */
        volatile int chain_a = x * y + z;
        volatile int chain_b = y ^ z - x;
        volatile int chain_c = (z << 2) | (x >> 1);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Complex conditional updates */
        if ((chain_a + i) % 3 == 0) {
            x = chain_b + arr1[i % ARRAY_SIZE];
            arr2[i % ARRAY_SIZE] = chain_c;
        } else if ((chain_b + i) % 5 == 0) {
            y = chain_a ^ arr2[(i + 1) % ARRAY_SIZE];
            arr1[i % ARRAY_SIZE] = chain_b;
        } else {
            z = chain_c * arr1[(i + 2) % ARRAY_SIZE];
            arr2[(i + 3) % ARRAY_SIZE] = chain_a;
        }
        
        /* More barriers and operations */
        __asm__ volatile ("" : : : "memory");
        
        for (volatile int j = 0; j < 3; j++) {
            volatile int tmp = (x + y + z) << j;
            acc += tmp;
            if (j % 2 == 0) {
                volatile int clock_val = clock();
                acc ^= (clock_val & 0xFF);
            }
        }
        
        /* Update for next iteration */
        x = (x * 1103515245 + 12345) & 0x7FFFFFFF;
        y = (y * 1664525 + 1013904223) & 0x7FFFFFFF;
        z = (z * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    return acc;
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
    
    volatile int mode = 1;
    volatile int total_result = 0;
    
    /* Multiple calls to create scheduling contexts */
    for (volatile int iter = 0; iter < 8; iter++) {
        volatile int result1 = complex_schedule_loop(array1, array2, mode, iter);
        volatile int result2 = alternate_schedule_pattern(array1, array2, iter);
        
        total_result ^= result1;
        total_result += result2;
        
        /* Change mode to vary behavior */
        mode = (mode * 3 + 1) % 7;
        
        /* Occasionally reset some array values */
        if (iter % 3 == 0) {
            for (volatile int j = 0; j < 50; j++) {
                int idx = (iter * 17 + j) % ARRAY_SIZE;
                array1[idx] = (array1[idx] * 13 + 7) & 0xFFF;
                array2[idx] = (array2[idx] ^ 0x5A5A) + j;
            }
        }
        
        /* Use results to prevent dead code elimination */
        use_result(result1);
        use_result(result2);
    }
    
    /* Compute final checksum */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    checksum ^= total_result;
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
