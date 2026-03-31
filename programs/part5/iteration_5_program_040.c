/* test-early-remat.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining to ensure complex expressions stay in place */
static __attribute__((noinline,noipa))
long long test_remat(int start, int iter, volatile int *barrier) {
    /* Large vectors to create virtual register pressure */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    int arr_int[16];
    
    /* Initialize arrays with values based on arguments */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (start + i) * 1.5;
        arr_ll[i] = (start + i) * 3LL;
        arr_int[i] = start * i;
    }
    
    /* Complex expression with many intermediate values */
    long long result = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    v4di vec_int_acc = {0, 0, 0, 0};
    
    for (int outer = 0; outer < iter; outer++) {
        /* Loop-invariant variable for control flow */
        int invariant = start * outer;
        
        /* Complex control flow with multiple basic blocks */
        if (invariant % 7 != 0) {  /* Non-trivial condition */
            /* Register pressure inducing expression */
            for (int inner = 0; inner < 8; inner++) {
                /* Volatile barrier prevents code motion */
                int idx = (*barrier + inner) & 0xF;
                
                /* Mix integer and floating-point operations */
                double dbl_val = arr_dbl[idx] * 2.5;
                long long ll_val = arr_ll[idx] * 3LL;
                int int_val = arr_int[idx] * 5;
                
                /* Vector operations create virtual registers */
                v4df vec1 = {dbl_val, dbl_val * 0.5, dbl_val * 0.25, dbl_val * 0.125};
                v4df vec2 = vec1 * (double)int_val;
                
                /* __builtin_shuffle creates virtual registers */
                v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4di){0, 5, 2, 7});
                
                /* More mixed-type operations */
                v4di vec_int1 = (v4di){ll_val, ll_val >> 2, ll_val >> 4, ll_val >> 6};
                v4di vec_int2 = __builtin_shuffle(vec_int1, 
                    (v4di){1, 0, 3, 2});
                
                /* Complex expression with many temporaries */
                double temp1 = vec_shuffled[0] + vec_shuffled[1];
                double temp2 = vec_shuffled[2] * vec_shuffled[3];
                long long temp3 = (long long)(temp1 * 1000.0);
                long long temp4 = vec_int2[0] + vec_int2[1];
                long long temp5 = vec_int2[2] * vec_int2[3];
                
                /* Chain computations to keep values live */
                arr_dbl[idx] = temp1 - temp2;
                arr_ll[idx] = temp3 + temp4 - temp5;
                arr_int[idx] = (int)(temp1 + temp2) * int_val;
                
                /* Accumulate results */
                vec_acc += vec_shuffled;
                vec_int_acc += vec_int2;
                
                /* More operations to increase register pressure */
                for (int k = 0; k < 4; k++) {
                    double d = vec_acc[k];
                    long long l = vec_int_acc[k];
                    result += (long long)(d * 100.0) + (l & 0xFFFF);
                }
            }
        } else {
            /* Alternative path to create control flow complexity */
            for (int inner = 0; inner < 4; inner++) {
                int idx = (outer + inner) & 0xF;
                arr_dbl[idx] = arr_dbl[idx] * 1.1;
                arr_ll[idx] = arr_ll[idx] + 1;
                result += (long long)arr_dbl[idx] + arr_ll[idx];
            }
        }
        
        /* Cross-iteration dependencies */
        for (int i = 0; i < 4; i++) {
            arr_dbl[i] = vec_acc[i] * 0.9;
            arr_ll[15 - i] = vec_int_acc[i] >> 1;
        }
    }
    
    /* Final reduction */
    for (int i = 0; i < 4; i++) {
        result += (long long)vec_acc[i];
        result += vec_int_acc[i];
    }
    
    return result;
}

int main(void) {
    volatile int barrier = 0;
    long long total = 0;
    
    /* Call multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        barrier = i & 0xF;
        total += test_remat(i, 10 + (i % 5), &barrier);
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
