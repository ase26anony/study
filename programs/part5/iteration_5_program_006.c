/* Test program to trigger early rematerialization in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_barrier(void) {
    barrier_counter++;
    return barrier_counter & 1;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Large arrays to force register pressure */
    double arr_double[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with non-constant indices */
    for (int i = 0; i < 32; i++) {
        arr_idx[i] = (start + i * 3) % 32;
        arr_double[i] = init_val + i * 0.5;
        arr_int[i] = start + i * 7LL;
    }
    
    /* Complex vector types using GCC extensions */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Loop with multiple basic blocks */
    long long total = 0;
    double acc_double = init_val;
    
    for (int outer = 0; outer < iterations; outer++) {
        /* Outer loop invariant that affects inner condition */
        int loop_invariant = (outer * start) & 31;
        
        /* Complex control flow with multiple basic blocks */
        if (loop_invariant > 15) {
            /* Register pressure block - many intermediate values */
            
            /* Vector operations that create virtual registers */
            v4df vec1 = {arr_double[0], arr_double[1], 
                         arr_double[2], arr_double[3]};
            v4df vec2 = {arr_double[4], arr_double[5], 
                         arr_double[6], arr_double[7]};
            
            /* Shuffle operations that need virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 1, 4, 5});
            
            /* Mix integer and floating-point operations */
            for (int inner = 0; inner < 8; inner++) {
                /* Code motion barrier using volatile */
                if (get_barrier()) {
                    /* Complex expression with many temporaries */
                    double temp1 = arr_double[arr_idx[inner]] * 1.5;
                    double temp2 = temp1 + acc_double;
                    long long temp3 = (long long)(temp2 * 100.0);
                    double temp4 = temp2 / (inner + 1.0);
                    
                    /* More intermediate values */
                    long long temp5 = arr_int[arr_idx[inner + 8]];
                    double temp6 = temp4 * temp1;
                    long long temp7 = temp3 ^ temp5;
                    double temp8 = temp6 - temp4;
                    
                    /* Use vector result */
                    double vec_elem = ((double*)&vec_shuffled)[inner & 3];
                    temp8 += vec_elem;
                    
                    /* Final accumulation with mixed types */
                    total += (long long)(temp8 * 10.0) + temp7;
                    acc_double = temp8 * 0.9;
                } else {
                    /* Alternative path with different operations */
                    double alt1 = arr_double[arr_idx[inner + 16]] * 0.75;
                    long long alt2 = arr_int[arr_idx[inner + 24]];
                    total += (long long)(alt1 * 20.0) ^ alt2;
                    acc_double += alt1 * 0.1;
                }
                
                /* More register pressure with array updates */
                arr_double[arr_idx[inner]] += 0.01;
                arr_int[arr_idx[inner]] += inner;
            }
            
            /* Additional vector operations outside inner loop */
            v4di vec_int1 = {arr_int[0], arr_int[1], arr_int[2], arr_int[3]};
            v4di vec_int2 = {arr_int[4], arr_int[5], arr_int[6], arr_int[7]};
            v4di vec_int_shuffled = __builtin_shuffle(vec_int1, vec_int2,
                (v4di){2, 3, 6, 7});
            
            /* Use shuffled result */
            for (int i = 0; i < 4; i++) {
                total += ((long long*)&vec_int_shuffled)[i];
            }
        } else {
            /* Simpler path but still with register pressure */
            for (int i = 0; i < 16; i++) {
                double simple1 = arr_double[i] * arr_double[31 - i];
                long long simple2 = arr_int[i] * arr_int[31 - i];
                total += (long long)(simple1 * 5.0) + (simple2 & 0xFFFF);
                acc_double = simple1 * 0.5;
            }
        }
        
        /* Loop-carried dependency with mixed operations */
        acc_double = acc_double * 0.99 + (outer * 0.01);
        total += (long long)(acc_double * 1000.0);
    }
    
    return total;
}

int main(void) {
    long long final_total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        final_total += test_remat(i, 50, i * 0.25);
        
        /* Also test with different modes */
        if (i % 3 == 0) {
            final_total ^= test_remat(i * 2, 25, i * 0.5);
        }
        if (i % 7 == 0) {
            final_total -= test_remat(i * 3, 10, i * 0.75);
        }
    }
    
    /* Print deterministic result */
    printf("Result: %lld\n", final_total);
    
    /* Additional test with more aggressive parameters */
    long long extra_test = test_remat(12345, 100, 678.9);
    printf("Extra: %lld\n", extra_test);
    
    return 0;
}
