/* Test program to trigger early rematerialization in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with __attribute__((noinline)) */
static __attribute__((noinline, noipa))
long long test_remat(int iterations, int seed, int threshold) {
    /* Large vectors to create register pressure */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_double[16];
    long long arr_long[16];
    int arr_int[16];
    
    /* Initialize arrays with seed-dependent values */
    for (int i = 0; i < 16; i++) {
        arr_double[i] = (seed + i) * 1.5;
        arr_long[i] = (seed + i) * 3LL;
        arr_int[i] = seed + i;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = volatile_barrier(threshold);
    
    /* Accumulators mixing types */
    double double_sum = 0.0;
    long long long_sum = 0;
    int int_sum = 0;
    
    /* Outer loop to create multiple basic blocks */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex control flow */
        if (iter % 4 == loop_invariant % 4) {
            /* Inner if block with register-pressure-inducing expression */
            
            /* Create vector variables - these often use virtual registers */
            v8si vec_int = {0};
            v4df vec_double = {0.0};
            v4di vec_long = {0};
            
            /* Complex expression with many intermediate values */
            for (int i = 0; i < 8; i++) {
                /* Non-constant array indexing */
                int idx1 = (iter + i) & 0xF;
                int idx2 = (seed + i) & 0xF;
                int idx3 = (iter * i) & 0xF;
                
                /* Mixed integer/floating operations */
                double temp_d = arr_double[idx1] * 2.5;
                long long temp_ll = arr_long[idx2] + iter;
                int temp_int = arr_int[idx3] * 3;
                
                /* Volatile barrier in the middle of computation */
                temp_int = volatile_barrier(temp_int);
                
                /* More mixed operations creating many temporaries */
                temp_d = temp_d / (arr_double[idx2] + 1.0);
                temp_ll = temp_ll * (arr_long[idx1] >> 2);
                temp_int = temp_int + (arr_int[idx2] & 0xFF);
                
                /* Vector operations using GCC builtins */
                vec_int[i] = temp_int;
                if (i < 4) {
                    vec_double[i] = temp_d;
                    vec_long[i] = temp_ll;
                }
                
                /* Shuffle operations that create virtual registers */
                if (i == 3) {
                    v4df shuffled = __builtin_shuffle(vec_double, 
                        (v4di){1, 0, 3, 2});
                    vec_double = vec_double + shuffled;
                }
            }
            
            /* More complex expression with many intermediate results */
            for (int j = 0; j < 4; j++) {
                /* Nested computations */
                double a = vec_double[j];
                long long b = vec_long[j];
                int c = vec_int[j*2];
                
                /* Chain of dependent operations */
                for (int k = 0; k < 2; k++) {
                    a = a * 1.1 + (double)(c % 17);
                    b = b + (long long)(a * 100.0);
                    c = c ^ (int)(b & 0xFFFF);
                    
                    /* Another volatile barrier */
                    c = volatile_barrier(c);
                }
                
                /* Accumulate results */
                double_sum += a;
                long_sum += b;
                int_sum += c;
            }
        } else {
            /* Alternative path to create control flow complexity */
            double temp = 0.0;
            for (int i = 0; i < 4; i++) {
                int idx = (iter + i) & 0xF;
                temp += arr_double[idx] * arr_int[idx];
            }
            double_sum += temp * 0.5;
        }
        
        /* Additional computation outside the if block */
        if (iter % 3 == 0) {
            /* More mixed-type operations */
            long_sum += (long long)(double_sum * 10.0);
            int_sum += iter * seed;
        }
    }
    
    /* Final mixed computation */
    long long result = long_sum + (long long)double_sum + int_sum;
    return volatile_barrier(result);
}

int main(void) {
    long long total = 0;
    const int iterations = 1000;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        long long res = test_remat(iterations, i * 17, i % 5);
        total += res;
        
        /* Prevent dead code elimination */
        if (res == 0) {
            printf("Zero result at iteration %d\n", i);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    return 0;
}
