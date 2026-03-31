/* Complex scheduling test to trigger haifa-sched.cc free_sched_context logic */
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
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11;
    volatile int result = 0;
    volatile int loop_limit = (mode % 7) + 10;  /* Volatile limit */
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < loop_limit; outer++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex dependency chains */
        for (volatile int idx = 0; idx < ARRAY_SIZE/4; idx++) {
            /* Multiple basic blocks created by if-else chain */
            int cond = (iter + outer + idx) % 13;
            
            /* Branch 1: Long arithmetic dependency chain */
            if (cond == 0) {
                a = b * c + d;
                __asm__ volatile ("" : : : "memory");
                e = a ^ f;
                g = e >> (h % 8);
                __asm__ volatile ("" : : : "memory");
                i = g * j - k;
                arr1[idx] = i + arr2[idx];
                result ^= arr1[idx];
            }
            /* Branch 2: Memory-intensive operations */
            else if (cond == 1 || cond == 2) {
                volatile int temp = arr2[idx * 2] + arr2[idx * 2 + 1];
                __asm__ volatile ("" : : : "memory");
                arr1[idx] = temp * (cond + 1);
                b = arr1[idx] % 97;
                c = b * 3 - 1;
                __asm__ volatile ("" : : : "memory");
                result += c;
            }
            /* Branch 3: Bit manipulation chain */
            else if (cond >= 3 && cond <= 5) {
                d = (arr1[idx] << 3) | (arr2[idx] >> 2);
                e = d ^ 0xDEADBEEF;
                __asm__ volatile ("" : : : "memory");
                f = e * 1103515245 + 12345;
                g = (f & 0x7FFFFFFF) % 1000;
                arr2[idx] = g;
                result |= g;
            }
            /* Branch 4: Function call with volatile guard */
            else if (cond == 6 && (mode & 1)) {
                volatile int pid = getpid();
                arr1[idx] = (pid & 0xFF) + idx;
                __asm__ volatile ("" : : : "memory");
                result += pid % 100;
            }
            /* Branch 5: Complex multi-operation chain */
            else if (cond == 7 || cond == 8) {
                h = arr1[idx] * arr2[idx];
                i = h + (arr1[(idx + 1) % (ARRAY_SIZE/4)] << 2);
                __asm__ volatile ("" : : : "memory");
                j = i / (cond - 6);
                k = j ^ arr2[(idx + 2) % (ARRAY_SIZE/4)];
                arr1[idx] = k;
                result -= k;
            }
            /* Branch 6: Switch-like behavior using bit tests */
            else {
                int bitmask = (iter + idx) & 0xF;
                volatile int acc = 0;
                
                for (int bit = 0; bit < 4; bit++) {
                    if (bitmask & (1 << bit)) {
                        acc += arr1[idx + bit] * (bit + 1);
                        __asm__ volatile ("" : : : "memory");
                    } else {
                        acc -= arr2[idx + bit] / (bit + 2);
                    }
                }
                arr2[idx] = acc;
                result ^= acc;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Additional memory barrier to create scheduling boundaries */
            if (idx % 8 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Pseudo-random branch to vary control flow */
        if ((outer % 3) == 0) {
            volatile int temp = arr1[outer % (ARRAY_SIZE/4)];
            arr2[outer % (ARRAY_SIZE/4)] = temp * 3 + 1;
            __asm__ volatile ("" : : : "memory");
        } else if ((outer % 3) == 1) {
            volatile int temp = arr2[outer % (ARRAY_SIZE/4)];
            arr1[outer % (ARRAY_SIZE/4)] = temp / 2 - 5;
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, optimize("O3", "no-tree-vectorize")))
secondary_schedule_func(volatile int *arr, volatile int seed) {
    volatile int x = seed, y = seed * 2, z = seed * 3;
    volatile int sum = 0;
    
    for (volatile int i = 0; i < MAX_LOOP_ITER/2; i++) {
        /* Interleaved dependency chains */
        x = y * z + arr[i % (ARRAY_SIZE/2)];
        __asm__ volatile ("" : : : "memory");
        y = x ^ (z << 2);
        z = y % 127 + arr[(i + 1) % (ARRAY_SIZE/2)];
        
        /* Memory operations with barriers */
        arr[i % (ARRAY_SIZE/2)] = x + y - z;
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional with function call */
        if ((i % 7) == 0) {
            volatile clock_t t = clock();
            sum += (t & 0xFF);
        }
        
        sum += arr[i % (ARRAY_SIZE/2)];
    }
    
    return sum;
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
    
    volatile int total_result = 0;
    volatile int mode_switch = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (volatile int main_iter = 0; main_iter < 8; main_iter++) {
        /* Vary parameters to force different scheduling decisions */
        volatile int mode = (main_iter * 17) % 11;
        volatile int iter_val = main_iter * 3 + 1;
        
        /* Call core scheduling function multiple times */
        volatile int res1 = complex_schedule_loop(array1, array2, mode, iter_val);
        __asm__ volatile ("" : : : "memory");
        
        volatile int res2 = complex_schedule_loop(array2, array1, mode + 1, iter_val + 1);
        
        /* Call secondary function */
        volatile int res3 = secondary_schedule_func(array1, main_iter);
        __asm__ volatile ("" : : : "memory");
        
        /* Mix results */
        total_result ^= res1 + res2 * 3 - res3;
        
        /* Occasionally swap arrays to vary memory access patterns */
        if (main_iter % 3 == 0) {
            for (int i = 0; i < ARRAY_SIZE/2; i++) {
                volatile int tmp = array1[i];
                array1[i] = array2[ARRAY_SIZE - 1 - i];
                array2[ARRAY_SIZE - 1 - i] = tmp;
            }
        }
        
        mode_switch = (mode_switch + 1) % 5;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    volatile int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array1[i] + array2[i] * (i + 1);
    }
    
    printf("Result: %d, Checksum: %d\n", total_result, checksum);
    
    return 0;
}
