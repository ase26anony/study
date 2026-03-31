/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
volatile int volatile_func(int x) {
    return x ^ 0x55AA55AA;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline)) 
long long test_remat(int iterations, int start_val, double scale_factor) {
    /* Large vectors to create virtual registers */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (i * 1.5) / scale_factor;
        arr_int[i] = i + start_val;
        arr_ll[i] = (long long)i * 1000000;
    }
    
    /* Loop-invariant variable for control flow */
    int loop_invariant = start_val * 2 + iterations;
    
    /* Accumulators of different types */
    double dbl_acc = 0.0;
    long long ll_acc = 0;
    int int_acc = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < iterations; outer++) {
        /* Vector operations that create virtual registers */
        v4df vec_dbl = {arr_dbl[0], arr_dbl[1], arr_dbl[2], arr_dbl[3]};
        v4di vec_ll = {arr_ll[0], arr_ll[1], arr_ll[2], arr_ll[3]};
        v8si vec_int = {arr_int[0], arr_int[1], arr_int[2], arr_int[3],
                       arr_int[4], arr_int[5], arr_int[6], arr_int[7]};
        
        /* Complex control flow */
        if (loop_invariant > (outer * 10)) {
            /* Register pressure inducing expression with volatile barrier */
            for (int inner = 0; inner < 16; inner++) {
                /* Mix integer and floating-point operations */
                double temp_dbl = arr_dbl[inner & 0x1F] * scale_factor;
                temp_dbl += (double)(arr_int[inner & 0x1F] ^ volatile_func(inner));
                
                /* Shuffle operations that create virtual registers */
                v4df shuffled = __builtin_shuffle(vec_dbl, 
                    (v4di){inner % 4, (inner + 1) % 4, (inner + 2) % 4, (inner + 3) % 4});
                
                /* More mixed operations */
                long long temp_ll = arr_ll[inner & 0x1F];
                temp_ll += (long long)(temp_dbl * 1000.0);
                temp_ll ^= (long long)arr_int[(inner + 1) & 0x1F] << 4;
                
                /* Vector operations with different modes */
                v4di vec_temp = vec_ll + (v4di){temp_ll, temp_ll >> 1, temp_ll >> 2, temp_ll >> 3};
                v8si vec_int_temp = vec_int * (v8si){inner, inner+1, inner+2, inner+3,
                                                   inner+4, inner+5, inner+6, inner+7};
                
                /* Accumulate results with volatile barrier */
                dbl_acc += temp_dbl + shuffled[0] + shuffled[1];
                ll_acc += temp_ll + vec_temp[0] + vec_temp[1];
                int_acc += arr_int[inner & 0x1F] + vec_int_temp[0] + vec_int_temp[4];
                
                /* Update arrays with non-constant indices */
                int idx = (inner + outer) & 0x1F;
                arr_dbl[idx] = temp_dbl * 0.99;
                arr_ll[idx] = temp_ll ^ 0x12345678;
                arr_int[idx] = volatile_func(arr_int[idx]) + inner;
            }
            
            /* Additional vector operations outside inner loop */
            v4df vec_mul = vec_dbl * (v4df){scale_factor, scale_factor * 2, 
                                           scale_factor * 3, scale_factor * 4};
            v4di vec_shift = vec_ll >> (v4di){2, 3, 4, 5};
            
            /* Use results */
            dbl_acc += vec_mul[0] + vec_mul[2];
            ll_acc += vec_shift[1] + vec_shift[3];
        } else {
            /* Alternative path with different operations */
            for (int i = 0; i < 8; i++) {
                double d = arr_dbl[i] * arr_dbl[i + 8];
                long long l = arr_ll[i] * arr_ll[i + 8];
                int iv = arr_int[i] * arr_int[i + 8];
                
                dbl_acc += d * scale_factor;
                ll_acc += l ^ (long long)d;
                int_acc += iv + volatile_func(i);
            }
        }
        
        /* Modify loop-invariant (but compiler might not realize it changes) */
        loop_invariant += volatile_func(outer) & 0xF;
    }
    
    /* Final mixed computation */
    long long result = ll_acc + (long long)dbl_acc + int_acc;
    
    /* Prevent dead code elimination */
    result ^= (long long)volatile_func((int)result);
    
    return result;
}

int main(int argc, char **argv) {
    long long total = 0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        total += test_remat(iterations + i, i * 100, 1.0 + i * 0.1);
        total += test_remat(iterations / 2, i * 50, 2.0 - i * 0.05);
        total ^= test_remat(5, i, 0.5) * 3;
    }
    
    /* Deterministic output */
    printf("Result: %lld\n", total);
    return (int)(total & 0x7FFFFFFF);
}
