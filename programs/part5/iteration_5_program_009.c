/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>

/* Volatile function to create code motion barriers */
volatile int volatile_counter = 0;
int get_volatile(void) {
    return volatile_counter++;
}

/* Non-inlineable function attribute */
__attribute__((noinline,noipa))
static long long test_remat(int start, int iterations, double factor) {
    /* Large vectors to create register pressure */
    typedef int v8si __attribute__((vector_size(32)));
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[256];
    long long arr_ll[256];
    int arr_int[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr_dbl[i] = (i * 1.5) / factor;
        arr_ll[i] = i * 3LL;
        arr_int[i] = i * 2;
    }
    
    /* Complex intermediate results */
    v8si vec_int_result = {0};
    v4df vec_dbl_result = {0};
    v4di vec_ll_result = {0};
    
    /* Loop-invariant variable for control flow */
    int invariant = start * 2;
    
    /* Accumulator for final result */
    long long final_result = 0;
    
    /* Outer loop */
    for (int outer = 0; outer < iterations; outer++) {
        /* Inner loop with complex control flow */
        for (int i = 0; i < 128; i++) {
            /* Complex condition that depends on invariant */
            if ((i + outer + invariant) % 7 < 4) {
                /* REGISTER PRESSURE BLOCK - complex expression with many temps */
                
                /* 1. Integer arithmetic with volatile barrier */
                int idx1 = (i * 3 + get_volatile()) % 256;
                int idx2 = (i * 5 + outer) % 256;
                int idx3 = (i * 7 + invariant) % 256;
                
                /* 2. Mixed integer/double operations */
                double temp1 = arr_dbl[idx1] * factor + arr_int[idx2];
                double temp2 = arr_dbl[idx2] / (factor + 1.0) - arr_int[idx1];
                
                /* 3. Long long operations with shuffles */
                long long ll1 = arr_ll[idx3] + (long long)(temp1 * 100.0);
                long long ll2 = arr_ll[idx1] - (long long)(temp2 * 50.0);
                
                /* 4. Vector operations using GCC extensions */
                v4di vec1 = {ll1, ll2, ll1 * 2, ll2 * 3};
                v4di vec2 = {arr_ll[idx1], arr_ll[idx2], arr_ll[idx3], 0};
                
                /* __builtin_shuffle creates virtual registers */
                v4di shuffled = __builtin_shuffle(vec1, vec2, 
                    (v4di){1, 3, 0, 2});
                
                /* 5. More mixed operations with array accesses */
                double dbl1 = arr_dbl[(idx1 + idx2) % 256];
                double dbl2 = arr_dbl[(idx2 + idx3) % 256];
                
                /* Complex expression chain - many intermediate values */
                double complex_dbl = (dbl1 * dbl2) + 
                                    (temp1 - temp2) * 
                                    (arr_int[idx3] / 100.0);
                
                /* Integer vector operations */
                v8si v1 = {arr_int[idx1], arr_int[idx2], arr_int[idx3],
                          i, outer, invariant, idx1, idx2};
                v8si v2 = {idx1, idx2, idx3, outer, i, start, idx3, idx1};
                
                /* More operations to increase register pressure */
                v8si v3 = v1 + v2;
                v8si v4 = v1 * v2;
                v8si v5 = v3 - v4;
                
                /* Use results to prevent dead code elimination */
                for (int j = 0; j < 8; j++) {
                    vec_int_result[j % 8] += v5[j];
                }
                
                /* Accumulate into final result */
                final_result += (long long)(complex_dbl * 10.0) + 
                               shuffled[0] + shuffled[1] + shuffled[2];
                
                /* Additional volatile call creates another barrier */
                if (get_volatile() % 100 == 0) {
                    final_result += arr_ll[idx3];
                }
            } else {
                /* Alternative path with different operations */
                int idx = (i + outer) % 256;
                double temp = arr_dbl[idx] * 0.5;
                final_result += (long long)temp;
            }
        }
        
        /* Modify invariant slightly each outer iteration */
        invariant = (invariant * 3 + 1) % 100;
    }
    
    /* Use vector results to prevent elimination */
    long long vec_sum = 0;
    for (int i = 0; i < 8; i++) {
        vec_sum += vec_int_result[i];
    }
    
    return final_result + vec_sum;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 10, 5 + (i % 3), 1.0 + i * 0.1);
        
        /* Print progress to prevent over-optimization */
        if (i % 3 == 0) {
            printf("Iteration %d: total = %lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    
    /* Verify result is deterministic */
    if (total != 0) {
        printf("Test completed successfully\n");
        return 0;
    } else {
        printf("Unexpected zero result\n");
        return 1;
    }
}
