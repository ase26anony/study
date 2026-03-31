/* haifa-sched-trigger.c
 * Program to trigger free_sched_context logic in haifa-sched.cc
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
    volatile int i, j, k;
    volatile int a, b, c, d, e, f, g, h;
    volatile int result = 0;
    volatile int outer_limit = (mode % 5) + 3;  /* Non-constant, volatile bound */
    
    /* Outer loop with volatile limit - forces dynamic scheduling decisions */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (iter % 7) + 4;  /* Varying inner bounds */
        
        /* Inner loop with complex operations */
        for (j = 0; j < inner_limit; j++) {
            /* Long dependency chain 1 */
            a = arr1[(i + j) % ARRAY_SIZE];
            b = arr2[(i * j) % ARRAY_SIZE];
            c = a * b + iter;
            d = c ^ (a << 3);
            e = d >> (b % 8);
            f = e + (i * j);
            
            /* Memory barrier to split scheduling region */
            __asm__ volatile ("" : : : "memory");
            
            /* Long dependency chain 2 */
            g = arr2[(i + j * 2) % ARRAY_SIZE];
            h = arr1[(j + i * 3) % ARRAY_SIZE];
            a = g * h - f;
            b = a ^ g;
            c = b >> (h % 6);
            d = c + (j << 2);
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Complex conditional structure creating multiple basic blocks */
            volatile int branch_selector = (a + b + c) % 8;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic operations */
                e = d * 3 + 7;
                f = e ^ 0x5A5A;
                arr1[(i + j) % ARRAY_SIZE] = f;
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 1) {
                /* Branch 1: Different arithmetic */
                e = d / 2 + 11;
                f = e | 0x3333;
                arr2[(i * j) % ARRAY_SIZE] = f;
            } else if (branch_selector == 2) {
                /* Branch 2: Memory intensive */
                for (k = 0; k < 3; k++) {
                    arr1[(i + j + k) % ARRAY_SIZE] += arr2[(i * j + k) % ARRAY_SIZE];
                }
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector == 3) {
                /* Branch 3: Function call (adds call instruction to schedule) */
                if ((iter & 1) && (mode > 2)) {
                    volatile int pid = getpid();
                    arr1[(i + j) % ARRAY_SIZE] ^= pid & 0xFF;
                }
            } else if (branch_selector == 4) {
                /* Branch 4: More complex operations */
                e = (d << 3) | (d >> 5);
                f = e + arr1[(j * 7) % ARRAY_SIZE];
                g = f * arr2[(i * 11) % ARRAY_SIZE];
                arr1[(i + j) % ARRAY_SIZE] = g % 1000;
            } else if (branch_selector == 5) {
                /* Branch 5: Bit manipulation chain */
                e = d ^ arr2[(i + j * 13) % ARRAY_SIZE];
                f = (e << 1) | (e >> 31);
                g = f + arr1[(j * 17) % ARRAY_SIZE];
                h = g ^ 0xAAAAAAAA;
                arr2[(i * j) % ARRAY_SIZE] = h;
            } else if (branch_selector == 6) {
                /* Branch 6: Mixed operations with barrier */
                e = d + arr1[(i * 19) % ARRAY_SIZE];
                f = e * arr2[(j * 23) % ARRAY_SIZE];
                __asm__ volatile ("" : : : "memory");
                g = f - arr1[(i + j * 29) % ARRAY_SIZE];
                arr1[(i + j) % ARRAY_SIZE] = g;
            } else {
                /* Branch 7: Default - complex calculation */
                e = (d * 3 + 1) & 0x7FFF;
                f = e * arr2[(i * 31) % ARRAY_SIZE];
                g = f / (arr1[(j * 37) % ARRAY_SIZE] + 1);
                arr2[(i + j) % ARRAY_SIZE] = g;
            }
            
            /* Final memory barrier in loop body */
            __asm__ volatile ("" : : : "memory");
            
            /* Accumulate result with dependency */
            result += (a ^ b) + (c & d) - (e | f) + (g ^ h);
        }
        
        /* Switch-like structure using bit tests (creates jump table potential) */
        volatile int switch_var = result & 0x7;
        for (k = 0; k < 4; k++) {
            if (switch_var & (1 << k)) {
                volatile int temp = arr1[(i + k * 2) % ARRAY_SIZE];
                arr2[(i * k) % ARRAY_SIZE] = temp * k + result;
                __asm__ volatile ("" : : : "memory");
            } else {
                volatile int temp = arr2[(i + k * 3) % ARRAY_SIZE];
                arr1[(i * k * 2) % ARRAY_SIZE] = temp / (k + 1) - result;
            }
        }
    }
    
    return result;
}

int main(void) {
    volatile int arr1[ARRAY_SIZE];
    volatile int arr2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Initialize arrays with pseudo-random but complex patterns */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        
        /* Create some dependencies between array elements */
        if (i > 0) {
            arr1[i] ^= arr1[i-1];
            arr2[i] += arr2[i-1] & 0xFF;
        }
    }
    
    /* Multiple calls with varying parameters to increase scheduling context usage */
    for (i = 0; i < MAX_LOOP_ITER; i++) {
        volatile int mode = (i % 3) + 1;  /* Varying mode */
        volatile int iter = i;
        
        /* Call the complex scheduling function */
        volatile int result = complex_schedule_loop(arr1, arr2, mode, iter);
        
        /* Use result to prevent elimination */
        checksum ^= result;
        
        /* Modify arrays between calls to create different scheduling contexts */
        for (j = 0; j < ARRAY_SIZE; j += 8) {
            arr1[j] += result & 0xFF;
            arr2[j] ^= result >> 8;
        }
        
        /* Memory barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
    }
    
    /* Print checksum to ensure all code executes */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
