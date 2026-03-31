/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int volatile_counter = 0;
static int get_volatile(void) {
    volatile_counter++;
    return volatile_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline,noipa))
long long test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (seed + i) * 0.5;
        arr_ll[i] = seed * i;
    }
    
    /* Mix of integer and FP computations */
    double sum_dbl = 0.0;
    long long sum_ll = 0;
    v4df vec_sum = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile() % 4;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 8; inner++) {
            /* Complex condition based on invariant */
            if ((inner + invariant) % 3 == 0) {
                /* REGISTER PRESSURE BLOCK - designed to trigger remat */
                
                /* 1. Vector operations creating virtual registers */
                v4df vec_a = {arr_dbl[inner], arr_dbl[inner+1], 
                              arr_dbl[inner+2], arr_dbl[inner+3]};
                v4df vec_b = {factor, factor * 2.0, factor * 3.0, factor * 4.0};
                
                /* Shuffle operation - often creates virtual regs */
                v4df vec_shuffled = __builtin_shuffle(vec_a, vec_b, 
                    (v4di){0, 2, 1, 3});
                
                /* 2. Mixed integer/FP operations */
                long long temp_ll = arr_ll[inner] * (inner + 1);
                double temp_dbl = arr_dbl[inner] * factor;
                
                /* Volatile barrier in expression */
                int barrier = get_volatile();
                
                /* Complex expression with many intermediates */
                double expr1 = temp_dbl * barrier;
                double expr2 = expr1 / (temp_ll % 256 + 1);
                double expr3 = expr2 + vec_shuffled[0];
                double expr4 = expr3 - vec_shuffled[1];
                double expr5 = expr4 * vec_shuffled[2];
                double expr6 = expr5 / vec_shuffled[3];
                
                /* More intermediates with different modes */
                long long ll_expr1 = temp_ll >> (barrier % 8);
                long long ll_expr2 = ll_expr1 * (long long)expr6;
                long long ll_expr3 = ll_expr2 + (long long)(expr6 * 1000.0);
                
                /* Use all intermediates to prevent dead code elimination */
                sum_dbl += expr1 + expr2 + expr3 + expr4 + expr5 + expr6;
                sum_ll += ll_expr1 + ll_expr2 + ll_expr3;
                
                /* Vector accumulation */
                vec_sum += vec_shuffled;
                
                /* Array updates with non-constant indices */
                int idx = (inner + barrier) % 16;
                arr_dbl[idx] = expr6;
                arr_ll[idx] = ll_expr3;
            }
            
            /* Additional computation outside if to increase live ranges */
            double extra = arr_dbl[inner] * 1.1;
            sum_dbl += extra;
        }
        
        /* Cross-iteration dependencies */
        factor *= 0.99;
        seed += outer;
    }
    
    /* Final reduction */
    double vec_result = vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    return sum_ll + (long long)(sum_dbl + vec_result);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i * 3, 1.0 + i * 0.01);
        
        /* Prevent loop unrolling from simplifying register pressure */
        if (i % 10 == 0) {
            volatile_counter++;
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Volatile operations: %d\n", volatile_counter);
    
    return 0;
}
