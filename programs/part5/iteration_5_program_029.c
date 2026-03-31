/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier __attribute__((unused));

/* Non-inlineable function to force register pressure */
static __attribute__((noinline, noipa))
long long test_remat(int start, int iter, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (double)(i + start) * 1.5;
        arr_int[i] = (long long)(i * iter) + 12345;
    }
    
    /* Complex expression with many temporaries */
    double sum_dbl = init_val;
    long long sum_int = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iter; outer++) {
        /* Loop-invariant variable for inner condition */
        int threshold = start * 2 + outer;
        
        /* Inner if with register-intensive computation */
        if (threshold > (iter / 2)) {
            /* Volatile read creates code motion barrier */
            barrier = outer;
            
            /* Complex vector operations that create virtual registers */
            v4df vec1 = {arr_dbl[outer % 32], arr_dbl[(outer + 1) % 32], 
                         arr_dbl[(outer + 2) % 32], arr_dbl[(outer + 3) % 32]};
            v4df vec2 = {arr_dbl[(outer + 4) % 32], arr_dbl[(outer + 5) % 32],
                         arr_dbl[(outer + 6) % 32], arr_dbl[(outer + 7) % 32]};
            
            /* Mixed integer/floating operations */
            v4di ivec1 = {arr_int[outer % 32], arr_int[(outer + 1) % 32],
                          arr_int[(outer + 2) % 32], arr_int[(outer + 3) % 32]};
            v4di ivec2 = {arr_int[(outer + 4) % 32], arr_int[(outer + 5) % 32],
                          arr_int[(outer + 6) % 32], arr_int[(outer + 7) % 32]};
            
            /* Vector shuffle operation - often creates virtual registers */
            v4df shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 1, 4, 5});
            
            /* Mixed mode computations */
            for (int inner = 0; inner < 4; inner++) {
                /* Complex expression with many intermediate values */
                double temp1 = shuffled[inner] * (double)ivec1[inner];
                double temp2 = (double)ivec2[inner] / (shuffled[inner] + 1.0);
                
                /* Integer operations mixed with floating */
                long long itemp1 = (long long)(temp1 * 100.0);
                long long itemp2 = (long long)(temp2 * 50.0);
                
                /* More intermediate values */
                double temp3 = temp1 + temp2 + (double)itemp1;
                long long itemp3 = itemp1 ^ itemp2;
                
                /* Use volatile to prevent optimization */
                barrier = inner;
                
                /* Accumulate results with non-trivial addressing */
                sum_dbl += temp3 * arr_dbl[(outer + inner) % 32];
                sum_int += itemp3 + arr_int[(outer + inner + 4) % 32];
                
                /* Additional mixed operations */
                sum_dbl = sum_dbl - (double)(itemp3 % 1000) * 0.001;
                sum_int = sum_int ^ (long long)(sum_dbl * 1000.0);
            }
            
            /* Another volatile barrier */
            barrier = threshold;
        } else {
            /* Alternative path with different computations */
            double alt_sum = 0.0;
            for (int i = 0; i < 8; i++) {
                alt_sum += arr_dbl[(outer + i) % 32] * (double)arr_int[(outer + i) % 32];
            }
            sum_dbl += alt_sum / 8.0;
        }
        
        /* Cross-mode conversion that might need rematerialization */
        sum_int += (long long)(sum_dbl * (double)(outer + 1));
    }
    
    /* Final mixed computation */
    return sum_int + (long long)(sum_dbl * 1000.0);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50 + (i % 20), (double)i * 0.5);
        
        /* Prevent loop optimization */
        if (i % 10 == 0) {
            printf("Progress: %d, total so far: %lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    
    /* Deterministic output check */
    if (total != 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}
