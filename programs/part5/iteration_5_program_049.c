/* Compile with: gcc -O3 -fno-inline -fdump-rtl-all test.c -o test */
/* Additional flags to try: -fno-schedule-insns -fno-schedule-insns2 */
/* Or: gcc -O2 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
static volatile int barrier __attribute__((unused));

/* Function to prevent optimization */
static int use_result(int64_t val) {
    barrier = 1;
    return (val & 0xFF) != 0;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
int64_t test_remat(int start, int iterations, int threshold) {
    /* Large vectors to create virtual registers */
    typedef int64_t v4i64 __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    int64_t arr_int[16];
    
    /* Initialize arrays with varying values */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + start) * 1.5;
        arr_int[i] = (i * start) ^ 0x12345678;
    }
    
    v4df vec_dbl = {0};
    v4i64 vec_int = {0};
    
    int64_t accumulator = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = (outer * start) & 0xF;
        
        /* Inner if condition */
        if (invariant > threshold) {
            /* COMPLEX REGISTER-PRESSURE INDUCING EXPRESSION */
            /* Mix of integer and floating-point operations */
            
            /* Load vectors with non-constant indices */
            int idx1 = (invariant + 1) & 0xF;
            int idx2 = (invariant + 3) & 0xF;
            int idx3 = (invariant + 5) & 0xF;
            int idx4 = (invariant + 7) & 0xF;
            
            /* Create vector from array elements */
            vec_dbl = (v4df){arr_dbl[idx1], arr_dbl[idx2], 
                            arr_dbl[idx3], arr_dbl[idx4]};
            
            /* Vector operations that create many temporaries */
            v4df temp1 = vec_dbl * 2.5;
            v4df temp2 = temp1 + (v4df){1.0, 2.0, 3.0, 4.0};
            
            /* Convert to integer vector */
            vec_int = (v4i64)temp2;
            
            /* Shuffle operations that create virtual registers */
            v4i64 shuffled = __builtin_shuffle(vec_int, 
                (v4i64){3, 2, 1, 0});
            
            /* More arithmetic mixing types */
            v4i64 temp3 = shuffled + (v4i64){1000, 2000, 3000, 4000};
            v4i64 temp4 = temp3 * (v4i64){2, 3, 4, 5};
            
            /* Horizontal reduction */
            int64_t sum = 0;
            for (int i = 0; i < 4; i++) {
                sum += temp4[i];
            }
            
            /* More mixed operations */
            double dbl_sum = (double)sum;
            dbl_sum = dbl_sum * arr_dbl[invariant & 0xF];
            
            /* Integer operations */
            int64_t int_result = (int64_t)dbl_sum;
            int_result = int_result ^ arr_int[invariant & 0xF];
            int_result = int_result * (outer + 1);
            
            /* Use volatile barrier in the middle of computation */
            barrier = outer;
            
            /* More computations after barrier */
            int_result = int_result + (int_result >> 4);
            int_result = int_result * 0x9e3779b97f4a7c15ULL;
            
            /* Accumulate result */
            accumulator += int_result;
            
            /* Use the result to prevent dead code elimination */
            if (use_result(int_result)) {
                accumulator ^= 0x5555555555555555ULL;
            }
        } else {
            /* Alternative path with different computations */
            double alt_val = arr_dbl[invariant & 0xF];
            int64_t alt_int = (int64_t)(alt_val * 1000.0);
            accumulator -= alt_int;
        }
        
        /* Modify arrays to prevent optimization */
        arr_dbl[outer & 0xF] += 0.1;
        arr_int[outer & 0xF] ^= accumulator & 0xFF;
    }
    
    return accumulator;
}

int main(void) {
    int64_t total = 0;
    
    /* Call the function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50, i % 8);
        
        /* Mix in some different parameter patterns */
        if (i % 3 == 0) {
            total ^= test_remat(i * 2, 30, (i + 1) % 8);
        }
        
        if (i % 7 == 0) {
            total -= test_remat(i / 2, 40, (i + 3) % 8);
        }
    }
    
    /* Print deterministic result */
    printf("Result: %lld\n", (long long)total);
    
    return 0;
}
