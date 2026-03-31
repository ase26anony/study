/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>

/* Volatile function to create code motion barriers */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Large vectors to create virtual registers */
    typedef long long v4ll __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[64];
    long long arr_ll[64];
    int arr_idx[64];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_ll[i] = start + i * 3;
        arr_idx[i] = (i * 7) % 64;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = volatile_barrier(start * 2);
    
    /* Accumulators mixing types */
    double dbl_acc = 0.0;
    long long ll_acc = 0;
    v4df vec_acc_dbl = {0.0, 0.0, 0.0, 0.0};
    v4ll vec_acc_ll = {0, 0, 0, 0};
    
    /* Outer loop */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner control flow */
        if (loop_invariant > (outer % 16)) {
            /* Register pressure inducing expression */
            for (int inner = 0; inner < 32; inner++) {
                /* Non-constant array accesses */
                int idx1 = arr_idx[inner] + outer;
                int idx2 = arr_idx[inner + 32] ^ outer;
                
                /* Mixed integer/FP operations with volatile barrier */
                double temp_dbl = arr_dbl[idx1 % 64] * 
                                 volatile_barrier(inner) * 1.5;
                
                long long temp_ll = arr_ll[idx2 % 64] + 
                                   (long long)(temp_dbl * 2.0);
                
                /* Vector operations creating virtual registers */
                v4df vec1 = {temp_dbl, temp_dbl * 0.5, 
                            temp_dbl * 0.25, temp_dbl * 0.125};
                v4df vec2 = {arr_dbl[(idx1 + 1) % 64], 
                            arr_dbl[(idx2 + 2) % 64],
                            arr_dbl[(idx1 + 3) % 64], 
                            arr_dbl[(idx2 + 4) % 64]};
                
                v4ll vec3 = {temp_ll, temp_ll / 2, 
                            temp_ll / 4, temp_ll / 8};
                v4ll vec4 = {arr_ll[(idx1 + 5) % 64], 
                            arr_ll[(idx2 + 6) % 64],
                            arr_ll[(idx1 + 7) % 64], 
                            arr_ll[(idx2 + 8) % 64]};
                
                /* Shuffle operations - often create virtual registers */
                v4df shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4ll){0, 5, 2, 7});
                
                v4ll shuffled_ll = __builtin_shuffle(vec3, vec4,
                    (v4ll){1, 4, 3, 6});
                
                /* Complex expression with many intermediates */
                double expr1 = shuffled[0] * shuffled[1] + 
                              shuffled[2] / (shuffled[3] + 1.0);
                double expr2 = expr1 * expr1 - expr1 + 2.0;
                double expr3 = expr2 * volatile_barrier(inner) - 
                              expr1 * volatile_barrier(inner + 1);
                
                long long expr4 = (long long)expr3 + 
                                 shuffled_ll[0] - shuffled_ll[1];
                long long expr5 = expr4 * expr4 / (expr4 + 1) + 
                                 shuffled_ll[2] ^ shuffled_ll[3];
                
                /* Update accumulators */
                dbl_acc += expr3;
                ll_acc += expr5;
                
                /* More vector mixing */
                vec_acc_dbl += shuffled * vec1 - vec2;
                vec_acc_ll += shuffled_ll | vec3 & vec4;
                
                /* Additional pressure: chain of dependent operations */
                for (int k = 0; k < 4; k++) {
                    double chain = dbl_acc;
                    chain = chain * chain - chain;
                    chain = chain / (volatile_barrier(k) + 1.0);
                    chain = chain + arr_dbl[(idx1 + k) % 64];
                    dbl_acc = chain * 0.99;
                }
            }
        } else {
            /* Alternative path to create control flow complexity */
            for (int inner = 0; inner < 16; inner++) {
                int idx = (inner * outer) % 64;
                dbl_acc -= arr_dbl[idx] * volatile_barrier(inner);
                ll_acc ^= arr_ll[idx] + volatile_barrier(inner * 2);
            }
        }
        
        /* Modify loop-invariant occasionally */
        if (outer % 8 == 0) {
            loop_invariant = volatile_barrier(loop_invariant + outer);
        }
    }
    
    /* Final reduction */
    double final_dbl = dbl_acc;
    for (int i = 0; i < 4; i++) {
        final_dbl += vec_acc_dbl[i];
    }
    
    long long final_ll = ll_acc;
    for (int i = 0; i < 4; i++) {
        final_ll += vec_acc_ll[i];
    }
    
    return (long long)final_dbl + final_ll;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 10 + (i % 5), 1.0 + i * 0.1);
        
        /* Prevent dead code elimination */
        if (i % 23 == 0) {
            printf("Progress: i=%d, total=%lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    return 0;
}
