#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Create many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    volatile int i, j;
    
    /* Outer loop with volatile bound to prevent optimization */
    for (i = 0; i < bound; i++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Load initial values - these create register pressure */
        v1 = arr1[i & (ARRAY_SIZE-1)];
        v2 = arr2[i & (ARRAY_SIZE-1)];
        v3 = arr3[i & (ARRAY_SIZE-1)];
        
        /* Complex chain of dependent computations with different data types */
        /* Using char/short/int to create partial register dependencies */
        char c1 = (char)(v1 & 0xFF);
        short s1 = (short)(v2 & 0xFFFF);
        
        /* First computation block - creates many live values */
        v4 = v1 + 1;          /* Immediate constant candidate for remat */
        v5 = v2 * 2;          /* Another immediate constant candidate */
        v6 = v3 & 0x7F;       /* Bitmask operation */
        v7 = v4 + v5;
        v8 = v6 | 0x80;       /* OR with immediate */
        
        /* Conditional branch creating basic blocks */
        if (v7 > 1000) {
            v9 = v7 - 500;
            v10 = v8 << 1;    /* Shift with immediate */
        } else {
            v9 = v7 + 500;
            v10 = v8 >> 1;    /* Shift with immediate */
        }
        
        /* Second computation block */
        v11 = v9 * 3;         /* Multiplication with immediate */
        v12 = v10 + 4;        /* Addition with immediate */
        v13 = v11 ^ 0x55;     /* XOR with immediate */
        
        /* Nested inner loop for additional pressure */
        for (j = 0; j < 5; j++) {
            /* More computations with immediate constants */
            v14 = v13 + j;
            v15 = v14 * 2;    /* Another multiplication candidate */
            v16 = v15 & 0x3F;
            
            /* Address computation with loop-invariant base */
            /* This is prime for rematerialization */
            int idx = (v16 + i) & (ARRAY_SIZE-1);
            v17 = arr1[idx] + 1;  /* Immediate constant in address context */
            v18 = arr2[idx] * 2;
            
            /* More conditional logic */
            if (v17 & 1) {
                v19 = v18 + v17;
                v20 = v19 | 0x01;
            } else {
                v19 = v18 - v17;
                v20 = v19 & 0xFE;
            }
            
            /* Accumulate result with dependency chain */
            result += v20;
            
            /* Rotate values to extend live ranges */
            v13 = v20;
        }
        
        /* Final computations with more immediates */
        v1 = v20 + 8;         /* Addition with immediate */
        v2 = v1 * 5;          /* Multiplication with immediate */
        v3 = v2 % 17;         /* Modulo with immediate */
        
        result += v3;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate and initialize arrays with volatile */
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        return -1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }
    
    /* Create volatile loop bound */
    volatile int bound = ITERATIONS;
    
    /* Call the high pressure function */
    volatile int result = high_pressure_loop(arr1, arr2, arr3, bound);
    
    /* Cleanup */
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    
    return result;
}

int main(void) {
    volatile int checksum = setup_and_run();
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
