#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, volatile int limit) {
    volatile int result = 0;
    
    /* Create many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile char c1, c2, c3, c4, c5;
    volatile short s1, s2, s3, s4, s5;
    volatile long l1, l2, l3, l4, l5;
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" : : : "memory");
    
    for (volatile int i = 0; i < limit; i++) {
        /* Complex address computations with loop-invariant components */
        volatile int idx1 = (i * 2) & (ARRAY_SIZE - 1);  /* Candidate for remat */
        volatile int idx2 = (i + 1) & (ARRAY_SIZE - 1);  /* Candidate for remat */
        volatile int idx3 = (i * 3) & (ARRAY_SIZE - 1);  /* Candidate for remat */
        
        /* Load values creating register pressure */
        v1 = arr1[idx1];
        v2 = arr2[idx2];
        v3 = arr1[idx3];
        v4 = arr2[idx1];
        
        /* Chain of dependent computations with immediate constants */
        v5 = v1 + 1;          /* +1 immediate - remat candidate */
        v6 = v2 * 2;          /* *2 immediate - remat candidate */
        v7 = v3 & 0xFF;       /* &0xFF immediate - remat candidate */
        v8 = v4 | 0x80;       /* |0x80 immediate - remat candidate */
        
        /* More computations creating overlapping live ranges */
        v9 = v5 + v6;
        v10 = v7 - v8;
        v11 = v9 * v10;
        v12 = v11 & 0xFFFF;
        v13 = v12 | 0x1000;
        v14 = v13 ^ 0x5555;   /* ^0x5555 immediate - remat candidate */
        
        /* Mixed-width operations to create partial register dependencies */
        c1 = (char)v14;
        s1 = (short)v13;
        l1 = (long)v12;
        
        v15 = c1 + s1;
        v16 = (int)l1 - v15;
        v17 = v16 * 3;        /* *3 immediate - remat candidate */
        
        /* Conditional branches creating multiple basic blocks */
        if (v17 & 1) {
            v18 = v17 + 5;    /* +5 immediate - remat candidate */
            v19 = v18 * 7;    /* *7 immediate - remat candidate */
            c2 = (char)v19;
            s2 = (short)(v19 >> 8);
            v20 = c2 * s2;
        } else {
            v18 = v17 - 3;    /* -3 immediate - remat candidate */
            v19 = v18 / 2;    /* /2 immediate - remat candidate */
            c3 = (char)v19;
            s3 = (short)(v19 >> 4);
            v20 = c3 + s3;
        }
        
        /* More computations with different data types */
        c4 = (char)v20;
        s4 = (short)(v20 >> 2);
        l2 = (long)c4 * (long)s4;
        
        v1 = (int)l2 + v1;    /* Reuse v1 to extend live range */
        v2 = v2 ^ v1;         /* Reuse v2 */
        
        /* Another conditional block */
        if (v20 > 1000) {
            v3 = v3 + 9;      /* +9 immediate - remat candidate */
            v4 = v4 * 4;      /* *4 immediate - remat candidate */
            c5 = (char)v3;
            s5 = (short)v4;
            l3 = (long)c5 * (long)s5;
        } else {
            v3 = v3 - 6;      /* -6 immediate - remat candidate */
            v4 = v4 / 3;      /* /3 immediate - remat candidate */
            l3 = (long)v3 + (long)v4;
        }
        
        /* Final computation chain */
        l4 = l2 + l3;
        l5 = l4 * 2;          /* *2 immediate - remat candidate */
        
        /* Memory barrier to prevent optimization across iterations */
        asm volatile ("" : : : "memory");
        
        result ^= (int)l5;
        
        /* Force spill/reload by using all variables again */
        v5 = v1 + v2;
        v6 = v3 + v4;
        v7 = v5 * v6;
        v8 = v7 & result;
        v9 = v8 | v20;
        v10 = v9 ^ v14;
        v11 = v10 + v15;
        v12 = v11 - v16;
        v13 = v12 * v17;
        v14 = v13 & v18;
        v15 = v14 | v19;
        
        /* Another conditional to split basic blocks */
        if ((i & 7) == 0) {
            v16 = v15 + 11;   /* +11 immediate - remat candidate */
            v17 = v16 * 13;   /* *13 immediate - remat candidate */
        } else if ((i & 7) == 1) {
            v16 = v15 - 17;   /* -17 immediate - remat candidate */
            v17 = v16 / 19;   /* /19 immediate - remat candidate */
        } else {
            v16 = v15 ^ 0xAA; /* ^0xAA immediate - remat candidate */
            v17 = v16 & 0x55; /* &0x55 immediate - remat candidate */
        }
        
        result += v17;
    }
    
    asm volatile ("" : : : "memory");
    return result;
}

int main() {
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
    }
    
    /* Create volatile loop bound to prevent optimization */
    volatile int loop_limit = ITERATIONS;
    
    /* Call the high-pressure function */
    volatile int checksum = high_pressure_loop(array1, array2, loop_limit);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
