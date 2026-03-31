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
    volatile int outer_bound = 50; /* Volatile to prevent constant propagation */
    
    /* Create many pseudo-registers with overlapping live ranges */
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Declare many variables in nested scope to force pseudo-registers */
        {
            volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
            volatile int v11, v12, v13, v14, v15, v16, v17, v18;
            volatile short s1, s2, s3, s4;
            volatile char c1, c2, c3, c4;
            volatile long l1, l2;
            
            /* Initialize with array accesses - creates address computations */
            v1 = arr1[outer % ARRAY_SIZE];
            v2 = arr2[outer % ARRAY_SIZE];
            v3 = arr3[outer % ARRAY_SIZE];
            v4 = arr4[outer % ARRAY_SIZE];
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
            
            /* Chain of dependent computations with immediate constants */
            /* These constants are rematerialization candidates */
            v5 = v1 + 1;          /* Immediate +1 */
            v6 = v2 * 2;          /* Immediate *2 */
            v7 = v3 & 0xFF;       /* Immediate mask */
            v8 = v4 | 0x80;       /* Immediate OR */
            
            /* More computations creating register pressure */
            v9 = v5 + v6;
            v10 = v7 - v8;
            v11 = v9 * v10;
            v12 = v11 / 3;        /* Immediate divisor */
            
            /* Mix different data widths */
            s1 = (short)(v12 & 0xFFFF);
            s2 = (short)((v12 >> 16) & 0xFFFF);
            c1 = (char)(v12 & 0xFF);
            c2 = (char)((v12 >> 8) & 0xFF);
            
            /* Conditional branch creating multiple basic blocks */
            if (v12 > 1000) {
                v13 = v12 + 5;    /* Another immediate */
                v14 = v13 * 7;    /* Immediate multiplier */
                s3 = (short)(v14 % 256); /* Immediate modulus */
            } else {
                v13 = v12 - 5;    /* Immediate subtraction */
                v14 = v13 / 4;    /* Immediate division */
                s3 = (short)(v14 & 0xFF);
            }
            
            /* More arithmetic with different operations */
            v15 = v14 ^ 0xAAAAAAAA; /* Immediate XOR */
            v16 = v15 << 3;       /* Immediate shift */
            v17 = v16 >> 2;       /* Immediate shift */
            
            /* Complex address computation (remat candidate) */
            /* Base + index * scale + displacement */
            int idx = outer * 4 + 1; /* Multiple immediates */
            v18 = arr1[idx % ARRAY_SIZE] + arr2[idx % ARRAY_SIZE];
            
            /* Final computation mixing all values */
            l1 = (long)v17 * (long)v18;
            l2 = l1 + (long)s1 + (long)s2 + (long)s3;
            c3 = (char)((l2 >> 8) & 0xFF);
            c4 = (char)(l2 & 0xFF);
            
            /* Another conditional with volatile check */
            volatile int check = arr3[outer % 16];
            if (check & 1) {
                result += (int)l2 + c3 + c4;
            } else {
                result -= (int)l2 - c3 - c4;
            }
            
            /* Memory barrier between iterations */
            asm volatile("" : : : "memory");
        }
        
        /* Small inner loop with additional pressure */
        for (volatile int inner = 0; inner < 10; inner++) {
            volatile int t1 = result + inner;
            volatile int t2 = t1 * 2;      /* Immediate */
            volatile int t3 = t2 & 0x7F;   /* Immediate mask */
            volatile int t4 = t3 | 0x40;   /* Immediate OR */
            volatile int t5 = t4 ^ 0x55;   /* Immediate XOR */
            
            /* Use in conditional */
            if (t5 > 100) {
                result += t5 + 1;         /* Immediate */
            } else {
                result -= t5 - 1;         /* Immediate */
            }
        }
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int prepare_data(volatile int *arr, int size) {
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    /* Allocate volatile arrays to prevent optimization */
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
    volatile int sum1 = prepare_data(array1, ARRAY_SIZE);
    volatile int sum2 = prepare_data(array2, ARRAY_SIZE);
    volatile int sum3 = prepare_data(array3, ARRAY_SIZE);
    volatile int sum4 = prepare_data(array4, ARRAY_SIZE);
    
    /* Call the high-pressure function multiple times */
    volatile int final_result = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        final_result += high_pressure_loop(array1, array2, array3, array4);
        
        /* Modify arrays slightly to prevent loop invariant removal */
        array1[i % ARRAY_SIZE] ^= 1;
        array2[i % ARRAY_SIZE] ^= 2;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    printf("Checksum: %d\n", final_result);
    
    /* Cleanup */
    free((void*)array1);
    free((void*)array2);
    free((void*)array3);
    free((void*)array4);
    
    return 0;
}
