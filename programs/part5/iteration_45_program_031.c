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
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Declare many variables with different types to create partial reg dependencies */
        volatile char c1, c2, c3;
        volatile short s1, s2, s3;
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
        volatile long l1, l2, l3;
        
        /* Initialize with array accesses - creates address computations */
        v1 = arr1[outer % ARRAY_SIZE];
        v2 = arr2[outer % ARRAY_SIZE];
        v3 = arr3[outer % ARRAY_SIZE];
        v4 = arr4[outer % ARRAY_SIZE];
        
        /* Create complex dependency chain with immediate constants */
        /* These constants are rematerialization candidates */
        v5 = v1 + 1;          /* Candidate for remat: constant 1 */
        v6 = v2 * 2;          /* Candidate for remat: constant 2 */
        v7 = v3 & 0xFF;       /* Candidate for remat: constant 0xFF */
        v8 = v4 | 0x80;       /* Candidate for remat: constant 0x80 */
        
        /* More computations creating register pressure */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * v10;
        v12 = v11 / 3;        /* Candidate for remat: constant 3 */
        
        /* Mix different data widths */
        c1 = (char)v12;
        s1 = (short)(v12 >> 8);
        l1 = (long)v12 * 100L; /* Candidate for remat: constant 100 */
        
        /* Conditional branch creating multiple basic blocks */
        if (v12 & 1) {        /* Candidate for remat: constant 1 */
            v13 = v11 + 7;    /* Candidate for remat: constant 7 */
            v14 = v13 << 2;   /* Candidate for remat: constant 2 */
        } else {
            v13 = v11 - 5;    /* Candidate for remat: constant 5 */
            v14 = v13 >> 1;   /* Candidate for remat: constant 1 */
        }
        
        /* Nested loop with more register pressure */
        for (volatile int inner = 0; inner < 10; inner++) {
            asm volatile("" : : : "memory");
            
            /* More computations using all variables */
            v15 = v14 + inner;
            c2 = (char)(v15 & 0xF);   /* Candidate for remat: constant 0xF */
            s2 = (short)(v15 >> 4);
            
            /* Address computation with loop-invariant base */
            /* This creates remat candidates for address calculations */
            int idx = (v15 + outer) % ARRAY_SIZE;
            l2 = (long)arr1[idx] + (long)arr2[idx];
            
            /* More arithmetic with constants */
            c3 = c2 ^ 0x55;           /* Candidate for remat: constant 0x55 */
            s3 = s2 + 256;            /* Candidate for remat: constant 256 */
            l3 = l2 * 10L;            /* Candidate for remat: constant 10 */
            
            /* Use all variables to keep them live */
            result += c1 + c2 + c3 + s1 + s2 + s3 + 
                     v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                     v11 + v12 + v13 + v14 + v15 + (int)l1 + (int)l2 + (int)l3;
        }
        
        /* Another conditional block */
        if (result % 1000 == 0) {     /* Candidate for remat: constant 1000 */
            /* Force spill/reload behavior */
            asm volatile("" : : : "memory");
            v1 = v1 ^ result;
            v2 = v2 | result;
        }
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    volatile int array4[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
        array4[i] = rand();
    }
    
    /* Call the high-pressure function multiple times */
    volatile int total = 0;
    for (int i = 0; i < ITERATIONS / 1000; i++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Periodically scramble arrays to prevent pattern recognition */
        if (i % 100 == 0) {
            array1[i % ARRAY_SIZE] = rand();
            array2[i % ARRAY_SIZE] = rand();
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
