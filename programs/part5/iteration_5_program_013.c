/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier __attribute__((unused));

/* Non-inlineable function to force register pressure */
static __attribute__((noinline, noipa))
long long test_remat(int start, int iter_count, double init_val) {
    /* Large arrays to force stack usage */
    double arr_dbl[64];
    long long arr_int[64];
    volatile double vol_dbl = 3.14159;
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 64; i++) {
        arr_dbl[i] = init_val * i + start;
        arr_int[i] = (long long)(init_val * 1000) + i * start;
    }
    
    /* Complex expression with many intermediate values */
    double sum_dbl = 0.0;
    long long sum_int = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iter_count; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = start + (outer & 3);
        
        for (int inner = 0; inner < 32; inner++) {
            /* Complex condition creating multiple basic blocks */
            if ((inner + invariant) % 7 == 0) {
                /* Register pressure inducing expression with barriers */
                barrier = inner;
                
                /* Mix integer and floating-point operations */
                double temp1 = arr_dbl[inner] * vol_dbl;
                long long temp2 = arr_int[inner] * (inner + 1);
                
                /* Vector-like operations using GCC extensions */
                typedef double v4df __attribute__((vector_size(32)));
                typedef long long v4di __attribute__((vector_size(32)));
                
                /* Create vector values (virtual registers likely) */
                v4df vec_dbl = {temp1, arr_dbl[inner], vol_dbl, init_val};
                v4di vec_int = {temp2, arr_int[inner], inner, outer};
                
                /* Shuffle operations that create virtual registers */
                v4df shuffled = __builtin_shuffle(vec_dbl, 
                    (v4di){1, 0, 3, 2});
                v4di shuffled_int = __builtin_shuffle(vec_int,
                    (v4di){3, 2, 1, 0});
                
                /* More complex mixed operations */
                for (int k = 0; k < 4; k++) {
                    /* Non-constant array indexing */
                    int idx = (inner + k + start) & 63;
                    
                    /* Expression with many temporaries */
                    double dbl_op = shuffled[k] * arr_dbl[idx];
                    long long int_op = shuffled_int[k] + arr_int[idx];
                    
                    /* Chain computations to keep values live */
                    dbl_op = dbl_op * dbl_op - dbl_op / 2.0;
                    int_op = (int_op * 3) / 2 + (int_op % 7);
                    
                    /* Use volatile to prevent optimization */
                    vol_dbl = dbl_op * 0.99;
                    
                    /* Accumulate results */
                    sum_dbl += dbl_op + (double)int_op;
                    sum_int += int_op + (long long)dbl_op;
                    
                    /* Another barrier */
                    barrier = k;
                }
                
                /* Additional computation with mode mixing */
                double dbl_from_int = (double)sum_int * 0.01;
                long long int_from_dbl = (long long)(sum_dbl * 100.0);
                
                sum_dbl += dbl_from_int * arr_dbl[inner & 63];
                sum_int += int_from_dbl ^ arr_int[inner & 63];
            } else {
                /* Alternate path with different computations */
                double alt_dbl = arr_dbl[inner] / (inner + 1.0);
                long long alt_int = arr_int[inner] - inner;
                
                /* More vector operations */
                typedef double v2df __attribute__((vector_size(16)));
                v2df vec2 = {alt_dbl, sum_dbl};
                v2df vec3 = {arr_dbl[inner], vol_dbl};
                
                /* Shuffle between different vector types */
                v2df mixed = vec2 + __builtin_shuffle(vec3, vec3);
                sum_dbl += mixed[0] + mixed[1];
                sum_int += alt_int * (inner + 1);
            }
        }
        
        /* Loop-carried dependency */
        start = (start * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final mixing to produce deterministic result */
    return (long long)sum_dbl + sum_int;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 5, 1.0 + i * 0.1);
        
        /* Prevent loop optimization */
        if (i % 10 == 0) {
            printf("Progress: %d\n", i);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
