#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Outer loop with volatile bound to prevent optimization */
    for (volatile int outer = 0; outer < bound; outer++) {
        /* Inner loop creating extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            /* Load operations creating many live values */
            v1 = arr1[i];           /* Candidate for remat - base address + offset */
            v2 = arr2[i];           /* Another live value */
            v3 = arr3[i];           /* Third live value */
            
            /* Chain of dependent computations with immediate constants */
            /* These constants (1, 2, 3, 4) are prime rematerialization candidates */
            v4 = v1 + 1;            /* REG with constant 1 - likely remat candidate */
            v5 = v2 * 2;            /* REG with constant 2 */
            v6 = v3 & 3;            /* REG with constant 3 */
            v7 = v4 | 4;            /* REG with constant 4 */
            
            /* More computations creating overlapping live ranges */
            v8 = v5 + v6;
            v9 = v7 - v8;
            v10 = v9 * v4;
            v11 = v10 & v5;
            v12 = v11 | v6;
            v13 = v12 ^ v7;
            v14 = v13 + v8;
            v15 = v14 - v9;
            v16 = v15 * v10;
            v17 = v16 & v11;
            v18 = v17 | v12;
            v19 = v18 ^ v13;
            v20 = v19 + v14;
            
            /* Conditional branch creating multiple basic blocks */
            /* Volatile check prevents dead code elimination */
            if (v20 & 1) {
                /* Use different width operations for partial register dependencies */
                volatile char c1 = (char)v15;
                volatile short s1 = (short)v16;
                volatile long l1 = (long)v17;
                
                /* More computations in the taken branch */
                v1 = c1 + s1;
                v2 = l1 - v1;
                v3 = v2 * 2;        /* Another constant 2 - remat candidate */
                result += v3;
            } else {
                /* Alternative path with different computations */
                volatile char c2 = (char)v18;
                volatile short s2 = (short)v19;
                volatile long l2 = (long)v20;
                
                v4 = c2 - s2;
                v5 = l2 + v4;
                v6 = v5 / 2;        /* Constant 2 again */
                result -= v6;
            }
            
            /* Address computation with loop-invariant base + variable offset */
            /* This creates REG references that DF_REF_REAL_LOC will point to */
            int idx = (i + 1) % ARRAY_SIZE;
            v7 = arr1[idx] + arr2[idx];  /* Complex address computation */
            v8 = v7 * 3;                  /* Constant 3 - remat candidate */
            
            /* Another conditional to split live ranges */
            if (v8 > v20) {
                v9 = v8 + 1;              /* Constant 1 */
                v10 = v9 * 4;             /* Constant 4 */
                result ^= v10;
            }
            
            /* Memory barrier to prevent fusion of iterations */
            asm volatile("" : : : "memory");
        }
        
        /* Modify arrays slightly to prevent loop-invariant code motion */
        arr1[outer % ARRAY_SIZE] ^= 1;
        arr2[outer % ARRAY_SIZE] += 1;
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large volatile arrays to force memory operations */
    static volatile int array1[ARRAY_SIZE];
    static volatile int array2[ARRAY_SIZE];
    static volatile int array3[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Volatile loop bound to prevent constant propagation */
    volatile int iterations = ITERATIONS;
    
    /* Call the high-pressure function */
    volatile int result = high_pressure_loop(array1, array2, array3, iterations);
    
    return result;
}

int main(void) {
    /* Seed random number generator */
    srand(42);
    
    /* Run the computation */
    volatile int checksum = setup_and_run();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
