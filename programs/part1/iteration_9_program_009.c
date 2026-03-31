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
#define NUM_BRANCHES 8

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int limit_outer = (iter % 7) + 3;  /* Volatile-like computation */
    volatile int limit_inner = (mode % 5) + 10;
    volatile int accumulator = 0;
    volatile int branch_selector;
    
    /* Outer loop with volatile bounds */
    for (i = 0; i < limit_outer; i++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex operations */
        for (j = 0; j < limit_inner; j++) {
            /* Pseudo-random branch selection */
            branch_selector = (i * 17 + j * 13 + mode) % NUM_BRANCHES;
            
            /* Long dependency chain starters */
            volatile int a = arr1[(i + j) % ARRAY_SIZE];
            volatile int b = arr2[(i * j) % ARRAY_SIZE];
            volatile int c = arr1[(i + iter) % ARRAY_SIZE];
            volatile int d = arr2[(j + mode) % ARRAY_SIZE];
            
            /* Complex multi-branch structure */
            if (branch_selector == 0) {
                /* Arithmetic dependency chain */
                volatile int t1 = a * b + c;
                volatile int t2 = t1 ^ d;
                volatile int t3 = t2 >> (mode & 3);
                volatile int t4 = t3 * t1 - t2;
                arr1[(i + j) % ARRAY_SIZE] = t4;
                accumulator += t4;
                
                /* Memory barrier */
                __asm__ volatile ("" : : : "memory");
            } 
            else if (branch_selector == 1) {
                /* Different arithmetic pattern */
                volatile int t1 = (a << 2) | (b & 0xFF);
                volatile int t2 = t1 * d;
                volatile int t3 = t2 - c;
                volatile int t4 = t3 ^ (t1 + 1);
                arr2[(i * j) % ARRAY_SIZE] = t4;
                accumulator -= t4;
                
                /* Introduce function call in some branches */
                if ((iter + j) % 11 == 0) {
                    volatile int pid = getpid();
                    arr1[(i + pid) % ARRAY_SIZE] ^= pid;
                }
            }
            else if (branch_selector == 2) {
                /* Memory intensive operations */
                volatile int t1 = arr1[a % ARRAY_SIZE];
                volatile int t2 = arr2[b % ARRAY_SIZE];
                volatile int t3 = t1 + t2;
                volatile int t4 = t3 * arr1[c % ARRAY_SIZE];
                volatile int t5 = t4 / (d ? d : 1);
                arr1[(i + j + 1) % ARRAY_SIZE] = t5;
                accumulator ^= t5;
            }
            else if (branch_selector == 3) {
                /* Bit manipulation chain */
                volatile int t1 = a ^ b;
                volatile int t2 = t1 << (c & 7);
                volatile int t3 = t2 | d;
                volatile int t4 = ~t3;
                volatile int t5 = t4 & 0xAAAAAAAA;
                arr2[(j + iter) % ARRAY_SIZE] = t5;
                accumulator |= t5;
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
            }
            else if (branch_selector == 4) {
                /* Mixed operations with conditional */
                volatile int t1 = a + b;
                volatile int t2 = c - d;
                volatile int t3 = (t1 > t2) ? t1 : t2;
                volatile int t4 = t3 * (a % 16);
                volatile int t5 = t4 + (b % 8);
                arr1[(i * 3 + j) % ARRAY_SIZE] = t5;
                accumulator = accumulator * 3 + t5;
            }
            else if (branch_selector == 5) {
                /* Complex chain with intermediate stores */
                volatile int t1 = (a * 3) / (b ? b : 1);
                arr1[(i + 5) % ARRAY_SIZE] = t1;
                
                volatile int t2 = (c ^ d) + t1;
                arr2[(j + 3) % ARRAY_SIZE] = t2;
                
                volatile int t3 = (t1 << 1) | (t2 & 1);
                volatile int t4 = t3 * 7 - 13;
                arr1[(i + j + 7) % ARRAY_SIZE] = t4;
                accumulator += t4;
            }
            else if (branch_selector == 6) {
                /* Nested conditionals */
                if (a > b) {
                    volatile int t1 = a - b;
                    if (c > d) {
                        volatile int t2 = c - d;
                        volatile int t3 = t1 * t2;
                        arr1[(i + j) % ARRAY_SIZE] = t3;
                        accumulator += t3;
                    } else {
                        volatile int t2 = d - c;
                        volatile int t3 = t1 + t2;
                        arr2[(i + j) % ARRAY_SIZE] = t3;
                        accumulator -= t3;
                    }
                } else {
                    volatile int t1 = b - a;
                    volatile int t2 = (c + d) / 2;
                    volatile int t3 = t1 ^ t2;
                    arr1[(i * 2) % ARRAY_SIZE] = t3;
                    accumulator ^= t3;
                }
            }
            else { /* branch_selector == 7 */
                /* Function call with side effects */
                volatile clock_t clk = clock();
                volatile int t1 = (int)(clk & 0xFF);
                volatile int t2 = a + t1;
                volatile int t3 = b * t2;
                volatile int t4 = t3 ^ (c + d);
                arr2[(j * 2) % ARRAY_SIZE] = t4;
                accumulator = (accumulator << 3) | (t4 & 7);
                
                /* Memory barrier after function call */
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Independent parallel operations to fill instruction queue */
            volatile int p1 = arr1[(i + 1) % ARRAY_SIZE];
            volatile int p2 = arr2[(j + 2) % ARRAY_SIZE];
            volatile int p3 = p1 * p2;
            volatile int p4 = p3 + accumulator;
            volatile int p5 = p4 ^ iter;
            
            /* Store results in both arrays */
            if ((i + j) & 1) {
                arr1[(i + j + 3) % ARRAY_SIZE] = p5;
            } else {
                arr2[(i + j + 5) % ARRAY_SIZE] = p5;
            }
            
            /* Another memory barrier to potentially cause state save */
            if ((i * j) % 17 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Switch-like structure using bit masks */
        volatile int mask = mode & 0xF;
        for (k = 0; k < 4; k++) {
            if (mask & (1 << k)) {
                volatile int idx = (i * 4 + k) % ARRAY_SIZE;
                volatile int val = arr1[idx] + arr2[idx];
                arr1[idx] = val * 3 - 7;
                arr2[idx] = val ^ 0x55;
            }
        }
    }
    
    return accumulator;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int size, volatile int rounds) {
    volatile int i, j, result = 0;
    
    for (i = 0; i < rounds; i++) {
        volatile int limit = (i % 5) + 2;
        
        for (j = 0; j < limit; j++) {
            /* Complex addressing patterns */
            volatile int idx1 = (i * 19 + j * 23) % size;
            volatile int idx2 = (i * 29 + j * 31) % size;
            volatile int idx3 = (i * 37 + j * 41) % size;
            
            /* Long dependency chain */
            volatile int x = arr[idx1];
            volatile int y = arr[idx2];
            volatile int z = arr[idx3];
            
            volatile int t1 = x * y + z;
            volatile int t2 = t1 ^ (x + y);
            volatile int t3 = t2 >> (j & 3);
            volatile int t4 = t3 * 7 - 11;
            volatile int t5 = t4 ^ t2;
            
            arr[idx1] = t5;
            result += t5;
            
            /* Memory barrier every few iterations */
            if ((i + j) % 7 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return result;
}

int main(void) {
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Volatile control variables */
    volatile int i, j;
    volatile int outer_limit = 8;
    volatile int mode_switch = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Main loop to create multiple scheduling contexts */
    for (i = 0; i < outer_limit; i++) {
        volatile int mode = (i * 3) % 11;
        volatile int iter_count = (i % 4) + 3;
        
        /* Call core scheduling function multiple times */
        for (j = 0; j < iter_count; j++) {
            volatile int result = complex_schedule_loop(array1, array2, mode, i * 10 + j);
            
            /* Use result to prevent dead code elimination */
            array1[(i + j) % ARRAY_SIZE] ^= result;
            
            /* Mode switching */
            mode_switch = (mode_switch + result) % 7;
        }
        
        /* Call secondary function to add more scheduling complexity */
        if (i % 2 == 0) {
            volatile int sec_result = secondary_schedule_func(array1, ARRAY_SIZE, 5);
            array2[i % ARRAY_SIZE] += sec_result;
        }
        
        /* Additional memory barrier between major iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    volatile int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
