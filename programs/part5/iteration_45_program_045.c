#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile uint64_t high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                          volatile char *carr, volatile short *sarr) {
    volatile uint64_t result = 0;
    volatile int outer_bound = 10; /* volatile to prevent loop unrolling */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        volatile int v11, v12, v13, v14, v15, v16, v17, v18;
        volatile char c1, c2, c3, c4;
        volatile short s1, s2, s3;
        volatile long l1, l2;
        
        /* Force register pressure with immediate constants */
        v1 = arr1[outer] + 1;          /* Candidate for remat: +1 */
        v2 = arr2[outer] * 2;          /* Candidate for remat: *2 */
        v3 = v1 & 0xFF;                /* Candidate for remat: &0xFF */
        v4 = v2 | 0x80;                /* Candidate for remat: |0x80 */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Complex interdependent calculations */
        for (volatile int i = 0; i < 100; i++) {
            /* Use different data types to create partial register dependencies */
            c1 = carr[i] ^ 0x55;       /* Immediate constant */
            s1 = sarr[i] + 256;        /* Immediate constant */
            
            /* Chain of dependent operations */
            v5 = v1 + v2;
            v6 = v3 * v4;
            v7 = v5 - v6;
            v8 = v7 & v1;
            v9 = v8 | v2;
            v10 = v9 ^ v3;
            
            /* More operations with immediate constants */
            v11 = v10 + 1;             /* Another +1 for remat */
            v12 = v11 * 2;             /* Another *2 for remat */
            v13 = v12 & 0x7F;          /* Another mask for remat */
            
            /* Conditional branch creating multiple basic blocks */
            if (v13 & 1) {             /* Volatile condition */
                v14 = v13 + arr1[i % ARRAY_SIZE];
                v15 = v14 * 3;         /* Immediate constant */
                v16 = v15 >> 1;        /* Immediate constant shift */
            } else {
                v14 = v13 - arr2[i % ARRAY_SIZE];
                v15 = v14 / 2;         /* Immediate constant */
                v16 = v15 << 1;        /* Immediate constant shift */
            }
            
            /* More operations with different widths */
            c2 = (char)v16;
            s2 = (short)(v16 + c2);
            l1 = (long)v16 * (long)s2;
            
            v17 = (int)l1 + v14;
            v18 = v17 * v15;
            
            /* Address computation with loop-invariant base + offset */
            /* This creates REG references that DF_REF_REAL_LOC can point to */
            int idx = (v18 & 0x3FF);   /* Mask to prevent overflow */
            c3 = carr[idx] + 1;        /* Immediate constant in address context */
            s3 = sarr[idx] * 2;        /* Immediate constant in address context */
            
            /* Final accumulation with volatile to prevent elimination */
            result += v18 + c3 + s3 + l1;
            
            /* Rotate values to extend live ranges */
            v1 = v18 & 0xFF;
            v2 = v17 | 0x80;
            v3 = v16 + 1;
            v4 = v15 * 2;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
        }
        
        /* Use all variables to keep them live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18;
        result += c1 + c2 + c3 + c4 + s1 + s2 + s3 + l1 + l2;
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile uint64_t intermediate_calc(volatile int *data, volatile int count) {
    volatile uint64_t sum = 0;
    volatile int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    for (volatile int i = 0; i < count; i++) {
        t1 = data[i] + 1;      /* Immediate constant */
        t2 = t1 * 2;           /* Immediate constant */
        t3 = data[i] & 0xFF;   /* Immediate constant */
        t4 = t2 | t3;
        t5 = t4 - 1;           /* Immediate constant */
        t6 = t5 * 3;           /* Immediate constant */
        t7 = t6 >> 2;          /* Immediate constant */
        t8 = t7 & 0x7F;        /* Immediate constant */
        t9 = t8 + i;
        t10 = t9 * 2;          /* Immediate constant */
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Force spill/reload behavior */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile char carray[ARRAY_SIZE];
    volatile short sarray[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        carray[i] = rand() % 256;
        sarray[i] = rand() % 1000;
    }
    
    /* Create register pressure before main calculation */
    volatile uint64_t prep = intermediate_calc(array1, 1000);
    
    /* Main high-pressure computation */
    volatile uint64_t result = high_pressure_loop(array1, array2, carray, sarray);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: %llu\n", (unsigned long long)(result + prep));
    
    return 0;
}
