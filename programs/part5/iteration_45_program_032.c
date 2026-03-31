#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    volatile int result = 0;
    volatile int i, j;
    
    /* Create many live variables with overlapping ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile char c1, c2, c3, c4, c5;
    volatile short s1, s2, s3, s4, s5;
    volatile long l1, l2, l3, l4, l5;
    
    /* Force memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Outer loop with volatile bound to prevent optimization */
    for (i = 0; i < bound; i++) {
        /* Initialize many variables with different types */
        v1 = arr1[i % ARRAY_SIZE];
        v2 = arr2[i % ARRAY_SIZE];
        v3 = arr3[i % ARRAY_SIZE];
        
        /* Create complex dependency chain */
        v4 = v1 + 1;          /* Immediate constant candidate for remat */
        v5 = v2 * 2;          /* Another immediate constant candidate */
        v6 = v3 & 0xFF;       /* Mask operation */
        v7 = v4 | v5;
        v8 = v6 ^ v7;
        
        /* More variables with partial register dependencies */
        c1 = (char)v8;
        s1 = (short)(v8 >> 8);
        l1 = (long)v8 * 256L;
        
        v9 = v4 + v5 + v6;
        v10 = v7 - v8 + v9;
        
        /* Conditional branch creating multiple basic blocks */
        if (v10 & 1) {
            v11 = v9 * 3;     /* Immediate constant */
            v12 = v10 / 2;    /* Another immediate */
            c2 = (char)v11;
            s2 = (short)v12;
            
            /* Nested conditional */
            if (v11 > v12) {
                v13 = v11 << 1;  /* Shift immediate */
                v14 = v12 >> 1;
                l2 = (long)v13 * (long)v14;
            } else {
                v13 = v12 << 2;
                v14 = v11 >> 2;
                l2 = (long)v13 / (long)v14;
            }
        } else {
            v11 = v9 + 4;     /* Immediate constant */
            v12 = v10 - 4;    /* Immediate constant */
            c2 = (char)(v11 & 0xF);
            s2 = (short)(v12 & 0xFFF);
            
            if (v11 < v12) {
                v13 = v11 * 5;  /* Immediate constant */
                v14 = v12 * 6;  /* Immediate constant */
                l2 = (long)v13 ^ (long)v14;
            } else {
                v13 = v11 / 7;
                v14 = v12 / 8;
                l2 = (long)v13 | (long)v14;
            }
        }
        
        /* More arithmetic with immediate constants */
        v15 = v13 + 9;
        v16 = v14 + 10;
        v17 = v15 * 11;
        v18 = v16 * 12;
        
        /* Mix different width operations */
        c3 = (char)v17;
        s3 = (short)v18;
        v19 = (int)c3 + (int)s3;
        
        /* Address computation with loop-invariant base (candidate for remat) */
        int idx = (i * 13) % ARRAY_SIZE;  /* Multiplication with immediate */
        v20 = arr1[idx] + arr2[idx] + arr3[idx];
        
        /* Final computation with all variables */
        l3 = (long)v19 * (long)v20;
        l4 = l2 + l3;
        l5 = l1 + l4;
        
        /* Memory barrier to prevent optimization across iterations */
        asm volatile("" : : : "memory");
        
        /* Accumulate result with volatile store */
        result += (int)l5;
        
        /* Create cross-iteration dependencies */
        arr1[(i + 1) % ARRAY_SIZE] = (v20 & 0x7F) + 1;
    }
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another noinline function to create more register pressure */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = x * 2;
    volatile int c = x & 0xFF;
    volatile int d = x | 0x55;
    volatile int e = x ^ 0xAA;
    volatile int f = a + b;
    volatile int g = c - d;
    volatile int h = e * f;
    volatile int i = g / 2;    /* Immediate constant */
    volatile int j = h % 3;    /* Immediate constant */
    volatile int k = i << 4;   /* Immediate constant */
    volatile int l = j >> 2;   /* Immediate constant */
    
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

int main(void) {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Create register pressure before main loop */
    volatile int preheat = 0;
    for (int i = 0; i < 1000; i++) {
        preheat += create_more_pressure(i);
    }
    
    /* Volatile bound to prevent loop unrolling */
    volatile int loop_bound = ITERATIONS;
    
    /* Main high-pressure computation */
    volatile int checksum = high_pressure_loop(array1, array2, array3, loop_bound);
    
    /* Use results to prevent dead code elimination */
    checksum += preheat;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
