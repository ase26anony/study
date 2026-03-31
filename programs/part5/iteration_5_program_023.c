/* Test program to trigger early rematerialization emit_copy logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barriers */
static volatile int volatile_counter = 0;
static int get_volatile(void) {
    return volatile_counter++;
}

/* Non-inlineable function with __attribute__((noinline)) */
static __attribute__((noinline,noipa))
long long test_remat(int start, int iterations, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_ll[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val * i;
        arr_ll[i] = (long long)(init_val * 1000) + i;
    }
    
    /* Mix of integer and FP variables */
    double fp_acc = init_val;
    long long int_acc = (long long)init_val;
    v4df vec_acc = {init_val, init_val + 1.0, init_val + 2.0, init_val + 3.0};
    v4di vec_int = {int_acc, int_acc + 1, int_acc + 2, int_acc + 3};
    
    /* Outer loop */
    for (int outer = start; outer < iterations; outer++) {
        /* Loop-invariant variable for inner condition */
        int invariant = get_volatile() % 2;
        
        /* Inner if with complex expression */
        if (invariant > 0) {
            /* Complex expression with register pressure */
            /* Use __builtin_shuffle to create virtual registers */
            v4df temp_vec1 = vec_acc + (v4df){1.0, 2.0, 3.0, 4.0};
            v4df temp_vec2 = temp_vec1 * (v4df){0.5, 1.5, 2.5, 3.5};
            
            /* Shuffle operations - often create virtual registers */
            v4df shuffled = __builtin_shuffle(temp_vec1, temp_vec2, 
                (v4di){0, 1, 2, 3});
            
            /* More intermediate vector results */
            v4df temp_vec3 = shuffled + vec_acc;
            v4df temp_vec4 = temp_vec3 * temp_vec2;
            
            /* Mix integer and FP operations */
            int idx1 = (outer * 7) % 16;
            int idx2 = (outer * 13) % 16;
            int idx3 = (outer * 11) % 16;
            
            /* Non-constant array accesses */
            double val1 = arr_dbl[idx1];
            double val2 = arr_dbl[idx2];
            long long ival1 = arr_ll[idx3];
            
            /* Complex expression with many temporaries */
            double tmp1 = val1 * val2 + fp_acc;
            double tmp2 = tmp1 / (val1 + 1.0);
            double tmp3 = tmp2 * tmp1 - val2;
            double tmp4 = tmp3 + (double)ival1;
            double tmp5 = tmp4 * tmp3 / tmp2;
            
            /* Integer operations mixed in */
            long long itmp1 = (long long)tmp5 + ival1;
            long long itmp2 = itmp1 * (outer + 1);
            long long itmp3 = itmp2 - (long long)(tmp4 * 100.0);
            long long itmp4 = itmp3 / (ival1 > 0 ? ival1 : 1);
            
            /* More vector operations */
            v4di vec_tmp1 = vec_int + (v4di){itmp1, itmp2, itmp3, itmp4};
            v4di vec_tmp2 = vec_tmp1 * (v4di){2, 3, 4, 5};
            v4di vec_tmp3 = __builtin_shuffle(vec_tmp1, vec_tmp2,
                (v4di){1, 0, 3, 2});
            
            /* Update accumulators with complex expressions */
            fp_acc = tmp5 + (double)itmp4 + (double)(vec_tmp3[0] % 1000);
            int_acc = itmp4 + (long long)tmp5 + vec_tmp3[1];
            
            /* Update vectors */
            vec_acc = temp_vec4 + (v4df){fp_acc, fp_acc * 0.5, 
                                         fp_acc * 0.25, fp_acc * 0.125};
            vec_int = vec_tmp3 + (v4di){int_acc, int_acc * 2, 
                                        int_acc * 3, int_acc * 4};
            
            /* Another volatile barrier */
            get_volatile();
        } else {
            /* Alternative path to create control flow complexity */
            double alt_tmp = fp_acc * 0.333;
            long long alt_int = int_acc / 3;
            
            /* More operations to keep values live */
            for (int j = 0; j < 4; j++) {
                alt_tmp += arr_dbl[(outer + j) % 16];
                alt_int += arr_ll[(outer + j * 3) % 16];
            }
            
            fp_acc = alt_tmp;
            int_acc = alt_int;
        }
        
        /* Cross-iteration dependencies */
        arr_dbl[outer % 16] = fp_acc;
        arr_ll[outer % 16] = int_acc;
        
        /* Periodic complex computation */
        if (outer % 7 == 0) {
            v4df periodic_vec = vec_acc * (v4df){0.9, 0.8, 0.7, 0.6};
            v4di periodic_int = vec_int + (v4di){outer, outer*2, outer*3, outer*4};
            
            /* Shuffle with computation */
            v4df shuffled_periodic = __builtin_shuffle(
                periodic_vec, vec_acc, (v4di){3, 2, 1, 0});
            
            vec_acc = shuffled_periodic;
            vec_int = periodic_int;
            
            get_volatile(); /* Another barrier */
        }
    }
    
    /* Final reduction */
    double final_fp = fp_acc;
    for (int i = 0; i < 4; i++) {
        final_fp += vec_acc[i];
    }
    
    long long final_int = int_acc;
    for (int i = 0; i < 4; i++) {
        final_int += vec_int[i];
    }
    
    return final_int + (long long)final_fp;
}

int main(void) {
    long long total = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i % 10, 50 + (i % 20), 1.0 + i * 0.1);
        
        /* Prevent dead code elimination */
        if (i % 23 == 0) {
            printf("Progress: %d, total so far: %lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    return 0;
}
