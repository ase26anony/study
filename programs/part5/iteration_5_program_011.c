/* Compile with: gcc -O2 -fno-inline -fdump-rtl-all test.c -o test */
/* Or: gcc -O3 -fsanitize=undefined -fno-omit-frame-pointer test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter & 0xFF;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double scale_factor) {
    /* Large arrays to force register pressure */
    double arr_double[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with non-constant indices */
    for (int i = 0; i < 32; i++) {
        arr_idx[i] = (start_val + i * 3) % 32;
        arr_double[i] = (double)(start_val + i) * 0.5;
        arr_int[i] = (long long)(start_val * i);
    }
    
    /* Complex mixed-mode computations */
    double sum_double = 0.0;
    long long sum_int = 0;
    double prod_double = 1.0;
    
    /* Outer loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Loop-invariant condition */
        int invariant_cond = (start_val * iter) % 7;
        
        /* Inner if with complex expression */
        if (invariant_cond > 2) {
            /* Register pressure inducing expression with volatile barrier */
            volatile int barrier = get_volatile_value();
            
            /* Complex mixed integer/float computation */
            for (int i = 0; i < 16; i++) {
                /* Non-constant array indexing */
                int idx1 = arr_idx[i] + barrier;
                int idx2 = arr_idx[31 - i] + (iter & 0xF);
                
                /* Mixed-mode operations that create many temporaries */
                double temp1 = arr_double[idx1 & 31] * scale_factor;
                double temp2 = arr_double[idx2 & 31] / (scale_factor + 1.0);
                
                /* Integer operations */
                long long int_temp1 = arr_int[idx1 & 31] + (long long)temp1;
                long long int_temp2 = arr_int[idx2 & 31] - (long long)temp2;
                
                /* More mixed operations */
                double mixed1 = (double)int_temp1 * temp1;
                double mixed2 = (double)int_temp2 * temp2;
                
                /* Vector-like operations using GCC extensions */
                typedef double v2df __attribute__((vector_size(16)));
                v2df vec1 = {mixed1, mixed2};
                v2df vec2 = {temp2, temp1};
                v2df vec_result = vec1 + vec2 * vec1 - vec2;
                
                /* Shuffle operation that often creates virtual registers */
                v2df shuffled = __builtin_shuffle(vec_result, vec_result, 
                                                 (v2df){1, 0});
                
                /* Accumulate results */
                sum_double += vec_result[0] + shuffled[1];
                sum_int += (long long)(vec_result[0] * 100.0);
                
                /* Cross-type operations */
                prod_double *= (mixed1 / (mixed2 + 1.0)) + 
                              (double)(int_temp1 % 256) * 0.01;
            }
            
            /* Additional computation outside the inner loop */
            double extra = 0.0;
            for (int j = 0; j < 8; j++) {
                /* More register pressure */
                extra += arr_double[(barrier + j) & 31] * 
                        arr_double[(start_val + j) & 31];
                
                /* Integer computation */
                sum_int += arr_int[(iter + j) & 31] * 
                          (long long)(extra * 10.0);
            }
            
            /* Final mixed computation */
            sum_double += extra * scale_factor;
        } else {
            /* Alternative path to create control flow complexity */
            for (int i = 0; i < 8; i++) {
                sum_int -= arr_int[(iter + i) & 31];
                sum_double -= arr_double[i] * 0.25;
            }
        }
        
        /* Loop-carried dependency */
        scale_factor *= 0.99;
        start_val = (start_val * 13 + 7) & 0xFF;
    }
    
    /* Combine results */
    return sum_int + (long long)(sum_double * prod_double);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 10), i * 3, 1.0 + i * 0.01);
        
        /* Also test with different modes */
        if (i % 3 == 0) {
            total += test_remat(30, i * 7, 2.0 - i * 0.005);
        }
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
