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
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; ++outer) {
        /* Inner loop with extreme register pressure */
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            /* Declare many variables in nested scope to force pseudo-registers */
            {
                /* Immediate constants - candidates for rematerialization */
                const int c1 = 1;    /* Will likely become REG with constant value */
                const int c2 = 2;
                const int c3 = 4;
                const int c4 = 8;
                const int c5 = 16;
                
                /* Variables with different widths to create partial register dependencies */
                char v1_char;
                short v2_short;
                int v3_int;
                long v4_long;
                
                /* Many integer variables with interdependent calculations */
                int v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
                
                /* Address computation with loop-invariant base - remat candidate */
                int *base_ptr = (int*)arr1;
                volatile int *volatile_base = arr2; /* Volatile pointer */
                
                /* Memory barrier to prevent reordering */
                asm volatile ("" : : : "memory");
                
                /* Complex interdependent calculations creating long live ranges */
                v1_char = (char)(arr1[i] & 0xFF);
                v2_short = (short)(arr2[i] + c1);  /* Uses immediate constant */
                v3_int = arr3[i] * c2;             /* Another immediate constant */
                v4_long = (long)arr4[i] << c3;     /* Shift with immediate */
                
                /* Chain of dependent operations */
                v5 = v1_char + v2_short;
                v6 = v3_int - v4_long;
                v7 = v5 * v6;
                v8 = v7 & c4;                      /* Bitwise with immediate */
                v9 = v8 | c5;                      /* Another immediate */
                
                /* More computations with address arithmetic */
                v10 = *(base_ptr + i);             /* Base pointer + offset */
                v11 = volatile_base[i] + v10;
                v12 = v11 * (i + c1);              /* i + immediate */
                v13 = v12 / (v9 + c1);
                v14 = v13 ^ v8;
                v15 = v14 << (c2 + c1);            /* Immediate expression */
                
                /* Conditional branches creating multiple basic blocks */
                if (v15 & 0x1) {
                    /* Use more immediates in different basic block */
                    v15 += c1;
                    v14 -= c2;
                    asm volatile ("" : : : "memory"); /* Barrier in branch */
                } else {
                    v15 *= c3;
                    v14 /= c4;
                }
                
                /* Nested conditional */
                if (v14 > 1000) {
                    v13 = v15 + c5;
                    /* More address computation */
                    int offset = i * c2 + c1;      /* Immediate arithmetic */
                    v12 = arr1[offset % ARRAY_SIZE];
                }
                
                /* Final computation mixing all variables */
                result += v15 + v14 + v13 + v12 + v11 + v10 + v9 + v8 + v7 + v6 + v5;
                
                /* Another memory barrier */
                asm volatile ("" : : : "memory");
                
                /* Use variables again to extend live ranges */
                if (i % 128 == 0) {
                    volatile int temp = v1_char + v2_short + v3_int;
                    result ^= temp;
                }
            }
        }
        
        /* Modify volatile bound to prevent loop unrolling */
        if (outer % 7 == 0) {
            asm volatile ("" : "+r" (outer_bound) : : "memory");
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Allocate volatile arrays to prevent optimization */
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr4 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        return -1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        total += high_pressure_loop(arr1, arr2, arr3, arr4);
        
        /* Modify arrays slightly each iteration */
        if (iter % 100 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i += 97) {
                arr1[i] ^= iter;
                arr2[i] += iter % 31;
            }
            asm volatile ("" : : : "memory");
        }
    }
    
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    free((void*)arr4);
    
    return total;
}

int main(void) {
    volatile int checksum = setup_and_run();
    
    /* Use checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    /* Additional volatile operation */
    asm volatile ("" : : "r" (checksum) : "memory");
    
    return 0;
}
