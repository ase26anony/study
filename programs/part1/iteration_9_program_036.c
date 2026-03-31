/* haifa-sched-trigger.c
 * Program designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 haifa-sched-trigger.c -o haifa-test
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
    volatile int i, j, k;
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0;
    volatile int result = 0;
    volatile int outer_limit = (iter % 7) + 3;  /* Volatile-like computation */
    
    /* Volatile array for intermediate results */
    volatile int temp[16];
    for (k = 0; k < 16; k++) temp[k] = k * iter;
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (mode + i) % 8 + 5;
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Create long dependency chains */
            a = arr1[(i * 16 + j) % ARRAY_SIZE];
            b = arr2[(j * 7 + i) % ARRAY_SIZE];
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            /* Branch 1: Arithmetic operations */
            if ((a ^ b) & 0x1) {
                c = a * b + (i << 3);
                d = c ^ (b >> 2);
                e = d * 3 - (a & 0xFF);
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Nested if for additional basic block */
                if (e > 1000) {
                    result += e >> 4;
                    /* Function call in one branch */
                    if ((iter + j) % 13 == 0) {
                        volatile int pid = getpid();
                        result ^= (pid & 0xFF);
                    }
                } else {
                    result -= e << 2;
                }
                
                /* Store intermediate result */
                temp[j % 16] = e;
            }
            /* Branch 2: Different operations */
            else if ((a + b) % 5 == 0) {
                c = (a << 4) | (b & 0xF);
                d = c * c - b;
                
                /* Memory access pattern */
                volatile int *ptr = &temp[(i + j) % 16];
                *ptr = d % 256;
                
                /* Complex computation chain */
                for (k = 0; k < 3; k++) {
                    e = (d << k) ^ (a >> k);
                    result += e;
                    __asm__ volatile ("" : : : "memory");
                }
            }
            /* Branch 3: More operations */
            else if ((a | b) < 256) {
                c = a ^ b ^ iter;
                d = (c * 7 + 11) & 0x3FF;
                
                /* Switch-like behavior using bit tests */
                for (k = 0; k < 4; k++) {
                    if (d & (1 << k)) {
                        e = temp[k] * (j + 1);
                        result ^= e;
                    } else {
                        e = temp[k + 4] / (i + 1);
                        result |= e;
                    }
                }
                
                __asm__ volatile ("" : : : "memory");
            }
            /* Default branch */
            else {
                c = (a + b) * (i - j);
                d = c % 127;
                e = (d << 1) | (mode & 1);
                
                /* Another scheduling barrier */
                __asm__ volatile ("" : : : "memory");
                
                /* Multiple independent operations */
                temp[0] = e + arr1[(i + 1) % ARRAY_SIZE];
                temp[1] = e - arr2[(j + 1) % ARRAY_SIZE];
                temp[2] = e * arr1[(i + 2) % ARRAY_SIZE];
                temp[3] = e ^ arr2[(j + 2) % ARRAY_SIZE];
                
                result += temp[0] + temp[1] - temp[2] ^ temp[3];
            }
            
            /* Final computation with memory barrier */
            __asm__ volatile ("" : : : "memory");
            arr1[(i * 16 + j) % ARRAY_SIZE] = result & 0xFF;
            arr2[(j * 7 + i) % ARRAY_SIZE] = (result >> 8) & 0xFF;
        }
        
        /* Additional control flow variation */
        switch (i % 4) {
            case 0:
                result += temp[0] * 2;
                break;
            case 1:
                result -= temp[1] / 2;
                /* Potential function call */
                if ((iter + i) % 17 == 0) {
                    volatile clock_t clk = clock();
                    result ^= (clk & 0xFF);
                }
                break;
            case 2:
                result ^= temp[2] | 0xAA;
                break;
            case 3:
                result = (result << 3) | (temp[3] & 7);
                break;
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
another_schedule_function(volatile int *arr, volatile int seed) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (seed % 10) + 5;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (seed + i) % 8 + 2;
        
        for (j = 0; j < inner; j++) {
            /* Complex dependency chain */
            volatile int x = arr[(i * 3 + j * 7) % ARRAY_SIZE];
            volatile int y = arr[(j * 5 + i * 11) % ARRAY_SIZE];
            
            __asm__ volatile ("" : : : "memory");
            
            /* Multiple computation paths */
            if (x > y) {
                volatile int t1 = x * y + (i << j);
                volatile int t2 = t1 ^ (x >> (j % 4));
                volatile int t3 = t2 * 3 - y;
                acc += t3;
                
                /* Memory operations */
                arr[(i + j) % ARRAY_SIZE] = t3 & 0xFF;
            } else {
                volatile int t1 = (x << 4) | (y & 0xF);
                volatile int t2 = t1 * t1 - x;
                acc ^= t2;
                
                __asm__ volatile ("" : : : "memory");
                
                arr[(i * 2 + j) % ARRAY_SIZE] = t2 % 256;
            }
            
            /* Additional barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Bit manipulation chain */
            for (volatile int k = 0; k < 3; k++) {
                volatile int mask = 1 << ((i + j + k) % 8);
                if (acc & mask) {
                    acc = (acc << 1) | 1;
                } else {
                    acc = (acc >> 1) & 0x7FFFFFFF;
                }
            }
        }
    }
    
    return acc;
}

int main() {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
    }
    
    /* Multiple calls to create scheduling contexts */
    for (i = 0; i < 8; i++) {
        volatile int mode = (i % 3) + 1;
        volatile int result;
        
        /* Call core scheduling function */
        result = complex_schedule_loop(array1, array2, mode, i);
        checksum ^= result;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Call secondary function */
        result = another_schedule_function(array1, i);
        checksum += result;
        
        /* Alternate array usage */
        if (i % 2 == 0) {
            result = complex_schedule_loop(array2, array1, mode + 1, i + 10);
            checksum ^= result;
        }
        
        /* Additional complexity with varying loop bounds */
        volatile int extra_loops = (i % 4) + 2;
        for (j = 0; j < extra_loops; j++) {
            volatile int temp = array1[(i * 17 + j * 23) % ARRAY_SIZE];
            volatile int temp2 = array2[(i * 13 + j * 29) % ARRAY_SIZE];
            
            /* More scheduling barriers */
            __asm__ volatile ("" : : : "memory");
            
            array1[(i + j) % ARRAY_SIZE] = (temp * temp2 + checksum) & 0xFF;
            array2[(i * 2 + j) % ARRAY_SIZE] = (temp ^ temp2 ^ checksum) & 0xFF;
        }
    }
    
    /* Final checksum computation to prevent elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum % 1000000);
    
    return checksum & 1;
}
