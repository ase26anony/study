/* test_early_remat.c
 * Designed to trigger early rematerialization in GCC's RTL passes,
 * specifically targeting the emit_copy function logic that creates
 * new virtual registers.
 */

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
static __attribute__((noinline,noipa))
long long test_remat(int iterations, int seed, int threshold) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[256];
    long long arr_ll[256];
    int arr_int[256];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        arr_dbl[i] = (i * 1.5) + seed;
        arr_ll[i] = (long long)i * i * seed;
        arr_int[i] = i ^ seed;
    }
    
    /* Mix of different data types for mode variety */
    double sum_dbl = 0.0;
    long long sum_ll = 0;
    int sum_int = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = (outer * seed) % 256;
        
        /* Inner computation with register pressure */
        for (int inner = 0; inner < 128; inner++) {
            /* Complex condition that depends on invariant */
            if ((inner + invariant) > threshold) {
                /* VOLATILE BARRIER - prevents code motion */
                int volatile_idx = get_volatile_value() % 256;
                
                /* Complex expression with many temporaries */
                /* This should create high register pressure */
                
                /* Vector operations that create virtual registers */
                v4si vec_a = {arr_int[volatile_idx], 
                              arr_int[(volatile_idx + 1) % 256],
                              arr_int[(volatile_idx + 2) % 256],
                              arr_int[(volatile_idx + 3) % 256]};
                
                v4si vec_b = {inner, outer, seed, invariant};
                v4si vec_c = vec_a + vec_b;
                v4si vec_d = vec_c * vec_a;
                
                /* Shuffle operation - often creates virtual registers */
                v4si vec_shuffled = __builtin_shuffle(vec_d, vec_b, 
                    (v4si){1, 3, 0, 2});
                
                /* Mixed integer/floating operations */
                double temp1 = arr_dbl[volatile_idx] * 3.14159;
                double temp2 = temp1 / (inner + 1.0);
                double temp3 = temp2 + arr_dbl[(volatile_idx + 1) % 256];
                
                /* More temporaries with different modes */
                long long temp_ll1 = arr_ll[volatile_idx];
                long long temp_ll2 = temp_ll1 * outer;
                long long temp_ll3 = temp_ll2 + (long long)(temp3 * 1000.0);
                
                /* Integer arithmetic chain */
                int temp_int1 = arr_int[volatile_idx];
                int temp_int2 = temp_int1 * inner;
                int temp_int3 = temp_int2 + invariant;
                int temp_int4 = temp_int3 ^ seed;
                int temp_int5 = temp_int4 * outer;
                
                /* Use all temporaries to prevent dead code elimination */
                sum_dbl += temp1 + temp2 + temp3;
                sum_ll += temp_ll1 + temp_ll2 + temp_ll3;
                sum_int += temp_int1 + temp_int2 + temp_int3 + 
                          temp_int4 + temp_int5;
                
                /* More vector operations */
                v2df vec_dbl1 = {temp1, temp2};
                v2df vec_dbl2 = {temp3, arr_dbl[volatile_idx]};
                v2df vec_dbl3 = vec_dbl1 * vec_dbl2;
                
                sum_dbl += vec_dbl3[0] + vec_dbl3[1];
                
                /* Extract elements from shuffled vector */
                for (int k = 0; k < 4; k++) {
                    sum_int += vec_shuffled[k];
                }
            } else {
                /* Alternate path with different computations */
                int idx = (inner * outer) % 256;
                sum_dbl += arr_dbl[idx] * 0.5;
                sum_ll += arr_ll[idx] >> 2;
                sum_int += arr_int[idx] & 0xFF;
            }
        }
        
        /* Cross-iteration dependency */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Combine results to return a value */
    return sum_ll + (long long)sum_dbl + sum_int;
}

/* Multiple calls with different parameters */
int main(void) {
    long long total = 0;
    
    /* Call test_remat multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += test_remat(5, i * 100, 64 + i);
        total += test_remat(3, i * 50 + 123, 32 + i);
        total += test_remat(2, i * 25 + 456, 96 + i);
    }
    
    /* Print deterministic result */
    printf("Result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    total = 0;
    for (int i = 0; i < 20; i++) {
        total ^= test_remat(1, i * 10, i * 3);
    }
    printf("Result2: %lld\n", total);
    
    return 0;
}
