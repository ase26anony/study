/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile function to create code motion barrier */
volatile int volatile_barrier(int x) {
    return x + (rand() % 2);  /* Non-deterministic but side-effect free */
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int seed, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays with values dependent on arguments */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val * i + seed;
        arr_ll[i] = (seed * i) ^ (iterations + i);
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = seed * iterations;
    
    /* Accumulators mixing types */
    double dbl_acc = 0.0;
    long long int_acc = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop creating register pressure */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex expression with many temporaries */
        double temp1 = arr_dbl[outer & 0xF] * 3.14159;
        long long temp2 = arr_ll[(outer + 1) & 0xF] << 2;
        
        /* Vector operations that create virtual registers */
        v4df vec1 = {temp1, temp1 * 2.0, temp1 * 3.0, temp1 * 4.0};
        v4di vec2 = {temp2, temp2 + 1, temp2 + 2, temp2 + 3};
        
        /* Control flow with loop-invariant condition */
        if (loop_invariant > (outer * 100)) {
            /* Register pressure block - many intermediate values */
            
            /* Mixed integer/floating operations */
            double d1 = arr_dbl[(outer + seed) & 0xF];
            double d2 = arr_dbl[(outer + iterations) & 0xF];
            long long i1 = arr_ll[(outer * 3) & 0xF];
            long long i2 = arr_ll[(outer * 7) & 0xF];
            
            /* Complex expression with volatile barrier */
            double complex_dbl = d1 * d2 + 
                                (double)volatile_barrier(i1 & 0xFF) / 256.0 -
                                (double)(i2 % 1000) * 0.001;
            
            long long complex_int = (i1 * i2) ^ 
                                   (long long)volatile_barrier((int)d1) ^
                                   (iterations * outer);
            
            /* More vector operations */
            v4df vec_tmp1 = __builtin_shuffle(vec1, vec1, 
                (v4di){0, 1, 2, 3});
            v4df vec_tmp2 = {complex_dbl, complex_dbl * 2.0, 
                            complex_dbl * 3.0, complex_dbl * 4.0};
            
            /* Vector arithmetic creating more temporaries */
            v4df vec_result = vec_tmp1 * vec_tmp2 + vec_acc;
            
            /* Extract and accumulate results */
            double vec_elems[4];
            __builtin_memcpy(vec_elems, &vec_result, sizeof(vec_result));
            
            for (int i = 0; i < 4; i++) {
                dbl_acc += vec_elems[i];
                int_acc += (long long)vec_elems[i];
            }
            
            /* Additional integer operations */
            int_acc += complex_int;
            dbl_acc += complex_dbl;
            
            /* More register pressure with array updates */
            arr_dbl[outer & 0xF] = dbl_acc * 0.01;
            arr_ll[outer & 0xF] = int_acc & 0xFFFFFFFF;
        }
        
        /* Update accumulators outside if block */
        vec_acc += vec1;
        int_acc += temp2;
        dbl_acc += temp1;
        
        /* Prevent dead code elimination */
        if (outer % 100 == 0) {
            volatile_barrier((int)dbl_acc);
        }
    }
    
    /* Final computation mixing types */
    long long result = (long long)dbl_acc + int_acc;
    
    /* Use arrays to prevent optimization */
    for (int i = 0; i < 16; i++) {
        result ^= (long long)arr_dbl[i];
        result += arr_ll[i];
    }
    
    return result;
}

int main(void) {
    long long total = 0;
    const int num_iterations = 1000;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += test_remat(num_iterations + i, i * 123, 1.0 + i * 0.1);
        
        /* Print progress to prevent optimization */
        if (i % 3 == 0) {
            printf("Iteration %d: partial sum = %lld\n", i, total);
        }
    }
    
    printf("Final checksum: %lld\n", total);
    return 0;
}
