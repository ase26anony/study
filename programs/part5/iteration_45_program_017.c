#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *a, volatile int *b, 
                                      volatile int *c, volatile int *d) {
    volatile int result = 0;
    
    /* Outer loop with volatile bound to prevent unrolling */
    volatile int outer_bound = ITERS;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Create many pseudo-registers with overlapping live ranges */
        volatile char v1, v2, v3, v4, v5;
        volatile short v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        volatile long v21, v22, v23, v24;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Load data with different widths to create partial reg dependencies */
        v1 = (char)a[outer % SIZE];
        v2 = (char)b[outer % SIZE];
        v6 = (short)c[outer % SIZE];
        v7 = (short)d[outer % SIZE];
        
        /* Chain of dependent computations using immediate constants */
        /* These constants are rematerialization candidates */
        v11 = (int)v1 + 1;          /* Candidate for remat: constant 1 */
        v12 = (int)v2 * 2;          /* Candidate for remat: constant 2 */
        v13 = v11 & 0xFF;           /* Candidate for remat: constant 0xFF */
        v14 = v12 | 0x80;           /* Candidate for remat: constant 0x80 */
        
        /* More computations creating register pressure */
        v15 = v13 + v14;
        v16 = v15 - 3;              /* Candidate for remat: constant 3 */
        v17 = v16 * 4;              /* Candidate for remat: constant 4 */
        
        /* Conditional branch creating multiple basic blocks */
        if (v17 & 1) {              /* Candidate for remat: constant 1 */
            v18 = v17 + 5;          /* Candidate for remat: constant 5 */
            v19 = v18 >> 1;         /* Candidate for remat: constant 1 */
            v20 = v19 * 6;          /* Candidate for remat: constant 6 */
            
            /* Nested conditional for more complexity */
            if (v20 < 1000) {       /* Candidate for remat: constant 1000 */
                v21 = (long)v20 + 7; /* Candidate for remat: constant 7 */
                v22 = v21 * 8;       /* Candidate for remat: constant 8 */
            } else {
                v21 = (long)v20 - 9; /* Candidate for remat: constant 9 */
                v22 = v21 / 10;      /* Candidate for remat: constant 10 */
            }
            
            v23 = v22 & 0xFFFF;     /* Candidate for remat: constant 0xFFFF */
            v24 = v23 | 0x8000;     /* Candidate for remat: constant 0x8000 */
        } else {
            v18 = v17 - 11;         /* Candidate for remat: constant 11 */
            v19 = v18 << 1;         /* Candidate for remat: constant 1 */
            v20 = v19 / 12;         /* Candidate for remat: constant 12 */
            
            /* Another nested conditional */
            if (v20 > 500) {        /* Candidate for remat: constant 500 */
                v21 = (long)v20 + 13; /* Candidate for remat: constant 13 */
                v22 = v21 * 14;       /* Candidate for remat: constant 14 */
            } else {
                v21 = (long)v20 - 15; /* Candidate for remat: constant 15 */
                v22 = v21 % 16;       /* Candidate for remat: constant 16 */
            }
            
            v23 = v22 ^ 0xFF00;     /* Candidate for remat: constant 0xFF00 */
            v24 = v23 | 0x4000;     /* Candidate for remat: constant 0x4000 */
        }
        
        /* More computations keeping variables live */
        v3 = (char)(v24 & 0xFF);
        v4 = v3 + 17;               /* Candidate for remat: constant 17 */
        v5 = v4 * 18;               /* Candidate for remat: constant 18 */
        
        v8 = (short)(v24 >> 8);
        v9 = v8 - 19;               /* Candidate for remat: constant 19 */
        v10 = v9 / 20;              /* Candidate for remat: constant 20 */
        
        /* Complex address computation with loop-invariant components */
        /* This creates REG rtx references for rematerialization */
        int idx1 = (outer * 31 + 21) % SIZE;  /* Constants 31, 21 */
        int idx2 = (outer * 37 + 29) % SIZE;  /* Constants 37, 29 */
        int idx3 = (outer * 41 + 33) % SIZE;  /* Constants 41, 33 */
        
        /* Use the computed indices */
        v11 += a[idx1] + 1;         /* Another constant 1 */
        v12 += b[idx2] * 2;         /* Another constant 2 */
        v13 += c[idx3] & 0xFF;      /* Another constant 0xFF */
        
        /* Final accumulation with memory barrier */
        asm volatile("" : : : "memory");
        result += v5 + v10 + v11 + v12 + v13 + (int)v24;
        
        /* Artificial dependency to prevent dead code elimination */
        a[outer % SIZE] = result & 1;
    }
    
    return result;
}

int main() {
    /* Initialize with pseudo-random data */
    volatile int array1[SIZE], array2[SIZE], array3[SIZE], array4[SIZE];
    
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
    
    /* Use results to prevent optimization */
    volatile int dummy = array1[0] + array2[0] + array3[0] + array4[0];
    return dummy & 0xFF;
}
