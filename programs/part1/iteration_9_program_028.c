/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa_test haifa-sched-trigger.c
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
    volatile int result = 0;
    volatile int i, j;
    volatile int outer_limit = (iter % 7) + 3;  /* Volatile-like calculation */
    volatile int inner_limit = (mode % 5) + 10;
    
    /* Volatile array for intermediate results */
    volatile int temp[16];
    for (i = 0; i < 16; i++) temp[i] = i * iter;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (i * 17 + iter) % 8;
        volatile int dep_chain_a = arr1[i % ARRAY_SIZE];
        volatile int dep_chain_b = arr2[i % ARRAY_SIZE];
        
        /* Create long dependency chains */
        for (j = 0; j < inner_limit; j++) {
            /* Multiple basic blocks created by if-else chain */
            if (branch_selector == 0) {
                dep_chain_a = dep_chain_a * 3 + dep_chain_b;
                dep_chain_b = dep_chain_b ^ (dep_chain_a >> 2);
                /* Memory barrier to split scheduling regions */
                __asm__ volatile("" : : : "memory");
                result += dep_chain_a - dep_chain_b;
            } 
            else if (branch_selector == 1) {
                dep_chain_a = (dep_chain_a << 1) | (dep_chain_b & 0xFF);
                dep_chain_b = dep_chain_b * 7 + iter;
                /* Independent memory operations */
                temp[j % 16] = dep_chain_a + arr1[(i + j) % ARRAY_SIZE];
                result ^= dep_chain_b;
            }
            else if (branch_selector == 2) {
                /* Complex arithmetic with memory barriers */
                dep_chain_a = dep_chain_a + (dep_chain_b * dep_chain_a);
                __asm__ volatile("" : : : "memory");
                dep_chain_b = dep_chain_b - (dep_chain_a / 3);
                __asm__ volatile("" : : : "memory");
                result |= dep_chain_a & dep_chain_b;
            }
            else if (branch_selector == 3) {
                /* Function call in one branch - adds call instruction */
                if ((iter & 0x1) && (j & 0x1)) {
                    volatile int pid = getpid();
                    dep_chain_a ^= pid & 0xFF;
                }
                dep_chain_b = dep_chain_b + (dep_chain_a << 3);
                result += dep_chain_b;
            }
            else if (branch_selector == 4) {
                /* More dependency chains */
                dep_chain_a = dep_chain_a * dep_chain_b + j;
                dep_chain_b = dep_chain_b ^ (dep_chain_a * 2);
                temp[(i + j) % 16] = dep_chain_a;
                result -= dep_chain_b;
            }
            else if (branch_selector == 5) {
                /* Switch-like behavior with bit tests */
                for (int k = 0; k < 4; k++) {
                    if (dep_chain_a & (1 << k)) {
                        dep_chain_b += (1 << (k * 2));
                    } else {
                        dep_chain_a |= (1 << k);
                    }
                }
                __asm__ volatile("" : : : "memory");
                result *= (dep_chain_a + dep_chain_b);
            }
            else if (branch_selector == 6) {
                /* Mixed operations with volatile memory access */
                volatile int *ptr = &temp[j % 16];
                *ptr = dep_chain_a * dep_chain_b + *ptr;
                dep_chain_a = (*ptr) >> 4;
                dep_chain_b = dep_chain_b ^ dep_chain_a;
                result = result + dep_chain_b - dep_chain_a;
            }
            else { /* branch_selector == 7 */
                /* Complex chain with multiple barriers */
                dep_chain_a = dep_chain_a + (dep_chain_b * 11);
                __asm__ volatile("" : : : "memory");
                dep_chain_b = dep_chain_b ^ (dep_chain_a * 13);
                __asm__ volatile("" : : : "memory");
                dep_chain_a = dep_chain_a | (dep_chain_b << 1);
                result = result ^ dep_chain_a ^ dep_chain_b;
            }
            
            /* Additional independent operations to fill instruction queue */
            volatile int indep1 = arr1[(i * j + 1) % ARRAY_SIZE];
            volatile int indep2 = arr2[(i * j + 3) % ARRAY_SIZE];
            volatile int indep3 = indep1 * indep2 + j;
            temp[(j + 3) % 16] += indep3;
            
            /* Another memory barrier */
            if (j % 4 == 0) {
                __asm__ volatile("" : : : "memory");
            }
        }
        
        /* Store results back to arrays */
        arr1[i % ARRAY_SIZE] = dep_chain_a;
        arr2[i % ARRAY_SIZE] = dep_chain_b;
        
        /* Change branch selector for next iteration */
        branch_selector = (branch_selector + 1) % 8;
    }
    
    /* Final computation with result */
    for (i = 0; i < 16; i++) {
        result ^= temp[i];
    }
    
    return result;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
another_scheduling_function(volatile int *arr, volatile int size, volatile int seed) {
    volatile int acc = seed;
    volatile int i;
    
    for (i = 0; i < size; i++) {
        volatile int x = arr[i];
        volatile int y = arr[(i * 3 + 1) % size];
        
        /* Multiple operation types */
        if (x & 1) {
            acc = acc * 3 + x;
            __asm__ volatile("" : : : "memory");
        } else if (x & 2) {
            acc = acc ^ (y << 1);
        } else if (x & 4) {
            acc = acc - (y / 2);
            __asm__ volatile("" : : : "memory");
        } else {
            acc = acc | (x & y);
        }
        
        /* Dependency chain */
        for (int j = 0; j < 3; j++) {
            x = (x << j) | (y >> j);
            y = y ^ x;
            acc += y;
        }
        
        arr[i] = x;
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int temp_array[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        temp_array[i] = 0;
    }
    
    volatile int final_result = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        volatile int outer_mode = (iter * 13) % 11;
        volatile int inner_mode = (iter * 7) % 5;
        
        /* Call core scheduling function multiple times with different modes */
        volatile int res1 = complex_schedule_loop(array1, array2, outer_mode, iter);
        volatile int res2 = complex_schedule_loop(array2, array1, inner_mode, iter + 1);
        
        /* Process results through another scheduling function */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            temp_array[i] = array1[i] ^ array2[i];
        }
        
        volatile int res3 = another_scheduling_function(temp_array, ARRAY_SIZE, res1 ^ res2);
        
        /* Accumulate results with volatile operations */
        final_result ^= res1;
        final_result += res2;
        final_result ^= res3;
        
        /* Change mode for next iteration */
        mode_switch = (mode_switch + 1) % 7;
        
        /* Memory barrier between iterations */
        __asm__ volatile("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    checksum ^= final_result;
    
    printf("Result: %d\n", checksum);
    
    return 0;
}
