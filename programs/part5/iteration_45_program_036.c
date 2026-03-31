#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    volatile int outer_bound = ITERATIONS / 4;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force register pressure with many live variables */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        
        /* Initialize with array accesses - creates address computations */
        v1 = a[outer % ARRAY_SIZE];
        v2 = b[outer % ARRAY_SIZE];
        v3 = c[outer % ARRAY_SIZE];
        v4 = d[outer % ARRAY_SIZE];
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Complex interdependent calculations with immediate constants */
        /* These constants are rematerialization candidates */
        v5 = v1 + 1;          /* Candidate for remat: constant 1 */
        v6 = v2 * 2;          /* Candidate for remat: constant 2 */
        v7 = v3 & 0xFF;       /* Candidate for remat: constant 0xFF */
        v8 = v4 | 0x80;       /* Candidate for remat: constant 0x80 */
        v9 = v5 - 1;          /* Candidate: might reuse v5's computation */
        
        /* More calculations creating long live ranges */
        v10 = v6 / 2;         /* Inverse of multiplication by 2 */
        v11 = v7 ^ 0x55;      /* Another constant */
        v12 = v8 << 3;        /* Constant shift */
        v13 = v9 >> 1;        /* Another constant operation */
        
        /* Create conditional basic blocks to split live ranges */
        if (v1 & 1) {
            /* Use different width operations for partial registers */
            volatile char c1 = v5 & 0xFF;
            volatile short s1 = v6 & 0xFFFF;
            v14 = c1 + s1 + 256;  /* Constant 256 */
            
            /* Nested conditionals create more BBs */
            if (v2 > 1000) {
                v15 = v14 * 3;    /* Constant 3 */
                v16 = v15 | 0x7F; /* Constant 0x7F */
            } else {
                v15 = v14 / 3;    /* Same constant reused */
                v16 = v15 & 0x7E; /* Similar constant */
            }
            
            /* More arithmetic with constants */
            v17 = v16 + 42;       /* Constant 42 */
            v18 = v17 - 42;       /* Same constant */
        } else {
            /* Alternative path with different constants */
            volatile long l1 = v5;
            volatile int i1 = v6;
            v14 = (l1 + i1) * 4;  /* Constant 4 */
            
            if (v3 < 500) {
                v15 = v14 + 100;  /* Constant 100 */
                v16 = v15 - 99;   /* Constant 99 */
            } else {
                v15 = v14 - 100;  /* Same constant */
                v16 = v15 + 101;  /* Constant 101 */
            }
            
            v17 = v16 & 0x3F;     /* Constant 0x3F */
            v18 = v17 | 0xC0;     /* Constant 0xC0 */
        }
        
        /* Final computations using all variables */
        v19 = v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18;
        
        /* Address computation with loop-invariant parts - remat candidate */
        int idx = (outer * 7) % ARRAY_SIZE;  /* Constant 7 */
        v20 = a[idx] + b[idx] + c[idx] + d[idx];
        
        /* Use all variables to keep them live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Inner loop for additional pressure */
        for (volatile int inner = 0; inner < 8; inner++) {
            /* Use different subsets of variables */
            volatile int t1 = v1 + inner;
            volatile int t2 = v2 - inner;
            volatile int t3 = v3 * (inner + 1);  /* Constant expression */
            volatile int t4 = v4 & (inner | 0xF); /* Mixed constant */
            
            result += t1 + t2 + t3 + t4;
            
            /* Conditional with immediate constants */
            if (inner & 1) {
                result += 8;  /* Constant 8 */
            } else {
                result -= 4;  /* Constant 4 */
            }
        }
    }
    
    return result;
}

/* Another noinline function to create more compilation context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large arrays to create memory pressure */
    static volatile int array1[ARRAY_SIZE];
    static volatile int array2[ARRAY_SIZE];
    static volatile int array3[ARRAY_SIZE];
    static volatile int array4[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 10000;
        array2[i] = rand() % 10000;
        array3[i] = rand() % 10000;
        array4[i] = rand() % 10000;
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int run = 0; run < 3; run++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array1[i] += run;
            array2[i] -= run;
        }
    }
    
    return total;
}

int main(void) {
    srand(42);  /* Deterministic seed */
    
    volatile int checksum = setup_and_run();
    
    printf("Result checksum: %d\n", checksum);
    
    /* Use result to prevent dead code elimination */
    if (checksum > 0) {
        return 0;
    }
    return 1;
}
