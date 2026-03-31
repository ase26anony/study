#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 8; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; ++outer) {
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : : "memory");
        
        /* Declare many variables in nested scope to create register pressure */
        {
            volatile int v1 = arr1[outer] ^ 0x55;  /* Immediate constant candidate */
            volatile int v2 = arr2[outer] + 1;     /* Another immediate */
            volatile int v3 = arr3[outer] * 2;     /* Multiplication by constant */
            volatile int v4 = arr4[outer] & 0xFF;  /* Mask with constant */
            volatile int v5 = v1 | v2;
            volatile int v6 = v3 - v4;
            volatile int v7 = v5 ^ v6;
            
            /* More variables with different types to create partial reg dependencies */
            volatile char c1 = (v7 >> 0) & 0xFF;
            volatile short s1 = (v7 >> 8) & 0xFFFF;
            volatile long l1 = v7 * 3;  /* Another constant */
            
            /* Complex conditional creating multiple basic blocks */
            if (c1 & 0x01) {  /* Volatile-based condition */
                volatile int v8 = s1 + l1;
                volatile int v9 = v8 * 5;  /* Constant */
                volatile int v10 = v9 & 0xAAAAAAAA;
                volatile int v11 = v10 | 0x55555555;
                volatile int v12 = v11 - 7;  /* Constant */
                
                result += v12;
                
                /* Nested condition for more blocks */
                if (s1 > 100) {
                    volatile int v13 = v12 * 11;  /* Constant */
                    volatile int v14 = v13 / 3;   /* Constant */
                    volatile int v15 = v14 ^ 0x12345678;
                    
                    result ^= v15;
                } else {
                    volatile int v16 = v12 + 13;  /* Constant */
                    volatile int v17 = v16 * 17;  /* Constant */
                    
                    result |= v17;
                }
            } else {
                volatile int v18 = l1 * 19;  /* Constant */
                volatile int v19 = v18 + 23; /* Constant */
                volatile int v20 = v19 & 0xCCCCCCCC;
                
                result -= v20;
            }
            
            /* More computations in loop to extend live ranges */
            volatile int v21 = result + 29;  /* Constant */
            volatile int v22 = v21 * 31;     /* Constant */
            volatile int v23 = v22 | 0x88888888;
            
            /* Address computation with loop-invariant base - remat candidate */
            volatile int *ptr1 = arr1 + outer;  /* Base + offset */
            volatile int *ptr2 = arr2 + (outer * 2);  /* Base + (offset * constant) */
            
            /* Use the pointers to create more dependencies */
            volatile int v24 = *ptr1 + *ptr2;
            volatile int v25 = v24 * 37;  /* Constant */
            
            result = v23 ^ v25;
            
            /* Another memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Additional scope with more variables */
        {
            volatile int w1 = result + 41;  /* Constant */
            volatile int w2 = w1 - 43;      /* Constant */
            volatile int w3 = w2 * 47;      /* Constant */
            volatile int w4 = w3 & 0x0F0F0F0F;
            volatile int w5 = w4 | 0xF0F0F0F0;
            volatile int w6 = w5 ^ 0xAAAAAAAA;
            volatile int w7 = w6 + 53;      /* Constant */
            volatile int w8 = w7 * 59;      /* Constant */
            volatile int w9 = w8 - 61;      /* Constant */
            volatile int w10 = w9 & 0x33333333;
            
            result = w10;
        }
    }
    
    return result;
}

/* Another noinline function to create more call context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate arrays with volatile to force memory ops */
    volatile int *arr1 = (volatile int*)malloc(SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(SIZE * sizeof(int));
    volatile int *arr4 = (volatile int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
    }
    
    /* Run the high-pressure loop multiple times */
    volatile int final_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        final_result ^= high_pressure_loop(arr1, arr2, arr3, arr4);
        
        /* Modify arrays slightly each iteration to prevent optimization */
        arr1[iter % SIZE] ^= iter;
        arr2[iter % SIZE] += iter;
    }
    
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    free((void*)arr4);
    
    return final_result;
}

int main(void) {
    srand(42);  /* Deterministic seed */
    
    volatile int result = setup_and_run();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
