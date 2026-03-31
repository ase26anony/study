/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 50

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int result = 0;
    volatile int i, j;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile to prevent optimization */
    volatile int inner_limit = (iter % 11) + 5; /* Volatile loop bounds */
    
    /* Volatile control variables */
    volatile int branch_selector;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < outer_limit; i++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex operations */
        for (j = 0; j < inner_limit; j++) {
            /* Pseudo-random branch selection using array values */
            branch_selector = (arr1[j % ARRAY_SIZE] ^ arr2[(j + 1) % ARRAY_SIZE]) & 0xF;
            
            /* Complex multi-basic-block structure */
            if (branch_selector < 4) {
                /* Basic block 1: Arithmetic chain */
                temp1 = arr1[j % ARRAY_SIZE] * arr2[(j + 3) % ARRAY_SIZE];
                temp2 = temp1 + (arr1[(j + 2) % ARRAY_SIZE] >> 2);
                temp3 = temp2 ^ (arr2[j % ARRAY_SIZE] & 0x7F);
                result += temp3;
                
                /* Memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Store result back with dependency */
                arr1[(j + i) % ARRAY_SIZE] = result;
            } 
            else if (branch_selector < 8) {
                /* Basic block 2: Different arithmetic pattern */
                temp1 = arr1[(j + 1) % ARRAY_SIZE] + arr2[(j + 2) % ARRAY_SIZE];
                temp2 = temp1 * (arr1[j % ARRAY_SIZE] | 1);
                temp3 = (temp2 << 3) - arr2[(j + 4) % ARRAY_SIZE];
                result ^= temp3;
                
                /* Independent memory operations */
                arr2[(j + 5) % ARRAY_SIZE] = temp3;
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector < 12) {
                /* Basic block 3: More complex operations */
                temp1 = arr2[j % ARRAY_SIZE] - arr1[(j + 3) % ARRAY_SIZE];
                temp2 = (temp1 * temp1) / ((arr1[j % ARRAY_SIZE] & 0x1F) + 1);
                temp3 = temp2 | (arr2[(j + 6) % ARRAY_SIZE] << 8);
                result = result * 3 + temp3;
                
                /* Function call in one branch - adds call instruction to schedule */
                if ((branch_selector & 1) && (mode & 1)) {
                    volatile int pid = getpid();
                    result ^= (pid & 0xFF);
                }
                
                __asm__ volatile ("" : : : "memory");
            }
            else {
                /* Basic block 4: Bit manipulation chain */
                temp1 = arr1[j % ARRAY_SIZE] & arr2[(j + 7) % ARRAY_SIZE];
                temp2 = (temp1 << 4) | (temp1 >> 4);
                temp3 = temp2 ^ (arr1[(j + 8) % ARRAY_SIZE] * 17);
                result = (result + temp3) & 0x3FFFFFFF;
                
                /* Additional memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Store with offset */
                arr2[(j + 9) % ARRAY_SIZE] = result;
            }
            
            /* Intermix independent operations to fill instruction queue */
            volatile int idx1 = (i * 13 + j * 7) % ARRAY_SIZE;
            volatile int idx2 = (i * 17 + j * 11) % ARRAY_SIZE;
            
            /* Independent arithmetic that can be scheduled in parallel */
            int dep1 = arr1[idx1] + 1;
            int dep2 = arr2[idx2] * 2;
            int dep3 = dep1 ^ dep2;
            int dep4 = dep3 >> (j & 3);
            
            /* Use results to prevent elimination */
            arr1[idx1] = dep1;
            arr2[idx2] = dep4;
            
            /* Final memory barrier in inner loop */
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Switch-like structure based on iteration */
        switch (i & 3) {
            case 0:
                result += arr1[(i * 19) % ARRAY_SIZE];
                break;
            case 1:
                result -= arr2[(i * 23) % ARRAY_SIZE];
                break;
            case 2:
                result ^= (arr1[(i * 29) % ARRAY_SIZE] * arr2[(i * 31) % ARRAY_SIZE]);
                break;
            case 3:
                result = (result << 1) | (result >> 31);
                if ((mode & 2) && (i % 5 == 0)) {
                    volatile clock_t t = clock();
                    result ^= (t & 0xFFFF);
                }
                break;
        }
    }
    
    return result;
}

/* Secondary function to create additional scheduling contexts */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int seed) {
    volatile int sum = 0;
    volatile int i, j;
    
    for (i = 0; i < (seed % 5) + 2; i++) {
        __asm__ volatile ("" : : : "memory");
        
        for (j = 0; j < ARRAY_SIZE / 4; j++) {
            /* Create long dependency chains */
            volatile int idx = (i * 37 + j * 41) % ARRAY_SIZE;
            volatile int val = arr[idx];
            
            /* Multi-step dependency chain */
            int chain1 = val * 3 + 1;
            int chain2 = chain1 ^ (val >> 2);
            int chain3 = chain2 * 7 - chain1;
            int chain4 = chain3 & 0x7FFF;
            int chain5 = (chain4 << 3) | (chain4 >> 13);
            
            /* Use chain result */
            arr[idx] = chain5;
            sum += chain5;
            
            /* Memory barrier every few iterations */
            if (j % 8 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Volatile control variables */
    volatile int mode_flags[5] = {1, 3, 5, 7, 9};
    volatile int outer_iterations = 8;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int total_result = 0;
    
    /* Main loop to trigger multiple scheduling contexts */
    for (volatile int iter = 0; iter < outer_iterations; iter++) {
        __asm__ volatile ("" : : : "memory");
        
        /* Vary mode to create different scheduling patterns */
        volatile int mode = mode_flags[iter % 5];
        
        /* Call core scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter);
        total_result ^= res1;
        
        /* Call secondary function */
        volatile int res2 = secondary_schedule_func(array1, iter);
        total_result += res2;
        
        /* Alternate between array1 and array2 */
        if (iter & 1) {
            volatile int res3 = complex_schedule_loop(array2, array1, mode ^ 1, iter + 1);
            total_result ^= res3;
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
