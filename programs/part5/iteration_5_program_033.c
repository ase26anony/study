/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x + (rand() % 2);  /* Non-deterministic but side-effect free */
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double init_val) {
    volatile int barrier = 0;
    double acc_double = init_val;
    long long acc_int = seed;
    
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_double[16];
    long long arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_double[i] = (i * 1.5) + init_val;
        arr_int[i] = i * 3 + seed;
    }
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = outer * 2 + seed;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 4; inner++) {
            /* Complex condition based on invariant */
            if ((invariant + inner) % 3 == 0) {
                /* REGISTER PRESSURE BLOCK - designed to trigger remat */
                
                /* 1. Vector operations creating virtual registers */
                v4df vec1 = {arr_double[inner], arr_double[inner+1], 
                            arr_double[inner+2], arr_double[inner+3]};
                v4df vec2 = {acc_double, acc_double * 0.5, 
                            acc_double * 0.25, acc_double * 0.125};
                
                /* Shuffle operation - often creates virtual registers */
                v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4di){inner, inner+1, inner+2, inner+3});
                
                /* 2. Mixed integer/floating operations */
                double temp1 = vec_shuffled[0] * vec_shuffled[1];
                long long temp2 = (long long)(temp1 * 100.0);
                
                /* 3. Complex expression with many intermediates */
                double a = arr_double[(inner + invariant) % 16];
                double b = arr_double[(inner * 2) % 16];
                double c = arr_double[(inner * 3) % 16];
                
                /* Volatile barrier in middle of computation */
                barrier = volatile_barrier(inner);
                
                /* More intermediates after barrier */
                double d = a * b + c;
                double e = d / (temp1 + 1.0);
                double f = e * e - d * d;
                
                /* Integer computation with many intermediates */
                long long x = arr_int[(inner + 1) % 16];
                long long y = arr_int[(inner + 2) % 16];
                long long z = arr_int[(inner + 3) % 16];
                
                long long sum1 = x + y + z + temp2;
                long long sum2 = sum1 * (inner + 1);
                long long sum3 = sum2 + (long long)(f * 1000.0);
                
                /* Array access with complex indexing */
                int idx1 = (inner * invariant) % 16;
                int idx2 = (inner + barrier) % 16;
                int idx3 = (sum3 % 16);
                
                /* More mixed operations */
                double g = arr_double[idx1] + arr_double[idx2];
                long long h = arr_int[idx3] * (long long)g;
                
                /* Update accumulators */
                acc_double = acc_double * 0.99 + f + g;
                acc_int = acc_int + sum3 + h;
                
                /* Another volatile barrier */
                barrier = volatile_barrier(acc_int & 0xFF);
            } else {
                /* Alternate path with different operations */
                double simple = arr_double[outer % 16] * 1.1;
                acc_double += simple;
                acc_int += (long long)simple;
            }
        }
        
        /* Cross-iteration dependency */
        arr_double[outer % 16] = acc_double * 0.01;
        arr_int[outer % 16] = acc_int % 1000;
    }
    
    /* Final mixed computation */
    return acc_int + (long long)acc_double;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i * 7, i * 0.1);
        
        /* Add some variation */
        if (i % 3 == 0) {
            total -= test_remat(10, i * 3, i * 0.05);
        }
        if (i % 7 == 0) {
            total ^= test_remat(20, i * 11, i * 0.02);
        }
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
