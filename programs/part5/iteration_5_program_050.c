/* test_early_remat.c
 * Designed to trigger early rematerialization's emit_copy logic
 * Compile with: gcc -O3 -fno-inline -fdump-rtl-all test_early_remat.c -o test_early_remat
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_barrier(void) {
    barrier_counter++;
    return barrier_counter & 1;
}

/* Non-inlineable function with __attribute__((noinline)) */
static __attribute__((noinline, noipa))
long long test_remat(int iterations, int seed, int threshold) {
    /* Large vectors to create register pressure */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[64];
    long long arr_ll[64];
    int arr_int[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_dbl[i] = (seed + i) * 0.5;
        arr_ll[i] = (long long)(seed * 1000) + i * i;
        arr_int[i] = seed ^ i;
    }
    
    /* Mix of different data types for mode variety */
    v4df vec_dbl = {0.0, 0.0, 0.0, 0.0};
    v4di vec_ll = {0, 0, 0, 0};
    v8si vec_int = {0, 0, 0, 0, 0, 0, 0, 0};
    
    long long accumulator = 0;
    double fp_accumulator = 0.0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = (seed * outer) & 0xFF;
        
        /* Inner if condition that depends on loop-invariant */
        if (invariant > threshold) {
            /* COMPLEX REGISTER-PRESSURE EXPRESSION */
            /* This block should create many temporary values */
            
            /* 1. Vector operations with shuffles */
            v4df temp_vec1 = {arr_dbl[invariant & 63], 
                             arr_dbl[(invariant + 1) & 63],
                             arr_dbl[(invariant + 2) & 63],
                             arr_dbl[(invariant + 3) & 63]};
            
            v4df temp_vec2 = {arr_dbl[(invariant + 4) & 63],
                             arr_dbl[(invariant + 5) & 63],
                             arr_dbl[(invariant + 6) & 63],
                             arr_dbl[(invariant + 7) & 63]};
            
            /* Shuffle operation - often creates virtual registers */
            v4df shuffled = __builtin_shuffle(temp_vec1, temp_vec2, 
                (v4di){1, 3, 0, 2});
            
            /* 2. Mixed integer/floating-point computations */
            for (int inner = 0; inner < 8; inner++) {
                /* Volatile barrier inside computation */
                if (get_barrier()) {
                    /* Complex expression with many intermediates */
                    int idx1 = (invariant + inner) & 63;
                    int idx2 = (invariant * inner) & 63;
                    int idx3 = (invariant ^ inner) & 63;
                    
                    /* Mixed-mode computations */
                    double dbl1 = arr_dbl[idx1] * 1.5;
                    double dbl2 = arr_dbl[idx2] / 2.0;
                    long long ll1 = arr_ll[idx1] + (long long)(dbl1 * 100.0);
                    long long ll2 = arr_ll[idx2] - (long long)(dbl2 * 50.0);
                    
                    /* Integer operations */
                    int int1 = arr_int[idx3] * 3;
                    int int2 = arr_int[idx1] + arr_int[idx2];
                    
                    /* More vector operations */
                    v4df vec_op1 = temp_vec1 * shuffled;
                    v4df vec_op2 = temp_vec2 + shuffled;
                    
                    /* Extract elements - creates more register pressure */
                    double extracted[4];
                    extracted[0] = vec_op1[0];
                    extracted[1] = vec_op1[1];
                    extracted[2] = vec_op2[2];
                    extracted[3] = vec_op2[3];
                    
                    /* Accumulate results with type conversions */
                    accumulator += ll1 + ll2 + (long long)(extracted[0] + extracted[1]);
                    fp_accumulator += dbl1 + dbl2 + extracted[2] + extracted[3];
                    
                    /* Integer vector operations */
                    v8si vec_int1 = {int1, int2, idx1, idx2, idx3, inner, outer, invariant};
                    v8si vec_int2 = vec_int + vec_int1;
                    vec_int = vec_int2;
                    
                    /* Another shuffle on integer vectors */
                    v8si shuffled_int = __builtin_shuffle(vec_int, vec_int1,
                        (v8si){2, 3, 0, 1, 6, 7, 4, 5});
                    
                    /* Use shuffled result */
                    for (int k = 0; k < 4; k++) {
                        accumulator += shuffled_int[k];
                    }
                }
            }
            
            /* 3. Additional computations outside inner loop */
            /* More register pressure with different modes */
            long long ll_temp[4];
            double dbl_temp[4];
            
            for (int i = 0; i < 4; i++) {
                int idx = (invariant + i * 7) & 63;
                ll_temp[i] = arr_ll[idx] * 2 - arr_ll[(idx + 1) & 63];
                dbl_temp[i] = arr_dbl[idx] * 3.14159 - arr_dbl[(idx + 2) & 63];
                
                /* Cross-type computations */
                accumulator += (long long)(dbl_temp[i] * 1000.0);
                fp_accumulator += (double)ll_temp[i] / 1000.0;
            }
            
            /* 4. Final complex expression with barrier */
            if (get_barrier()) {
                /* Expression with many intermediate values */
                double final_dbl = (fp_accumulator * 0.01) + 
                                  (accumulator % 1000) * 0.001;
                long long final_ll = (long long)(final_dbl * 1000000.0) +
                                    accumulator * 17;
                
                /* Use results to prevent dead code elimination */
                arr_dbl[invariant & 63] += final_dbl;
                arr_ll[invariant & 63] ^= final_ll;
            }
        } else {
            /* Alternate path to create control flow complexity */
            /* Still uses registers but different computations */
            for (int i = 0; i < 4; i++) {
                int idx = (invariant + i * 3) & 63;
                accumulator -= arr_ll[idx];
                fp_accumulator -= arr_dbl[idx];
            }
        }
        
        /* Loop-carried dependency */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final computation mixing types */
    long long result = accumulator + (long long)fp_accumulator;
    
    /* Use array elements to prevent optimization */
    for (int i = 0; i < 8; i++) {
        result += arr_ll[i] + (long long)arr_dbl[i];
    }
    
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i * 17, 128 + (i % 64));
        
        /* Add some variation */
        if (i % 3 == 0) {
            total += test_remat(10, i * 23, 64);
        }
        
        if (i % 7 == 0) {
            total -= test_remat(5, i * 47, 192);
        }
    }
    
    /* Print deterministic result */
    printf("Result: %lld\n", total);
    
    /* Additional check to ensure code isn't dead */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
