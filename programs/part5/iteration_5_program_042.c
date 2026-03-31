/* Test program to trigger early rematerialization in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + seed) * factor;
        arr_ll[i] = i * seed;
    }
    
    /* Mix of integer and FP operations */
    double fp_acc = 0.0;
    long long int_acc = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = volatile_barrier(seed + outer) % 8;
        
        /* Inner if with complex expression */
        if (invariant > 2) {
            /* Complex expression creating register pressure */
            v4df vec1 = {arr_dbl[invariant], arr_dbl[invariant + 1], 
                         arr_dbl[invariant + 2], arr_dbl[invariant + 3]};
            v4df vec2 = {arr_dbl[invariant + 4], arr_dbl[invariant + 5],
                         arr_dbl[invariant + 6], arr_dbl[invariant + 7]};
            
            /* Vector operations creating many temporaries */
            v4df temp1 = vec1 * vec2;
            v4df temp2 = vec1 + vec2;
            v4df temp3 = temp1 - temp2;
            
            /* Shuffle operation often creates virtual registers */
            v4df shuffled = __builtin_shuffle(temp1, temp2, 
                (v4di){3, 2, 1, 0});
            
            /* Mix with integer vectors */
            v4di ivec1 = {arr_ll[invariant], arr_ll[invariant + 1],
                          arr_ll[invariant + 2], arr_ll[invariant + 3]};
            v4di ivec2 = {arr_ll[invariant + 4], arr_ll[invariant + 5],
                          arr_ll[invariant + 6], arr_ll[invariant + 7]};
            
            /* More temporaries with different modes */
            v4di itemp1 = ivec1 * ivec2;
            v4di itemp2 = ivec1 + ivec2;
            v4di itemp3 = itemp1 - itemp2;
            
            /* Cross-type conversions increase pressure */
            double d1 = (double)itemp1[0] + (double)itemp2[1];
            double d2 = (double)itemp3[2] + (double)itemp1[3];
            
            /* Complex expression with many intermediate results */
            double complex_expr = 
                (temp1[0] * temp2[1] + temp3[2] * shuffled[3]) *
                (d1 - d2 + arr_dbl[invariant % 8]) /
                (factor + 1.0 + (double)invariant);
            
            /* Use volatile to prevent optimization */
            fp_acc += complex_expr + volatile_barrier(invariant);
            
            /* Integer side with similar complexity */
            long long int_expr =
                (itemp1[0] * itemp2[1] + itemp3[2] * itemp1[3]) /
                (seed + 1 + invariant) *
                (arr_ll[(invariant * 3) % 8] - arr_ll[(invariant * 5) % 8]);
            
            int_acc += int_expr + volatile_barrier(outer);
            
            /* Array updates with non-constant indices */
            arr_dbl[(invariant + outer) % 8] = complex_expr;
            arr_ll[(invariant * 2 + outer) % 8] = int_expr;
        } else {
            /* Alternative path to create control flow complexity */
            double alt_expr = arr_dbl[invariant] * factor;
            long long alt_int = arr_ll[invariant] * seed;
            
            for (int j = 0; j < 4; j++) {
                alt_expr += arr_dbl[(invariant + j) % 8] * 
                           arr_dbl[(invariant + j + 1) % 8];
                alt_int ^= arr_ll[(invariant + j) % 8] * 
                          arr_ll[(invariant + j + 1) % 8];
            }
            
            fp_acc += alt_expr;
            int_acc += alt_int;
        }
        
        /* Loop-carried dependency */
        factor *= 0.999;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final mixing */
    return (long long)(fp_acc * 1000.0) + int_acc;
}

int main(int argc, char **argv) {
    long long total = 0;
    int base_iterations = 100;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += test_remat(base_iterations + i, i * 17, 1.0 + i * 0.1);
        total += test_remat(base_iterations + i * 2, i * 23 + 1, 1.5 + i * 0.05);
        total += test_remat(base_iterations + i * 3, i * 37 + 2, 0.8 + i * 0.02);
    }
    
    /* Deterministic output */
    printf("Result: %lld\n", total);
    
    /* Additional test with more aggressive parameters */
    if (argc > 1) {
        int extra = atoi(argv[1]);
        if (extra > 0) {
            long long extra_result = test_remat(extra * 10, extra, 2.0);
            printf("Extra: %lld\n", extra_result);
        }
    }
    
    return 0;
}
