/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Large arrays to force register pressure */
    double arr_dbl[32];
    long long arr_int[32];
    volatile double vol_dbl = 3.14159;
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = init_val * i + start;
        arr_int[i] = (long long)(init_val * 1000) + i * start;
    }
    
    /* Complex accumulator mixing types */
    double dbl_acc = init_val;
    long long int_acc = start;
    
    /* Outer loop with multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for control flow */
        int invariant = volatile_barrier(outer) & 0xF;
        
        /* Inner if with complex expression */
        if (invariant > (start & 0x7)) {
            /* REGISTER PRESSURE BLOCK - complex expression with many temps */
            
            /* Vector-like operations using GCC extensions */
            typedef double v4df __attribute__((vector_size(32)));
            typedef long long v4di __attribute__((vector_size(32)));
            
            /* Load vectors from arrays with non-constant indexing */
            v4df vec1 = { 
                arr_dbl[(invariant + 0) & 31],
                arr_dbl[(invariant + 1) & 31],
                arr_dbl[(invariant + 2) & 31],
                arr_dbl[(invariant + 3) & 31]
            };
            
            v4df vec2 = {
                arr_dbl[(invariant + 4) & 31],
                arr_dbl[(invariant + 5) & 31],
                arr_dbl[(invariant + 6) & 31],
                arr_dbl[(invariant + 7) & 31]
            };
            
            /* Complex vector expression with many intermediate results */
            v4df vec_temp1 = vec1 * vec2 + (v4df){vol_dbl, vol_dbl, vol_dbl, vol_dbl};
            v4df vec_temp2 = __builtin_shuffle(vec_temp1, vec_temp1, 
                                              (v4di){3, 2, 1, 0});
            v4df vec_temp3 = vec_temp1 + vec_temp2;
            v4df vec_temp4 = vec_temp3 * (v4df){2.0, 1.5, 1.0, 0.5};
            
            /* Mix with integer operations */
            v4di int_vec = {
                arr_int[(invariant + 0) & 31],
                arr_int[(invariant + 1) & 31],
                arr_int[(invariant + 2) & 31],
                arr_int[(invariant + 3) & 31]
            };
            
            /* More intermediate results with type mixing */
            v4df vec_temp5 = (v4df)int_vec * 0.001 + vec_temp4;
            v4df vec_temp6 = __builtin_shuffle(vec_temp5, vec_temp5,
                                              (v4di){1, 0, 3, 2});
            
            /* Final computation with volatile barrier in middle */
            double temp_result[4];
            __builtin_memcpy(temp_result, &vec_temp6, sizeof(vec_temp6));
            
            /* Use all intermediate results to prevent elimination */
            dbl_acc += temp_result[0] + temp_result[1] + 
                      temp_result[2] + temp_result[3];
            
            /* Integer side computation */
            long long temp_int = (long long)(temp_result[0] * 1000);
            int_acc ^= temp_int;
            int_acc += (long long)(volatile_barrier(invariant) * 17);
            
            /* More register pressure with scalar operations */
            for (int j = 0; j < 4; j++) {
                double t1 = arr_dbl[(invariant + j * 2) & 31];
                double t2 = arr_dbl[(invariant + j * 2 + 1) & 31];
                double t3 = t1 * t2 + vol_dbl;
                double t4 = t3 / (j + 1);
                long long t5 = (long long)(t4 * 100);
                int_acc += t5 ^ arr_int[(invariant + j) & 31];
                dbl_acc += t4;
            }
        } else {
            /* Alternative path to create control flow complexity */
            double alt_temp = arr_dbl[invariant & 31] * 2.0;
            int_acc += (long long)alt_temp;
            dbl_acc -= alt_temp * 0.5;
        }
        
        /* Loop-carried dependency with volatile */
        vol_dbl = volatile_barrier(outer) * 0.01;
        
        /* Modify arrays to prevent optimization */
        arr_dbl[outer & 31] += dbl_acc * 0.01;
        arr_int[outer & 31] ^= int_acc;
    }
    
    /* Final mixed-type computation */
    long long result = int_acc + (long long)(dbl_acc * 1000.0);
    return result ^ (long long)(vol_dbl * 10000.0);
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50, 1.0 + i * 0.1);
        total ^= test_remat(i * 3, 25, 2.0 - i * 0.05);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %lld\n", total);
    
    /* Additional test with different modes */
    volatile double v = 0.0;
    for (int i = 0; i < 10; i++) {
        v += test_remat(100 + i, 10, v);
    }
    
    return total != 0 ? 0 : 1;
}
