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
    volatile int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    volatile long l1, l2, l3, l4, l5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    volatile int result = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop with volatile bound to prevent optimization */
    while (outer_counter < bound) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Initialize many variables with different data types */
        v1 = arr1[outer_counter % ARRAY_SIZE];
        v2 = arr2[outer_counter % ARRAY_SIZE];
        v3 = arr3[outer_counter % ARRAY_SIZE];
        
        /* Create complex dependency chain with immediate constants */
        /* These constants are candidates for rematerialization */
        v4 = v1 + 1;          /* Immediate constant +1 */
        v5 = v2 * 2;          /* Immediate constant *2 */
        v6 = v3 & 0xFF;       /* Immediate constant 0xFF */
        v7 = v4 | 0x80;       /* Immediate constant 0x80 */
        v8 = v5 - 3;          /* Immediate constant -3 */
        v9 = v6 ^ 0x55;       /* Immediate constant 0x55 */
        v10 = v7 << 1;        /* Immediate constant <<1 */
        
        /* More variables with different widths */
        l1 = (long)v1 * (long)v2;
        s1 = (short)(v3 & 0xFFFF);
        c1 = (char)(v4 & 0xFF);
        
        /* Nested conditional blocks create multiple basic blocks */
        if (v1 > 0) {
            v11 = v2 + v3;
            v12 = v11 * 4;    /* Immediate constant *4 */
            v13 = v12 >> 2;   /* Immediate constant >>2 */
            
            /* Address computation with loop-invariant base */
            /* This creates REG references for rematerialization */
            int idx = (v13 + outer_counter) % ARRAY_SIZE;
            v14 = arr1[idx] + arr2[idx];
            
            /* More operations with different data types */
            l2 = l1 + (long)v14;
            s2 = s1 + (short)v14;
            c2 = c1 ^ (char)v14;
        } else {
            v11 = v2 - v3;
            v12 = v11 / 2;    /* Immediate constant /2 */
            v13 = v12 % 10;   /* Immediate constant %10 */
            
            int idx = (v13 * outer_counter) % ARRAY_SIZE;
            v14 = arr1[idx] - arr2[idx];
            
            l2 = l1 - (long)v14;
            s2 = s1 - (short)v14;
            c2 = c1 | (char)v14;
        }
        
        /* Second conditional block */
        if (v2 < 0) {
            v15 = v3 + v4;
            v16 = v15 & 0x0F;  /* Immediate constant 0x0F */
            v17 = v16 | 0xF0;  /* Immediate constant 0xF0 */
            
            l3 = l2 * 3L;      /* Immediate constant 3L */
            s3 = s2 * 2;       /* Immediate constant *2 */
            c3 = c2 + 1;       /* Immediate constant +1 */
        } else {
            v15 = v3 - v4;
            v16 = v15 ^ 0xAA;  /* Immediate constant 0xAA */
            v17 = v16 << 2;    /* Immediate constant <<2 */
            
            l3 = l2 / 2L;      /* Immediate constant /2 */
            s3 = s2 / 2;       /* Immediate constant /2 */
            c3 = c2 - 1;       /* Immediate constant -1 */
        }
        
        /* Third level of nesting */
        volatile int temp = v5 + v6;
        for (int j = 0; j < 3; j++) {  /* Small inner loop */
            v18 = temp + j;
            v19 = v18 * (j + 1);  /* Varying immediate constant */
            v20 = v19 & (0xFF >> j);
            
            l4 = l3 + (long)v20;
            s4 = s3 + (short)v20;
            c4 = c3 ^ (char)v20;
            
            /* More arithmetic with immediate constants */
            v21 = v20 + 42;    /* Immediate constant 42 */
            v22 = v21 * 7;     /* Immediate constant 7 */
            v23 = v22 % 13;    /* Immediate constant 13 */
            
            /* Complex expression with many operands */
            v24 = (v17 + v18 + v19 + v20 + v21 + v22 + v23) / 8;
            
            /* Force register pressure with unused computations */
            v25 = v24 ^ 0xCC;
            v26 = v25 | 0x33;
            v27 = v26 & 0x99;
            v28 = v27 << 3;
            v29 = v28 >> 1;
            v30 = v29 + 999;   /* Large immediate constant */
            
            l5 = l4 * 5L + (long)v30;
            s5 = s4 * 3 + (short)v30;
            c5 = c4 - (char)v30;
        }
        
        /* Final accumulation with memory barrier */
        asm volatile("" : : : "memory");
        result += v14 + v17 + v24 + (int)l5 + (int)s5 + (int)c5;
        
        /* Volatile update prevents loop optimization */
        outer_counter++;
    }
    
    return result;
}

int main(void) {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Volatile bound to prevent optimization */
    volatile int iterations = ITERATIONS;
    
    /* Call the high-pressure function */
    volatile int checksum = high_pressure_loop(array1, array2, array3, iterations);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
