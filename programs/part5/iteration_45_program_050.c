#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Prevent interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int *arr4) {
    volatile int result = 0;
    volatile int outer_bound = 10; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Force memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Declare many variables in nested scope to create register pressure */
        {
            volatile int v1 = arr1[outer] + 1;  /* Immediate constant candidate */
            volatile int v2 = arr2[outer] * 2;  /* Another immediate constant */
            volatile int v3 = v1 & 0xFF;        /* Mask operation */
            volatile int v4 = v2 | 0x55;        /* OR with immediate */
            volatile int v5 = v3 - v4;
            
            /* Different width variables to create partial register dependencies */
            volatile char c1 = (v5 >> 8) & 0xFF;
            volatile short s1 = v5 & 0xFFFF;
            volatile long l1 = v5 * 3;          /* Multiplication with immediate */
            
            /* More variables with complex dependencies */
            volatile int v6 = l1 + s1;
            volatile int v7 = c1 * 4;           /* Another immediate */
            volatile int v8 = v6 ^ v7;
            volatile int v9 = v8 << 2;          /* Shift with immediate */
            volatile int v10 = v9 % 17;         /* Modulo with immediate prime */
            
            /* Address computation with loop-invariant base - remat candidate */
            volatile int *base_ptr = arr3;
            volatile int idx = outer * 2;       /* Multiplication candidate */
            volatile int v11 = base_ptr[idx] + 5; /* Base + offset + immediate */
            
            volatile int v12 = v10 + v11;
            volatile int v13 = v12 & ~0x3;      /* AND with immediate mask */
            volatile int v14 = v13 | 0x80000000;
            volatile int v15 = v14 ^ 0xAAAAAAAA;
            
            /* Conditional branch creating multiple basic blocks */
            if (v15 & 1) {
                /* Different computation path */
                v15 = v15 * 7 + 11;            /* More immediates */
                asm volatile("" : : : "memory"); /* Barrier in different block */
            } else {
                v15 = v15 / 3 - 9;             /* Division with immediate */
            }
            
            /* Nested conditional */
            volatile int temp = arr4[outer];
            for (volatile int inner = 0; inner < 5; inner++) {
                /* More register pressure in inner loop */
                volatile int w1 = temp + inner;
                volatile int w2 = w1 * (inner + 1); /* Varying immediate */
                volatile int w3 = w2 & (0xFF << inner);
                volatile int w4 = w3 | (1 << inner);
                
                /* Complex expression with many operands */
                result += w4 + (v15 % (inner + 2)) + (w1 ^ w3);
                
                /* Force spill/reload behavior */
                asm volatile("" : "+r" (result) : : "memory");
            }
            
            /* Use all variables to keep them live */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 
                    + v11 + v12 + v13 + v14 + v15;
        }
        
        /* Another memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more compilation context */
__attribute__((noinline, noipa))
static void initialize_arrays(volatile int *a, volatile int *b, 
                             volatile int *c, volatile int *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
        d[i] = rand() % 1000;
    }
}

int main(void) {
    /* Allocate arrays with volatile to force memory operations */
    volatile int *array1 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array2 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array3 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile int *array4 = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    initialize_arrays(array1, array2, array3, array4, ARRAY_SIZE);
    
    volatile int total = 0;
    
    /* Multiple calls to increase compilation complexity */
    for (int i = 0; i < ITERATIONS; i++) {
        total += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent complete optimization */
        if (i % 1000 == 0) {
            array1[i % ARRAY_SIZE] = rand() % 1000;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", total);
    
    free((void*)array1);
    free((void*)array2);
    free((void*)array3);
    free((void*)array4);
    
    return 0;
}
