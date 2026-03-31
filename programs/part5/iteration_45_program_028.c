#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 10000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile char *carr, volatile short *sarr) {
    volatile int result = 0;
    volatile int outer_bound = ITERATIONS;
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force register pressure with many live variables */
        volatile int v1 = arr1[outer % ARRAY_SIZE];
        volatile int v2 = arr2[outer % ARRAY_SIZE];
        volatile int v3 = v1 + v2;          /* Candidate for remat: v1+v2 */
        volatile int v4 = v3 * 2;           /* IMMEDIATE CONSTANT: prime remat candidate */
        volatile int v5 = v4 & 0xFF;
        volatile int v6 = v5 | 0x80;
        
        /* Different width operations create partial register dependencies */
        volatile char c1 = carr[outer % ARRAY_SIZE];
        volatile short s1 = sarr[outer % ARRAY_SIZE];
        volatile int v7 = v6 + (int)c1;
        volatile int v8 = v7 - (int)s1;
        
        /* More variables to increase pressure */
        volatile int v9 = v8 * 3;           /* Another immediate constant */
        volatile int v10 = v9 >> 2;
        volatile int v11 = v10 ^ v3;
        volatile int v12 = v11 + 1;         /* +1 immediate */
        volatile int v13 = v12 * v4;
        volatile int v14 = v13 & 0xFFFF;
        volatile int v15 = v14 | 0x8000;
        
        /* Complex address computation with loop-invariant components */
        /* This creates REG references that might be rematerialized */
        volatile int idx = (outer * 7 + 3) % ARRAY_SIZE;  /* Complex but loop-invariant-ish */
        volatile int addr_calc = idx + 16;  /* Immediate constant 16 */
        
        /* Multiple basic blocks with conditional jumps */
        if (v15 & 1) {
            /* Use immediate constants in different basic block */
            v1 = v15 + 4;      /* +4 immediate */
            v2 = v1 * 5;       /* *5 immediate */
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
        } else {
            v1 = v15 - 4;      /* -4 immediate */
            v2 = v1 / 3;       /* /3 immediate */
            asm volatile("" : : : "memory");
        }
        
        /* Nested conditional for more complexity */
        volatile int temp = arr1[idx] + arr2[addr_calc % ARRAY_SIZE];
        if (temp > 1000) {
            v3 = v2 + 8;       /* +8 immediate */
            v4 = v3 * 9;       /* *9 immediate */
        } else if (temp < 500) {
            v3 = v2 - 8;       /* -8 immediate */
            v4 = v3 / 7;       /* /7 immediate */
        } else {
            v3 = v2 * 11;      /* *11 immediate */
            v4 = v3 & 0x7F;
        }
        
        /* More arithmetic with immediate constants */
        v5 = v4 + 12;
        v6 = v5 * 13;
        v7 = v6 & 0x3F;
        v8 = v7 | 0x40;
        v9 = v8 - 14;
        v10 = v9 ^ 0x55;
        
        /* Final computation mixing all variables */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 
                + v11 + v12 + v13 + v14 + v15;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Force spill/reload by using all variables again */
        if (outer % 7 == 0) {
            result -= v1 * 2 + v3 * 3 + v5 * 4 + v7 * 5 + v9 * 6 
                    + v11 * 7 + v13 * 8 + v15 * 9;
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate arrays with volatile to prevent optimization */
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile char *carr = (volatile char*)malloc(ARRAY_SIZE * sizeof(char));
    volatile short *sarr = (volatile short*)malloc(ARRAY_SIZE * sizeof(short));
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        carr[i] = rand() % 256;
        sarr[i] = rand() % 65536;
    }
    
    /* Memory barrier before computation */
    asm volatile("" : : : "memory");
    
    /* Call the high-pressure function */
    volatile int result = high_pressure_loop(arr1, arr2, carr, sarr);
    
    /* Cleanup */
    free((void*)arr1);
    free((void*)arr2);
    free((void*)carr);
    free((void*)sarr);
    
    return result;
}

int main(void) {
    volatile int final_result = setup_and_run();
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", final_result);
    
    /* Additional computation to keep compiler from optimizing everything away */
    volatile int verify = 0;
    for (int i = 0; i < 100; i++) {
        verify += final_result * i;
        asm volatile("" : : : "memory");
    }
    printf("Verification: %d\n", verify);
    
    return 0;
}
