/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
volatile int volatile_func(int x) {
    return x ^ 0x1234;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double init_double) {
    /* Large vectors to create virtual registers */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_double[16];
    long long arr_llong[16];
    int arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_double[i] = init_double + i * 0.5;
        arr_llong[i] = start_val + i * 3LL;
        arr_int[i] = start_val + i * 2;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = volatile_func(start_val);
    
    /* Accumulators of different types */
    double double_acc = init_double;
    long long llong_acc = start_val;
    int int_acc = 0;
    
    /* Vector accumulators */
    v4df vec_double_acc = {init_double, init_double + 1.0, 
                          init_double + 2.0, init_double + 3.0};
    v8si vec_int_acc = {start_val, start_val + 1, start_val + 2, start_val + 3,
                       start_val + 4, start_val + 5, start_val + 6, start_val + 7};
    
    /* Outer loop */
    for (int iter = 0; iter < iterations; iter++) {
        /* Inner if condition depending on loop-invariant */
        if ((loop_invariant + iter) % 7 < 4) {
            /* COMPLEX REGISTER-PRESSURE EXPRESSION */
            /* This creates many intermediate values that need registers */
            
            /* 1. Vector operations with shuffles (create virtual registers) */
            v4df vec_a = {arr_double[iter % 16], arr_double[(iter + 1) % 16],
                         arr_double[(iter + 2) % 16], arr_double[(iter + 3) % 16]};
            v4df vec_b = {arr_double[(iter + 4) % 16], arr_double[(iter + 5) % 16],
                         arr_double[(iter + 6) % 16], arr_double[(iter + 7) % 16]};
            
            /* Shuffle operation - often creates virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec_a, vec_b, 
                (v4df){1, 3, 0, 2});
            
            /* 2. Mixed integer/floating-point computations */
            for (int j = 0; j < 8; j++) {
                /* Non-constant array indexing */
                int idx1 = (iter + j) % 16;
                int idx2 = (iter + j + 4) % 16;
                int idx3 = (iter + j + 8) % 16;
                
                /* Volatile call creates code motion barrier */
                int barrier = volatile_func(idx1);
                
                /* Complex expression with many intermediates */
                double temp1 = arr_double[idx1] * 1.5 + barrier * 0.01;
                long long temp2 = arr_llong[idx2] * (barrier % 5 + 1);
                int temp3 = arr_int[idx3] ^ barrier;
                
                /* Mix operations with different modes */
                double_acc += temp1 + (temp2 % 100) * 0.001;
                llong_acc += (long long)(temp1 * 100.0) ^ temp2;
                int_acc += temp3 * (int)(temp1 - (int)temp1);
                
                /* More vector operations inside loop */
                v4df vec_temp = vec_a + vec_b * (j + 1);
                vec_double_acc += vec_temp * vec_shuffled;
                
                /* Integer vector operations */
                v8si vec_int_temp = {idx1, idx2, idx3, barrier, 
                                    idx1^idx2, idx2^idx3, idx3^barrier, barrier^idx1};
                vec_int_acc += vec_int_temp * (j + 2);
            }
            
            /* 3. Additional computations after the inner loop */
            /* Use __builtin_shuffle again to force virtual register creation */
            v8si shuffled_int = __builtin_shuffle(vec_int_acc, vec_int_acc,
                (v8si){7, 6, 5, 4, 3, 2, 1, 0});
            
            /* More mixed-mode computations */
            for (int k = 0; k < 4; k++) {
                double temp_d = ((double*)&vec_double_acc)[k];
                int temp_i = ((int*)&shuffled_int)[k * 2];
                
                /* Another volatile barrier */
                int barrier2 = volatile_func(temp_i);
                
                /* Complex expression spanning multiple operations */
                double intermediate1 = temp_d * barrier2 * 0.5;
                long long intermediate2 = (long long)(intermediate1 * 1000.0);
                int intermediate3 = (int)(intermediate1 - (int)intermediate1) * 10000;
                
                double_acc += intermediate1 + intermediate3 * 0.0001;
                llong_acc ^= intermediate2;
                int_acc += intermediate3 + barrier2;
            }
        } else {
            /* Simpler path but still with computations */
            int_acc += volatile_func(iter) * 2;
            double_acc += arr_double[iter % 16] * 0.25;
        }
        
        /* Loop-carried dependencies */
        arr_double[iter % 16] = double_acc * 0.99;
        arr_llong[iter % 16] = llong_acc ^ iter;
        arr_int[iter % 16] = int_acc % 1000;
    }
    
    /* Final mixed computation */
    long long result = llong_acc;
    result += (long long)(double_acc * 100.0);
    result ^= (int_acc * 123456789LL);
    
    /* Extract from vectors */
    for (int i = 0; i < 4; i++) {
        result += (long long)(((double*)&vec_double_acc)[i] * 10.0);
    }
    for (int i = 0; i < 8; i++) {
        result += ((int*)&vec_int_acc)[i];
    }
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        long long result = test_remat(iterations, i * 100, i * 1.5);
        total += result;
        printf("Iteration %d: result = %lld, total = %lld\n", i, result, total);
    }
    
    printf("Final checksum: %lld\n", total);
    return 0;
}
