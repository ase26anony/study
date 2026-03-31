#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int result = 0;
    volatile int counter = 0;
    
    /* Outer loop with volatile limit to prevent compile-time optimization */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Memory barrier to split scheduling regions */
        __asm__ volatile ("" : : : "memory");
        
        /* Inner loop with complex dependency chains */
        for (int inner = 0; inner < 128; inner++) {
            /* Multiple basic blocks created by if-else chains */
            if (mode_flag & (1 << 0)) {
                /* Long dependency chain 1 */
                a = b * c + d;
                __asm__ volatile ("" : : : "memory");
                e = a ^ f;
                g = e >> (h & 7);
                arr1[inner] = g + arr2[inner];
            } else if (mode_flag & (1 << 1)) {
                /* Long dependency chain 2 */
                b = c - d * e;
                __asm__ volatile ("" : : : "memory");
                f = b & g;
                h = f | (i << 2);
                arr2[inner] = h ^ arr1[inner];
            } else if (mode_flag & (1 << 2)) {
                /* Independent memory operations */
                arr1[inner] = arr1[inner] * 3 + 7;
                arr2[inner] = arr2[inner] / 2 - 1;
                __asm__ volatile ("" : : : "memory");
            } else if (mode_flag & (1 << 3)) {
                /* Mixed operations with barrier */
                j = k * l - a;
                __asm__ volatile ("" : : : "memory");
                arr1[inner] = j + inner;
                arr2[inner] = arr1[inner] ^ arr2[inner];
            } else {
                /* Default path with function call */
                if ((inner & 15) == 0) {
                    /* Dummy function call to add complexity */
                    volatile int pid = getpid();
                    arr1[inner] = pid & 0xFF;
                }
                arr2[inner] = arr1[inner] + inner;
            }
            
            /* Switch-like behavior using bit checks */
            volatile int selector = inner & 7;
            if (selector == 0) {
                a = b + c;
                d = e * f;
            } else if (selector == 1) {
                g = h ^ i;
                j = k | l;
            } else if (selector == 2) {
                a = b - c;
                d = e & f;
            } else if (selector == 3) {
                g = h << 2;
                j = k >> 1;
            } else if (selector == 4) {
                /* Another memory barrier */
                __asm__ volatile ("" : : : "memory");
                a = arr1[inner];
                b = arr2[inner];
            } else if (selector == 5) {
                c = d * e + f;
                g = h ^ j;
            } else if (selector == 6) {
                /* Complex expression chain */
                result = a * b + c * d - e * f + g * h - i * j + k * l;
                arr1[inner] = result & 0xFF;
            } else {
                /* Final case with barrier */
                __asm__ volatile ("" : : : "memory");
                arr2[inner] = (arr1[inner] + arr2[inner]) & 0xFF;
            }
            
            /* Counter update with volatile to prevent optimization */
            counter++;
            if (counter > 1000) {
                counter = 0;
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Pseudo-random branch to vary control flow */
        if ((outer & 3) == 0) {
            mode_flag = (mode_flag + 1) & 15;
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    /* Compute final result to prevent elimination */
    volatile int sum = 0;
    for (int idx = 0; idx < 64; idx++) {
        sum += arr1[idx] + arr2[idx];
    }
    return sum;
}

/* Secondary function to increase scheduling complexity */
static volatile int __attribute__((noinline, optimize("O2", "no-tree-vectorize")))
schedule_helper(volatile int *buf1, volatile int *buf2, volatile int iter) {
    volatile int temp = 0;
    
    for (volatile int i = 0; i < iter; i++) {
        /* Multiple independent operations */
        int idx = i & 255;
        buf1[idx] = buf1[idx] * 1103515245 + 12345;
        buf2[idx] = buf2[idx] * 1664525 + 1013904223;
        
        /* Dependency chain */
        temp = buf1[idx] ^ buf2[idx];
        buf1[idx] = temp >> 16;
        buf2[idx] = temp & 0xFFFF;
        
        /* Barrier every 16 iterations */
        if ((i & 15) == 0) {
            __asm__ volatile ("" : : : "memory");
        }
    }
    
    return temp;
}

int main(void) {
    /* Seed for deterministic behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() & 0xFF;
        array2[i] = rand() & 0xFF;
    }
    
    volatile int checksum = 0;
    volatile int outer_loop_limit = 5;  /* Volatile to prevent constant folding */
    
    /* Main loop to create multiple scheduling contexts */
    for (int main_iter = 0; main_iter < outer_loop_limit; main_iter++) {
        volatile int mode = main_iter & 7;
        
        /* Call core scheduling function with varying parameters */
        volatile int result1 = complex_schedule_loop(array1, array2, 
                                                    (rand() % 3) + 2, mode);
        
        /* Call helper function to add more scheduling complexity */
        volatile int result2 = schedule_helper(array1, array2, 
                                              (rand() % 50) + 10);
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= result1 ^ result2;
        
        /* Modify mode flag for next iteration */
        mode = (mode * 13 + 7) & 15;
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Final computation to ensure all code executes */
    volatile int final_sum = 0;
    for (int i = 0; i < 256; i++) {
        final_sum += array1[i] + array2[i];
    }
    checksum ^= final_sum;
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
