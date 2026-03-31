/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o haifa-test haifa-sched-trigger.c
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
    volatile int i, j, k;
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int loop_limit = (mode % 7) + 10;  /* Non-constant, volatile bound */
    
    /* Outer loop with volatile limit */
    for (i = 0; i < loop_limit; i++) {
        volatile int inner_limit = (iter + i) % MAX_LOOP_BOUND + 5;
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Create instruction queue pressure with long dependency chains */
            a = b * c + d;
            __asm__ volatile("" : : : "memory");  /* Scheduling barrier */
            
            e = a ^ f;
            f = e >> (j & 0x3);
            
            /* Multiple independent memory operations */
            arr1[(i + j) % ARRAY_SIZE] = a + e;
            arr2[(i * j) % ARRAY_SIZE] = b - f;
            
            /* Complex multi-way branching to create multiple basic blocks */
            volatile int branch_selector = (a + b + c + iter) % 8;
            
            /* Branch 1: Arithmetic operations */
            if (branch_selector & 0x1) {
                c = d * e - f;
                d = c ^ a;
                __asm__ volatile("" : : : "memory");
                arr1[(i + 3) % ARRAY_SIZE] = c * d;
            }
            
            /* Branch 2: Memory intensive */
            if (branch_selector & 0x2) {
                volatile int temp = arr2[(j * 2) % ARRAY_SIZE];
                b = temp + a;
                arr1[(i + 5) % ARRAY_SIZE] = b * temp;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Branch 3: Function call (adds call instruction to schedule) */
            if (branch_selector & 0x4) {
                if ((iter + j) % 13 == 0) {  /* Volatile condition */
                    volatile int pid = getpid();
                    a = a ^ (pid & 0xFF);
                }
            }
            
            /* Branch 4: More complex arithmetic chain */
            if (branch_selector & 0x8) {
                e = f * a + b;
                f = e >> (c & 0x7);
                d = e ^ f;
                __asm__ volatile("" : : : "memory");
                arr2[(i + j + 1) % ARRAY_SIZE] = d * e;
            }
            
            /* Additional dependency chain */
            k = (a + b + c + d) % 100;
            result += k;
            
            /* More memory operations to increase pressure */
            if (j % 3 == 0) {
                arr1[(result + i) % ARRAY_SIZE] = result;
                __asm__ volatile("" : : : "memory");
            }
            
            /* Switch-like behavior using if-else chain */
            volatile int switch_val = result % 5;
            if (switch_val == 0) {
                a = b + c;
                b = c * d;
            } else if (switch_val == 1) {
                c = d - e;
                d = e ^ f;
            } else if (switch_val == 2) {
                e = f * a;
                f = b >> 2;
            } else if (switch_val == 3) {
                /* Nested operations */
                a = (b * c + d) / (e + 1);
                __asm__ volatile("" : : : "memory");
            } else {
                /* Default case with memory barrier */
                __asm__ volatile("" : : : "memory");
                arr2[(i * 3 + j) % ARRAY_SIZE] = a + b + c;
            }
        }
        
        /* Intermediate scheduling barrier between outer loop iterations */
        __asm__ volatile("" : : : "memory");
        
        /* Varying operations based on outer loop index */
        if (i % 4 == 0) {
            volatile int temp = clock() % 1000;  /* Potential function call */
            result ^= temp;
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling diversity */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
another_scheduling_function(volatile int *arr, volatile int seed) {
    volatile int x = 1, y = 2, z = 3;
    volatile int limit = (seed % 20) + 15;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Create artificial dependencies */
        x = y * z + i;
        y = x ^ seed;
        z = y >> (i & 0x3);
        
        /* Memory operations with barriers */
        arr[i % ARRAY_SIZE] = x + y + z;
        __asm__ volatile("" : : : "memory");
        
        /* Conditional with function call */
        if ((x + y) % 7 == 0) {
            volatile int pid = getpid();
            z = z ^ (pid & 0xFF);
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
    
    /* Main loop to repeatedly trigger scheduling contexts */
    for (int outer = 0; outer < 8; outer++) {
        volatile int loop_bound = (outer % 3) + 5;  /* Volatile bound */
        
        for (volatile int inner = 0; inner < loop_bound; inner++) {
            /* Call core scheduling function with varying parameters */
            volatile int result = complex_schedule_loop(
                array1, array2, mode_switch, outer * 10 + inner);
            
            checksum ^= result;
            
            /* Alternate between modes */
            mode_switch = (mode_switch + 1) % 4;
            
            /* Call secondary function to add scheduling complexity */
            if (inner % 2 == 0) {
                volatile int alt_result = another_scheduling_function(
                    array1, outer + inner);
                checksum += alt_result;
            }
        }
        
        /* Modify array contents between iterations */
        for (int i = 0; i < ARRAY_SIZE; i += 4) {
            array1[i] = (array1[i] * 3 + outer) % 1000;
            __asm__ volatile("" : : : "memory");
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum ^= array1[i];
        final_sum += array2[i];
    }
    
    checksum ^= final_sum;
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", (int)checksum);
    
    return 0;
}
