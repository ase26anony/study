/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier __attribute__((unused));

/* Non-inlineable function to prevent optimization */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double init_val) {
    /* Large arrays to create register pressure */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_ll[i] = start_val + i * 3LL;
        arr_int[i] = start_val * i;
    }
    
    /* Complex mixed-type computations */
    double sum_dbl = 0.0;
    long long sum_ll = 0;
    int sum_int = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = start_val * outer % 7;
        
        /* Inner if with complex expression */
        if (invariant > 2) {
            /* Register pressure inducing expression with volatile barrier */
            barrier = outer;
            
            /* Complex mixed-type computation using vector-like operations */
            for (int i = 0; i < 16; i++) {
                /* Non-constant array indexing */
                int idx1 = (i + outer) % 32;
                int idx2 = (i * 3 + start_val) % 32;
                int idx3 = (i * 5 + invariant) % 32;
                
                /* Mixed integer/floating-point operations */
                double temp1 = arr_dbl[idx1] * arr_dbl[idx2];
                long long temp2 = arr_ll[idx1] + arr_ll[idx2];
                int temp3 = arr_int[idx1] * arr_int[idx2];
                
                /* More complex expressions with intermediate values */
                double temp4 = temp1 * (double)temp2 + (double)temp3;
                long long temp5 = (long long)temp1 * temp2 + temp3;
                
                /* Use __builtin_shufflevector to create virtual registers */
                typedef int v4si __attribute__((vector_size(16)));
                v4si v1 = {temp3, temp3/2, temp3/3, temp3/4};
                v4si v2 = {idx1, idx2, idx3, i};
                v4si v3 = __builtin_shuffle(v1, v2, 
                    (v4si){0, 4, 1, 5});  /* Creates virtual registers */
                
                /* Barrier to prevent code motion */
                barrier = v3[0];
                
                /* More mixed computations */
                sum_dbl += temp4 * (double)v3[0] + (double)v3[1];
                sum_ll += temp5 * (long long)v3[2] + (long long)v3[3];
                sum_int += temp3 * v3[0] + v3[1] * v3[2];
                
                /* Additional pressure with many intermediate results */
                double d1 = sum_dbl * 0.1;
                double d2 = d1 * arr_dbl[idx3];
                double d3 = d2 + (double)sum_ll * 0.01;
                sum_dbl = d3 * (double)(sum_int % 100);
                
                long long ll1 = sum_ll >> 2;
                long long ll2 = ll1 * arr_ll[idx3];
                long long ll3 = ll2 + (long long)(sum_dbl * 100.0);
                sum_ll = ll3 ^ (long long)sum_int;
                
                int i1 = sum_int * 3;
                int i2 = i1 + arr_int[idx3];
                int i3 = i2 ^ (int)sum_ll;
                sum_int = i3 & 0x7FFFFFFF;
            }
        } else {
            /* Alternate path to create control flow complexity */
            for (int i = 0; i < 8; i++) {
                int idx = (i + invariant) % 32;
                sum_dbl += arr_dbl[idx] * 2.0;
                sum_ll += arr_ll[idx] * 2;
                sum_int += arr_int[idx] * 2;
            }
        }
        
        /* Cross-iteration dependencies */
        arr_dbl[outer % 32] = sum_dbl * 0.01;
        arr_ll[outer % 32] = sum_ll >> 1;
        arr_int[outer % 32] = sum_int & 0xFF;
    }
    
    /* Final mixed computation */
    long long result = (long long)sum_dbl + sum_ll + sum_int;
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i, (double)i * 0.3);
        
        /* Prevent loop optimization */
        if (i % 10 == 0) {
            printf("Progress: %d, total so far: %lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    return 0;
}
