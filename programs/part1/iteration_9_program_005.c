#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL-level scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    volatile int result = 0;
    
    /* Outer loop with volatile limit to prevent compile-time optimization */
    for (volatile int outer = 0; outer < outer_limit; outer++) {
        /* Create multiple basic blocks with complex control flow */
        if (mode & (1 << 0)) {
            /* Branch 1: Long dependency chain with arithmetic */
            a = b * c + d;
            __asm__ volatile ("" : : : "memory");  /* Scheduling barrier */
            e = a ^ f;
            g = e >> (h & 3);
            __asm__ volatile ("" : : : "memory");
            i = g * j - k;
            l = i | (m & 0xFF);
        } else if (mode & (1 << 1)) {
            /* Branch 2: Memory-intensive operations */
            arr1[(a + b) & 0xFF] = arr2[(c + d) & 0xFF] * 3;
            __asm__ volatile ("" : : : "memory");
            arr2[(e + f) & 0xFF] = arr1[(g + h) & 0xFF] ^ 0x5A5A;
            a = arr1[(i + j) & 0xFF] + arr2[(k + l) & 0xFF];
        } else if (mode & (1 << 2)) {
            /* Branch 3: Mixed operations with function call */
            if ((rand() % 100) < 10) {  /* Volatile condition */
                volatile int pid = getpid();  /* Function call in scheduling region */
                a = pid & 0xFF;
            }
            b = c * d - e;
            __asm__ volatile ("" : : : "memory");
            f = g ^ h;
            i = j << (k % 4);
        } else if (mode & (1 << 3)) {
            /* Branch 4: Complex arithmetic chain */
            a = ((b * c) >> 2) + ((d * e) >> 2);
            __asm__ volatile ("" : : : "memory");
            f = (g ^ h) | (i & j);
            k = (l + m) * (n - o);
            __asm__ volatile ("" : : : "memory");
            p = q / (r + 1);
        } else {
            /* Default branch: Simple operations but many of them */
            a = b + c; b = c + d; c = d + e; d = e + f;
            e = f + g; f = g + h; g = h + i; h = i + j;
            __asm__ volatile ("" : : : "memory");
            i = j + k; j = k + l; k = l + m; l = m + n;
        }
        
        /* Switch-like behavior using bit operations */
        volatile int selector = rand() % 32;
        for (int bit = 0; bit < 5; bit++) {
            if (selector & (1 << bit)) {
                /* Create independent instructions that can fill instruction queue */
                volatile int temp1 = arr1[(a + bit) & 0xFF];
                volatile int temp2 = arr2[(b + bit) & 0xFF];
                arr1[(c + bit) & 0xFF] = temp1 * temp2 + bit;
                __asm__ volatile ("" : : : "memory");
                arr2[(d + bit) & 0xFF] = (temp1 ^ temp2) >> (bit & 3);
            }
        }
        
        /* Another scheduling barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Cross-dependent operations to create complex data flow */
        volatile int x = arr1[outer & 0xFF];
        volatile int y = arr2[(outer + 1) & 0xFF];
        volatile int z = arr1[(outer + 2) & 0xFF];
        
        arr2[outer & 0xFF] = (x * y) + (z << 2);
        arr1[(outer + 1) & 0xFF] = (x ^ y) | (z & 0x7F);
        __asm__ volatile ("" : : : "memory");
        arr2[(outer + 2) & 0xFF] = (y - z) * (x + 1);
        
        /* Accumulate result to prevent dead code elimination */
        result ^= a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k ^ l;
        result ^= arr1[outer & 0xFF];
        result ^= arr2[outer & 0xFF];
    }
    
    return result;
}

/* Wrapper function to create additional scheduling context */
static volatile int __attribute__((noinline))
schedule_wrapper(volatile int *arr1, volatile int *arr2, volatile int iter) {
    volatile int total = 0;
    
    /* Vary loop bounds and modes across calls */
    volatile int outer_limits[] = {3, 5, 7, 9, 11};
    volatile int modes[] = {0x1, 0x3, 0x7, 0xF, 0x1F};
    
    for (int i = 0; i < 3; i++) {
        volatile int limit = outer_limits[(iter + i) % 5];
        volatile int mode = modes[(iter * 2 + i) % 5];
        
        total += complex_schedule_loop(arr1, arr2, limit, mode);
        
        /* Additional scheduling barrier between calls */
        __asm__ volatile ("" : : : "memory");
    }
    
    return total;
}

int main() {
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
    
    /* Multiple iterations to increase scheduling activity */
    for (volatile int iter = 0; iter < 8; iter++) {
        /* Vary parameters to create different scheduling contexts */
        volatile int param1 = (rand() % 10) + 1;
        volatile int param2 = (rand() % 16);
        
        /* Call scheduling function multiple times */
        volatile int result = complex_schedule_loop(array1, array2, param1, param2);
        checksum ^= result;
        
        /* Call wrapper function for additional complexity */
        volatile int wrapped = schedule_wrapper(array1, array2, iter);
        checksum ^= wrapped;
        
        /* Modify arrays between calls to create new data dependencies */
        for (int i = 0; i < 256; i += 8) {
            array1[i] = (array1[i] * 3 + iter) & 0xFFF;
            array2[i] = (array2[i] ^ 0xABCD) + iter;
        }
        
        /* Scheduling barrier between iterations */
        __asm__ volatile ("" : : : "memory");
    }
    
    /* Compute final checksum to prevent dead code elimination */
    volatile int final_checksum = 0;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= array1[i];
        final_checksum ^= array2[i];
    }
    final_checksum ^= checksum;
    
    printf("Result: %d\n", final_checksum);
    
    return 0;
}
