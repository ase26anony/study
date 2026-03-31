#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure complex expressions stay in place */
static __attribute__((noinline)) 
long long test_remat(int start, int iter_count, volatile int* barrier) {
    /* Large vectors to create register pressure */
    typedef int64_t v4i64 __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + start) * 1.5;
        arr_ll[i] = (i + start) * 3;
    }
    
    /* Volatile function call barrier */
    int barrier_val = *barrier;
    
    /* Loop-invariant variable for control flow */
    int invariant = start * 2 + barrier_val;
    
    /* Accumulator for results */
    long long total = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < iter_count; outer++) {
        /* Complex control flow */
        if (invariant > (outer * 3)) {
            /* Register pressure: many intermediate values */
            v4df vec1 = {arr_dbl[outer & 15], arr_dbl[(outer + 1) & 15], 
                         arr_dbl[(outer + 2) & 15], arr_dbl[(outer + 3) & 15]};
            v4df vec2 = {arr_dbl[(outer + 4) & 15], arr_dbl[(outer + 5) & 15],
                         arr_dbl[(outer + 6) & 15], arr_dbl[(outer + 7) & 15]};
            
            /* Mixed integer/floating operations */
            v4df vec3 = vec1 * vec2 + (v4df){1.0, 2.0, 3.0, 4.0};
            
            /* Use __builtin_shuffle to create virtual registers */
            v4i64 shuffle_idx = {0, 2, 1, 3};
            v4df vec4 = __builtin_shuffle(vec3, vec3, shuffle_idx);
            
            /* More operations with different modes */
            double temp1 = vec4[0] + vec4[1];
            double temp2 = vec4[2] * vec4[3];
            
            /* Integer operations */
            long long ll1 = arr_ll[outer & 15];
            long long ll2 = arr_ll[(outer + 8) & 15];
            long long ll3 = ll1 * ll2 + (outer * 5);
            
            /* Mix types - forces mode changes */
            double mixed1 = temp1 * ll3;
            double mixed2 = temp2 / (ll1 + 1);
            
            /* Complex expression with barrier in middle */
            *barrier = barrier_val + outer;
            
            /* More vector operations */
            v4i64 vec_int1 = {ll1, ll2, ll3, total};
            v4i64 vec_int2 = {outer, start, barrier_val, invariant};
            v4i64 vec_int3 = vec_int1 + vec_int2 * 2;
            
            /* Shuffle with different pattern */
            v4i64 shuffle_idx2 = {3, 1, 0, 2};
            v4i64 vec_int4 = __builtin_shuffle(vec_int3, vec_int3, shuffle_idx2);
            
            /* Final computation using all intermediates */
            total += (long long)(mixed1 + mixed2) + vec_int4[0] + vec_int4[1];
            
            /* Additional operations to increase register pressure */
            for (int inner = 0; inner < 4; inner++) {
                double d1 = arr_dbl[(outer + inner) & 15];
                double d2 = arr_dbl[(outer + inner + 4) & 15];
                long long l1 = arr_ll[(outer + inner) & 15];
                long long l2 = arr_ll[(outer + inner + 8) & 15];
                
                /* Complex expression that can't be easily moved */
                double complex_expr = (d1 * l1) / (d2 + 1.0) + (l2 * 0.5);
                total += (long long)complex_expr;
                
                /* Another barrier */
                if (inner == 2) {
                    *barrier = barrier_val + inner;
                }
            }
        } else {
            /* Alternate path to create control flow complexity */
            total += arr_ll[outer & 15] * 2;
        }
    }
    
    return total;
}

int main(void) {
    volatile int barrier = 42;
    long long checksum = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        checksum += test_remat(i, 50, &barrier);
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
