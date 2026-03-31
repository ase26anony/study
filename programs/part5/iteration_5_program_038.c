#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure complex expressions aren't simplified */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, volatile int* barrier) {
    /* Use volatile to create code motion barriers */
    volatile int v = *barrier;
    
    /* Large arrays to create register pressure */
    double arr_d[32];
    long long arr_ll[32];
    int arr_i[32];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 32; i++) {
        arr_d[i] = (double)(start + i) * 1.5;
        arr_ll[i] = (long long)(start + i) * 3;
        arr_i[i] = start + i + v;
    }
    
    /* Complex mixed-type computation with many intermediates */
    double sum_d = 0.0;
    long long sum_ll = 0;
    int sum_i = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = (outer * start) & 0xF;
        
        /* Inner if with complex expression */
        if (invariant > 5) {
            /* Register pressure inducing expression with vector operations */
            typedef double v4df __attribute__((vector_size(32)));
            typedef long long v4di __attribute__((vector_size(32)));
            
            /* Load vectors with non-constant indexing */
            v4df vec_d = { 
                arr_d[(invariant + 0) & 31],
                arr_d[(invariant + 1) & 31],
                arr_d[(invariant + 2) & 31],
                arr_d[(invariant + 3) & 31]
            };
            
            v4di vec_ll = {
                arr_ll[(invariant + 4) & 31],
                arr_ll[(invariant + 5) & 31],
                arr_ll[(invariant + 6) & 31],
                arr_ll[(invariant + 7) & 31]
            };
            
            /* Complex vector operations creating many virtual registers */
            v4df temp1 = vec_d * vec_d;
            v4df temp2 = temp1 + (v4df){1.0, 2.0, 3.0, 4.0};
            
            /* Use __builtin_shuffle to create virtual registers */
            v4df shuffled = __builtin_shuffle(temp1, temp2, 
                (v4di){0, 4, 1, 5});
            
            /* Mix integer and floating point operations */
            v4di conv_ll = (v4di)shuffled;
            v4di temp3 = vec_ll + conv_ll;
            v4di temp4 = temp3 * (v4di){2, 3, 4, 5};
            
            /* Another code motion barrier */
            *barrier = outer;
            
            /* More complex operations with type mixing */
            double d1 = shuffled[0] + shuffled[1];
            double d2 = shuffled[2] * shuffled[3];
            long long ll1 = temp4[0] - temp4[1];
            long long ll2 = temp4[2] + temp4[3];
            
            /* Use all results to prevent dead code elimination */
            sum_d += d1 * d2;
            sum_ll += ll1 ^ ll2;
            sum_i += (int)(d1 + d2) + (int)(ll1 & 0xFFFF);
        } else {
            /* Alternative path with different computations */
            for (int j = 0; j < 8; j++) {
                /* More register pressure with mixed operations */
                double x = arr_d[(invariant + j) & 31];
                long long y = arr_ll[(invariant + j + 1) & 31];
                int z = arr_i[(invariant + j + 2) & 31];
                
                /* Complex expression chain */
                double t1 = x * x + (double)z;
                long long t2 = y * y + (long long)z;
                int t3 = z * z + (int)(x * 100.0);
                
                sum_d += t1;
                sum_ll += t2;
                sum_i += t3;
                
                /* Another barrier */
                if (j == 3) *barrier = t3;
            }
        }
        
        /* Modify arrays to prevent optimization */
        arr_d[outer & 31] += sum_d * 0.01;
        arr_ll[outer & 31] ^= sum_ll;
        arr_i[outer & 31] += sum_i;
    }
    
    /* Final mixed computation */
    return (long long)sum_d + sum_ll + sum_i;
}

int main(void) {
    volatile int barrier = 0;
    long long total = 0;
    
    /* Call multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50, &barrier);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            barrier = i;
        }
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
