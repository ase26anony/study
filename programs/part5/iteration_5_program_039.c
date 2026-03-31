/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double factor) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = (i + seed) * factor;
        arr_ll[i] = i * seed * 1234567LL;
    }
    
    v4df vec_result = {0.0, 0.0, 0.0, 0.0};
    v4di int_result = {0, 0, 0, 0};
    
    long long final_sum = 0;
    
    /* Outer loop with complex control flow */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = volatile_barrier(seed + outer) % 8;
        
        /* Inner if with register-intensive computation */
        if (invariant > 2 && invariant < 7) {
            /* Complex expression mixing FP and integer ops */
            for (int inner = 0; inner < 4; inner++) {
                /* Non-constant array indexing */
                int idx1 = (invariant + inner) & 0xF;
                int idx2 = (invariant * inner + 3) & 0xF;
                int idx3 = (outer * inner + seed) & 0xF;
                
                /* Register pressure: many intermediate values */
                double temp1 = arr_dbl[idx1] * factor + arr_dbl[idx2];
                double temp2 = arr_dbl[idx3] / (factor + 1.0);
                
                /* Integer operations */
                long long ll_temp1 = arr_ll[idx1] + arr_ll[idx2];
                long long ll_temp2 = arr_ll[idx3] * (inner + 1);
                
                /* Mix FP and integer with conversions */
                double mixed1 = temp1 * (double)ll_temp1;
                double mixed2 = temp2 + (double)ll_temp2;
                
                /* Vector operations creating virtual registers */
                v4df vec1 = {temp1, temp2, mixed1, mixed2};
                v4df vec2 = {mixed2, mixed1, temp2, temp1};
                
                /* Shuffle operation - often creates virtual regs */
                v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4di){1, 3, 0, 2});
                
                /* More intermediate computations */
                v4df vec_scaled = vec_shuffled * (v4df){1.1, 1.2, 1.3, 1.4};
                
                /* Integer vector operations */
                v4di int_vec1 = {ll_temp1, ll_temp2, 
                                (long long)temp1, (long long)temp2};
                v4di int_vec2 = __builtin_shuffle(int_vec1, 
                    (v4di){2, 3, 0, 1});
                
                /* Accumulate results - keeps values live */
                vec_result += vec_scaled;
                int_result += int_vec2;
                
                /* Another volatile barrier */
                volatile_barrier(inner);
            }
            
            /* Extract and mix results */
            double* vp = (double*)&vec_result;
            long long* ip = (long long*)&int_result;
            
            for (int i = 0; i < 4; i++) {
                /* Complex expression with many temporaries */
                double dbl_val = vp[i] * 0.5 + arr_dbl[(invariant + i) & 0xF];
                long long int_val = ip[i] / 3 + arr_ll[(invariant * i) & 0xF];
                
                /* Mix types with conversion */
                final_sum += (long long)(dbl_val * 100.0) + int_val;
                
                /* More intermediate computations */
                double chain1 = dbl_val * factor;
                double chain2 = chain1 / (int_val + 1);
                long long chain3 = (long long)chain2 * int_val;
                double chain4 = (double)chain3 / factor;
                
                /* Use results to prevent dead code elimination */
                final_sum += (long long)chain4;
            }
        } else {
            /* Alternative path with different computations */
            for (int i = 0; i < 4; i++) {
                int idx = (invariant + i * 3) & 0xF;
                double val = arr_dbl[idx] * 0.75;
                long long ival = arr_ll[idx] >> 2;
                final_sum += (long long)(val * 50.0) + ival;
            }
        }
        
        /* Modify arrays to prevent optimization */
        arr_dbl[outer & 0xF] += 0.01;
        arr_ll[outer & 0xF] += 1;
    }
    
    return final_sum;
}

int main(int argc, char** argv) {
    long long total = 0;
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 10; i++) {
        total += test_remat(iterations, i * 17, 1.0 + i * 0.1);
        total += test_remat(iterations / 2, i * 23 + 1, 0.5 + i * 0.05);
    }
    
    printf("Result checksum: %lld\n", total);
    return 0;
}
