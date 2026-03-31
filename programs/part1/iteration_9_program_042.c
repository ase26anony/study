/* haifa-sched-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 256
#define MAX_QUEUE_PRESSURE 128

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11;
    volatile int result = 0;
    volatile int temp_store[16];
    
    /* Outer loop with volatile limit to prevent compile-time optimization */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Create scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex dependency chains */
        for (int idx = 0; idx < MAX_QUEUE_PRESSURE; idx++) {
            /* Multiple basic blocks created by if-else chain */
            if (mode & (1 << 0)) {
                /* Long dependency chain 1 */
                a = b * c + d;
                __asm__ volatile ("" : : : "memory");
                e = a ^ f;
                g = e >> (h & 7);
                arr1[idx % ARRAY_SIZE] = g + arr2[(idx + 1) % ARRAY_SIZE];
            } else if (mode & (1 << 1)) {
                /* Long dependency chain 2 */
                b = c * d - e;
                f = b ^ g;
                h = f << (i & 7);
                arr2[idx % ARRAY_SIZE] = h - arr1[(idx + 2) % ARRAY_SIZE];
            } else if (mode & (1 << 2)) {
                /* Memory-intensive operations */
                for (int t = 0; t < 4; t++) {
                    temp_store[t] = arr1[(idx + t) % ARRAY_SIZE] * 
                                   arr2[(idx + t + 1) % ARRAY_SIZE];
                    __asm__ volatile ("" : : : "memory");
                }
                /* Complex calculation with memory results */
                j = temp_store[0] + temp_store[1] - temp_store[2] * temp_store[3];
                arr1[idx % ARRAY_SIZE] = j;
            } else if (mode & (1 << 3)) {
                /* Function call in one branch - adds call instruction to scheduling mix */
                if ((idx & 15) == 0) {
                    volatile int pid = getpid();
                    arr2[idx % ARRAY_SIZE] ^= (pid & 255);
                    __asm__ volatile ("" : : : "memory");
                }
                /* Mixed operations */
                k = (arr1[idx % ARRAY_SIZE] << 3) | (arr2[idx % ARRAY_SIZE] >> 5);
                arr1[(idx + 8) % ARRAY_SIZE] = k;
            } else {
                /* Default path with arithmetic operations */
                d = e * f / (g + 1);
                i = d ^ j;
                arr2[idx % ARRAY_SIZE] = i;
            }
            
            /* Pseudo-random branching to create varying control flow */
            int branch_selector = (idx * 1103515245 + 12345) & 255;
            if (branch_selector < 64) {
                /* Path A: More arithmetic */
                a = b + c;
                c = d - e;
                __asm__ volatile ("" : : : "memory");
            } else if (branch_selector < 128) {
                /* Path B: Memory operations */
                arr1[(idx + 4) % ARRAY_SIZE] = arr2[idx % ARRAY_SIZE] * 3;
            } else if (branch_selector < 192) {
                /* Path C: Bit operations */
                f = g ^ h;
                h = i | j;
                arr2[(idx + 8) % ARRAY_SIZE] = f & h;
            } else {
                /* Path D: Scheduling barrier and computation */
                __asm__ volatile ("" : : : "memory");
                k = (a << 2) | (b << 1) | c;
                arr1[idx % ARRAY_SIZE] = k;
            }
            
            /* Intermix independent operations to fill instruction queue */
            volatile int x = arr1[(idx + 16) % ARRAY_SIZE];
            volatile int y = arr2[(idx + 32) % ARRAY_SIZE];
            volatile int z = x * y + idx;
            
            /* Another scheduling barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Store result with complex addressing */
            arr1[(idx * 3) % ARRAY_SIZE] = z;
            arr2[(idx * 5) % ARRAY_SIZE] = z ^ (idx * 7);
            
            /* Create loop-carried dependency */
            result += z;
            if (result > 1000000) result = result % 1000;
        }
        
        /* Switch-like structure compiled to jump table */
        switch (outer & 7) {
            case 0:
                a = b + c;
                __asm__ volatile ("" : : : "memory");
                break;
            case 1:
                b = c * d;
                break;
            case 2:
                c = d ^ e;
                __asm__ volatile ("" : : : "memory");
                break;
            case 3:
                d = e - f;
                break;
            case 4:
                e = f | g;
                __asm__ volatile ("" : : : "memory");
                break;
            case 5:
                f = g & h;
                break;
            case 6:
                g = h << 2;
                __asm__ volatile ("" : : : "memory");
                break;
            case 7:
                h = i >> 1;
                break;
        }
    }
    
    return result;
}

/* Secondary function to create additional scheduling contexts */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int limit) {
    volatile int sum = 0;
    volatile int local_var = 1;
    
    for (volatile int i = 0; i < limit; i++) {
        /* Create multiple dependency chains */
        int idx = i % ARRAY_SIZE;
        
        /* Chain 1 */
        volatile int t1 = arr[idx] * 3;
        volatile int t2 = t1 + arr[(idx + 1) % ARRAY_SIZE];
        __asm__ volatile ("" : : : "memory");
        
        /* Chain 2 */
        volatile int t3 = arr[(idx + 2) % ARRAY_SIZE] >> 2;
        volatile int t4 = t3 ^ t2;
        
        /* Chain 3 */
        volatile int t5 = local_var * 7;
        volatile int t6 = t5 - t4;
        
        /* Store with barrier */
        arr[idx] = t6;
        __asm__ volatile ("" : : : "memory");
        
        /* Accumulate */
        sum += t6;
        local_var = (local_var * 1103515245 + 12345) & 0x7FFF;
        
        /* Conditional with volatile to prevent optimization */
        if (local_var > 10000) {
            __asm__ volatile ("" : : : "memory");
            arr[(idx + 8) % ARRAY_SIZE] = sum;
        }
    }
    
    return sum;
}

int main() {
    /* Seed for deterministic pseudo-random behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int outer_loop_limit = 5;
    volatile int total_result = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < outer_loop_limit; iter++) {
        /* Vary parameters to create different scheduling scenarios */
        volatile int mode = (iter * 17) & 15;
        volatile int limit = 3 + (iter % 3);
        
        /* Call core scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, limit, mode);
        __asm__ volatile ("" : : : "memory");
        
        volatile int res2 = secondary_schedule_func(array1, limit * 2);
        __asm__ volatile ("" : : : "memory");
        
        volatile int res3 = complex_schedule_loop(array2, array1, limit + 1, mode ^ 7);
        
        /* Accumulate results to prevent dead code elimination */
        total_result += res1 + res2 + res3;
        
        /* Occasionally call clock() to add function call scheduling complexity */
        if ((iter & 3) == 0) {
            volatile clock_t t = clock();
            array1[iter % ARRAY_SIZE] ^= (t & 255);
        }
    }
    
    /* Compute checksum to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i];
        checksum ^= array2[i];
    }
    checksum ^= total_result;
    
    printf("Result checksum: %d\n", checksum);
    
    return 0;
}
