#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Core scheduling function with attributes to force RTL scheduling */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
complex_schedule_loop(volatile int *arr1, volatile int *arr2, 
                      volatile int outer_limit, volatile int mode_flag) {
    volatile int result = 0;
    volatile int i, j, k;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop with volatile limit to prevent optimization */
    for (i = 0; i < outer_limit; i++) {
        /* Create multiple basic blocks with if-else chains */
        if (mode_flag & 0x01) {
            /* Branch 1: Arithmetic operations with dependencies */
            temp1 = arr1[i] * 3 + arr2[i];
            temp2 = temp1 ^ (arr1[i] >> 2);
            temp3 = temp2 * 7 - arr2[i];
            
            /* Memory barrier to split scheduling regions */
            __asm__ volatile ("" : : : "memory");
            
            arr1[i] = temp3;
            result += temp3;
        } else if (mode_flag & 0x02) {
            /* Branch 2: Different arithmetic pattern */
            temp1 = arr1[i] + arr2[i] * 5;
            temp2 = temp1 & 0x7FFFFFFF;
            temp3 = temp2 | (arr1[i] << 3);
            
            /* Another memory barrier */
            __asm__ volatile ("" : : : "memory");
            
            arr2[i] = temp3;
            result ^= temp3;
        } else if (mode_flag & 0x04) {
            /* Branch 3: More complex operations */
            temp1 = (arr1[i] * arr2[i]) / 2;
            temp2 = temp1 + (arr1[i] % 17);
            temp3 = temp2 ^ temp1;
            
            /* Introduce a function call in one branch */
            if ((i & 0x3) == 0) {
                volatile int pid = getpid();
                temp3 ^= (pid & 0xFF);
            }
            
            arr1[i] = temp3;
            arr2[i] = temp3 * 2;
            result |= temp3;
        } else {
            /* Branch 4: Default operations */
            temp1 = arr1[i] - arr2[i];
            temp2 = temp1 * temp1;
            temp3 = temp2 % 1023;
            
            __asm__ volatile ("" : : : "memory");
            
            arr1[i] = temp3;
            arr2[i] = temp2;
            result -= temp3;
        }
        
        /* Inner loop with varying bounds */
        volatile int inner_limit = (i % 8) + 2;
        for (j = 0; j < inner_limit; j++) {
            /* Create long dependency chains */
            volatile int chain = result;
            for (k = 0; k < 4; k++) {
                chain = chain * 3 + j;
                chain = chain ^ (k * 7);
                chain = (chain << 1) | (chain >> 31);
            }
            
            /* Mix with array accesses */
            int idx = (i + j) & 0xFF;
            arr1[idx] = (arr1[idx] + chain) & 0xFFFF;
            arr2[idx] = (arr2[idx] ^ chain) & 0xFFFF;
            
            /* Strategic memory barriers */
            if ((j & 0x1) == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
        
        /* Switch-like behavior using bit tests */
        volatile int selector = arr1[i] & 0x7;
        if (selector & 0x1) {
            temp1 = arr1[i] * 11;
            __asm__ volatile ("" : : : "memory");
        }
        if (selector & 0x2) {
            temp2 = arr2[i] / 3;
            __asm__ volatile ("" : : : "memory");
        }
        if (selector & 0x4) {
            temp3 = temp1 + temp2;
            result += temp3;
        }
    }
    
    return result;
}

/* Secondary complex function to increase scheduling pressure */
static volatile int __attribute__((noinline, noipa, optimize("O3", "no-tree-vectorize")))
another_schedule_function(volatile int *arr, volatile int size, volatile int rounds) {
    volatile int acc = 0;
    volatile int i, r;
    
    for (r = 0; r < rounds; r++) {
        /* Pseudo-random walk through array */
        volatile int idx = r;
        for (i = 0; i < size; i++) {
            idx = (idx * 1103515245 + 12345) & (size - 1);
            
            /* Complex dependency web */
            volatile int a = arr[idx];
            volatile int b = arr[(idx + 1) & (size - 1)];
            volatile int c = arr[(idx + 2) & (size - 1)];
            
            volatile int x = a * b + c;
            volatile int y = (a ^ b) | c;
            volatile int z = (x << 3) ^ (y >> 2);
            
            __asm__ volatile ("" : : : "memory");
            
            arr[idx] = z;
            acc = (acc + z) & 0x7FFFFFFF;
            
            /* More barriers at specific points */
            if ((i & 0xF) == 0) {
                __asm__ volatile ("" : : : "memory");
            }
        }
    }
    
    return acc;
}

int main(void) {
    /* Seed for deterministic behavior */
    srand(42);
    
    /* Volatile arrays to prevent optimization */
    volatile int array1[256];
    volatile int array2[256];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        array1[i] = rand() & 0xFFFF;
        array2[i] = rand() & 0xFFFF;
    }
    
    volatile int checksum = 0;
    
    /* Multiple calls to create scheduling contexts */
    for (int iter = 0; iter < 8; iter++) {
        /* Volatile loop bounds to prevent compile-time simplification */
        volatile int outer_limit = (iter % 5) + 10;
        volatile int mode_flag = iter;
        
        /* Call the core scheduling function */
        volatile int res1 = complex_schedule_loop(array1, array2, outer_limit, mode_flag);
        checksum ^= res1;
        
        /* Call another scheduling-intensive function */
        volatile int size = 256;
        volatile int rounds = (iter % 3) + 2;
        volatile int res2 = another_schedule_function(array1, size, rounds);
        checksum += res2;
        
        /* Alternate between array1 and array2 */
        if (iter & 0x1) {
            volatile int res3 = complex_schedule_loop(array2, array1, outer_limit + 1, mode_flag ^ 0x55);
            checksum ^= res3;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int final_checksum = 0;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= array1[i];
        final_checksum += array2[i];
    }
    final_checksum ^= checksum;
    
    printf("Result: %d\n", final_checksum);
    
    return 0;
}
