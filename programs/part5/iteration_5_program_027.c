/* Compile with: gcc -O2 -fno-inline -fdump-rtl-all -fno-schedule-insns test.c */
/* Or: gcc -O3 -fsanitize=undefined -fno-omit-frame-pointer test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile function to create code motion barrier */
static volatile int barrier;

/* Non-inlineable function to force register pressure */
static __attribute__((noinline,noipa))
long long test_remat(int start, int n, double init_val) {
    /* Large vectors to create virtual registers */
    typedef double v4df __attribute__((vector_size(32)));
    typedef long long v4di __attribute__((vector_size(32)));
    
    /* Local arrays with non-constant indexing */
    double arr_dbl[16];
    long long arr_int[16];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        arr_dbl[i] = init_val + i;
        arr_int[i] = start + i * 3;
    }
    
    /* Loop-invariant variable for control flow */
    int invariant = start * 2 + 1;
    
    /* Accumulators mixing types */
    double sum_dbl = 0.0;
    long long sum_int = 0;
    v4df vec_sum = {0.0, 0.0, 0.0, 0.0};
    
    /* Outer loop */
    for (int iter = 0; iter < n; iter++) {
        /* Complex expression with many temporaries */
        double temp1 = arr_dbl[iter & 0xF] * 1.5;
        double temp2 = temp1 + arr_dbl[(iter + 1) & 0xF];
        
        /* Code motion barrier */
        barrier = iter;
        
        /* More temporaries */
        long long temp3 = arr_int[iter & 0xF] * 2LL;
        double temp4 = (double)temp3 / 3.0;
        
        /* Vector operations that create virtual registers */
        v4df vec1 = {temp1, temp2, temp4, arr_dbl[iter & 0xF]};
        v4df vec2 = {arr_dbl[(iter + 2) & 0xF], arr_dbl[(iter + 3) & 0xF],
                     arr_dbl[(iter + 4) & 0xF], arr_dbl[(iter + 5) & 0xF]};
        
        /* Shuffle operation - often creates virtual registers */
        v4df vec_shuffled = __builtin_shuffle(vec1, vec2, 
            (v4di){2, 1, 6, 3});
        
        /* Inner if with loop-invariant condition */
        if (iter % invariant != 0) {
            /* Register pressure inducing block */
            
            /* Many intermediate calculations */
            double a = vec_shuffled[0] * 2.0;
            double b = vec_shuffled[1] * 3.0;
            double c = vec_shuffled[2] * 4.0;
            double d = vec_shuffled[3] * 5.0;
            
            /* More temporaries with mixing */
            long long ai = (long long)a;
            long long bi = (long long)b;
            long long ci = (long long)c;
            long long di = (long long)d;
            
            /* Complex expression chain */
            double e = (a + b) * (c - d);
            long long f = (ai * bi) + (ci - di);
            
            /* Another barrier */
            barrier = (int)f;
            
            /* More vector operations */
            v4df vec3 = {a, b, c, d};
            v4df vec4 = {e, (double)f, a * 0.5, b * 0.25};
            
            /* Another shuffle */
            v4df vec_shuffled2 = __builtin_shuffle(vec3, vec4,
                (v4di){1, 5, 2, 6});
            
            /* Use all temporaries in final calculation */
            sum_dbl += e + vec_shuffled2[0] + vec_shuffled2[1];
            sum_int += f + (long long)vec_shuffled2[2] + (long long)vec_shuffled2[3];
            
            /* Update array with non-constant index */
            arr_dbl[iter & 0xF] = e * 0.1;
            arr_int[iter & 0xF] = f & 0xFFFF;
        }
        
        /* Outside if but still using temporaries */
        vec_sum += vec_shuffled;
        
        /* More mixing of types */
        sum_dbl += temp1 + temp2 + temp4;
        sum_int += temp3 + (long long)(temp1 * 100.0);
    }
    
    /* Final reduction */
    double final_vec_sum = vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    
    /* Return mixed type result */
    return sum_int + (long long)sum_dbl + (long long)final_vec_sum;
}

int main(void) {
    long long total = 0;
    
    /* Call multiple times with different arguments */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i % 10, 50 + (i % 20), 1.0 + i * 0.1);
        
        /* Prevent dead code elimination */
        if (i % 23 == 0) {
            printf("Progress: %d, total so far: %lld\n", i, total);
        }
    }
    
    printf("Final result: %lld\n", total);
    
    /* Deterministic output check */
    if (total == 0) {
        printf("WARNING: Result is zero - check optimization\n");
    }
    
    return 0;
}
