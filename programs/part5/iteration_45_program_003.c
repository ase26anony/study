#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define INNER_ITERS 100

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Force many pseudo-registers with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int v21, v22, v23, v24, v25;
    volatile long l1, l2, l3, l4, l5;
    volatile short s1, s2, s3, s4;
    volatile char c1, c2, c3, c4;
    
    volatile int result = 0;
    volatile int outer_counter = 0;
    
    /* Outer loop with volatile bound to prevent unrolling */
    while (outer_counter < bound) {
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Load initial values - creates many register references */
        v1 = arr1[outer_counter];
        v2 = arr2[outer_counter];
        v3 = arr3[outer_counter];
        
        /* Complex chain of dependent computations */
        /* Each step uses immediate constants (remat candidates) */
        v4 = v1 + 1;          /* Constant 1 - prime remat candidate */
        v5 = v2 * 2;          /* Constant 2 - prime remat candidate */
        v6 = v3 & 0xFF;       /* Constant 0xFF - remat candidate */
        v7 = v4 | 0x7F;       /* Constant 0x7F - remat candidate */
        
        /* More computations with different widths */
        l1 = (long)v5 * 256L;  /* Constant 256 - remat candidate */
        s1 = (short)(v6 + 128); /* Constant 128 - remat candidate */
        c1 = (char)(v7 ^ 0x55); /* Constant 0x55 - remat candidate */
        
        /* Conditional branch creating multiple basic blocks */
        if (v1 > 0) {
            v8 = v4 + v5;
            v9 = v6 * 3;       /* Constant 3 - remat candidate */
            v10 = v7 - 1;      /* Constant 1 - remat candidate */
            
            /* More width mixing */
            l2 = l1 >> 2;      /* Constant 2 - remat candidate */
            s2 = s1 << 1;      /* Constant 1 - remat candidate */
        } else {
            v8 = v5 - v4;
            v9 = v6 / 4;       /* Constant 4 - remat candidate */
            v10 = v7 + 2;      /* Constant 2 - remat candidate */
            
            l2 = l1 << 1;      /* Constant 1 - remat candidate */
            s2 = s1 >> 2;      /* Constant 2 - remat candidate */
        }
        
        /* Another conditional with more computations */
        v11 = (v8 > v9) ? v8 : v9;
        v12 = v10 ^ 0xAA;      /* Constant 0xAA - remat candidate */
        v13 = v11 & 0x0F;      /* Constant 0x0F - remat candidate */
        
        /* Nested conditional with address-like computation */
        if (v12 != 0) {
            /* Array indexing with loop-invariant-like computation */
            int idx = (v13 * 8) + 1;  /* Constants 8 and 1 - remat candidates */
            v14 = arr1[idx % SIZE];
            v15 = arr2[idx % SIZE];
            
            v16 = v14 + v15;
            v17 = v16 * 5;     /* Constant 5 - remat candidate */
        } else {
            v14 = v13 + 16;    /* Constant 16 - remat candidate */
            v15 = v14 - 8;     /* Constant 8 - remat candidate */
            v16 = v15 * 4;     /* Constant 4 - remat candidate */
            v17 = v16 / 2;     /* Constant 2 - remat candidate */
        }
        
        /* More computations to extend live ranges */
        v18 = v17 | 0xF0;      /* Constant 0xF0 - remat candidate */
        v19 = v18 & 0x3F;      /* Constant 0x3F - remat candidate */
        v20 = v19 ^ 0xCC;      /* Constant 0xCC - remat candidate */
        
        /* Use different data types to create partial reg dependencies */
        l3 = (long)v20 * 1000L;
        s3 = (short)(l3 & 0xFFFF);
        c2 = (char)(s3 ^ 0x33);
        
        /* Final computations with many live values */
        v21 = (int)l3 + v20;
        v22 = (int)s3 * v19;
        v23 = (int)c2 + v18;
        
        v24 = v21 + v22 + v23;
        v25 = v24 * 7;         /* Constant 7 - remat candidate */
        
        /* Accumulate result with memory barrier */
        asm volatile("" : : : "memory");
        result ^= v25;
        
        /* Inner loop to increase pressure */
        volatile int inner = 0;
        for (inner = 0; inner < INNER_ITERS; inner++) {
            /* Use all variables in overlapping ways */
            v1 = v1 + inner;
            v2 = v2 - inner;
            v3 = v3 ^ inner;
            v4 = v4 | inner;
            v5 = v5 & inner;
            
            /* More computations with immediate constants */
            v6 = v6 + 1;       /* Constant 1 - remat candidate */
            v7 = v7 * 2;       /* Constant 2 - remat candidate */
            v8 = v8 & 0x7F;    /* Constant 0x7F - remat candidate */
            v9 = v9 | 0x80;    /* Constant 0x80 - remat candidate */
            
            /* Conditional inside inner loop */
            if (inner & 1) {
                v10 = v10 + 3;  /* Constant 3 - remat candidate */
                v11 = v11 - 1;  /* Constant 1 - remat candidate */
            } else {
                v12 = v12 ^ 0xFF; /* Constant 0xFF - remat candidate */
                v13 = v13 & 0x0F; /* Constant 0x0F - remat candidate */
            }
            
            /* Force spill/reload behavior */
            asm volatile("" : : : "memory");
        }
        
        outer_counter++;
    }
    
    return result;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[SIZE];
    volatile int array2[SIZE];
    volatile int array3[SIZE];
    
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    /* Volatile bound to prevent optimization */
    volatile int iterations = 50;
    
    /* Call the high-pressure function */
    volatile int checksum = high_pressure_loop(array1, array2, array3, iterations);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
