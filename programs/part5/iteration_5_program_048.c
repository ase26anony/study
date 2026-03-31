/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (seed + i) * factor;
        arr_ll[i] = seed * i;
    }
    
    /* Complex intermediate results */
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    v4di int_acc = {0, 0, 0, 0};
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = volatile_barrier(seed + outer) % 4;
        
        /* Inner if with register-intensive computation */
        if (invariant > 0) {
            /* Complex expression mixing FP and integer ops */
            for (int inner = 0; inner < 8; inner++) {
                /* Non-constant array access */
                int idx1 = (inner * invariant) & 0xF;
                int idx2 = (inner + invariant) & 0xF;
                int idx3 = (inner ^ invariant) & 0xF;
                
                /* Vector operations creating virtual registers */
                v4df vec1 = {arr_dbl[idx1], arr_dbl[idx2], 
                            arr_dbl[idx3], arr_dbl[inner]};
                v4df vec2 = {factor * 1.1, factor * 2.2, 
                            factor * 3.3, factor * 4.4};
                
                /* Shuffle operations */
                v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4di){0, 2, 1, 3});
                
                /* Mixed integer/FP computation */
                double temp_fp = vec_shuffled[0] * vec_shuffled[1] 
                               - vec_shuffled[2] / vec_shuffled[3];
                
                /* Integer computation using same indices */
                long long temp_int = arr_ll[idx1] * arr_ll[idx2] 
                                   + arr_ll[idx3] - arr_ll[inner];
                
                /* More volatile barriers */
                temp_fp += volatile_barrier(inner) * 0.01;
                temp_int ^= volatile_barrier(idx1) << 3;
                
                /* Accumulate results */
                vec_acc += vec_shuffled * temp_fp;
                
                /* Integer vector operations */
                v4di vec_int1 = {temp_int, temp_int / 2, 
                                temp_int / 3, temp_int / 4};
                v4di vec_int2 = {arr_ll[idx1], arr_ll[idx2], 
                                arr_ll[idx3], arr_ll[inner]};
                
                /* Another shuffle on integer vectors */
                v4di vec_int_shuffled = __builtin_shuffle(vec_int1, vec_int2,
                    (v4di){3, 1, 0, 2});
                
                int_acc += vec_int_shuffled;
                
                /* Cross-type computation */
                double cross_val = (double)temp_int * temp_fp;
                arr_dbl[inner] += cross_val * 0.5;
                arr_ll[inner] ^= (long long)(cross_val);
            }
            
            /* Additional computation in the if block */
            for (int i = 0; i < 4; i++) {
                /* More register pressure with different modes */
                double d1 = vec_acc[i];
                double d2 = arr_dbl[i * 2] * arr_dbl[i * 2 + 1];
                long long ll1 = int_acc[i];
                long long ll2 = arr_ll[i * 3] & arr_ll[i * 3 + 1];
                
                /* Complex expression with many intermediates */
                double result_fp = d1 * d2 
                                 + (double)ll1 * 0.25 
                                 - (double)ll2 * 0.125;
                
                long long result_int = (long long)d1 
                                     ^ (long long)d2 
                                     + ll1 * ll2 
                                     - (ll1 >> 4);
                
                /* Store with barrier */
                arr_dbl[i] = result_fp + volatile_barrier(i);
                arr_ll[i + 4] = result_int ^ volatile_barrier(i + 8);
            }
        }
        
        /* Loop-carried dependency */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        factor += 0.01 * (outer % 3);
    }
    
    /* Final reduction */
    double final_fp = 0.0;
    long long final_int = 0;
    
    for (int i = 0; i < 16; i++) {
        final_fp += arr_dbl[i];
        final_int ^= arr_ll[i];
    }
    
    /* Mix FP and integer results */
    return (long long)(final_fp * 1000.0) + final_int;
}

int main(int argc, char **argv) {
    long long total = 0;
    int base_iterations = 100;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 10; i++) {
        total += test_remat(base_iterations + i, i * 17, 1.0 + i * 0.1);
        
        /* Alternate calls with different patterns */
        if (i % 3 == 0) {
            total ^= test_remat(base_iterations / 2, i * 23, 2.0 - i * 0.05);
        }
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
