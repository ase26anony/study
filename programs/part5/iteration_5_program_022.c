/* Compile with: gcc -O2 -fno-inline -fdump-rtl-all -fno-schedule-insns test.c */
/* Or: gcc -O3 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier;

/* Non-inlineable function to force register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int offset, double seed) 
{
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = seed + i * 0.5;
        arr_ll[i] = (long long)(seed * 1000) + i * 3;
    }
    
    /* Complex mixed-type computations */
    v4df vec_sum = {0.0, 0.0, 0.0, 0.0};
    v4di int_sum = {0, 0, 0, 0};
    
    double fp_acc = 0.0;
    long long int_acc = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int threshold = (outer * 17) % 13;
        
        /* Inner if with register-intensive computations */
        if (outer % 3 == threshold % 2) {
            /* Complex expression with many temporaries */
            double temp1 = arr_dbl[(outer + offset) & 0xF];
            double temp2 = arr_dbl[(outer + offset + 1) & 0xF];
            
            /* Code motion barrier */
            barrier = outer;
            
            /* Mixed integer/floating operations */
            long long temp3 = arr_ll[(outer + offset) & 0xF];
            long long temp4 = arr_ll[(outer + offset + 1) & 0xF];
            
            /* Vector operations that create virtual registers */
            v4df vec1 = {temp1, temp2, temp1 * 0.7, temp2 * 1.3};
            v4df vec2 = {temp2, temp1, temp2 * 0.3, temp1 * 1.7};
            
            /* __builtin_shuffle to force virtual register creation */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 1, 6, 7});
            
            /* More barriers */
            barrier = (int)temp1;
            
            /* Complex chain of operations */
            for (int inner = 0; inner < 4; inner++) {
                /* Register pressure: many intermediate values */
                double a = vec_shuffled[inner] + fp_acc;
                double b = a * 1.61803398875; /* Golden ratio */
                double c = b - vec_shuffled[(inner + 1) & 3];
                double d = c * c;
                
                /* Integer operations in parallel */
                long long x = temp3 + int_acc;
                long long y = x ^ temp4;
                long long z = y * 1103515245;
                
                /* Mix results */
                fp_acc += d + (double)(z & 0xFFF);
                int_acc += z + (long long)d;
                
                /* Another barrier */
                barrier = inner;
            }
            
            /* More vector operations */
            v4di int_vec1 = {temp3, temp4, temp3 >> 2, temp4 << 1};
            v4di int_vec2 = {temp4, temp3, temp4 >> 1, temp3 << 2};
            
            v4di shuffled_int = __builtin_shuffle(int_vec1, int_vec2,
                (v4di){2, 3, 4, 5});
            
            /* Accumulate to vectors */
            for (int i = 0; i < 4; i++) {
                vec_sum[i] += vec_shuffled[i] * shuffled_int[i];
                int_sum[i] ^= shuffled_int[i] + (long long)vec_shuffled[i];
            }
        } else {
            /* Alternate path with different computations */
            double alt_temp = arr_dbl[outer & 0xF] * 3.14159;
            long long alt_int = arr_ll[outer & 0xF] * 271828182;
            
            /* Keep values live across blocks */
            fp_acc += alt_temp;
            int_acc ^= alt_int;
            
            barrier = (int)alt_temp;
        }
        
        /* Cross-block value usage */
        if (outer % 7 == 0) {
            /* Use values computed in different blocks */
            double check = fp_acc * 0.01;
            long long check_int = int_acc / 100;
            
            arr_dbl[outer & 0xF] += check;
            arr_ll[outer & 0xF] += check_int;
        }
    }
    
    /* Final reduction */
    double final_fp = 0.0;
    long long final_int = 0;
    
    for (int i = 0; i < 4; i++) {
        final_fp += vec_sum[i];
        final_int += int_sum[i];
    }
    
    /* Return mixed result */
    return (long long)(final_fp * 1000.0) + final_int + (long long)fp_acc + int_acc;
}

int main(void) 
{
    long long total = 0;
    
    /* Call with different arguments to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 10), i % 8, (double)i * 0.12345);
        
        /* Prevent loop optimization */
        barrier = i;
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
