/* Test program to trigger early rematerialization in GCC */
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
long long test_remat(int iterations, int seed) {
    /* Large vectors to create virtual registers */
    typedef int64_t v4i64 __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (seed + i) * 1.5;
        arr_ll[i] = (seed * 3LL) + i;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = seed % 7;
    
    /* Accumulators of different types */
    v4df vec_acc1 = {0.0, 0.0, 0.0, 0.0};
    v4df vec_acc2 = {0.0, 0.0, 0.0, 0.0};
    v4i64 int_acc1 = {0, 0, 0, 0};
    v4i64 int_acc2 = {0, 0, 0, 0};
    
    long long scalar_acc = 0;
    double double_acc = 0.0;
    
    /* Outer loop to create multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex control flow with if condition */
        if ((outer % loop_invariant) == 0) {
            /* Inner loop with register pressure */
            for (int inner = 0; inner < 8; inner++) {
                /* Volatile call creates code motion barrier */
                int barrier = get_volatile();
                
                /* Complex expression with many temporaries */
                /* Mix integer and floating-point operations */
                double temp1 = arr_dbl[inner] * 2.5 + barrier;
                long long temp2 = arr_ll[inner] * 3LL + barrier;
                
                /* Vector operations that create virtual registers */
                v4df vec1 = {temp1, temp1 * 0.5, temp1 * 0.25, temp1 * 0.125};
                v4df vec2 = {temp2 * 0.1, temp2 * 0.2, temp2 * 0.3, temp2 * 0.4};
                
                /* Shuffle operations - often create virtual registers */
                v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4i64){0, 5, 2, 7});
                
                /* More arithmetic creating intermediate values */
                v4i64 int_vec1 = {temp2, temp2 + 1, temp2 + 2, temp2 + 3};
                v4i64 int_vec2 = {barrier, barrier * 2, barrier * 3, barrier * 4};
                
                /* Another shuffle with integer vectors */
                v4i64 int_shuffled = __builtin_shuffle(int_vec1, int_vec2,
                    (v4i64){0, 5, 2, 7});
                
                /* Cross-type conversions increase register pressure */
                double temp3 = (double)temp2 * 1.7;
                long long temp4 = (long long)(temp1 * 3.8);
                
                /* Array accesses with non-constant indices */
                int idx1 = (inner + barrier) & 0xF;
                int idx2 = (inner * 3 + barrier) & 0xF;
                
                /* More computations using array elements */
                double temp5 = arr_dbl[idx1] * arr_dbl[idx2] + temp3;
                long long temp6 = arr_ll[idx1] * arr_ll[idx2] + temp4;
                
                /* Update accumulators - keep values live */
                vec_acc1 += vec_shuffled;
                vec_acc2 += vec1 * vec2;
                int_acc1 += int_shuffled;
                int_acc2 += int_vec1 | int_vec2;
                
                scalar_acc += temp6;
                double_acc += temp5;
                
                /* Additional volatile barrier in the middle */
                if ((inner & 3) == 0) {
                    barrier = get_volatile();
                    scalar_acc ^= barrier;
                }
            }
        } else {
            /* Different path to create control flow complexity */
            for (int inner = 0; inner < 4; inner++) {
                int barrier = get_volatile();
                double temp = arr_dbl[inner] / (barrier + 1);
                long long temp_ll = arr_ll[inner] / (barrier + 1);
                
                scalar_acc += temp_ll;
                double_acc += temp;
            }
        }
        
        /* Modify loop-invariant occasionally */
        if ((outer & 15) == 0) {
            loop_invariant = (loop_invariant * 3 + 1) & 7;
        }
    }
    
    /* Final reduction */
    double final_dbl = double_acc;
    for (int i = 0; i < 4; i++) {
        final_dbl += vec_acc1[i] + vec_acc2[i];
        scalar_acc += int_acc1[i] + int_acc2[i];
    }
    
    /* Mix results */
    long long result = scalar_acc + (long long)final_dbl;
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i);
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
