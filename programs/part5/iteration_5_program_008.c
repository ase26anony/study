/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int start, int iterations, double init_val) {
    /* Large arrays to force register pressure */
    double arr_dbl[32];
    long long arr_int[32];
    volatile int v;
    
    /* Initialize arrays with non-constant patterns */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = init_val * (i + 1) * 0.5;
        arr_int[i] = (long long)(start + i) * (i % 7 + 1);
    }
    
    /* Complex intermediate results that mix types */
    double sum_dbl = 0.0;
    long long sum_int = 0;
    double prod_dbl = 1.0;
    
    /* Outer loop with multiple basic blocks */
    for (int outer = 0; outer < iterations; outer++) {
        /* Loop-invariant variable for control flow */
        int invariant = volatile_barrier(outer % 4);
        
        /* Inner if with complex expression */
        if (invariant > 0) {
            /* Register pressure: many intermediate values */
            double t1 = arr_dbl[outer % 32] * 2.5;
            double t2 = t1 + arr_dbl[(outer + 1) % 32];
            
            /* Volatile barrier prevents code motion */
            v = volatile_barrier((int)t2);
            
            /* More intermediates with type mixing */
            long long t3 = arr_int[outer % 32] + (long long)t2;
            double t4 = (double)t3 * 0.75;
            
            /* Vector-like operations using GCC extensions */
            typedef double v4df __attribute__((vector_size(32)));
            typedef long long v4di __attribute__((vector_size(32)));
            
            /* Create vector intermediates */
            v4df vec1 = {t1, t2, t4, arr_dbl[outer % 32]};
            v4df vec2 = {arr_dbl[(outer + 3) % 32], 
                         arr_dbl[(outer + 5) % 32],
                         arr_dbl[(outer + 7) % 32],
                         arr_dbl[(outer + 11) % 32]};
            
            /* Vector operations create virtual registers */
            v4df vec3 = vec1 * vec2 + vec1;
            v4df vec4 = __builtin_shuffle(vec3, vec3, 
                (v4di){3, 2, 1, 0});
            
            /* Extract results - forces materialization */
            double extract[4];
            __builtin_memcpy(&extract, &vec4, sizeof(extract));
            
            /* Complex expression with many temporaries */
            double complex_expr = 
                (extract[0] * extract[1]) / (extract[2] + 0.001) +
                (extract[3] * t4) / (t1 + 0.001) +
                (arr_dbl[outer % 16] * arr_dbl[(outer + 8) % 16]) +
                (double)arr_int[outer % 16] / 256.0;
            
            /* More integer intermediates */
            long long t5 = arr_int[(outer + 4) % 32] ^ 
                          (long long)complex_expr;
            long long t6 = t5 * (outer + 1);
            long long t7 = t6 + arr_int[(outer + 8) % 32];
            
            /* Update accumulators with barrier */
            v = volatile_barrier((int)t7);
            sum_dbl += complex_expr + extract[0] + extract[1];
            sum_int += t5 + t6 + t7;
            prod_dbl *= (complex_expr + 1.0) * 0.5;
        } else {
            /* Alternate path to create control flow complexity */
            double alt = arr_dbl[outer % 32] * arr_dbl[(outer + 16) % 32];
            sum_dbl += alt * 0.25;
            sum_int += arr_int[outer % 32] >> 2;
        }
        
        /* Cross-type operations that need different register modes */
        if (outer % 3 == 0) {
            double mixed = (double)sum_int * 0.01 + sum_dbl;
            prod_dbl *= mixed + 1.0;
            
            /* Another volatile barrier */
            v = volatile_barrier((int)mixed);
        }
        
        /* Array updates with non-constant indices */
        int idx1 = (outer * 7) % 32;
        int idx2 = (outer * 13) % 32;
        arr_dbl[idx1] = sum_dbl * 0.01;
        arr_int[idx2] = sum_int / (outer + 2);
    }
    
    /* Final computation mixing all types */
    long long result = (long long)sum_dbl + sum_int + (long long)prod_dbl;
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i, 50, (double)i * 0.1 + 1.0);
        
        /* Prevent loop optimization */
        if (i % 10 == 0) {
            printf("Progress: %d\n", i);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    return 0;
}
