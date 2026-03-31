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
    volatile int result = 0;
    volatile int i, j;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Outer loop with volatile bound to prevent hoisting */
    for (i = 0; i < bound; i++) {
        /* Create register pressure with many live variables */
        v1 = arr1[i] & 0xFF;          /* Candidate for remat: 0xFF immediate */
        v2 = arr2[i] | 0x55;          /* Candidate: 0x55 immediate */
        v3 = v1 + v2;
        v4 = v3 * 2;                  /* Candidate: multiplication by 2 */
        v5 = v4 - 1;                  /* Candidate: subtraction of 1 */
        
        /* More variables to increase pressure */
        v6 = arr3[i] + i;
        v7 = v6 << 3;                 /* Candidate: shift by 3 */
        v8 = v7 ^ 0xAAAAAAAA;         /* Candidate: large immediate */
        v9 = v8 & 0x55555555;
        v10 = v9 | v5;
        
        /* Conditional branch creating multiple basic blocks */
        if (v10 & 1) {
            /* Different computation path */
            v11 = v10 + 7;            /* Candidate: addition of 7 */
            v12 = v11 * 4;            /* Candidate: multiplication by 4 */
            v13 = v12 >> 1;
            v14 = v13 ^ v1;
            v15 = v14 + 0x1000;       /* Candidate: 0x1000 immediate */
        } else {
            /* Alternative path with different constants */
            v11 = v10 - 3;            /* Candidate: subtraction of 3 */
            v12 = v11 / 2;
            v13 = v12 & 0xFFFF;
            v14 = v13 | 0x8000;       /* Candidate: 0x8000 immediate */
            v15 = v14 ^ 0x7FFF;
        }
        
        /* More arithmetic with different data widths to create
           partial register dependencies */
        volatile char c1 = (v15 >> 8) & 0xFF;
        volatile short s1 = v15 & 0xFFFF;
        volatile long l1 = (long)v15 * (long)v10;
        
        v16 = c1 + s1;
        v17 = (int)l1 & 0xFFFFFF;
        v18 = v16 * v17;
        v19 = v18 + 0xDEADBEEF;       /* Candidate: large constant */
        v20 = v19 ^ v15;
        
        /* Use in address computation (base + index * scale) 
           Creates REG references in MEM addresses */
        int idx = v20 % SIZE;
        volatile int addr_calc = arr1[idx] + idx * 4;  /* Candidate: 4 immediate */
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Complex dependent chain */
        result ^= addr_calc + v20 + v15 + v10 + v5;
        
        /* Inner loop with more pressure */
        for (j = 0; j < 4; j++) {
            volatile int t1 = result + j;
            volatile int t2 = t1 * (j + 1);    /* Candidate: j+1 computation */
            volatile int t3 = t2 & ~(1 << j);  /* Candidate: bitmask computation */
            volatile int t4 = t3 | (1 << (j + 2));
            result = t4 ^ result;
        }
    }
    
    asm volatile("" : : : "memory");
    return result;
}

/* Another noinline function to create more context */
__attribute__((noinline, noipa))
static volatile int setup_and_run(void) {
    volatile int arr1[SIZE], arr2[SIZE], arr3[SIZE];
    volatile int i, seed = 42;
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = seed;
        arr2[i] = seed ^ 0xAAAAAAAA;
        arr3[i] = seed * 3 + 1;
    }
    
    /* Volatile bound prevents loop unrolling from eliminating pressure */
    volatile int bound = ITERS % SIZE;
    
    return high_pressure_loop(arr1, arr2, arr3, bound);
}

int main(void) {
    volatile int checksum = 0;
    
    /* Run multiple times to ensure ER triggers */
    for (int run = 0; run < 10; run++) {
        checksum ^= setup_and_run();
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
