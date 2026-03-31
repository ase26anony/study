/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x + (rand() % 3);
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val * i + seed;
        arr_ll[i] = (long long)(init_val * 100) + i * seed;
    }
    
    /* Loop-invariant variable for control flow */
    int invariant = seed % 7;
    if (invariant == 0) invariant = 1;
    
    /* Accumulators mixing types */
    double dbl_acc = 0.0;
    long long ll_acc = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop creating register pressure */
    for (int iter = 0; iter < iterations; iter++) {
        /* Inner if with complex expression */
        if ((iter % invariant) == 0) {
            /* Complex expression with many temporaries */
            double temp1 = arr_dbl[iter & 0xF] * 3.14159;
            double temp2 = arr_dbl[(iter + 1) & 0xF] * 2.71828;
            
            /* Volatile barrier prevents code motion */
            int barrier = volatile_barrier(iter);
            
            /* Mix integer and FP operations */
            long long temp3 = arr_ll[iter & 0xF] + barrier;
            double temp4 = temp1 / (temp2 + 1.0);
            
            /* Vector operations creating virtual registers */
            v4df vec1 = {temp1, temp2, temp4, temp1 + temp2};
            v4df vec2 = {temp2, temp4, temp1, temp2 - temp1};
            
            /* Shuffle operation often uses virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 2, 1, 3});
            
            /* More mixed operations */
            double temp5 = vec_shuffled[0] + vec_shuffled[1];
            long long temp6 = (long long)(temp5 * 1000.0) + temp3;
            
            /* Complex control flow within the if */
            for (int j = 0; j < 4; j++) {
                double vec_elem = vec_shuffled[j];
                if (vec_elem > 0.0) {
                    /* Nested expression with many uses */
                    dbl_acc += vec_elem * arr_dbl[(iter + j) & 0xF];
                    ll_acc += (long long)(vec_elem) * arr_ll[(iter + j) & 0xF];
                    
                    /* Another volatile barrier */
                    int barrier2 = volatile_barrier(j);
                    dbl_acc += barrier2 * 0.01;
                }
            }
            
            /* Final accumulation with type mixing */
            dbl_acc += temp4 * temp5;
            ll_acc += temp6;
            
            /* Vector accumulation */
            vec_acc += vec_shuffled;
            
            /* More array accesses with non-constant indices */
            int idx1 = (iter * 3) & 0xF;
            int idx2 = (iter * 5 + 1) & 0xF;
            arr_dbl[idx1] = dbl_acc * 0.9;
            arr_ll[idx2] = ll_acc / (iter + 1);
        } else {
            /* Alternate path still using registers */
            double alt_temp = arr_dbl[iter & 0xF] * arr_dbl[(iter + 3) & 0xF];
            dbl_acc += alt_temp;
            ll_acc += (long long)alt_temp;
        }
        
        /* Loop-carried dependency */
        if (iter > 0) {
            dbl_acc *= 0.999;
            ll_acc -= iter % 100;
        }
    }
    
    /* Final reduction */
    double vec_sum = vec_acc[0] + vec_acc[1] + vec_acc[2] + vec_acc[3];
    return ll_acc + (long long)(dbl_acc + vec_sum);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i % 20), i, 1.0 + i * 0.1);
        
        /* Prevent dead code elimination */
        if (total < 0) {
            printf("Unexpected negative\n");
        }
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
