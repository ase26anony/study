#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure complex expressions stay in place */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Volatile function to create code motion barrier */
    extern volatile int barrier(void);
    
    /* Large vectors to create register pressure */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_ll[i] = start + i * 3;
    }
    
    /* Loop-invariant variable for control flow */
    int invariant = start * 2 + 1;
    
    /* Accumulators mixing types */
    double dbl_acc = 0.0;
    long long int_acc = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    v8si vec_int_acc = {0, 0, 0, 0, 0, 0, 0, 0};
    
    /* Outer loop */
    for (int outer = 0; outer < iterations; outer++) {
        /* Inner loop with complex control flow */
        for (int inner = 0; inner < 8; inner++) {
            /* Condition depending on loop-invariant */
            if ((outer * inner + invariant) % 7 < 4) {
                /* COMPLEX REGISTER-PRESSURE EXPRESSION */
                /* This should create many temporary values */
                
                /* 1. Load from arrays with non-constant indices */
                int idx1 = (inner + outer) & 0xF;
                int idx2 = (inner * 3) & 0xF;
                
                double d1 = arr_dbl[idx1];
                double d2 = arr_dbl[idx2];
                long long ll1 = arr_ll[idx1];
                long long ll2 = arr_ll[idx2];
                
                /* 2. Vector operations creating virtual registers */
                v4df v1 = {d1, d2, d1 * 0.3, d2 * 0.7};
                v4df v2 = {d2, d1, d2 * 0.4, d1 * 0.6};
                
                /* Shuffle operation often creates virtual registers */
                v4df v_shuffled = __builtin_shuffle(v1, v2, 
                    (v8si){0, 5, 2, 7, 4, 1, 6, 3});
                
                /* 3. Mixed integer/floating-point computations */
                /* Barrier prevents code motion */
                int barrier_val = barrier();
                
                /* Complex expression with many intermediates */
                double temp1 = d1 * d2 + (double)ll1 * 0.25;
                double temp2 = d1 / (d2 + 1.0) - (double)ll2 * 0.125;
                
                /* Integer computation */
                long long temp3 = ll1 * ll2 + (long long)(d1 * 100.0);
                long long temp4 = (ll1 << 3) | (ll2 & 0xFF);
                
                /* More vector operations */
                v8si vi1 = {barrier_val, inner, outer, idx1, idx2, 
                           (int)temp3, (int)temp4, invariant};
                v8si vi2 = __builtin_shuffle(vi1, 
                    (v8si){7, 6, 5, 4, 3, 2, 1, 0});
                
                /* Use results in accumulators */
                dbl_acc += temp1 - temp2 + v_shuffled[0] * v_shuffled[1];
                int_acc += temp3 ^ temp4 + vi2[0] * vi2[1];
                
                /* Update vector accumulators */
                vec_acc += v_shuffled;
                vec_int_acc += vi2;
                
                /* Update arrays to create data dependencies */
                arr_dbl[idx1] = dbl_acc * 0.01;
                arr_ll[idx2] = int_acc & 0xFFFF;
            }
        }
        
        /* Additional computation outside if-block */
        for (int i = 0; i < 4; i++) {
            dbl_acc += vec_acc[i] * 0.5;
            int_acc += vec_int_acc[i] * 2;
        }
    }
    
    /* Final mixed-type computation */
    long long result = (long long)dbl_acc + int_acc;
    for (int i = 0; i < 4; i++) {
        result += (long long)vec_acc[i];
    }
    
    return result;
}

/* Dummy barrier function */
volatile int barrier(void) {
    static int counter = 0;
    return ++counter;
}

int main(int argc, char **argv) {
    long long total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 10, iterations, i * 1.5);
        total += test_remat(i * 7 + 1, iterations / 2, i * 2.3);
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
