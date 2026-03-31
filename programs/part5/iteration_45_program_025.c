#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = 100;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force register pressure with many live variables */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18;
        volatile char c1, c2, c3;
        volatile short s1, s2, s3;
        volatile long l1, l2, l3;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        for (volatile int i = 0; i < 100; i++) {
            /* Complex interdependent calculations with immediate constants */
            /* These constants are candidates for rematerialization */
            v1 = a[i] + 1;          /* Immediate constant +1 */
            v2 = b[i] * 2;          /* Immediate constant *2 */
            v3 = v1 & 0xFF;         /* Immediate constant mask */
            v4 = v2 | 0x80;         /* Immediate constant OR */
            v5 = v3 - 7;            /* Immediate constant -7 */
            v6 = v4 ^ 0x55;         /* Immediate constant XOR */
            v7 = v5 << 3;           /* Immediate constant shift */
            v8 = v6 >> 1;           /* Immediate constant shift */
            
            /* More calculations creating data dependencies */
            v9 = v7 + v8;
            v10 = v9 * v1;
            v11 = v10 & v2;
            v12 = v11 | v3;
            v13 = v12 ^ v4;
            v14 = v13 - v5;
            v15 = v14 + v6;
            v16 = v15 * v7;
            v17 = v16 & v8;
            v18 = v17 | v9;
            
            /* Mixed width operations to create partial register dependencies */
            c1 = (char)v10;
            s1 = (short)v11;
            l1 = (long)v12;
            c2 = (char)v13;
            s2 = (short)v14;
            l2 = (long)v15;
            c3 = (char)v16;
            s3 = (short)v17;
            l3 = (long)v18;
            
            /* Conditional branches creating multiple basic blocks */
            if (c1 & 0x1) {
                v1 = v1 + c2;       /* Use different width variables */
                v2 = v2 * s1;
            } else {
                v3 = v3 - c3;
                v4 = v4 / (s2 + 1); /* Avoid division by zero */
            }
            
            if (s1 > s2) {
                v5 = v5 | c1;
                v6 = v6 & s3;
            }
            
            /* Address computation with loop-invariant base - 
               prime candidate for rematerialization */
            int idx = (i + outer) % ARRAY_SIZE;
            volatile int *addr = &a[idx];  /* Base address computation */
            
            /* More calculations using the computed address */
            v7 = *addr + v1;
            v8 = b[idx] * v2;
            v9 = c[idx] & v3;
            v10 = d[idx] | v4;
            
            /* Accumulate result with volatile to prevent dead code elimination */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18;
            result += c1 + c2 + c3 + s1 + s2 + s3;
            result += (int)(l1 % 256) + (int)(l2 % 256) + (int)(l3 % 256);
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Nested scope to create more register pressure */
        {
            volatile int extra1 = outer * 3;
            volatile int extra2 = extra1 + 5;
            volatile int extra3 = extra2 * 2;
            volatile int extra4 = extra3 & 0x7F;
            volatile int extra5 = extra4 | 0x80;
            
            result += extra1 + extra2 + extra3 + extra4 + extra5;
            
            /* Complex conditional with immediate constants */
            if ((extra1 & 0x1) && (extra2 > 10) && (extra3 < 1000)) {
                result += 42;  /* Another immediate constant */
            }
        }
    }
    
    return result;
}

/* Another noinline function to create more compilation unit complexity */
__attribute__((noinline, noipa))
static void initialize_arrays(volatile int *a, volatile int *b, 
                             volatile int *c, volatile int *d) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
        c[i] = rand() % 256;
        d[i] = rand() % 256;
    }
}

int main() {
    srand(time(NULL));
    
    /* Allocate volatile arrays to force memory operations */
    volatile int *array_a = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array_b = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array_c = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array_d = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    initialize_arrays(array_a, array_b, array_c, array_d);
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < 10; iter++) {
        total += high_pressure_loop(array_a, array_b, array_c, array_d);
        
        /* Modify arrays slightly to prevent complete optimization */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array_a[i] = (array_a[i] + 1) % 256;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %d\n", total);
    
    free((void*)array_a);
    free((void*)array_b);
    free((void*)array_c);
    free((void*)array_d);
    
    return 0;
}
