/* haifa-sched-trigger.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-sched-trigger
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
    volatile int loop_limit = (iter % 5) + 10;  /* Volatile-like computation */
    
    /* Outer loop with volatile limit */
    for (i = 0; i < loop_limit; i++) {
        volatile int inner_limit = (mode + i) % 7 + 3;
        
        /* Complex inner loop with scheduling barriers */
        for (j = 0; j < inner_limit; j++) {
            /* Create long dependency chains */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            
            e = a ^ f;
            f = e >> (j & 3);
            
            /* Memory accesses to volatile arrays */
            arr1[(i + j) % ARRAY_SIZE] = a + e;
            arr2[(i * j) % ARRAY_SIZE] = b ^ f;
            
            __asm__ volatile ("" : : : "memory");  /* Another barrier */
            
            /* Multi-way branching creating multiple basic blocks */
            volatile int branch_selector = (a + b + c) % 8;
            
            if (branch_selector == 0) {
                /* Branch 0: Arithmetic chain */
                c = d * e - f;
                d = c ^ a;
                arr1[i % ARRAY_SIZE] = d + mode;
            } 
            else if (branch_selector == 1) {
                /* Branch 1: Different operations */
                b = a | c;
                e = b & d;
                arr2[j % ARRAY_SIZE] = e * 3;
            } 
            else if (branch_selector == 2) {
                /* Branch 2: Memory intensive */
                for (k = 0; k < 3; k++) {
                    arr1[(i + k) % ARRAY_SIZE] += arr2[(j + k) % ARRAY_SIZE];
                }
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector == 3) {
                /* Branch 3: Function call (adds call instruction) */
                if ((mode & 1) && (iter > 2)) {
                    volatile int pid = getpid();
                    arr1[0] ^= pid & 0xFF;
                }
            } 
            else if (branch_selector == 4) {
                /* Branch 4: Complex bit operations */
                a = (a << 2) | (b >> 1);
                b = (b ^ c) + d;
                c = c * 2 - e;
                arr2[(i + 1) % ARRAY_SIZE] = a + b + c;
            } 
            else if (branch_selector == 5) {
                /* Branch 5: Nested conditionals */
                if (a > b) {
                    d = a - b;
                    arr1[(j + 2) % ARRAY_SIZE] = d * 2;
                } else {
                    d = b - a;
                    arr2[(j + 3) % ARRAY_SIZE] = d / 2;
                }
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector == 6) {
                /* Branch 6: More arithmetic chains */
                e = (a * b) + (c * d);
                f = (e ^ 0x55AA) >> 1;
                a = f + mode;
                b = a * iter;
            } 
            else { /* branch_selector == 7 */
                /* Branch 7: Switch-like behavior with memory barriers */
                volatile int sub_sel = (a + j) & 3;
                if (sub_sel == 0) {
                    arr1[i % ARRAY_SIZE] = arr2[j % ARRAY_SIZE] + 1;
                } else if (sub_sel == 1) {
                    arr2[j % ARRAY_SIZE] = arr1[i % ARRAY_SIZE] - 1;
                    __asm__ volatile ("" : : : "memory");
                } else if (sub_sel == 2) {
                    arr1[i % ARRAY_SIZE] ^= arr2[j % ARRAY_SIZE];
                } else {
                    arr2[j % ARRAY_SIZE] ^= arr1[i % ARRAY_SIZE];
                }
            }
            
            /* More independent instructions to fill instruction queue */
            volatile int tmp1 = arr1[(i + j + 1) % ARRAY_SIZE];
            volatile int tmp2 = arr2[(i + j + 2) % ARRAY_SIZE];
            volatile int tmp3 = tmp1 * tmp2 + a;
            volatile int tmp4 = tmp3 ^ b;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Use results to prevent elimination */
            result += tmp4;
            arr1[(i + j) % ARRAY_SIZE] = result & 0xFF;
        }
        
        /* Additional operations between inner loop iterations */
        if (i % 3 == 0) {
            volatile int x = arr1[i % ARRAY_SIZE];
            volatile int y = arr2[(i + 1) % ARRAY_SIZE];
            arr1[(i + 2) % ARRAY_SIZE] = x * y + mode;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int i, j;
    volatile int acc = seed;
    volatile int limit = (seed % 10) + 5;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (i * 7 + seed) % 8 + 2;
        
        for (j = 0; j < inner; j++) {
            /* Different pattern of operations */
            volatile int x = arr1[(i + j * 3) % ARRAY_SIZE];
            volatile int y = arr2[(i * 2 + j) % ARRAY_SIZE];
            
            /* Bit manipulation chain */
            x = (x << 1) | (y & 1);
            y = (y >> 2) ^ x;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Conditional with function call */
            if ((x + y) % 5 == 0) {
                volatile int t = clock() & 0xFF;
                arr1[(i + j) % ARRAY_SIZE] = t;
            }
            
            /* Arithmetic dependency chain */
            acc = acc * 1103515245 + 12345;
            volatile int r = (acc >> 16) & 0x7FFF;
            
            arr2[(i * j) % ARRAY_SIZE] = r;
            arr1[(j * 2) % ARRAY_SIZE] = x + y + r;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Multi-way branch */
            switch ((x + y + i) & 3) {
                case 0:
                    arr1[i % ARRAY_SIZE] += r;
                    break;
                case 1:
                    arr2[j % ARRAY_SIZE] -= r;
                    break;
                case 2:
                    arr1[(i + 1) % ARRAY_SIZE] ^= r;
                    break;
                case 3:
                    arr2[(j + 1) % ARRAY_SIZE] |= r;
                    break;
            }
        }
    }
    
    return acc;
}

int main(void) {
    volatile int arr1[ARRAY_SIZE];
    volatile int arr2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create scheduling contexts */
    for (i = 0; i < MAX_LOOP_ITER; i++) {
        volatile int mode = i % 4;
        volatile int iter = i;
        
        /* Call core scheduling function */
        volatile int res1 = complex_schedule_loop(arr1, arr2, mode, iter);
        
        /* Call alternate function */
        volatile int res2 = alternate_schedule_pattern(arr1, arr2, i * 17 + 123);
        
        /* Use results to prevent elimination */
        arr1[i % ARRAY_SIZE] ^= res1 & 0xFF;
        arr2[(i + 1) % ARRAY_SIZE] ^= res2 & 0xFF;
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
