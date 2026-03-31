/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int loop_limit = (iter % 4) + 3;  /* Volatile-like variation */
    
    /* Outer loop with volatile bound */
    for (i = 0; i < loop_limit; i++) {
        volatile int inner_limit = (mode + i) % 5 + 2;
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Create instruction queue pressure with long dependency chains */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            e = a ^ f;
            f = e >> (j & 3);
            
            /* Memory accesses to volatile arrays */
            arr1[(i * 16 + j) % ARRAY_SIZE] = a + e;
            arr2[(j * 8 + i) % ARRAY_SIZE] = f - b;
            
            /* Multi-way branch creating multiple basic blocks */
            volatile int branch_selector = (i * 17 + j * 13) % 10;
            
            if (branch_selector < 3) {
                /* Branch 1: Arithmetic chain */
                c = d * e - f;
                d = c ^ a;
                __asm__ volatile ("" : : : "memory");
                arr1[(i + j) % ARRAY_SIZE] = d * 2;
            } 
            else if (branch_selector < 6) {
                /* Branch 2: Different operations */
                b = a | c;
                e = b & d;
                __asm__ volatile ("" : : : "memory");
                arr2[(i * 3 + j) % ARRAY_SIZE] = e + f;
                
                /* Nested condition for extra complexity */
                if ((j & 1) && (mode > 0)) {
                    f = arr1[j % ARRAY_SIZE] * arr2[i % ARRAY_SIZE];
                    __asm__ volatile ("" : : : "memory");
                }
            }
            else if (branch_selector < 8) {
                /* Branch 3: Memory intensive */
                for (k = 0; k < 2; k++) {
                    arr1[(i + k) % ARRAY_SIZE] = arr2[(j + k) % ARRAY_SIZE] * 3;
                    arr2[(j + k) % ARRAY_SIZE] = arr1[(i + k + 1) % ARRAY_SIZE] / 2;
                }
                __asm__ volatile ("" : : : "memory");
            }
            else {
                /* Branch 4: Function call in some cases */
                if ((iter & 1) && (j == inner_limit - 1)) {
                    /* Dummy system call to add call instruction */
                    volatile int pid = getpid();
                    arr1[i % ARRAY_SIZE] ^= pid & 0xFF;
                }
                b = (a << 2) | (c >> 1);
                __asm__ volatile ("" : : : "memory");
            }
            
            /* More arithmetic to extend dependency chains */
            c = (a + b) * (d - e);
            d = c % (f + 1);
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Switch-like structure using bit tests */
            volatile int switch_var = (i + j) & 7;
            if (switch_var & 1) {
                a = b + arr1[(i + 1) % ARRAY_SIZE];
            }
            if (switch_var & 2) {
                e = d - arr2[(j + 1) % ARRAY_SIZE];
            }
            if (switch_var & 4) {
                f = (a * e) / (b + 1);
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Final store with complex addressing */
            result += a + b + c + d + e + f;
            arr1[(i * 7 + j * 11) % ARRAY_SIZE] = result;
        }
        
        /* Loop-carried dependency */
        b = a + iter;
        c = d - i;
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int x = 1, y = 2, z = 3;
    volatile int sum = 0;
    volatile int limit = (seed % 5) + 2;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Different arithmetic pattern */
        x = y * z + seed;
        y = x ^ (i * 7);
        z = y >> (x & 3);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Complex addressing with modulo */
        arr1[(x + i) % ARRAY_SIZE] = y;
        arr2[(y + i) % ARRAY_SIZE] = z;
        
        /* Multi-branch with varying operations */
        volatile int cond = (x + y + z) % 4;
        if (cond == 0) {
            x = arr1[i % ARRAY_SIZE] * arr2[(i + 1) % ARRAY_SIZE];
            __asm__ volatile ("" : : : "memory");
        } else if (cond == 1) {
            y = (x << 2) | (z >> 1);
            arr1[(i * 3) % ARRAY_SIZE] = y;
        } else if (cond == 2) {
            z = x + y + arr2[i % ARRAY_SIZE];
            __asm__ volatile ("" : : : "memory");
        } else {
            /* Function call in some paths */
            if ((seed & 3) == 3) {
                volatile clock_t t = clock();
                arr1[i % ARRAY_SIZE] ^= t & 0xFF;
            }
        }
        
        sum += x + y + z;
    }
    
    return sum;
}

int main() {
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int total_result = 0;
    volatile int mode_switch = 0;
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int iter = 0; iter < MAX_LOOP_ITER; iter++) {
        volatile int loop_bound = (iter % 3) + 2;  /* Volatile variation */
        
        /* Alternate between two different scheduling patterns */
        if (iter & 1) {
            volatile int res1 = complex_schedule_loop(array1, array2, 
                                                     mode_switch, iter);
            total_result ^= res1;
            
            /* Modify mode for next iteration */
            mode_switch = (mode_switch + res1) & 0xF;
            __asm__ volatile ("" : : : "memory");
        } else {
            volatile int res2 = alternate_schedule_pattern(array1, array2, iter);
            total_result ^= res2;
            
            mode_switch = (mode_switch * 3 + res2) & 0xF;
            __asm__ volatile ("" : : : "memory");
        }
        
        /* Additional complexity between calls */
        for (volatile int k = 0; k < 2; k++) {
            array1[(iter * 7 + k) % ARRAY_SIZE] = total_result + k;
            array2[(iter * 11 + k) % ARRAY_SIZE] = total_result - k;
        }
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
