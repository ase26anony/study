/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int volatile_counter = 0;
static int get_volatile(void) {
    return volatile_counter++;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, int threshold) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (seed + i) * 1.5;
        arr_ll[i] = (seed + i) * 3LL;
        arr_int[i] = seed * i;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = get_volatile() % 100;
    
    /* Accumulators of different types */
    double dbl_acc = 0.0;
    long long ll_acc = 0;
    int int_acc = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Inner loop with complex control flow */
        for (int inner = 0; inner < 32; inner++) {
            /* Condition depending on loop-invariant */
            if ((loop_invariant + inner) > threshold) {
                /* COMPLEX REGISTER-PRESSURE EXPRESSION */
                /* This should create many temporary values */
                
                /* Vector operations creating virtual registers */
                v4si vec_a = {arr_int[inner], arr_int[(inner+1)&31], 
                              arr_int[(inner+2)&31], arr_int[(inner+3)&31]};
                v4si vec_b = {inner, outer, seed, threshold};
                
                /* Shuffle operation - often creates virtual registers */
                v4si vec_shuffled = __builtin_shuffle(vec_a, vec_b, 
                    (v4si){1, 3, 0, 2});
                
                /* Mixed integer/floating operations */
                double temp1 = arr_dbl[inner] * 2.5;
                long long temp2 = arr_ll[inner] + (inner * outer);
                
                /* Volatile barrier in the middle of computation */
                int volatile_barrier = get_volatile();
                
                /* More mixed operations after barrier */
                double temp3 = temp1 + (volatile_barrier * 0.01);
                long long temp4 = temp2 ^ (volatile_barrier << 3);
                
                /* Vector double operations */
                v2df vd1 = {temp1, temp3};
                v2df vd2 = {arr_dbl[(inner+4)&31], arr_dbl[(inner+5)&31]};
                v2df vd_result = vd1 * vd2 + vd1 - vd2;
                
                /* Vector long long operations */
                v2di vll1 = {temp2, temp4};
                v2di vll2 = {arr_ll[(inner+6)&31], arr_ll[(inner+7)&31]};
                v2di vll_result = vll1 & vll2 | vll1 ^ vll2;
                
                /* Complex expression with many intermediates */
                double complex_dbl = (vd_result[0] * vd_result[1]) 
                                   / (temp1 + 1.0)
                                   + (inner * 0.5)
                                   - (outer * 0.25);
                
                long long complex_ll = (vll_result[0] * vll_result[1])
                                     / ((temp2 & 0xFF) + 1)
                                     + (inner << 2)
                                     - (outer << 1);
                
                /* Array accesses with complex indexing */
                int idx1 = (inner + volatile_barrier) & 31;
                int idx2 = (inner * outer + seed) & 31;
                int idx3 = (threshold + inner - outer) & 31;
                
                /* More mixed operations with array accesses */
                dbl_acc += complex_dbl 
                         + arr_dbl[idx1] 
                         - arr_dbl[idx2] 
                         * arr_dbl[idx3];
                
                ll_acc += complex_ll 
                        ^ arr_ll[idx1] 
                        | arr_ll[idx2] 
                        & arr_ll[idx3];
                
                int_acc += vec_shuffled[0] 
                         * vec_shuffled[1] 
                         - vec_shuffled[2] 
                         + vec_shuffled[3];
                
                /* Another volatile barrier */
                get_volatile();
            } else {
                /* Alternate path to create control flow complexity */
                double simple_dbl = arr_dbl[inner] * 0.5;
                long long simple_ll = arr_ll[inner] >> 2;
                
                dbl_acc -= simple_dbl;
                ll_acc ^= simple_ll;
                int_acc += inner;
            }
        }
        
        /* Modify loop-invariant occasionally */
        if (outer % 7 == 0) {
            loop_invariant = get_volatile() % 50;
        }
        
        /* Cross-iteration dependencies */
        arr_dbl[outer & 31] = dbl_acc * 0.01;
        arr_ll[outer & 31] = ll_acc ^ int_acc;
        arr_int[outer & 31] = int_acc + outer;
    }
    
    /* Final mixed computation */
    long long result = (long long)dbl_acc 
                     + ll_acc 
                     + (int_acc * 1000LL);
    
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 100; i++) {
        total += test_remat(5, i * 17, 25 + (i % 10));
        
        /* Add some variation */
        if (i % 3 == 0) {
            total ^= test_remat(2, i * 23, 15 + (i % 5));
        }
        
        if (i % 7 == 0) {
            total -= test_remat(3, i * 11, 30 + (i % 7));
        }
    }
    
    /* Print deterministic result */
    printf("Result checksum: %lld\n", total);
    
    return 0;
}
