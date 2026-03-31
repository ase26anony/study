/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter & 0xFF;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, int use_vector) {
    /* Large arrays to increase register pressure */
    double arr_double[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with non-constant indices */
    for (int i = 0; i < 32; i++) {
        arr_idx[i] = (seed + i * 3) % 32;
        arr_double[i] = (seed + i) * 1.5;
        arr_int[i] = (seed + i) * 7LL;
    }
    
    /* Vector types for virtual register creation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Complex accumulator variables mixing types */
    double double_acc = 0.0;
    long long int_acc = 0;
    v4si vec_acc_int = {0, 0, 0, 0};
    v2df vec_acc_double = {0.0, 0.0};
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for conditional */
        int invariant = seed * outer;
        
        /* Inner if with complex expression */
        if ((invariant % 4) == 0) {
            /* Register pressure inducing expression with volatile barrier */
            int volatile_val = get_volatile_value();
            
            /* Complex mixed-type computation */
            for (int inner = 0; inner < 16; inner++) {
                /* Non-constant array indexing */
                int idx1 = arr_idx[inner];
                int idx2 = arr_idx[31 - inner];
                
                /* Mixed integer/float operations */
                double temp_double = arr_double[idx1] * 2.5 + 
                                    (double)volatile_val / 128.0;
                
                long long temp_int = arr_int[idx2] * 3LL + 
                                    (long long)(temp_double * 100.0);
                
                /* Vector operations that create virtual registers */
                if (use_vector) {
                    v4si vec1 = {inner, idx1, idx2, volatile_val};
                    v4si vec2 = {temp_int & 0xFF, (temp_int >> 8) & 0xFF,
                                 (temp_int >> 16) & 0xFF, (temp_int >> 24) & 0xFF};
                    
                    /* Shuffle operation often creates virtual registers */
                    v4si vec_shuffled = __builtin_shuffle(vec1, vec2, 
                        (v4si){1, 3, 0, 2});
                    
                    vec_acc_int += vec_shuffled;
                    
                    /* Double vector operations */
                    v2df vd1 = {temp_double, arr_double[idx1]};
                    v2df vd2 = {arr_double[idx2], temp_double * 0.5};
                    vec_acc_double += vd1 * vd2;
                }
                
                /* Complex expression with many intermediate values */
                double_acc += temp_double * 
                             (double)(temp_int % 256) / 64.0 +
                             (double)(volatile_val & 0xF) * 0.25;
                
                int_acc += (temp_int >> 4) * 
                          (inner + 1) * 
                          (volatile_val % 8 + 1);
                
                /* Additional computation to increase register pressure */
                arr_double[idx1] = temp_double * 0.9 + 
                                  (double)(inner % 4) * 0.1;
                arr_int[idx2] = temp_int / 2 + 
                               (long long)(double_acc * 0.01);
            }
            
            /* Another volatile barrier in the middle */
            volatile_val = get_volatile_value();
            
            /* More mixed operations */
            for (int i = 0; i < 8; i++) {
                double temp = arr_double[i] * arr_double[31 - i];
                long long temp_ll = (long long)(temp * 1000.0);
                
                /* Complex conditional expression */
                double_acc += (i % 2) ? temp : -temp;
                int_acc += (volatile_val & (1 << (i % 8))) ? 
                          temp_ll : -temp_ll;
            }
        } else {
            /* Different path to create control flow complexity */
            int volatile_val = get_volatile_value();
            
            for (int i = 0; i < 8; i++) {
                int idx = (invariant + i) % 32;
                arr_double[idx] += (double)volatile_val * 0.01;
                arr_int[idx] += volatile_val * 3LL;
            }
        }
        
        /* Periodic reshuffling of indices */
        if (outer % 3 == 0) {
            for (int i = 0; i < 32; i++) {
                arr_idx[i] = (arr_idx[i] * 13 + 7) % 32;
            }
        }
    }
    
    /* Final reduction */
    long long result = int_acc + (long long)double_acc;
    
    /* Add vector results if used */
    if (use_vector) {
        for (int i = 0; i < 4; i++) {
            result += vec_acc_int[i];
        }
        result += (long long)(vec_acc_double[0] + vec_acc_double[1]);
    }
    
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to explore different optimization paths */
    for (int i = 0; i < 10; i++) {
        total += test_remat(5 + i, i * 17, i % 2);
        total += test_remat(3 + i, i * 23 + 1, (i + 1) % 2);
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
