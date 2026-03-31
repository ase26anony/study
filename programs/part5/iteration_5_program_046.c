/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int offset, double seed) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = seed + i * 0.5;
        arr_int[i] = (long long)(seed * 1000) + i * 3;
    }
    
    /* Complex expression with mixed types */
    double sum_dbl = 0.0;
    long long sum_int = 0;
    v4df vec_sum = {0, 0, 0, 0};
    
    /* Outer loop with multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for control flow */
        int invariant = get_volatile_value() % 4;
        
        /* Inner if with complex register pressure */
        if (invariant > 0) {
            /* Complex expression with many temporaries */
            v4df vec1 = {arr_dbl[offset % 16], 
                        arr_dbl[(offset + 1) % 16],
                        arr_dbl[(offset + 2) % 16],
                        arr_dbl[(offset + 3) % 16]};
            
            v4df vec2 = {arr_dbl[(offset + 4) % 16] * 1.1,
                        arr_dbl[(offset + 5) % 16] * 1.2,
                        arr_dbl[(offset + 6) % 16] * 1.3,
                        arr_dbl[(offset + 7) % 16] * 1.4};
            
            /* Vector operations creating many virtual registers */
            v4df vec3 = vec1 + vec2;
            v4df vec4 = vec1 * vec2;
            v4df vec5 = __builtin_shuffle(vec3, vec4, 
                (v4di){0, 2, 4, 6});
            
            /* Mix integer and floating point */
            v4di int_vec1 = {arr_int[offset % 16],
                           arr_int[(offset + 1) % 16],
                           arr_int[(offset + 2) % 16],
                           arr_int[(offset + 3) % 16]};
            
            v4di int_vec2 = {arr_int[(offset + 4) % 16] + 1000LL,
                           arr_int[(offset + 5) % 16] + 2000LL,
                           arr_int[(offset + 6) % 16] + 3000LL,
                           arr_int[(offset + 7) % 16] + 4000LL};
            
            /* More vector operations */
            v4di int_vec3 = int_vec1 + int_vec2;
            v4di int_vec4 = int_vec1 * int_vec2;
            v4di int_vec5 = __builtin_shuffle(int_vec3, int_vec4,
                (v4di){1, 3, 5, 7});
            
            /* Cross-type conversions creating register pressure */
            double temp1 = (double)int_vec5[0] * 0.001;
            double temp2 = (double)int_vec5[1] * 0.002;
            double temp3 = (double)int_vec5[2] * 0.003;
            double temp4 = (double)int_vec5[3] * 0.004;
            
            /* Complex expression with volatile barrier */
            double complex_expr = temp1 + temp2 * get_volatile_value() 
                                - temp3 / (get_volatile_value() + 1)
                                + temp4 * vec5[0];
            
            /* Accumulate results */
            sum_dbl += complex_expr + vec5[1] + vec5[2] + vec5[3];
            
            /* Integer calculations */
            long long int_expr = int_vec5[0] * 2LL 
                               + int_vec5[1] * 3LL 
                               - int_vec5[2] * 4LL 
                               + int_vec5[3] * 5LL;
            
            sum_int += int_expr + (long long)(complex_expr * 1000.0);
            
            /* Update vec_sum with mixed operations */
            vec_sum = vec_sum + vec5 * 0.5 + (v4df){temp1, temp2, temp3, temp4};
        }
        
        /* Else branch with different computations */
        else {
            /* Different computation path to create control flow complexity */
            double alt_dbl = arr_dbl[(offset + outer) % 16] 
                           * arr_dbl[(offset + outer + 1) % 16];
            
            long long alt_int = arr_int[(offset + outer) % 16] 
                              ^ arr_int[(offset + outer + 1) % 16];
            
            sum_dbl += alt_dbl * get_volatile_value();
            sum_int += alt_int * get_volatile_value();
            
            /* More vector operations */
            v4df alt_vec = {alt_dbl, alt_dbl * 2.0, 
                          alt_dbl * 3.0, alt_dbl * 4.0};
            vec_sum = vec_sum + alt_vec;
        }
        
        /* Modify offset to change array access patterns */
        offset = (offset * 13 + 7) % 16;
    }
    
    /* Final reduction */
    double final_dbl = sum_dbl + vec_sum[0] + vec_sum[1] 
                     + vec_sum[2] + vec_sum[3];
    
    long long final_int = sum_int + (long long)final_dbl;
    
    return final_int;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i % 8, (double)i * 0.1);
        
        /* Add some variation */
        if (i % 3 == 0) {
            total += test_remat(25, (i * 2) % 8, (double)i * 0.05);
        }
        
        if (i % 7 == 0) {
            total -= test_remat(10, (i * 3) % 8, (double)i * 0.02);
        }
    }
    
    printf("Result checksum: %lld\n", total);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
