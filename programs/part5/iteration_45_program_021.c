#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Declare many variables in nested scope to create register pressure */
        {
            volatile char v1 = arr1[outer] & 0xFF;
            volatile short v2 = arr2[outer] & 0xFFFF;
            volatile int v3 = arr3[outer];
            volatile long v4 = arr4[outer];
            
            /* Complex interdependent calculations with immediate constants */
            /* These constants are candidates for rematerialization */
            volatile int t1 = v1 + 1;          /* Constant +1 */
            volatile int t2 = v2 * 2;          /* Constant *2 */
            volatile int t3 = v3 & 0x7F;       /* Constant mask */
            volatile int t4 = v4 | 0x1000;     /* Constant OR */
            
            /* More variables with different types */
            volatile char c1 = t1 >> 1;
            volatile short s1 = t2 + 5;        /* Another constant */
            volatile int i1 = t3 - 3;          /* Constant -3 */
            volatile long l1 = t4 ^ 0xABCD;    /* Constant XOR */
            
            /* Address computations with loop-invariant base */
            /* These are prime rematerialization candidates */
            volatile intptr_t base1 = (intptr_t)&arr1[0];
            volatile intptr_t base2 = (intptr_t)&arr2[0];
            volatile intptr_t offset1 = outer * sizeof(int);
            volatile intptr_t offset2 = (outer + 1) * sizeof(int);
            
            /* More arithmetic with constants */
            volatile int a1 = c1 + s1;
            volatile int a2 = i1 * 4;          /* Constant *4 */
            volatile int a3 = (l1 & 0xFFFF) + 7; /* Constant +7 */
            volatile int a4 = a1 ^ a2;
            volatile int a5 = a3 | a4;
            volatile int a6 = a5 & 0xFF;
            volatile int a7 = a6 << 2;         /* Constant shift */
            volatile int a8 = a7 >> 1;         /* Constant shift */
            volatile int a9 = a8 + 100;        /* Constant +100 */
            volatile int a10 = a9 - 50;        /* Constant -50 */
            
            /* Conditional branches create multiple basic blocks */
            if (v1 & 1) {
                /* Different computation path */
                a10 = a10 * 3;                 /* Constant *3 */
                result += base1 + offset1;
            } else {
                a10 = a10 / 2;                 /* Constant /2 */
                result += base2 + offset2;
            }
            
            /* Nested conditional */
            if (v2 > 1000) {
                volatile int b1 = a10 + 255;   /* Constant +255 */
                volatile int b2 = b1 & 0x7F;
                volatile int b3 = b2 | 0x80;
                result += b3;
            } else if (v2 > 500) {
                volatile int b4 = a10 * 5;     /* Constant *5 */
                volatile int b5 = b4 % 17;     /* Constant modulus */
                result += b5;
            } else {
                volatile int b6 = a10 + 999;   /* Constant +999 */
                result += b6;
            }
            
            /* Use all variables to keep them live */
            result += v1 + v2 + v3 + v4;
            result += t1 + t2 + t3 + t4;
            result += c1 + s1 + i1 + l1;
            result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Additional scope with more variables */
        {
            volatile int x1 = arr1[outer + 1];
            volatile int x2 = arr2[outer + 2];
            volatile int x3 = x1 + x2;
            volatile int x4 = x3 * 11;         /* Constant *11 */
            volatile int x5 = x4 & 0x3FF;
            volatile int x6 = x5 | 0x400;
            volatile int x7 = x6 ^ 0x200;
            volatile int x8 = x7 << 3;         /* Constant shift */
            volatile int x9 = x8 >> 2;         /* Constant shift */
            volatile int x10 = x9 + 1234;      /* Constant +1234 */
            
            result += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
        }
    }
    
    return result;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
        array4[i] = rand();
    }
    
    volatile int total = 0;
    
    /* Call the high-pressure function multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        if (iter % 1000 == 0) {
            array1[iter % ARRAY_SIZE] = rand();
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
