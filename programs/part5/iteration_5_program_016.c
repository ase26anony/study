/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int offset, int seed) {
    /* Large vectors to create virtual registers */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    v4si vec_int[8];
    v2df vec_dbl[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (i + seed) * 1.5;
        arr_ll[i] = (i + seed) * 3LL;
    }
    for (int i = 0; i < 8; i++) {
        vec_int[i] = (v4si){i*4, i*4+1, i*4+2, i*4+3};
        vec_dbl[i] = (v2df){i*2.0, i*2.0 + 1.0};
    }
    
    /* Mix integer and floating-point accumulators */
    double fp_acc = 0.0;
    long long int_acc = 0;
    v2df vec_acc = {0.0, 0.0};
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile_value() % 16;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 32; inner++) {
            /* Complex condition creating multiple basic blocks */
            if ((inner + outer + offset) % 8 == invariant % 4) {
                /* REGISTER PRESSURE BLOCK - complex expression with many temps */
                
                /* 1. Vector operations creating virtual registers */
                v4si v1 = vec_int[inner % 8];
                v4si v2 = vec_int[(inner + 1) % 8];
                v4si v3 = v1 + v2;
                v4si v4 = v3 * (v4si){2, 3, 4, 5};
                
                /* Shuffle operation - often creates virtual registers */
                v4si v5 = __builtin_shuffle(v3, v4, 
                    (v4si){0, 2, 1, 3});
                
                /* 2. Mixed integer/float computations */
                double d1 = arr_dbl[inner];
                double d2 = arr_dbl[(inner + offset) % 32];
                double d3 = d1 * d2 + (double)inner;
                
                /* Volatile barrier in the middle of computation */
                int barrier = get_volatile_value();
                
                /* More computations after barrier */
                long long ll1 = arr_ll[inner];
                long long ll2 = arr_ll[(inner + barrier) % 32];
                long long ll3 = ll1 * ll2 / (inner + 1);
                
                /* Vector double operations */
                v2df vd1 = vec_dbl[inner % 8];
                v2df vd2 = vec_dbl[(inner + 2) % 8];
                v2df vd3 = vd1 * vd2 + (v2df){d3, d3 * 0.5};
                
                /* 3. Complex expression with many intermediate results */
                double temp1 = d1 + d2;
                double temp2 = temp1 * (double)ll3;
                double temp3 = temp2 / (inner + 2.0);
                double temp4 = temp3 + vd3[0] + vd3[1];
                
                long long temp5 = ll1 + ll2;
                long long temp6 = temp5 * (long long)inner;
                long long temp7 = temp6 / (barrier + 1);
                long long temp8 = temp7 + (long long)temp4;
                
                /* 4. More vector operations */
                v4si v6 = v4 + v5;
                v4si v7 = v6 * (v4si){inner, barrier, offset, seed};
                int vsum = v7[0] + v7[1] + v7[2] + v7[3];
                
                /* Update accumulators with all the temps */
                fp_acc += temp4 + (double)vsum;
                int_acc += temp8 + vsum;
                vec_acc += vd3;
                
                /* Array update with non-constant index */
                arr_dbl[(inner + vsum) % 32] = temp4;
                arr_ll[(inner + barrier) % 32] = temp8;
            } else {
                /* Alternate path to create control flow complexity */
                double simple = arr_dbl[inner] * 0.5;
                arr_dbl[inner] = simple;
                fp_acc += simple;
            }
        }
        
        /* Cross-iteration dependency */
        offset = (offset + outer) % 32;
    }
    
    /* Final reduction */
    double final_fp = fp_acc + vec_acc[0] + vec_acc[1];
    long long final_int = int_acc + (long long)final_fp;
    
    return final_int;
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(5, i % 16, i * 3);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            barrier_counter += i;
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
