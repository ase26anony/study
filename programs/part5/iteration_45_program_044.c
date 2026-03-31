#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    
    /* Create many live variables with overlapping ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int v21, v22, v23, v24, v25;
    
    /* Outer loop with volatile bound to prevent optimization */
    volatile int outer_bound = 100;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Initialize variables with different data types to create
           partial register dependencies */
        v1 = a[outer % ARRAY_SIZE] & 0xFF;          /* char width */
        v2 = b[outer % ARRAY_SIZE] & 0xFFFF;        /* short width */
        v3 = c[outer % ARRAY_SIZE];                 /* int width */
        v4 = d[outer % ARRAY_SIZE];                 /* int width */
        
        /* Create complex dependency chain with immediate constants
           that are candidates for rematerialization */
        v5 = v1 + 1;        /* Immediate constant +1 */
        v6 = v2 * 2;        /* Immediate constant *2 */
        v7 = v3 & 0x7F;     /* Immediate constant mask */
        v8 = v4 | 0x80;     /* Immediate constant mask */
        
        /* More variables to increase register pressure */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * 3;       /* Another immediate constant */
        v12 = v10 / 4;      /* Another immediate constant */
        
        /* Nested conditional blocks to create multiple basic blocks */
        if (v11 > 1000) {
            /* Branch 1 */
            v13 = v11 << 1;     /* Immediate shift */
            v14 = v12 >> 2;     /* Immediate shift */
            v15 = v13 + 5;      /* Immediate constant */
            v16 = v14 - 3;      /* Immediate constant */
            
            /* Address computation with loop-invariant base + offset
               This creates REG references for rematerialization */
            volatile int *ptr1 = a + (v15 & 0x3F);
            volatile int *ptr2 = b + (v16 & 0x3F);
            
            v17 = *ptr1 + 7;    /* Immediate constant */
            v18 = *ptr2 * 8;    /* Immediate constant */
            
            v19 = v17 & v18;
            v20 = v19 | 0xFF00;
        } else {
            /* Branch 2 */
            v13 = v11 >> 1;     /* Immediate shift */
            v14 = v12 << 2;     /* Immediate shift */
            v15 = v13 + 9;      /* Immediate constant */
            v16 = v14 - 7;      /* Immediate constant */
            
            /* Different address computation pattern */
            volatile int *ptr1 = c + (v15 & 0x7F);
            volatile int *ptr2 = d + (v16 & 0x7F);
            
            v17 = *ptr1 + 11;   /* Immediate constant */
            v18 = *ptr2 * 12;   /* Immediate constant */
            
            v19 = v17 ^ v18;
            v20 = v19 & 0x00FF;
        }
        
        /* Another conditional to create more basic blocks */
        if (v19 < v20) {
            v21 = v19 + v20;
            v22 = v21 * 13;     /* Immediate constant */
            v23 = v22 + 17;     /* Immediate constant */
            v24 = v23 - 19;     /* Immediate constant */
        } else {
            v21 = v20 - v19;
            v22 = v21 / 5;      /* Immediate constant */
            v23 = v22 + 23;     /* Immediate constant */
            v24 = v23 - 29;     /* Immediate constant */
        }
        
        /* Final computation mixing all variables */
        v25 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
              v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
              v20 + v21 + v22 + v23 + v24;
        
        /* Use result to prevent dead code elimination */
        result += v25;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array_a[ARRAY_SIZE];
    volatile int array_b[ARRAY_SIZE];
    volatile int array_c[ARRAY_SIZE];
    volatile int array_d[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = rand();
        array_b[i] = rand();
        array_c[i] = rand();
        array_d[i] = rand();
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int i = 0; i < 10; i++) {
        total += high_pressure_loop(array_a, array_b, array_c, array_d);
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
