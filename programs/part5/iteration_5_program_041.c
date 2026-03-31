/* Compile with: gcc -O2 -fno-inline -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 test.c */
/* Alternative: gcc -O3 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barriers */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable static function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double scale_factor) {
    /* Large vectors to create virtual registers */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[64];
    long long arr_ll[64];
    int arr_int[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_dbl[i] = (i * 1.5) / scale_factor;
        arr_ll[i] = i * 3LL;
        arr_int[i] = i * 2;
    }
    
    /* Complex intermediate values that need many registers */
    long long result = start_val;
    double fp_acc = 0.0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = volatile_barrier(outer % 16);
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 32; inner++) {
            /* Complex condition that depends on invariant */
            if ((invariant + inner) % 8 < 4) {
                /* REGISTER PRESSURE BLOCK - designed to force spills/rematerialization */
                
                /* 1. Vector operations creating virtual registers */
                v8si vec_a = {arr_int[inner], arr_int[inner+1], arr_int[inner+2], 
                              arr_int[inner+3], arr_int[inner+4], arr_int[inner+5],
                              arr_int[inner+6], arr_int[inner+7]};
                v8si vec_b = {inner, inner+1, inner+2, inner+3, 
                              inner+4, inner+5, inner+6, inner+7};
                
                /* Shuffle operation often creates virtual registers */
                v8si vec_shuffled = __builtin_shuffle(vec_a, vec_b, 
                    (v8si){7, 6, 5, 4, 3, 2, 1, 0});
                
                /* 2. Mixed integer/floating-point computations */
                double d1 = arr_dbl[inner] * scale_factor;
                double d2 = arr_dbl[inner + 1] / (scale_factor + 1.0);
                
                /* Volatile barrier prevents code motion */
                int barrier_val = volatile_barrier(inner);
                
                /* 3. Complex expression with many intermediates */
                long long ll1 = arr_ll[inner] * barrier_val;
                long long ll2 = arr_ll[inner + 1] * (barrier_val + 1);
                long long ll3 = ll1 + ll2;
                long long ll4 = ll3 * (result & 0xFF);
                
                /* 4. More floating point with type mixing */
                fp_acc += d1 * d2 + (double)ll4;
                
                /* 5. Integer arithmetic with array indexing */
                int idx1 = (inner * 3) % 64;
                int idx2 = (inner * 5) % 64;
                int idx3 = (inner * 7) % 64;
                
                /* Complex chain of computations */
                int int1 = arr_int[idx1] * arr_int[idx2];
                int int2 = int1 + arr_int[idx3];
                int int3 = int2 * barrier_val;
                int int4 = int3 - (invariant * 2);
                
                /* 6. Vector reduction */
                long long vec_sum = 0;
                for (int k = 0; k < 8; k++) {
                    vec_sum += vec_shuffled[k];
                }
                
                /* 7. Final accumulation with mixed types */
                result += (ll4 >> 3) + (long long)fp_acc + vec_sum + int4;
                
                /* 8. More floating point to use FP registers */
                double d3 = fp_acc * 0.5;
                double d4 = d3 + arr_dbl[inner % 16];
                fp_acc = d4 * 0.9;
                
                /* 9. Additional shuffle operations */
                v4df vec_dbl1 = {arr_dbl[0], arr_dbl[1], arr_dbl[2], arr_dbl[3]};
                v4df vec_dbl2 = {arr_dbl[4], arr_dbl[5], arr_dbl[6], arr_dbl[7]};
                v4df vec_dbl_shuffled = __builtin_shuffle(vec_dbl1, vec_dbl2,
                    (int[4]){3, 2, 1, 0});
                
                /* Use the shuffled result */
                for (int k = 0; k < 4; k++) {
                    fp_acc += vec_dbl_shuffled[k];
                }
            } else {
                /* Alternate path to create control flow complexity */
                result -= (outer * inner) & 0xFF;
                fp_acc *= 0.99;
            }
        }
        
        /* Cross-iteration dependency */
        result = (result * 1103515245LL + 12345LL) & 0x7FFFFFFF;
    }
    
    return result + (long long)fp_acc;
}

int main(void) {
    long long total = 0;
    
    /* Call test_remat multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(10 + (i % 5), i * 100, 1.0 + (i * 0.01));
        
        /* Add some variation to arguments */
        if (i % 3 == 0) {
            total -= test_remat(5 + (i % 3), i * 50, 2.0 - (i * 0.005));
        }
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
