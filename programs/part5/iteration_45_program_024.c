#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define INNER_ITERS 100
#define OUTER_ITERS 50

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = OUTER_ITERS;
    
    /* Create many live variables with overlapping ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force register pressure with many live variables */
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18;
        volatile char c1, c2, c3;
        volatile short s1, s2, s3;
        volatile long l1, l2;
        
        /* Initialize with array accesses - creates address computations */
        v1 = arr1[outer % SIZE];
        v2 = arr2[outer % SIZE];
        v3 = arr3[outer % SIZE];
        v4 = arr4[outer % SIZE];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Complex chain of dependent computations with immediate constants */
        /* These constants are candidates for rematerialization */
        v5 = v1 + 1;          /* Immediate +1 */
        v6 = v2 * 2;          /* Immediate *2 */
        v7 = v3 & 0xFF;       /* Immediate mask */
        v8 = v4 | 0x80;       /* Immediate OR */
        
        /* More computations creating register pressure */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * v10;
        
        /* Conditional branch creating multiple basic blocks */
        if (v11 & 1) {
            /* Branch 1: More computations with different widths */
            c1 = (char)v11;
            s1 = (short)(v11 >> 8);
            v12 = c1 * s1 + 3;  /* Another immediate */
            
            /* Nested conditional */
            if (v12 > 100) {
                v13 = v12 / 2;  /* Immediate division */
                v14 = v13 | 0x7F;
            } else {
                v13 = v12 * 3;  /* Immediate multiplication */
                v14 = v13 & 0x3F;
            }
            
            v15 = v14 + v11;
        } else {
            /* Branch 2: Alternative computation path */
            c2 = (char)(v11 >> 4);
            s2 = (short)(v11 >> 16);
            v12 = c2 - s2 - 1;  /* Immediate -1 */
            
            /* Another nested conditional */
            if (v12 < 0) {
                v13 = v12 + 5;  /* Immediate addition */
                v14 = v13 ^ 0xAA;
            } else {
                v13 = v12 - 5;  /* Immediate subtraction */
                v14 = v13 ^ 0x55;
            }
            
            v15 = v14 * v11;
        }
        
        /* More computations in all paths */
        l1 = (long)v15 * (long)v11;
        v16 = (int)(l1 >> 16);
        v17 = v16 + outer;      /* Loop invariant used */
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Second conditional with more computations */
        if (v17 % 4 == 0) {
            c3 = (char)v17;
            s3 = (short)(v17 >> 8);
            v18 = c3 | s3;
            result += v18 + 7;  /* Another immediate */
        } else {
            l2 = (long)v17 * 10L;  /* Immediate multiplication */
            v18 = (int)(l2 & 0xFFFFFFFF);
            result += v18 - 9;     /* Immediate subtraction */
        }
        
        /* Force spilling by using all variables again */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18;
        result += c1 + c2 + c3 + s1 + s2 + s3 + l1 + l2;
    }
    
    return result;
}

int main() {
    volatile int array1[SIZE], array2[SIZE], array3[SIZE], array4[SIZE];
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
        array4[i] = rand() % 256;
    }
    
    /* Call the high-pressure function */
    volatile int checksum = high_pressure_loop(array1, array2, array3, array4);
    
    printf("Checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (checksum == 0x1234) {
        printf("Impossible!\n");
    }
    
    return 0;
}
