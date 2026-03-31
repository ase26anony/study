/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure separate function analysis */
static __attribute__((noinline)) 
long long test_remat(int start, int end, double init_val) {
    /* Volatile variable to create code motion barrier */
    volatile int barrier = start;
    
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_int[i] = start + i * 3;
    }
    
    /* Loop-invariant variable for control flow */
    int invariant = (start * end) & 0xF;
    
    /* Accumulators of different types */
    double dbl_acc = 0.0;
    long long int_acc = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop */
    for (int outer = 0; outer < 100; outer++) {
        barrier = outer; /* Update volatile */
        
        /* Inner loop with complex control flow */
        for (int i = start; i < end; i++) {
            /* Complex condition based on invariant */
            if ((i & invariant) == 0) {
                /* REGISTER PRESSURE BLOCK - many temporaries */
                
                /* 1. Vector operations creating virtual registers */
                v4df vec1 = {arr_dbl[i & 31], arr_dbl[(i+1) & 31], 
                            arr_dbl[(i+2) & 31], arr_dbl[(i+3) & 31]};
                v4df vec2 = {arr_dbl[(i+4) & 31], arr_dbl[(i+5) & 31],
                            arr_dbl[(i+6) & 31], arr_dbl[(i+7) & 31]};
                
                /* Shuffle operation often creates virtual registers */
                v4df vec_shuf = __builtin_shuffle(vec1, vec2, 
                    (v4di){0, 2, 4, 6});
                
                /* 2. Mixed integer/floating computations */
                double temp1 = vec_shuf[0] * vec_shuf[1];
                double temp2 = vec_shuf[2] / (vec_shuf[3] + 1.0);
                
                /* Integer computation with barrier */
                long long temp3 = arr_int[i & 31] * barrier;
                long long temp4 = arr_int[(i+1) & 31] + barrier;
                
                /* 3. More temporaries with type mixing */
                double temp5 = (double)temp3 * temp1;
                double temp6 = (double)temp4 * temp2;
                
                /* 4. Complex expression with many intermediates */
                double temp7 = temp5 + temp6;
                double temp8 = temp5 - temp6;
                double temp9 = temp7 * temp8;
                
                /* 5. Integer conversion chain */
                long long temp10 = (long long)temp7;
                long long temp11 = (long long)temp8;
                long long temp12 = temp10 ^ temp11;
                long long temp13 = temp12 * (i + 1);
                
                /* 6. Another vector operation */
                v4di vec_int1 = {temp10, temp11, temp12, temp13};
                v4di vec_int2 = {temp13, temp12, temp11, temp10};
                v4di vec_int3 = vec_int1 + vec_int2;
                
                /* 7. Final accumulation with all temporaries */
                dbl_acc += temp9 + (double)vec_int3[0];
                int_acc += temp13 + vec_int3[1];
                
                /* Update vector accumulator */
                vec_acc += vec_shuf;
                
                /* Use all temporaries to prevent dead code elimination */
                arr_dbl[i & 31] += temp9;
                arr_int[i & 31] ^= temp13;
            } else {
                /* Alternate path with different operations */
                double alt_temp = arr_dbl[i & 31] * 2.0;
                long long alt_int = arr_int[i & 31] << 1;
                dbl_acc -= alt_temp;
                int_acc -= alt_int;
            }
            
            /* Additional computation to increase register pressure */
            if (i & 1) {
                double extra1 = dbl_acc * 0.99;
                long long extra2 = int_acc / 2;
                dbl_acc = extra1;
                int_acc = extra2;
            }
        }
        
        /* Loop-carried dependency with barrier */
        barrier = (int)dbl_acc;
    }
    
    /* Final reduction */
    double final_dbl = dbl_acc + vec_acc[0] + vec_acc[1] + vec_acc[2] + vec_acc[3];
    long long final_int = int_acc;
    
    /* Return mixed type result */
    return (long long)final_dbl + final_int;
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i, i + 20, 1.0 + i * 0.1);
        total += test_remat(i * 2, i * 2 + 15, 2.0 + i * 0.2);
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
