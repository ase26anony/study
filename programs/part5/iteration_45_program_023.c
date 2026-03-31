#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 10; /* volatile to prevent constant propagation */
    
    /* Create many variables with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile char c1, c2, c3, c4, c5;
    volatile short s1, s2, s3, s4, s5;
    volatile long l1, l2, l3, l4, l5;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    for (volatile int outer = 0; outer < outer_bound; ++outer) {
        /* Initialize variables with different widths to create partial reg dependencies */
        c1 = (char)(outer & 0xFF);
        s1 = (short)(outer * 2);      /* Candidate for remat: multiplication by 2 */
        l1 = (long)(outer + 1);       /* Candidate for remat: addition of 1 */
        
        /* Complex address computation with loop-invariant base */
        volatile int *base_ptr = arr1 + (outer * 16);
        
        for (volatile int i = 0; i < ITERATIONS; ++i) {
            /* Create long dependency chain with many live variables */
            v1 = base_ptr[i & 0x3FF];          /* Address computation candidate */
            v2 = v1 + arr2[i & 0x3FF];         /* Use immediate constant 1 implicitly */
            v3 = v2 * 2;                       /* Explicit constant - remat candidate */
            v4 = v3 | 0x1F;                    /* Bitwise OR with constant */
            v5 = v4 & 0xFF00;                  /* Mask operation */
            v6 = v5 - s1;                      /* Mix widths */
            v7 = v6 + (int)c1;                 /* More width mixing */
            v8 = v7 * 3;                       /* Another constant multiplication */
            v9 = v8 >> 4;                      /* Shift operation */
            v10 = v9 ^ 0x55;                   /* XOR with constant */
            
            /* Second dependency chain */
            v11 = arr3[i & 0x3FF];
            v12 = v11 + 1;                     /* Constant addition candidate */
            v13 = v12 * v2;
            v14 = v13 & 0x0F0F;
            v15 = v14 | v4;
            v16 = v15 - 7;                     /* Constant subtraction */
            v17 = v16 * 2;                     /* Another constant multiplication */
            v18 = v17 + (int)s1;
            v19 = v18 ^ v10;
            v20 = v19 & 0x7F;
            
            /* Third chain with different data types */
            c2 = (char)(v20 & 0xFF);
            s2 = (short)(v19 + v18);
            l2 = (long)v17 * 5L;               /* Long constant multiplication */
            c3 = c2 + 1;                       /* char + constant */
            s3 = s2 - 2;                       /* short - constant */
            l3 = l2 + 100L;                    /* long + constant */
            
            /* Conditional branches to create multiple basic blocks */
            if (v10 & 0x01) {                  /* Volatile check */
                v5 = v5 * 2;                   /* Modify variable in one path */
                c4 = (char)(v5 & 0xFF);
                s4 = (short)(v6 + v7);
                l4 = l3 - 50L;
            } else {
                v5 = v5 / 2;                   /* Different modification */
                c4 = (char)(v8 & 0xFF);
                s4 = (short)(v9 - v10);
                l4 = l3 + 50L;
            }
            
            /* Another conditional */
            if (v15 > 1000) {
                v12 = v12 + arr4[i & 0x3FF];
                c5 = (char)(v12 & 0xFF);
                s5 = (short)(v13 + v14);
                l5 = l4 * 2L;
            } else {
                v12 = v12 - arr4[i & 0x3FF];
                c5 = (char)(v16 & 0xFF);
                s5 = (short)(v17 - v18);
                l5 = l4 / 2L;
            }
            
            /* Final computation mixing all variables */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                     v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                     (int)c1 + (int)c2 + (int)c3 + (int)c4 + (int)c5 +
                     (int)s1 + (int)s2 + (int)s3 + (int)s4 + (int)s5 +
                     (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
            
            /* Memory barrier to prevent optimization across iterations */
            asm volatile("" : : : "memory");
        }
        
        /* Modify base pointer with constant offset - remat candidate */
        base_ptr = base_ptr + 8;               /* Constant addition to pointer */
    }
    
    return result;
}

int main(void) {
    /* Initialize with pseudo-random data */
    srand(42);
    
    volatile int *arr1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *arr4 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    volatile int result = high_pressure_loop(arr1, arr2, arr3, arr4);
    
    printf("Result: %d\n", result);
    
    free((void*)arr1);
    free((void*)arr2);
    free((void*)arr3);
    free((void*)arr4);
    
    return 0;
}
