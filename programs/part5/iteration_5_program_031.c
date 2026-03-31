/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, int threshold) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (i + start) * 1.5;
        arr_ll[i] = (i + start) * 3LL;
        arr_int[i] = (i + start) * 2;
    }
    
    /* Complex intermediate results */
    v4si vec_int_result = {0, 0, 0, 0};
    v2df vec_dbl_result = {0.0, 0.0};
    v2di vec_ll_result = {0LL, 0LL};
    
    long long final_result = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile_value() % 100;
        
        /* Inner if with register-intensive computation */
        if (invariant > threshold) {
            /* Complex expression mixing types - creates register pressure */
            for (int inner = 0; inner < 16; inner++) {
                /* Non-constant array indexing */
                int idx1 = (inner + outer) & 31;
                int idx2 = (inner * 3) & 31;
                int idx3 = (inner + start) & 31;
                
                /* Mixed integer/float operations with many temporaries */
                double temp1 = arr_dbl[idx1] * 2.5 + arr_int[idx2];
                long long temp2 = arr_ll[idx3] * 3LL + (long long)temp1;
                double temp3 = temp1 * 0.75 - (double)temp2;
                int temp4 = (int)temp3 + arr_int[idx1] * arr_int[idx2];
                long long temp5 = temp2 + (long long)temp4 * 2LL;
                double temp6 = temp3 * 1.25 + (double)temp5;
                
                /* Vector operations that create virtual registers */
                v4si v1 = {temp4, temp4 + 1, temp4 + 2, temp4 + 3};
                v4si v2 = {arr_int[idx1], arr_int[idx2], arr_int[idx3], inner};
                v4si v3 = v1 + v2 * 2;
                
                v2df dv1 = {temp1, temp6};
                v2df dv2 = {arr_dbl[idx1], arr_dbl[idx2]};
                v2df dv3 = dv1 * dv2 + (v2df){1.0, 2.0};
                
                v2di lv1 = {temp2, temp5};
                v2di lv2 = {arr_ll[idx1], arr_ll[idx2]};
                v2di lv3 = lv1 + lv2 / 2LL;
                
                /* __builtin_shuffle creates virtual registers */
                v4si shuffled = __builtin_shuffle(v1, v2, 
                    (v4si){1, 3, 0, 2});
                
                /* More operations to increase register pressure */
                vec_int_result += v3 + shuffled;
                vec_dbl_result += dv3;
                vec_ll_result += lv3;
                
                /* Use volatile to prevent optimization */
                arr_dbl[idx1] = temp6 + (double)get_volatile_value() * 0.01;
                arr_ll[idx3] = temp5 + get_volatile_value();
            }
            
            /* Accumulate results with more mixed operations */
            for (int i = 0; i < 4; i++) {
                final_result += vec_int_result[i] + (long long)vec_dbl_result[i % 2];
            }
            for (int i = 0; i < 2; i++) {
                final_result += vec_ll_result[i] * 2LL;
            }
        } else {
            /* Alternate path to create control flow complexity */
            for (int i = 0; i < 8; i++) {
                int idx = (i + outer) & 31;
                arr_dbl[idx] *= 0.99;
                arr_ll[idx] -= get_volatile_value();
            }
        }
        
        /* Cross-iteration dependencies */
        for (int i = 0; i < 4; i++) {
            int idx = (i + outer) & 31;
            arr_int[idx] = (arr_int[idx] + 1) & 0xFF;
        }
    }
    
    /* Final computation mixing all types */
    double dbl_sum = 0.0;
    long long ll_sum = 0;
    
    for (int i = 0; i < 32; i++) {
        dbl_sum += arr_dbl[i];
        ll_sum += arr_ll[i];
        final_result += arr_int[i];
    }
    
    final_result += (long long)(dbl_sum * 100.0) + ll_sum;
    
    return final_result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50, 30 + (i % 20));
        
        /* Prevent loop optimization */
        if (i % 10 == 0) {
            barrier_counter++;
        }
    }
    
    /* Deterministic output */
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
