/* 
 * Complex scheduling stress test targeting haifa-sched.cc free_sched_context
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-tree-vectorize -fdump-rtl-sched1 -fdump-rtl-sched2 -o sched_test sched_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_LOOP_BOUND 50

/* Non-inlineable core scheduling function with aggressive optimization */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int mode, volatile int iter) {
    volatile int i, j, k;
    volatile int result = 0;
    volatile int outer_limit = (mode % 7) + 3;  /* Volatile to prevent constant propagation */
    volatile int inner_limit = (iter % 5) + 8;
    
    /* Create multiple basic blocks with complex control flow */
    for (i = 0; i < outer_limit; i++) {
        volatile int branch_selector = (i + mode + iter) % 11;
        
        /* Branch 1: Arithmetic dependency chain */
        if (branch_selector < 3) {
            volatile int a = arr1[i * 3];
            volatile int b = arr2[i * 2];
            volatile int c = arr1[i * 3 + 1];
            volatile int d = arr2[i * 2 + 1];
            
            /* Long dependency chain */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            b = a ^ (c << 2);
            c = b + (d >> 1);
            d = c * a - b;
            __asm__ volatile ("" : : : "memory");
            
            arr1[i * 3] = a;
            arr2[i * 2] = b;
            arr1[i * 3 + 1] = c;
            arr2[i * 2 + 1] = d;
            
            result += a + b - c + d;
        }
        /* Branch 2: Memory intensive operations */
        else if (branch_selector < 6) {
            for (j = 0; j < inner_limit; j++) {
                volatile int idx = (i * 5 + j) % ARRAY_SIZE;
                volatile int temp = arr1[idx];
                
                /* Independent operations that can be reordered */
                arr1[idx] = temp * 3 + arr2[idx];
                __asm__ volatile ("" : : : "memory");
                arr2[idx] = arr1[idx] ^ (temp << 1);
                arr1[(idx + 1) % ARRAY_SIZE] = arr2[idx] - temp;
                __asm__ volatile ("" : : : "memory");
                
                result ^= arr1[idx] | arr2[idx];
            }
        }
        /* Branch 3: Mixed operations with function call */
        else if (branch_selector < 9) {
            volatile int x = arr1[i * 4];
            volatile int y = arr2[i * 4];
            
            /* Complex arithmetic sequence */
            for (k = 0; k < 4; k++) {
                x = (x << k) | (y >> (4 - k));
                y = y ^ (x + k);
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Conditional function call to add call instruction to scheduling */
            if ((x & 0x7) == 0) {
                volatile int pid = getpid();
                x ^= pid & 0xFF;
                __asm__ volatile ("" : : : "memory");
            }
            
            arr1[i * 4] = x;
            arr2[i * 4] = y;
            result += x - y;
        }
        /* Branch 4: Switch-like multi-way computation */
        else {
            volatile int op_type = (i + iter) & 0x3;
            
            switch (op_type) {
                case 0: {
                    volatile int a = arr1[i * 2];
                    volatile int b = arr2[i * 2];
                    a = (a * b) + (a >> 2) - (b << 1);
                    b = (a ^ b) * 3;
                    arr1[i * 2] = a;
                    arr2[i * 2] = b;
                    result |= a & b;
                    break;
                }
                case 1: {
                    volatile int a = arr1[i * 2 + 1];
                    volatile int b = arr2[i * 2 + 1];
                    for (k = 0; k < 3; k++) {
                        a = (a + k) * (b - k);
                        b = (b << 1) | (a & 1);
                        __asm__ volatile ("" : : : "memory");
                    }
                    arr1[i * 2 + 1] = a;
                    arr2[i * 2 + 1] = b;
                    result ^= a + b;
                    break;
                }
                case 2: {
                    volatile int a = arr1[i];
                    volatile int b = arr2[ARRAY_SIZE - i - 1];
                    a = a * 7 + b * 3;
                    b = (a << 3) - (b >> 2);
                    arr1[i] = a;
                    arr2[ARRAY_SIZE - i - 1] = b;
                    result &= a | b;
                    break;
                }
                case 3: {
                    volatile int a = arr1[ARRAY_SIZE - i - 1];
                    volatile int b = arr2[i];
                    /* Multiple independent operations */
                    a = a + b * 2;
                    __asm__ volatile ("" : : : "memory");
                    b = b - a / 4;
                    a = a ^ 0xAA;
                    b = b | 0x55;
                    __asm__ volatile ("" : : : "memory");
                    arr1[ARRAY_SIZE - i - 1] = a;
                    arr2[i] = b;
                    result += a * b;
                    break;
                }
            }
        }
        
        /* Additional scheduling barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

/* Secondary scheduling function with different pattern */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int seed) {
    volatile int i, j;
    volatile int sum = 0;
    volatile int limit = (seed % 10) + 5;
    
    for (i = 0; i < limit; i++) {
        volatile int pattern = (seed + i) & 0xF;
        
        /* Create instruction queue pressure with parallel operations */
        volatile int a = arr1[i * 2];
        volatile int b = arr2[i * 2];
        volatile int c = arr1[i * 2 + 1];
        volatile int d = arr2[i * 2 + 1];
        
        /* Multiple independent computation chains */
        a = a * 3 + pattern;
        b = b ^ (pattern << 1);
        c = c + a - b;
        d = d * 2 ^ c;
        
        __asm__ volatile ("" : : : "memory");
        
        /* More operations with dependencies */
        for (j = 0; j < 2; j++) {
            a = (a << j) | (b >> (8 - j));
            b = b + (c * j);
            c = c ^ (d - j);
            d = d | (a & 0xFF);
        }
        
        arr1[i * 2] = a;
        arr2[i * 2] = b;
        arr1[i * 2 + 1] = c;
        arr2[i * 2 + 1] = d;
        
        sum += a + b + c + d;
        
        __asm__ volatile ("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int i, j;
    volatile int checksum = 0;
    
    /* Seed for deterministic but complex patterns */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple calls to create different scheduling contexts */
    for (i = 0; i < 8; i++) {
        volatile int mode = i % 4;
        volatile int result;
        
        /* Alternate between two different scheduling patterns */
        if (i & 1) {
            result = complex_schedule_loop(array1, array2, mode, i);
        } else {
            result = alternate_schedule_pattern(array1, array2, i);
        }
        
        checksum ^= result;
        
        /* Modify array contents between calls */
        for (j = 0; j < ARRAY_SIZE / 8; j++) {
            volatile int idx = (i * 31 + j * 7) % ARRAY_SIZE;
            array1[idx] = (array1[idx] + result) & 0x7FF;
            array2[idx] = (array2[idx] ^ result) & 0x7FF;
        }
        
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to ensure all results are used */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] - array2[i];
        checksum ^= (array1[i] * array2[i]) & 0xFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
