#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    
    /* Create many live variables with overlapping ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Force register pressure with volatile bounds */
    volatile int outer_bound = bound;
    
    for (volatile int i = 0; i < outer_bound; i++) {
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Load initial values - creates REG references */
        v1 = arr1[i % ARRAY_SIZE];
        v2 = arr2[i % ARRAY_SIZE];
        v3 = arr3[i % ARRAY_SIZE];
        
        /* Chain of dependent computations with immediate constants */
        /* These constants are rematerialization candidates */
        v4 = v1 + 1;          /* Immediate constant +1 */
        v5 = v2 * 2;          /* Immediate constant *2 */
        v6 = v3 & 0xFF;       /* Immediate constant 0xFF */
        v7 = v4 | 0x80;       /* Immediate constant 0x80 */
        v8 = v5 - 42;         /* Immediate constant 42 */
        v9 = v6 ^ 0x55;       /* Immediate constant 0x55 */
        v10 = v7 << 3;        /* Immediate constant 3 */
        
        /* More computations with different data widths */
        volatile char c1 = (char)v8;
        volatile short s1 = (short)v9;
        volatile long l1 = (long)v10;
        
        /* Complex conditional creating multiple basic blocks */
        if (v4 > 100) {
            /* Branch 1: More computations */
            v11 = v1 + v2;
            v12 = v11 * v3;
            v13 = v12 / (v4 + 1);  /* Another +1 constant */
            v14 = v13 & 0x7F;       /* Immediate constant 0x7F */
            
            /* Address computation with loop-invariant base */
            /* This creates REG references for rematerialization */
            volatile int *ptr = arr1;
            v15 = ptr[(i + 1) % ARRAY_SIZE];
            v16 = v15 * 3;          /* Immediate constant 3 */
        } else {
            /* Branch 2: Different computations */
            v11 = v2 - v3;
            v12 = v11 & v1;
            v13 = v12 | 0x3F;       /* Immediate constant 0x3F */
            v14 = v13 << 2;         /* Immediate constant 2 */
            
            /* More address computations */
            volatile int *ptr = arr2;
            v15 = ptr[(i + 2) % ARRAY_SIZE];
            v16 = v15 / 4;          /* Immediate constant 4 */
        }
        
        /* Merge point - all variables live again */
        v17 = v14 + v16;
        v18 = v17 * c1;
        v19 = v18 | s1;
        v20 = v19 ^ l1;
        
        /* Another conditional */
        if (v20 & 1) {
            v1 = v20 + 5;           /* Immediate constant 5 */
            v2 = v1 * 6;            /* Immediate constant 6 */
        } else {
            v1 = v20 - 7;           /* Immediate constant 7 */
            v2 = v1 / 8;            /* Immediate constant 8 */
        }
        
        /* Final computation using many live variables */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Prevent loop unrolling from reducing pressure */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = a * 2;
    volatile int c = b - 3;
    volatile int d = c & 0xF;
    volatile int e = d | 0x10;
    volatile int f = e << 1;
    volatile int g = f / 2;
    volatile int h = g ^ 0xFF;
    volatile int i = h + 100;
    volatile int j = i * 2;
    
    return a + b + c + d + e + f + g + h + i + j;
}

int main() {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Create register pressure before main loop */
    volatile int pressure1 = create_more_pressure(array1[0]);
    volatile int pressure2 = create_more_pressure(array2[0]);
    volatile int pressure3 = create_more_pressure(array3[0]);
    
    /* Force many iterations with high register pressure */
    volatile int bound = ITERATIONS;
    volatile int result = high_pressure_loop(array1, array2, array3, bound);
    
    /* Use results to prevent dead code elimination */
    result += pressure1 + pressure2 + pressure3;
    
    printf("Result: %d\n", (int)result);
    
    return 0;
}
