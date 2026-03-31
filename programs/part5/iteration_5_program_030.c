/* Compile with: gcc -O2 -fno-inline -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 test.c */
/* Or: gcc -O3 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int barrier_counter = 0;
static int get_barrier(void) {
    barrier_counter++;
    return barrier_counter & 1;
}

/* Non-inlineable function attribute */
__attribute__((noinline,noipa))
static long long test_remat(int iterations, int seed, int threshold) {
    /* Large arrays to create register pressure */
    double arr_dbl[32];
    long long arr_int[32];
    int arr_idx[32];
    
    /* Initialize arrays with non-constant indices */
    for (int i = 0; i < 32; i++) {
        arr_idx[i] = (seed + i * 3) % 32;
        arr_dbl[i] = (seed + i) * 1.5;
        arr_int[i] = (seed + i) * 7LL;
    }
    
    /* Vector types to encourage virtual register creation */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Complex accumulator variables mixing types */
    double dbl_acc = 0.0;
    long long int_acc = 0;
    v4df vec_acc = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Loop-invariant condition */
        int condition = (iter % 7) < threshold;
        
        if (condition) {
            /* Inner block with register pressure */
            
            /* Load vectors with non-constant indexing */
            v4df vec1 = {arr_dbl[arr_idx[0]], arr_dbl[arr_idx[1]], 
                         arr_dbl[arr_idx[2]], arr_dbl[arr_idx[3]]};
            v4df vec2 = {arr_dbl[arr_idx[4]], arr_dbl[arr_idx[5]], 
                         arr_dbl[arr_idx[6]], arr_dbl[arr_idx[7]]};
            
            /* Use __builtin_shuffle to create virtual registers */
            v4df shuffled = __builtin_shuffle(vec1, vec2, 
                (v4di){0, 1, 4, 5});
            
            /* Complex expression with many intermediates */
            for (int j = 0; j < 8; j++) {
                /* Volatile barrier in the middle of computation */
                if (get_barrier()) {
                    /* Mix integer and FP operations */
                    double temp1 = arr_dbl[arr_idx[j]] * 2.5;
                    long long temp2 = arr_int[arr_idx[j]] / 3;
                    
                    /* Complex expression with many live values */
                    double temp3 = temp1 + (double)temp2;
                    double temp4 = temp3 * arr_dbl[arr_idx[j+1]];
                    double temp5 = temp4 - arr_dbl[arr_idx[j+2]];
                    
                    /* Use vector operations */
                    vec_acc += shuffled * temp5;
                    
                    /* More intermediates */
                    long long temp6 = (long long)(temp5 * 100.0);
                    int temp7 = (int)(temp6 & 0xFF);
                    double temp8 = temp5 * (double)temp7;
                    
                    /* Accumulate results */
                    dbl_acc += temp8;
                    int_acc += temp6;
                    
                    /* Array updates with dependent indices */
                    arr_dbl[arr_idx[j]] = temp8;
                    arr_int[arr_idx[j]] = temp6;
                }
            }
            
            /* Additional vector operations */
            v4df vec3 = vec_acc + shuffled;
            v4df vec4 = __builtin_shuffle(vec3, vec_acc,
                (v4di){2, 3, 0, 1});
            
            /* Extract elements to scalar */
            double extracted[4];
            __builtin_memcpy(extracted, &vec4, sizeof(vec4));
            
            for (int k = 0; k < 4; k++) {
                dbl_acc += extracted[k];
            }
        }
        
        /* Modify indices to prevent optimization */
        for (int i = 0; i < 32; i++) {
            arr_idx[i] = (arr_idx[i] + iter) % 32;
        }
    }
    
    /* Final computation mixing types */
    long long result = int_acc + (long long)dbl_acc;
    return result;
}

int main(void) {
    long long total = 0;
    
    /* Call multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(50, i, 3);
        total += test_remat(25, i + 100, 5);
        total += test_remat(10, i + 200, 2);
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
