#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int result = 0;
    volatile int i, j;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Create scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Complex inner loop with varying bounds */
        volatile int inner_limit = (mode_flag % 8) + 16;
        for (j = 0; j < inner_limit; j++) {
            /* Multiple basic blocks created by if-else chain */
            int branch = (i + j) % 7;
            
            /* Branch 0: Long dependency chain */
            if (branch == 0) {
                a = b * c + d;
                e = a ^ f;
                f = e >> (j % 4);
                d = f * a - b;
                c = d + e * f;
                b = c ^ a;
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 1: Memory operations with barriers */
            else if (branch == 1) {
                arr1[j % 256] = arr2[(j + 1) % 256] * arr1[(j + 2) % 256];
                __asm__ volatile ("" : : : "memory");
                arr2[j % 256] = arr1[(j + 3) % 256] + arr2[(j + 4) % 256];
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 2: Complex arithmetic with memory */
            else if (branch == 2) {
                volatile int t1 = arr1[j % 256];
                volatile int t2 = arr2[(j + i) % 256];
                a = (t1 * t2) + (a << 2);
                b = (t1 ^ t2) | (b >> 1);
                c = (t1 + t2) * (c % 17);
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 3: Function call in one branch */
            else if (branch == 3 && (j % 16 == 0)) {
                volatile int pid = getpid();
                arr1[j % 256] ^= pid;
                arr2[j % 256] += pid % 256;
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 4: More dependency chains */
            else if (branch == 4) {
                d = (a * b) + (c << (j % 3));
                e = (d ^ a) | (b >> 2);
                f = (e + d) * (f % 13);
                a = f ^ e ^ d;
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 5: Switch-like behavior with bit tests */
            else if (branch == 5) {
                for (int k = 0; k < 4; k++) {
                    if (j & (1 << k)) {
                        arr1[(j + k) % 256] += arr2[(j - k + 256) % 256];
                    } else {
                        arr2[(j + k) % 256] -= arr1[(j - k + 256) % 256];
                    }
                }
                __asm__ volatile ("" : : : "memory");
            }
            /* Branch 6: Mixed operations */
            else {
                volatile int x = arr1[j % 256];
                volatile int y = arr2[(j * 13) % 256];
                a = x * y + a;
                b = (x ^ y) * b;
                c = (x + y) ^ c;
                arr1[j % 256] = a + b + c;
                __asm__ volatile ("" : : : "memory");
            }
            
            /* Additional scheduling barrier */
            if (j % 8 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Store results back to arrays */
        arr1[i % 256] = (a + b + c + d + e + f) % 65536;
        arr2[i % 256] = (a ^ b ^ c ^ d ^ e ^ f) % 65536;
        
        /* Mode-dependent operation */
        if (mode_flag & 1) {
            volatile int clock_val = clock() % 1000;
            result += clock_val;
        }
    }
    
    return result;
}

/* Secondary function to create different scheduling contexts */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
alternate_schedule_pattern(volatile int *arr1, volatile int *arr2, 
                           volatile int iterations) {
    volatile int sum = 0;
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        /* Different pattern: nested loops with varying operations */
        for (int i = 0; i < 32; i++) {
            /* Create artificial dependencies */
            volatile int acc = arr1[i];
            for (int j = 0; j < 8; j++) {
                acc = (acc * 1103515245 + 12345) & 0x7fffffff;
                arr2[(i * 8 + j) % 256] ^= acc;
                __asm__ volatile ("" : : : "memory");
            }
            arr1[i] = acc;
            
            /* Conditional barrier */
            if (i % 4 == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        sum += arr1[iter % 32];
    }
    
    return sum;
}

int main(void) {
    /* Seed for deterministic behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int checksum = 0;
    
    /* Multiple calls to create different scheduling contexts */
    for (int round = 0; round < 8; round++) {
        volatile int outer_limit = (rand() % 5) + 10;  /* Volatile limit */
        volatile int mode = round % 4;
        
        /* Call the complex scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, 
                                                  outer_limit, mode);
        
        /* Alternate between different scheduling patterns */
        if (round % 2 == 0) {
            volatile int res2 = alternate_schedule_pattern(array1, array2, 
                                                           (rand() % 3) + 2);
            checksum ^= res2;
        }
        
        checksum ^= res1;
        
        /* Modify array contents between rounds */
        for (int i = 0; i < 64; i++) {
            array1[(round * 16 + i) % 256] ^= checksum;
            array2[(round * 16 + i) % 256] += round;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Final checksum computation to prevent dead code elimination */
    volatile int final_checksum = 0;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= array1[i];
        final_checksum ^= array2[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
