/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter & 0xFF;
}

/* Non-inlineable function attribute */
__attribute__((noinline, noipa))
static long long test_remat(int iterations, int seed, int use_vector) {
    /* Local arrays with non-constant indexing */
    double arr_double[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with values based on seed */
    for (int i = 0; i < 32; i++) {
        arr_double[i] = (seed + i) * 1.2345;
        arr_int[i] = (seed * 1000LL) + i;
        arr_idx[i] = (seed + i * 3) % 32;
    }
    
    /* Complex intermediate results that need many registers */
    double sum_double = 0.0;
    long long sum_int = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = seed + (outer & 0xF);
        
        /* Inner if condition */
        if ((invariant % 7) < 4) {
            /* REGISTER PRESSURE BLOCK - many temporary values */
            
            /* Mixed integer and floating-point operations */
            double temp1 = arr_double[arr_idx[0]] * arr_double[arr_idx[1]];
            double temp2 = arr_double[arr_idx[2]] / (arr_double[arr_idx[3]] + 1.0);
            
            /* Integer operations */
            long long temp3 = arr_int[arr_idx[4]] * arr_int[arr_idx[5]];
            long long temp4 = arr_int[arr_idx[6]] + arr_int[arr_idx[7]];
            
            /* Code motion barrier using volatile */
            int barrier = get_volatile_value();
            
            /* More mixed operations after barrier */
            double temp5 = temp1 + (double)temp3 * 0.001;
            double temp6 = temp2 - (double)temp4 * 0.0001;
            
            /* Vector operations using GCC extensions */
            if (use_vector) {
                typedef int v4si __attribute__((vector_size(16)));
                typedef double v2df __attribute__((vector_size(16)));
                
                v4si vec_int1 = {barrier, seed, outer, invariant};
                v4si vec_int2 = {temp3 & 0xFF, temp4 & 0xFF, 
                                 arr_idx[8], arr_idx[9]};
                v4si vec_int3 = vec_int1 + vec_int2;
                v4si vec_int4 = vec_int1 * vec_int2;
                
                /* Shuffle operation that creates virtual registers */
                v4si shuffled = __builtin_shuffle(vec_int3, vec_int4, 
                    (v4si){2, 3, 0, 1});
                
                v2df vec_double1 = {temp5, temp6};
                v2df vec_double2 = {arr_double[arr_idx[10]], 
                                   arr_double[arr_idx[11]]};
                v2df vec_double3 = vec_double1 * vec_double2;
                
                /* Use vector results */
                sum_double += vec_double3[0] + vec_double3[1];
                sum_int += shuffled[0] + shuffled[1] + shuffled[2] + shuffled[3];
            } else {
                /* Non-vector path still has register pressure */
                double temp7 = temp5 * temp6;
                double temp8 = temp7 / (arr_double[arr_idx[12]] + 1.0);
                
                long long temp9 = (temp3 >> 4) + (temp4 << 2);
                long long temp10 = temp9 * (arr_int[arr_idx[13]] & 0xFFFF);
                
                /* Complex expression with many intermediates */
                sum_double += temp8 + (double)barrier * 0.01;
                sum_int += temp10 + (barrier * 1000LL);
            }
            
            /* More operations to increase live ranges */
            for (int inner = 0; inner < 4; inner++) {
                int idx = (invariant + inner) % 32;
                double dval = arr_double[idx] * (inner + 1);
                long long ival = arr_int[idx] / (inner + 1);
                
                /* Conditional update based on complex expression */
                if ((dval > 0.0) && (ival & 1)) {
                    sum_double += dval;
                    sum_int += ival;
                }
            }
        } else {
            /* Alternate path with different operations */
            double alt_temp = arr_double[arr_idx[14]] * 2.0;
            long long alt_int = arr_int[arr_idx[15]] / 2;
            
            sum_double += alt_temp;
            sum_int += alt_int;
        }
        
        /* Update arrays with non-constant indices */
        int update_idx = (outer * 13) % 32;
        arr_double[update_idx] = sum_double * 0.99;
        arr_int[update_idx] = sum_int & 0x7FFFFFFF;
    }
    
    /* Final mixed computation */
    long long result = (long long)(sum_double * 1000.0) + sum_int;
    return result;
}

int main(void) {
    long long total = 0;
    int iterations = 100;
    
    /* Call test function multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        long long res = test_remat(iterations, i * 17, i % 2);
        total += res;
        printf("Iteration %d: result = %lld, total = %lld\n", i, res, total);
    }
    
    /* Also test with larger iteration counts */
    long long big_res = test_remat(500, 12345, 1);
    total += big_res;
    
    printf("Final total: %lld\n", total);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
