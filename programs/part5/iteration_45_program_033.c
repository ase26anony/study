#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 10000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int v21, v22, v23, v24, v25;
    volatile long vl1, vl2, vl3;
    volatile short vs1, vs2, vs3;
    volatile char vc1, vc2, vc3;
    
    volatile int result = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop with volatile bound to prevent optimization */
    while (outer_counter < bound) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Inner loop creating extreme register pressure */
        for (volatile int i = 0; i < SIZE; i++) {
            /* Create complex dependency chain with many live variables */
            v1 = arr1[i];                     /* Candidate for remat - array access */
            v2 = v1 + 1;                      /* Immediate constant +1 - remat candidate */
            v3 = v2 * 2;                      /* Immediate constant *2 - remat candidate */
            v4 = v3 & 0xFF;                   /* Immediate constant &0xFF */
            v5 = v4 | 0x80;                   /* Immediate constant |0x80 */
            
            /* More computations creating overlapping live ranges */
            v6 = arr2[i];
            v7 = v6 - 1;                      /* Immediate constant -1 */
            v8 = v7 << 3;                     /* Immediate constant <<3 */
            v9 = v8 >> 1;                     /* Immediate constant >>1 */
            v10 = v9 ^ 0x55;                  /* Immediate constant ^0x55 */
            
            /* Mix different data types to create partial register dependencies */
            vs1 = (short)v5;
            vs2 = (short)v10;
            vs3 = vs1 + vs2;                  /* Requires promotion to int */
            
            vc1 = (char)v3;
            vc2 = (char)v8;
            vc3 = vc1 * vc2;                  /* Requires promotion to int */
            
            /* More variables to increase pressure */
            v11 = arr3[i];
            v12 = v11 + v1;                   /* Use earlier values */
            v13 = v12 * v2;
            v14 = v13 / (v3 + 1);             /* Immediate constant in divisor */
            v15 = v14 % (v4 + 2);             /* Another immediate constant */
            
            /* Long type operations */
            vl1 = (long)v5 * (long)v10;
            vl2 = vl1 + 0x1000L;              /* Large immediate constant */
            vl3 = vl2 - 0x800L;               /* Another large immediate */
            
            /* Conditional branches creating multiple basic blocks */
            if (v5 & 0x1) {                   /* Immediate constant &0x1 */
                v16 = v6 + v7;
                v17 = v16 * 3;                /* Immediate constant *3 */
                v18 = v17 & 0xAAAAAAAA;       /* Large immediate constant */
            } else {
                v16 = v6 - v7;
                v17 = v16 / 4;                /* Immediate constant /4 */
                v18 = v17 | 0x55555555;       /* Large immediate constant */
            }
            
            /* Another conditional with different constants */
            if (v10 > 100) {                  /* Immediate constant 100 */
                v19 = v8 + v9;
                v20 = v19 * 5;                /* Immediate constant *5 */
                v21 = v20 + 0x1234;           /* Large immediate constant */
            } else {
                v19 = v8 - v9;
                v20 = v19 / 6;                /* Immediate constant /6 */
                v21 = v20 - 0x5678;           /* Large immediate constant */
            }
            
            /* More arithmetic with constants */
            v22 = v12 + 7;                    /* Immediate constant +7 */
            v23 = v13 * 8;                    /* Immediate constant *8 */
            v24 = v14 & 9;                    /* Immediate constant &9 */
            v25 = v15 | 10;                   /* Immediate constant |10 */
            
            /* Complex expression with many operands */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                     v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                     v20 + v21 + v22 + v23 + v24 + v25 +
                     (int)vs3 + (int)vc3 + (int)vl1 + (int)vl2 + (int)vl3;
            
            /* Memory barrier to prevent optimization across iterations */
            asm volatile("" : : : "memory");
        }
        
        outer_counter++;
        
        /* Modify arrays slightly to prevent loop-invariant code motion */
        arr1[outer_counter % SIZE] = result & 0xFF;
        arr2[outer_counter % SIZE] = (result >> 8) & 0xFF;
        arr3[outer_counter % SIZE] = (result >> 16) & 0xFF;
    }
    
    return result;
}

/* Another noinline function to create more call context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(volatile int iter) {
    volatile int array1[SIZE];
    volatile int array2[SIZE];
    volatile int array3[SIZE];
    
    /* Initialize with pseudo-random data */
    for (volatile int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
    }
    
    /* Create volatile bound to prevent constant propagation */
    volatile int bound = iter % 10 + 5;
    
    return high_pressure_loop(array1, array2, array3, bound);
}

int main(void) {
    volatile int total = 0;
    
    /* Seed random number generator */
    srand(42);
    
    /* Multiple calls to increase overall register pressure */
    for (int run = 0; run < 5; run++) {
        volatile int iter_count = ITERS + (run * 100);
        volatile int result = setup_and_run(iter_count);
        total += result;
        
        /* Print progress to prevent dead code elimination */
        printf("Run %d: result = %d\n", run, result);
    }
    
    printf("Total checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
