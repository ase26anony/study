#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int sum = 0;
    volatile int i, j;
    
    /* Outer loop with volatile bound to prevent hoisting */
    for (i = 0; i < bound; i++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Load initial values - creates register pressure */
        v1 = arr1[i];      /* Candidate for remat: arr1[i] */
        v2 = arr2[i];
        v3 = v1 + 1;       /* Immediate constant +1 - remat candidate */
        v4 = v2 * 2;       /* Immediate constant *2 - remat candidate */
        
        /* Inner loop with complex dependency chain */
        for (j = 0; j < 8; j++) {
            /* Create many basic blocks with conditionals */
            if (v3 & 1) {
                v5 = v1 + v2;
                v6 = v5 * 3;       /* Another constant */
                v7 = v6 & 0xFF;    /* Mask operation */
            } else {
                v5 = v1 - v2;
                v6 = v5 / 2;       /* Division by constant 2 */
                v7 = v6 | 0x80;
            }
            
            /* More arithmetic with different widths to create partial regs */
            volatile char c1 = (v7 >> 8) & 0xFF;
            volatile short s1 = v7 * 5;
            volatile long l1 = v7 + 0x1000;  /* Large constant */
            
            /* Cross-type operations to inhibit optimization */
            v8 = v7 + c1;
            v9 = v8 * s1;
            v10 = v9 + (l1 & 0xFFFF);
            
            /* Conditional based on volatile to prevent dead code elimination */
            if (arr3[j] > 0) {
                v11 = v10 + arr1[j];
                v12 = v11 - 4;      /* Constant -4 */
                v13 = v12 * v3;
            } else {
                v11 = v10 - arr2[j];
                v12 = v11 + 8;      /* Constant +8 */
                v13 = v12 / v4;
            }
            
            /* More operations with immediate constants */
            v14 = v13 & 0x7F;       /* Mask with constant */
            v15 = v14 ^ 0x55;       /* XOR with constant */
            v16 = v15 << 3;         /* Shift by constant */
            v17 = v16 >> 1;
            v18 = v17 + 16;         /* Another constant */
            v19 = v18 * v5;
            v20 = v19 % 17;         /* Modulo with constant */
            
            /* Accumulate with dependency */
            sum += v20;
            
            /* Rotate values to extend live ranges */
            v1 = v20;
            v2 = v19;
            v3 = v18 + 1;           /* Recompute with constant */
            v4 = v17 * 2;           /* Recompute with constant */
        }
        
        /* Address computation with loop-invariant parts - remat candidate */
        volatile int idx = i * 4 + 1;  /* i*4 is loop invariant, +1 is constant */
        if (idx < SIZE) {
            sum += arr1[idx] + 1;      /* +1 is constant remat candidate */
        }
    }
    
    return sum;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    /* Large volatile arrays to force memory operations */
    static volatile int array1[SIZE];
    static volatile int array2[SIZE];
    static volatile int array3[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
    }
    
    /* Volatile loop bound to prevent constant propagation */
    volatile int outer_bound = 50;
    
    /* Call the high pressure function multiple times */
    volatile int total = 0;
    for (int k = 0; k < 10; k++) {
        total += high_pressure_loop(array1, array2, array3, outer_bound);
        /* Change bound slightly to prevent loop unrolling elimination */
        outer_bound = (outer_bound + 1) % 100 + 10;
    }
    
    return total;
}

int main(void) {
    srand(42);
    
    volatile int result = setup_and_run();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", (int)result);
    
    return 0;
}
