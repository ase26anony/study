#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Prevent interprocedural optimizations */
__attribute__((noinline, noipa))
static volatile int high_pressure_loop(volatile int *arr1, volatile int *arr2, 
                                      volatile int *arr3, volatile int bound) {
    /* Create many variables with overlapping live ranges */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile int result = 0;
    
    /* Force register pressure with complex dependencies */
    for (volatile int i = 0; i < bound; i++) {
        /* Multiple basic blocks created by conditionals */
        if (arr1[i] & 1) {
            /* Chain of dependent computations using immediate constants */
            v1 = arr1[i] + 1;          /* Candidate for remat: +1 */
            v2 = v1 * 2;               /* Candidate for remat: *2 */
            v3 = v2 & 0xFF;            /* Candidate for remat: &0xFF */
            v4 = v3 | 0x80;            /* Candidate for remat: |0x80 */
            v5 = v4 - 1;               /* Candidate for remat: -1 */
            
            /* More variables with different widths to create partial regs */
            volatile char c1 = v5 & 0xFF;
            volatile short s1 = v5 >> 8;
            v6 = c1 + s1;
            v7 = v6 * 3;               /* Candidate for remat: *3 */
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
            
            /* Nested scope with additional variables */
            {
                volatile long l1 = v7;
                volatile int t1 = l1 & 0xFFFF;
                volatile int t2 = t1 + 4;  /* Candidate for remat: +4 */
                v8 = t2 * arr2[i];
            }
        } else {
            /* Alternative path with different computations */
            v1 = arr1[i] - 1;          /* Candidate for remat: -1 */
            v2 = v1 / 2;               /* Candidate for remat: /2 */
            v3 = v2 | 0x7F;            /* Candidate for remat: |0x7F */
            v4 = v3 ^ 0x55;            /* Candidate for remat: ^0x55 */
            v5 = v4 + 2;               /* Candidate for remat: +2 */
            
            volatile unsigned char uc1 = v5;
            volatile unsigned short us1 = v5 >> 4;
            v6 = uc1 * us1;
            v7 = v6 & 0x3F;            /* Candidate for remat: &0x3F */
            
            asm volatile("" : : : "memory");
            
            {
                volatile long l2 = v7;
                volatile int t3 = l2 | 0x1000;
                volatile int t4 = t3 - 8;  /* Candidate for remat: -8 */
                v8 = t4 + arr3[i];
            }
        }
        
        /* Second level of computations with more variables */
        v9 = v8 + arr2[i % ARRAY_SIZE];
        v10 = v9 * 5;                  /* Candidate for remat: *5 */
        v11 = v10 >> 1;                /* Candidate for remat: >>1 */
        v12 = v11 & arr3[i % ARRAY_SIZE];
        v13 = v12 + 7;                 /* Candidate for remat: +7 */
        v14 = v13 * v8;
        v15 = v14 - 3;                 /* Candidate for remat: -3 */
        
        /* Address computation with loop-invariant components */
        volatile int idx = i;
        volatile int *ptr1 = &arr1[idx];
        volatile int *ptr2 = &arr2[idx];
        v16 = *ptr1 + *ptr2;
        v17 = v16 * 9;                 /* Candidate for remat: *9 */
        
        /* More overlapping live ranges */
        v18 = v15 + v17;
        v19 = v18 & 0x7FFFFFFF;
        v20 = v19 | 0x80000000;
        
        /* Conditional store to create more pressure */
        if (v20 > 0) {
            result += v20;
        } else {
            result -= v20;
        }
        
        /* Rotate variables to extend live ranges */
        v1 = v20;
        v2 = v19;
        v3 = v18;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

/* Another noinline function to create more register pressure context */
__attribute__((noinline, noipa))
static volatile int outer_wrapper(volatile int *a, volatile int *b, 
                                 volatile int *c, volatile int outer_bound) {
    volatile int total = 0;
    
    for (volatile int outer = 0; outer < outer_bound; outer++) {
        /* Create additional variables in outer loop */
        volatile int x1 = outer;
        volatile int x2 = x1 * 2;
        volatile int x3 = x2 + a[outer % ARRAY_SIZE];
        volatile int x4 = x3 & b[outer % ARRAY_SIZE];
        volatile int x5 = x4 | c[outer % ARRAY_SIZE];
        volatile int x6 = x5 ^ 0xAAAAAAAA;
        volatile int x7 = x6 * 11;
        volatile int x8 = x7 + 13;
        volatile int x9 = x8 - 17;
        volatile int x10 = x9 & 0x55555555;
        
        /* Call the high pressure function */
        total += high_pressure_loop(a, b, c, 100);
        
        /* Use the variables to prevent dead code elimination */
        total ^= x10;
    }
    
    return total;
}

int main(void) {
    /* Initialize with pseudo-random data */
    volatile int array1[ARRAY_SIZE];
    volatile int array2[ARRAY_SIZE];
    volatile int array3[ARRAY_SIZE];
    
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = rand();
    }
    
    /* Create register pressure with nested loops */
    volatile int final_result = 0;
    
    for (int iter = 0; iter < 10; iter++) {
        /* Vary the bounds to create different flow patterns */
        volatile int bound = 50 + (iter % 10);
        
        /* Call the wrapper which calls the high pressure loop */
        final_result += outer_wrapper(array1, array2, array3, bound);
        
        /* Modify arrays slightly to prevent complete optimization */
        array1[iter % ARRAY_SIZE] ^= final_result;
        array2[iter % ARRAY_SIZE] += iter;
        array3[iter % ARRAY_SIZE] -= final_result;
    }
    
    printf("Result: %d\n", final_result);
    return 0;
}
