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
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    volatile int result = 0;
    volatile int outer_limit = (iter % 3) + 5;  /* Non-constant bounds */
    
    /* Volatile array for additional memory pressure */
    volatile int temp[16];
    for (k = 0; k < 16; k++) {
        temp[k] = k * iter;
    }
    
    /* Outer loop with volatile limit */
    for (i = 0; i < outer_limit; i++) {
        volatile int inner_limit = (mode + i) % 7 + 3;
        
        /* Complex inner loop with multiple basic blocks */
        for (j = 0; j < inner_limit; j++) {
            /* Branch 1: Arithmetic chain with dependencies */
            if ((iter + i + j) % 4 == 0) {
                a = arr1[(i * 16 + j) % ARRAY_SIZE];
                b = arr2[(j * 8 + i) % ARRAY_SIZE];
                c = a * b + iter;
                d = c ^ (a >> 2);
                e = d * 3 - b;
                f = e & 0xFF;
                
                /* Memory barrier to split scheduling regions */
                __asm__ volatile ("" : : : "memory");
                
                arr1[(i + j) % ARRAY_SIZE] = f;
                result += f;
            }
            /* Branch 2: Different arithmetic pattern */
            else if ((iter + i + j) % 4 == 1) {
                a = arr2[(i * 4 + j * 3) % ARRAY_SIZE];
                b = arr1[(j * 5 + i * 2) % ARRAY_SIZE];
                c = (a + b) * 7;
                d = c | (a << 3);
                e = d - (b % 17);
                
                __asm__ volatile ("" : : : "memory");
                
                arr2[(i * 2 + j) % ARRAY_SIZE] = e;
                result ^= e;
            }
            /* Branch 3: Memory intensive operations */
            else if ((iter + i + j) % 4 == 2) {
                /* Long dependency chain */
                a = temp[j % 16];
                b = a * 3 + 1;
                c = b ^ 0x5A5A;
                d = c << (j % 4);
                e = d + arr1[(i + j * 2) % ARRAY_SIZE];
                f = e * 11;
                
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                
                for (k = 0; k < 4; k++) {
                    temp[(j + k) % 16] += f >> k;
                }
                
                arr1[(i * 3) % ARRAY_SIZE] = f;
                result |= f;
            }
            /* Branch 4: Function call and complex operations */
            else {
                volatile int r = rand() % 100;
                if (r > 50) {
                    /* Introduce function call in scheduling region */
                    volatile int pid = getpid();
                    a = pid & 0xFF;
                } else {
                    a = clock() & 0xFF;
                }
                
                b = arr2[(i * 5 + j * 7) % ARRAY_SIZE];
                c = (a * b) % 7919;  /* Prime number for variability */
                d = c + (iter << 4);
                e = d ^ (j * 173);
                f = e * 2 - a;
                
                __asm__ volatile ("" : : : "memory");
                
                /* Multi-way store */
                if (f % 3 == 0) {
                    arr1[(i + j * 3) % ARRAY_SIZE] = f;
                } else if (f % 3 == 1) {
                    arr2[(j + i * 4) % ARRAY_SIZE] = f;
                } else {
                    temp[i % 16] = f;
                }
                
                result = result * 13 + f;
            }
            
            /* Additional independent operations to fill instruction queue */
            volatile int x = arr1[(i * 11 + j * 13) % ARRAY_SIZE];
            volatile int y = arr2[(i * 17 + j * 19) % ARRAY_SIZE];
            volatile int z = x * y - (i * j);
            volatile int w = z ^ (x + y);
            
            /* More memory barriers to create scheduling boundaries */
            __asm__ volatile ("" : : : "memory");
            
            /* Store results with different addressing modes */
            if ((i + j) & 1) {
                arr1[(z % ARRAY_SIZE)] = w;
            } else {
                arr2[(w % ARRAY_SIZE)] = z;
            }
            
            /* Switch-like multi-branch structure */
            switch ((i + j + iter) % 5) {
                case 0:
                    result += x * 2;
                    break;
                case 1:
                    result -= y / 3;
                    break;
                case 2:
                    result ^= (x | y);
                    break;
                case 3:
                    result = result << 1;
                    break;
                case 4:
                    result = result >> 1;
                    break;
            }
        }
        
        /* Loop-carried dependency to force scheduling complexity */
        volatile int carry = result % 1024;
        for (k = 0; k < 4; k++) {
            temp[(i + k) % 16] = (temp[(i + k) % 16] + carry) & 0xFF;
            carry = (carry * 3) % 1024;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary complex function to increase scheduling contexts */
static volatile int __attribute__((noinline, noipa, optimize("O3")))
secondary_schedule_func(volatile int *arr, volatile int seed) {
    volatile int i, j;
    volatile int acc = 0;
    volatile int limit = (seed % 5) + 2;
    
    for (i = 0; i < limit; i++) {
        volatile int inner = (seed + i) % 3 + 4;
        for (j = 0; j < inner; j++) {
            /* Complex bit manipulation chain */
            volatile int x = arr[(i * 31 + j * 7) % ARRAY_SIZE];
            volatile int y = arr[(j * 23 + i * 11) % ARRAY_SIZE];
            volatile int t = x ^ y;
            
            t = (t << 3) | (t >> 5);
            t = t * 9 + 1;
            t = t ^ (x & y);
            t = t + (seed << j);
            
            __asm__ volatile ("" : : : "memory");
            
            arr[(i + j * 2) % ARRAY_SIZE] = t;
            acc += t;
            
            /* Conditional function call */
            if ((t % 7) == 0) {
                volatile int dummy = clock();
                acc ^= (dummy & 0xFF);
            }
        }
    }
    
    return acc;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex behavior */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple iterations to increase scheduling activity */
    for (i = 0; i < MAX_LOOP_ITER; i++) {
        volatile int mode = (i % 4);
        volatile int iter = i;
        
        /* Call core scheduling function multiple times */
        volatile int result1 = complex_schedule_loop(array1, array2, mode, iter);
        volatile int result2 = complex_schedule_loop(array2, array1, mode ^ 1, iter + 1);
        
        /* Call secondary function */
        volatile int result3 = secondary_schedule_func(array1, i);
        volatile int result4 = secondary_schedule_func(array2, i + 3);
        
        checksum ^= result1;
        checksum ^= result2;
        checksum ^= result3;
        checksum ^= result4;
        
        /* Additional mixing */
        for (j = 0; j < ARRAY_SIZE / 8; j++) {
            array1[(i * 17 + j) % ARRAY_SIZE] ^= checksum;
            array2[(i * 23 + j) % ARRAY_SIZE] += checksum;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final checksum computation to prevent elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
