/* Compile with: gcc -O3 -fno-inline -fdump-rtl-all test.c -o test */
/* Or: gcc -O2 -fsanitize=undefined -fno-omit-frame-pointer test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter & 0xFF;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double factor) {
    /* Large arrays to force register pressure */
    double arr_double[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with non-constant indices */
    for (int i = 0; i < 32; i++) {
        arr_idx[i] = (i * start_val) & 31;
        arr_double[i] = (double)(i + start_val) * 0.5;
        arr_int[i] = (long long)(i * start_val) * 3LL;
    }
    
    /* Vector types for virtual register creation */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Complex accumulator variables mixing types */
    double double_acc = 0.0;
    long long int_acc = 0LL;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop with multiple basic blocks */
    for (int iter = 0; iter < iterations; iter++) {
        /* Loop-invariant condition */
        int invariant_cond = (iter & 0x100) != 0;
        
        if (invariant_cond) {
            /* Inner block with high register pressure */
            
            /* Volatile call creates rematerialization barrier */
            int volatile_val = get_volatile_value();
            
            /* Complex expression with many temporaries */
            for (int i = 0; i < 16; i++) {
                /* Non-constant array indexing */
                int idx1 = arr_idx[i] ^ volatile_val;
                int idx2 = arr_idx[i + 16] ^ (volatile_val >> 1);
                
                /* Mixed integer/float operations */
                double temp1 = arr_double[idx1 & 31] * factor;
                long long temp2 = arr_int[idx2 & 31] + (long long)iter;
                
                /* Vector operations that create virtual registers */
                v4df vec1 = {temp1, temp1 * 2.0, temp1 * 3.0, temp1 * 4.0};
                v4di vec2 = {temp2, temp2 * 2, temp2 * 3, temp2 * 4};
                
                /* __builtin_shuffle creates virtual registers */
                v4df shuffled = __builtin_shuffle(vec1, 
                    (v4di){0, 2, 1, 3});
                
                /* More mixed operations */
                double_acc += shuffled[0] + shuffled[1] + 
                             shuffled[2] + shuffled[3];
                
                /* Integer operations on vector results */
                long long vec_sum = vec2[0] + vec2[1] + 
                                   vec2[2] + vec2[3];
                int_acc += vec_sum * (i + 1);
                
                /* Cross-type conversions increase register pressure */
                arr_double[i] = (double)vec_sum * 0.01;
                arr_int[i + 16] = (long long)(shuffled[0] * 100.0);
                
                /* Another volatile barrier in the middle */
                if ((i & 3) == 0) {
                    volatile_val = get_volatile_value();
                }
            }
            
            /* Additional control flow within the if block */
            for (int j = 0; j < 8; j++) {
                /* More register-intensive computations */
                v4df vec_a = {double_acc, double_acc * 0.5, 
                             double_acc * 0.25, double_acc * 0.125};
                v4df vec_b = {factor, factor * 2.0, 
                             factor * 3.0, factor * 4.0};
                
                /* Vector multiply-add */
                v4df vec_c = vec_a + vec_b * vec_a;
                
                /* Horizontal reduction */
                double_acc += vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
                
                /* Integer parallel computation */
                v4di vec_d = {int_acc, int_acc >> 1, 
                             int_acc >> 2, int_acc >> 3};
                v4di vec_e = {j, j * 2, j * 3, j * 4};
                v4di vec_f = vec_d + vec_e * vec_d;
                
                int_acc += vec_f[0] + vec_f[1] + vec_f[2] + vec_f[3];
            }
        } else {
            /* Alternative path with different computations */
            for (int i = 0; i < 8; i++) {
                int idx = (i * iter) & 31;
                double_acc += arr_double[idx] * 0.75;
                int_acc += arr_int[idx] >> 2;
            }
        }
        
        /* Loop-carried dependency with mixing */
        factor = factor * 0.99 + double_acc * 0.0001;
    }
    
    /* Final mixing of results */
    return int_acc + (long long)(double_acc * 1000.0);
}

int main(void) {
    long long total = 0;
    
    /* Call with different parameters to avoid constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50 + (i & 15), i * 3, 1.0 + i * 0.01);
        
        /* Prevent loop unrolling from simplifying too much */
        if ((i & 7) == 0) {
            barrier_counter += i;
        }
    }
    
    printf("Result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
