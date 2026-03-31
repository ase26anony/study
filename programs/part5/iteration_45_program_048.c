#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    volatile int i, j;
    
    /* Create register pressure with mixed-width operations */
    volatile char c1, c2, c3;
    volatile short s1, s2, s3;
    volatile long l1, l2, l3;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Outer loop with volatile bound to prevent optimization */
    for (i = 0; i < bound; i++) {
        /* Inner loop creates complex def-use chains */
        for (j = 0; j < 8; j++) {
            /* Load operations creating register pressure */
            v1 = arr1[i * 8 + j];
            v2 = arr2[i * 8 + j];
            v3 = arr3[i * 8 + j];
            
            /* Chain of dependent operations with immediate constants */
            /* These constants are rematerialization candidates */
            v4 = v1 + 1;          /* Candidate for remat: constant 1 */
            v5 = v2 * 2;          /* Candidate for remat: constant 2 */
            v6 = v3 & 0xFF;       /* Candidate for remat: constant 0xFF */
            v7 = v4 | 0x80;       /* Candidate for remat: constant 0x80 */
            v8 = v5 - 1;          /* Candidate for remat: constant 1 */
            v9 = v6 ^ 0x55;       /* Candidate for remat: constant 0x55 */
            
            /* More operations with different constants */
            v10 = v7 << 3;        /* Candidate for remat: constant 3 */
            v11 = v8 >> 1;        /* Candidate for remat: constant 1 */
            v12 = v9 + 42;        /* Candidate for remat: constant 42 */
            v13 = v10 * 3;        /* Candidate for remat: constant 3 */
            v14 = v11 & 0x0F;     /* Candidate for remat: constant 0x0F */
            v15 = v12 | 0xC0;     /* Candidate for remat: constant 0xC0 */
            
            /* Mixed-width operations to create partial register dependencies */
            c1 = (char)v13;
            s1 = (short)v14;
            l1 = (long)v15;
            
            v16 = c1 + s1;
            v17 = l1 - v16;
            v18 = v17 * 7;        /* Candidate for remat: constant 7 */
            
            /* Conditional branches create multiple basic blocks */
            if (v18 & 1) {        /* Volatile check */
                v19 = v18 + 256;  /* Candidate for remat: constant 256 */
                c2 = (char)v19;
                result += c2;
            } else {
                v20 = v18 - 128;  /* Candidate for remat: constant 128 */
                s2 = (short)v20;
                result -= s2;
            }
            
            /* More operations to extend live ranges */
            v1 = v19 + v20;
            v2 = v1 * 5;          /* Candidate for remat: constant 5 */
            v3 = v2 / 4;          /* Candidate for remat: constant 4 */
            
            /* Another conditional block */
            if (v3 > 1000) {
                l2 = v3 * 11;     /* Candidate for remat: constant 11 */
                result ^= l2;
            }
            
            /* Address computation with loop-invariant components */
            /* This creates remat candidates for address calculations */
            int idx = i * 8 + j + 1;  /* Multiple constants in address */
            c3 = (char)arr1[idx];
            s3 = (short)arr2[idx];
            l3 = (long)arr3[idx];
            
            v4 = c3 + s3 + l3;
            v5 = v4 * 9;          /* Candidate for remat: constant 9 */
            
            /* Memory barrier to prevent fusion */
            asm volatile("" : : : "memory");
        }
        
        /* Additional computations to increase pressure */
        v6 = result * 13;         /* Candidate for remat: constant 13 */
        v7 = v6 & 0x7F;
        v8 = v7 | 0x40;
        v9 = v8 - 32;             /* Candidate for remat: constant 32 */
        
        /* Nested scope to create more pseudo-registers */
        {
            volatile int inner1 = v9 + 64;
            volatile int inner2 = inner1 * 17;
            volatile int inner3 = inner2 / 8;
            result = inner3;
        }
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 256;
        array2[i] = rand() % 256;
        array3[i] = rand() % 256;
    }
    
    /* Volatile bound to prevent loop unrolling */
    volatile int iterations = ITERATIONS;
    
    /* Call the high-pressure function */
    volatile int checksum = high_pressure_loop(array1, array2, array3, iterations);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
