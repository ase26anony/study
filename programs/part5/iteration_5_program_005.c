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
long long test_remat(int iterations, int seed, double scale) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + seed) * 1.5;
        arr_ll[i] = (i * seed) ^ 0x12345678;
    }
    
    /* Complex intermediate results */
    v4df vec_acc1 = {0, 0, 0, 0};
    v4df vec_acc2 = {0, 0, 0, 0};
    v4di int_acc = {0, 0, 0, 0};
    
    double fp_sum = 0.0;
    long long int_sum = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = seed * outer % 7;
        
        /* Inner if with register-intensive computation */
        if (invariant > 2) {
            /* Complex expression with many temporaries */
            double temp1 = arr_dbl[invariant % 16] * scale;
            double temp2 = arr_dbl[(invariant + 1) % 16] / scale;
            
            /* Volatile barrier in the middle of computation */
            int volatile_val = get_volatile_value();
            
            /* More temporaries after barrier */
            double temp3 = temp1 + temp2 + volatile_val;
            double temp4 = temp1 - temp2 - (volatile_val % 100);
            
            /* Vector operations that create virtual registers */
            v4df vec1 = {temp1, temp2, temp3, temp4};
            v4df vec2 = {temp3, temp4, temp1, temp2};
            
            /* Shuffle operation - often creates virtual registers */
            v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 2, 1, 3});
            
            /* Mixed integer/floating operations */
            long long ll_temp1 = (long long)(temp1 * 1000);
            long long ll_temp2 = arr_ll[invariant % 16];
            long long ll_temp3 = ll_temp1 ^ ll_temp2;
            long long ll_temp4 = ll_temp1 & (ll_temp2 << 2);
            
            /* More vector operations with different modes */
            v4di int_vec1 = {ll_temp1, ll_temp2, ll_temp3, ll_temp4};
            v4di int_vec2 = __builtin_shuffle(int_vec1, int_vec1,
                (v4di){3, 1, 2, 0});
            
            /* Accumulate results - keeps values live */
            vec_acc1 += vec_shuffled;
            vec_acc2 += vec1 * vec2;
            int_acc += int_vec1 - int_vec2;
            
            /* More mixed-mode computations */
            fp_sum += temp1 + temp2 + temp3 + temp4;
            int_sum += ll_temp1 + ll_temp2 + ll_temp3 + ll_temp4;
            
            /* Another volatile barrier */
            volatile_val = get_volatile_value();
            
            /* Additional computations using the barrier value */
            fp_sum += (volatile_val % 1000) * 0.001;
            int_sum ^= (volatile_val << 16);
        } else {
            /* Different path with simpler computation */
            double simple = arr_dbl[outer % 16] * 0.5;
            fp_sum += simple;
            int_sum += (long long)simple;
        }
        
        /* Cross-iteration dependencies */
        if (outer > 0) {
            arr_dbl[outer % 16] = fp_sum * 0.01;
            arr_ll[outer % 16] = int_sum >> 2;
        }
    }
    
    /* Final reduction */
    double final_fp = 0;
    long long final_int = 0;
    
    for (int i = 0; i < 4; i++) {
        final_fp += vec_acc1[i] + vec_acc2[i];
        final_int += int_acc[i];
    }
    
    /* Combine results */
    return (long long)(final_fp * 1000) + final_int + int_sum;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i, 1.0 + i * 0.01);
        
        /* Prevent dead code elimination */
        if (i % 10 == 0) {
            printf("Progress: i=%d, total=%lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    printf("Barrier calls: %d\n", barrier_counter);
    
    return 0;
}
