#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    
    /* Create many live variables with overlapping ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile char c1, c2, c3, c4, c5;
    volatile short s1, s2, s3, s4, s5;
    volatile long l1, l2, l3, l4, l5;
    
    /* Outer loop with volatile bound to prevent optimization */
    volatile int outer_bound = ITERATIONS;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Initialize many variables with different types */
        v1 = arr1[outer % ARRAY_SIZE];
        v2 = arr2[outer % ARRAY_SIZE];
        v3 = arr3[outer % ARRAY_SIZE];
        v4 = arr4[outer % ARRAY_SIZE];
        
        /* Create complex dependency chain with immediate constants */
        /* These constants are prime candidates for rematerialization */
        v5 = v1 + 1;          /* Constant +1 - likely remat candidate */
        v6 = v2 * 2;          /* Constant *2 - likely remat candidate */
        v7 = v3 & 0xFF;       /* Constant mask */
        v8 = v4 | 0x80;       /* Constant OR */
        
        /* More operations creating register pressure */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * v10;
        v12 = v11 / 3;        /* Constant divisor */
        
        /* Mix different data widths to create partial register dependencies */
        c1 = (char)v12;
        s1 = (short)(v12 >> 8);
        l1 = (long)v12 * 1000L;
        
        /* Conditional branch creating multiple basic blocks */
        if (v12 & 1) {
            /* Branch 1: More computations */
            v13 = v12 + 42;   /* Another constant */
            v14 = v13 << 2;   /* Constant shift */
            v15 = v14 ^ 0x55; /* Constant XOR */
            
            /* Nested conditionals */
            if (v15 > 1000) {
                v16 = v15 - 999;
                v17 = v16 * 4;  /* Constant multiplier */
            } else {
                v16 = v15 + 999;
                v17 = v16 / 4;  /* Constant divisor */
            }
            
            v18 = v17 & 0x0F0F; /* Constant mask */
            result += v18;
        } else {
            /* Branch 2: Different computation path */
            v13 = v12 - 42;   /* Constant subtraction */
            v14 = v13 >> 1;   /* Constant shift */
            v15 = v14 | 0xAA; /* Constant OR */
            
            /* Another nested conditional */
            if (v15 < 500) {
                v16 = v15 * 3;  /* Constant multiplier */
                v17 = v16 + 256; /* Constant addition */
            } else {
                v16 = v15 / 3;  /* Constant divisor */
                v17 = v16 - 256; /* Constant subtraction */
            }
            
            v18 = v17 ^ 0xF0F0; /* Constant XOR */
            result -= v18;
        }
        
        /* More overlapping live ranges */
        v19 = v18 + c1 + s1;
        v20 = (int)l1 + v19;
        
        /* Address computation with loop-invariant base + offset */
        /* This creates REG references that might be rematerialized */
        int idx1 = (outer * 7) % ARRAY_SIZE;  /* Constant multiplier */
        int idx2 = (outer * 13) % ARRAY_SIZE; /* Constant multiplier */
        int idx3 = (outer * 17) % ARRAY_SIZE; /* Constant multiplier */
        
        /* Complex expression with immediate constants */
        v1 = arr1[idx1] + (idx2 * 4);  /* Constant 4 */
        v2 = arr2[idx2] - (idx3 / 2);  /* Constant 2 */
        v3 = arr3[idx3] & (idx1 | 1);  /* Constant 1 */
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Final computation mixing all variables */
        result ^= (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                  v9 + v10 + v11 + v12 + v13 + v14 + v15 + 
                  v16 + v17 + v18 + v19 + v20);
    }
    
    return result;
}

/* Another noinline function to create more register pressure */
__attribute__((noinline, noipa))
static volatile int create_more_pressure(volatile int x) {
    volatile int a = x + 1;
    volatile int b = x * 2;
    volatile int c = x & 0xFF;
    volatile int d = x | 0x80;
    volatile int e = a + b;
    volatile int f = c - d;
    volatile int g = e * f;
    volatile int h = g / 3;
    volatile int i = h << 2;
    volatile int j = i ^ 0x55;
    
    /* Many overlapping live ranges */
    return a + b + c + d + e + f + g + h + i + j;
}

int main() {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int arr1[ARRAY_SIZE];
    volatile int arr2[ARRAY_SIZE];
    volatile int arr3[ARRAY_SIZE];
    volatile int arr4[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
    }
    
    /* Create register pressure */
    volatile int total = 0;
    
    /* Call high pressure function multiple times */
    for (int i = 0; i < 10; i++) {
        total += high_pressure_loop(arr1, arr2, arr3, arr4);
        
        /* Additional pressure between calls */
        total ^= create_more_pressure(total);
    }
    
    printf("Result checksum: %d\n", total);
    
    return 0;
}
