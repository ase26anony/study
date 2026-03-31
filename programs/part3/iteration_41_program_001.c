/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force values to be recomputable but not constant-folded */
static volatile int vol_cond = 1;
static volatile int vol_arg_seed = 42;

/* Vector extensions for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    int local_array[100];
    
    /* Initialize local array */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 10;  /* arg1 + 10 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg2 + 5];  /* &local_array[arg2 + 5] */
        
        /* Candidate 3: More complex but still recomputable */
        int cand3 = (arg3 * 2) + (arg4 >> 1);
        
        /* Candidate 4: Pointer arithmetic */
        int *cand4 = local_array + (arg1 * 3);
        
        /* Immediate use in BLOCK A */
        result += *cand2;
        result += cand1;
        result += cand3;
        result += *cand4;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many distinct local variables to consume registers */
            int t1 = arg1 * arg2;
            int t2 = arg2 + arg3;
            int t3 = arg3 - arg4;
            int t4 = arg4 * arg1;
            long t5 = (long)arg1 * arg2 * arg3;
            long t6 = t5 + arg4;
            float t7 = (float)arg1 * 1.5f;
            float t8 = (float)arg2 * 2.5f;
            double t9 = (double)arg3 * 3.14159;
            double t10 = (double)arg4 * 2.71828;
            int t11 = t1 + t2;
            int t12 = t3 * t4;
            long t13 = t5 - t6;
            float t14 = t7 + t8;
            double t15 = t9 * t10;
            
            /* Vector operations for additional pressure */
            #ifdef __SSE2__
            v4si vec1 = {arg1, arg2, arg3, arg4};
            v4si vec2 = {10, 20, 30, 40};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {(float)arg1, (float)arg2, (float)arg3, (float)arg4};
            v4sf fvec2 = {1.1f, 2.2f, 3.3f, 4.4f};
            v4sf fvec3 = fvec1 * fvec2;
            
            v2df dvec1 = {(double)arg1, (double)arg2};
            v2df dvec2 = {3.14, 6.28};
            v2df dvec3 = dvec1 + dvec2;
            #endif
            
            /* More scalar variables */
            int t16 = t11 * t12;
            long t17 = t13 + 1000;
            float t18 = t14 * 2.0f;
            double t19 = t15 / 2.0;
            int t20 = t16 + (int)t17;
            float t21 = t18 + (float)t19;
            double t22 = t19 + (double)t21;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += t1 + t2 + t3 + t4 + t11 + t12 + t16 + t20;
            result += (int)(t5 + t6 + t13 + t17);
            result += (int)(t7 + t8 + t14 + t18 + t21);
            result += (int)(t9 + t10 + t15 + t19 + t22);
            
            #ifdef __SSE2__
            /* Use vector results */
            int *vptr = (int*)&vec5;
            result += vptr[0] + vptr[1] + vptr[2] + vptr[3];
            float *fptr = (float*)&fvec3;
            result += (int)(fptr[0] + fptr[1] + fptr[2] + fptr[3]);
            #endif
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces reconsideration of rematerialization */
        result += cand1 * 2;
        result += *cand2 + 5;
        result += cand3 / 2;
        result += *cand4 * 3;
        
        /* Additional use in different context */
        if (iter % 2 == 0) {
            result += cand1 + cand3;
        } else {
            result += *cand2 - *cand4;
        }
    }
    
    return result;
}

/* Main function with loop to accumulate results */
int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int final_result = 0;
    
    /* Vary arguments to prevent constant propagation */
    volatile int arg_base = vol_arg_seed;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Changing arguments create different but recomputable values */
        int arg1 = arg_base + i;
        int arg2 = arg_base * 2 + i;
        int arg3 = arg_base + i * 3;
        int arg4 = arg_base * 3 - i;
        
        final_result += test_remat(arg1, arg2, arg3, arg4);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : "+r" (final_result));
    }
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
