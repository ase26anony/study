/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier_counter = 0;
static int get_volatile_value(void) {
    barrier_counter++;
    return barrier_counter;
}

/* Non-inlineable function with complex register pressure */
static __attribute__((noinline,noipa))
long long test_remat(int iterations, int seed, int *results) {
    /* Large vectors to force virtual register creation */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[32];
    long long arr_ll[32];
    int arr_int[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_dbl[i] = (seed + i) * 0.5;
        arr_ll[i] = (long long)(seed * i) * 3LL;
        arr_int[i] = seed ^ i;
    }
    
    long long total = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    v4di ll_acc = {0, 0, 0, 0};
    
    /* Outer loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile_value() & 0xF;
        
        /* Inner if with complex expression */
        if ((iter + seed) % 7 < 4) {
            /* COMPLEX REGISTER-PRESSURE EXPRESSION */
            /* Mix of integer and FP operations with many temporaries */
            
            /* Vector operations that create virtual registers */
            v8si v1 = {arr_int[0], arr_int[1], arr_int[2], arr_int[3],
                       arr_int[4], arr_int[5], arr_int[6], arr_int[7]};
            v8si v2 = {arr_int[8], arr_int[9], arr_int[10], arr_int[11],
                       arr_int[12], arr_int[13], arr_int[14], arr_int[15]};
            
            /* Shuffle operations force virtual register creation */
            v8si v3 = __builtin_shuffle(v1, v2, 
                (v8si){0,2,4,6,8,10,12,14});
            v8si v4 = __builtin_shuffle(v1, v2,
                (v8si){1,3,5,7,9,11,13,15});
            
            /* Mixed precision computations */
            double d1 = arr_dbl[iter & 31];
            double d2 = arr_dbl[(iter + 1) & 31];
            double d3 = arr_dbl[(iter + 2) & 31];
            double d4 = arr_dbl[(iter + 3) & 31];
            
            /* Chain of dependent FP operations */
            double t1 = d1 * d2 + (double)invariant;
            double t2 = d3 / d4 - (double)(seed & 0xFF);
            double t3 = t1 * t2 + get_volatile_value() * 0.01;
            double t4 = t2 / t1 - get_volatile_value() * 0.001;
            
            /* Integer computations interleaved */
            long long ll1 = arr_ll[iter & 31];
            long long ll2 = arr_ll[(iter + 4) & 31];
            long long ll3 = ll1 * ll2 + (long long)invariant;
            long long ll4 = ll2 / (ll1 + 1) + (long long)seed;
            
            /* More vector operations */
            v4df vd1 = {t1, t2, t3, t4};
            v4df vd2 = {d4, d3, d2, d1};
            v4df vd3 = vd1 * vd2 + (v4df){1.0, 2.0, 3.0, 4.0};
            
            /* Integer vector operations */
            v4di vl1 = {ll1, ll2, ll3, ll4};
            v4di vl2 = {ll4, ll3, ll2, ll1};
            v4di vl3 = vl1 + vl2 * (v4di){1, 2, 3, 4};
            
            /* Accumulate results with barrier */
            vec_acc += vd3 * (v4df){0.5, 0.25, 0.125, 0.0625};
            ll_acc += vl3 >> (invariant & 3);
            
            /* Non-constant array access pattern */
            int idx1 = (iter * seed) & 31;
            int idx2 = (iter + seed * 3) & 31;
            int idx3 = (iter * 5 + seed) & 31;
            
            /* Complex expression with many intermediate values */
            double complex_dbl = 
                (arr_dbl[idx1] * arr_dbl[idx2] + 
                 arr_dbl[idx3] * get_volatile_value()) /
                (1.0 + (double)(iter & 0xFF));
            
            long long complex_ll =
                (arr_ll[idx1] * arr_ll[idx2] +
                 arr_ll[idx3] * (long long)get_volatile_value()) /
                (1LL + (long long)(seed & 0xFF));
            
            /* Update total with mixed types */
            total += (long long)(complex_dbl * 1000.0) + complex_ll;
            
            /* Store to memory to keep values live */
            if (results) {
                results[iter & 31] = (int)complex_dbl + (int)complex_ll;
            }
        } else {
            /* Alternative path to create control flow complexity */
            double simple = arr_dbl[iter & 31] * 2.0;
            total += (long long)simple;
        }
        
        /* Modify arrays to prevent optimization */
        arr_dbl[iter & 31] += 0.1;
        arr_ll[iter & 31] += iter;
        arr_int[iter & 31] ^= seed;
    }
    
    /* Final reduction */
    total += (long long)(vec_acc[0] + vec_acc[1] + vec_acc[2] + vec_acc[3]);
    total += ll_acc[0] + ll_acc[1] + ll_acc[2] + ll_acc[3];
    
    return total;
}

int main(void) {
    int results[32];
    long long final_total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        final_total += test_remat(50, i, results);
        
        /* Use results to prevent dead code elimination */
        if (i % 10 == 0) {
            int sum = 0;
            for (int j = 0; j < 32; j++) {
                sum += results[j];
            }
            printf("Iteration %d, partial sum: %d\n", i, sum);
        }
    }
    
    printf("Final total: %lld\n", final_total);
    printf("Barrier counter: %d\n", barrier_counter);
    
    return 0;
}
