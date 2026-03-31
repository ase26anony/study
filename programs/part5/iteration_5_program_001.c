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
long long test_remat(int iterations, int start_val, double init_val) {
    /* Large arrays to force register pressure */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = init_val + i * 0.5;
        arr_ll[i] = start_val * (long long)i;
        arr_int[i] = start_val + i * get_volatile_value(); /* Barrier */
    }
    
    /* Use GCC vector extensions for virtual register creation */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Complex control flow with multiple basic blocks */
    long long total = 0;
    double acc_dbl = init_val;
    
    for (int outer = 0; outer < iterations; outer++) {
        /* Outer loop with invariant check */
        int invariant = get_volatile_value() % 16;
        
        for (int inner = 0; inner < 32; inner++) {
            /* Inner if condition depending on invariant */
            if ((inner + outer) % (invariant + 1) == 0) {
                /* COMPLEX REGISTER-PRESSURE EXPRESSION */
                /* Mix integer and floating-point operations */
                double temp1 = arr_dbl[inner] * 3.14159;
                long long temp2 = arr_ll[(inner + 1) % 32] * 7;
                int temp3 = arr_int[(inner + 2) % 32] ^ outer;
                
                /* Vector operations that create virtual registers */
                v4df vec1 = {temp1, temp1 * 2.0, temp1 * 3.0, temp1 * 4.0};
                v4df vec2 = {acc_dbl, acc_dbl * 0.5, acc_dbl * 0.25, acc_dbl * 0.125};
                
                /* Use __builtin_shuffle for vector permutation */
                v4df vec3 = __builtin_shuffle(vec1, vec2, 
                    (v4di){0, 1, 4, 5});
                
                /* More mixed operations */
                double vec_sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
                long long int_part = (long long)(vec_sum * 1000.0);
                
                /* Complex expression with many intermediates */
                double complex_expr = 
                    (temp1 * vec_sum) / (temp3 + 1.0) +
                    (arr_dbl[(inner + temp3) % 32] * 2.71828) -
                    (acc_dbl * 0.333333);
                
                /* Integer operations with shifts and masks */
                long long ll_expr = 
                    (temp2 << 3) | 
                    (int_part & 0xFFFF) |
                    ((long long)temp3 << 16);
                
                /* Final accumulation with barrier */
                total += ll_expr + (long long)(complex_expr * get_volatile_value());
                acc_dbl = complex_expr * 0.99;
                
                /* Array update with non-constant indexing */
                arr_dbl[(inner + outer) % 32] = complex_expr;
                arr_ll[(inner * 3) % 32] = ll_expr;
            } else {
                /* Alternative path to create control flow complexity */
                double simple = arr_dbl[inner] * acc_dbl;
                total += (long long)(simple * 100.0);
            }
            
            /* Additional operations to increase register pressure */
            if (inner % 8 == 0) {
                /* More vector operations */
                v4di shuffle_mask = {1, 0, 3, 2};
                v4df vec_temp = {acc_dbl, acc_dbl * 2.0, 0.0, 0.0};
                v4df vec_shuffled = __builtin_shuffle(vec_temp, vec_temp, shuffle_mask);
                
                double shuffled_sum = vec_shuffled[0] + vec_shuffled[1];
                total += (long long)(shuffled_sum * 50.0);
            }
        }
        
        /* Loop-carried dependency with barrier */
        acc_dbl += get_volatile_value() * 0.01;
    }
    
    return total;
}

int main(void) {
    long long final_total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        final_total += test_remat(
            10 + (i % 5),      /* iterations */
            100 + i * 3,       /* start_val */
            1.5 + i * 0.1      /* init_val */
        );
        
        /* Print progress occasionally */
        if (i % 20 == 0) {
            printf("Iteration %d, partial total: %lld\n", i, final_total);
        }
    }
    
    printf("Final checksum: %lld\n", final_total);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
