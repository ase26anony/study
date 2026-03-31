#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 10; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; ++outer) {
        /* Inner loop with extreme register pressure */
        for (volatile int i = 0; i < ARRAY_SIZE; ++i) {
            /* Force many live variables with complex dependencies */
            volatile char v1 = arr1[i] & 0xFF;          /* Different widths */
            volatile short v2 = arr2[i] & 0xFFFF;
            volatile int v3 = arr1[i] + 1;              /* Immediate constant candidate */
            volatile long v4 = arr2[i] * 2;             /* Another immediate constant */
            volatile int v5 = v1 + v2;
            volatile int v6 = v3 - v4;
            volatile int v7 = v5 * v6;
            volatile int v8 = arr3[i] | 0x55;           /* Immediate constant */
            volatile int v9 = arr4[i] & 0xAA;           /* Immediate constant */
            volatile int v10 = v7 + v8;
            volatile int v11 = v9 - v10;
            volatile int v12 = v11 * 3;                 /* Rematerialization candidate */
            volatile int v13 = v12 / 2;
            volatile int v14 = v13 | v11;
            volatile int v15 = v14 & v10;
            
            /* Address computation with loop-invariant components */
            volatile int idx = (i + outer) % ARRAY_SIZE;
            volatile int addr_calc = (arr1[idx] + 4) * 2; /* Rematerialization candidate */
            
            /* Create multiple basic blocks with conditional branches */
            if (v15 & 1) {
                /* Branch 1: More computations */
                volatile int t1 = v15 + addr_calc;
                volatile int t2 = t1 * 5;               /* Immediate constant */
                volatile int t3 = t2 | 0xF0;
                result += t3;
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
            } else {
                /* Branch 2: Different computation chain */
                volatile int t4 = v15 - addr_calc;
                volatile int t5 = t4 / 4;               /* Immediate constant */
                volatile int t6 = t5 & 0x0F;
                result -= t6;
                
                /* Another memory barrier */
                asm volatile("" : : : "memory");
            }
            
            /* Nested conditional to create more complex CFG */
            if (v10 > v11) {
                volatile int t7 = v10 * 7;              /* Immediate constant */
                volatile int t8 = t7 + 8;               /* Immediate constant */
                result ^= t8;
            }
            
            /* Force spill/reload behavior with volatile operations */
            volatile int spill_test = result;
            spill_test = spill_test + v1 - v2 + v3 - v4;
            result = spill_test;
            
            /* More immediate constants in address computations */
            volatile int offset = i * sizeof(int);      /* Could be rematerialized */
            volatile int *ptr = (volatile int*)((char*)arr1 + offset);
            volatile int indirect = *ptr + 16;          /* Immediate constant */
            
            result = result + indirect - (i & 0x3F);
        }
        
        /* Modify arrays to prevent loop-invariant code motion */
        arr1[outer % ARRAY_SIZE] = result & 0xFF;
        arr2[outer % ARRAY_SIZE] = (result >> 8) & 0xFF;
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate arrays with volatile to force real memory ops */
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr4 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 256;
        arr2[i] = rand() % 256;
        arr3[i] = rand() % 256;
        arr4[i] = rand() % 256;
    }
    
    /* Run the high-pressure computation */
    volatile int checksum = high_pressure_loop(arr1, arr2, arr3, arr4);
    
    /* Cleanup */
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    free((void*)arr4);
    
    return checksum;
}

int main(void) {
    /* Run multiple times to ensure JIT-like behavior doesn't skip ER */
    volatile int final_result = 0;
    for (int run = 0; run < 3; run++) {
        volatile int run_result = setup_and_run();
        final_result += run_result;
        
        /* Memory barrier between runs */
        asm volatile("" : : : "memory");
    }
    
    printf("Checksum: %d\n", final_result);
    return 0;
}
