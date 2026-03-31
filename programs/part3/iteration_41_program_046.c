/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 12345;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i + global_seed;
    }
    
    /* Volatile result to prevent optimization */
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    volatile int loop_counter = 100;
    volatile int always_true = 1;
    
    for (int iter = 0; iter < loop_counter; iter++) {
        /* --- BLOCK A: Create rematerialization candidates --- */
        
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 10;  /* arg1 + constant */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2 + arg3;
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4 + 5];  /* &array[arg4 + 5] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg2) - (arg3 << 2);
        
        /* Immediate use of candidates in Block A */
        result += cand1;
        result += *cand3;
        result += cand2 + cand4;
        
        /* --- Conditional jump to split live ranges --- */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* --- BLOCK B: High register pressure region --- */
            
            /* Many distinct local variables to create register pressure */
            int t1 = result + 1;
            int t2 = t1 * arg1;
            long t3 = t2 + arg2;
            long t4 = t3 - arg3;
            float t5 = t4 * 0.5f;
            double t6 = t5 + arg4;
            double t7 = t6 * 1.1;
            int t8 = (int)t7;
            long t9 = t8 * 3L;
            float t10 = t9 * 0.25f;
            double t11 = t10 + 1.0;
            int t12 = (int)t11;
            long t13 = t12 ^ arg1;
            float t14 = t13 * 0.33f;
            double t15 = t14 - 0.5;
            
            /* More variables to increase pressure */
            int u1 = arg1 + iter;
            int u2 = arg2 - iter;
            int u3 = arg3 * iter;
            int u4 = arg4 / (iter + 1);
            long u5 = u1 + u2;
            long u6 = u3 - u4;
            float u7 = u5 * 1.5f;
            double u8 = u6 * 0.75;
            int u9 = (int)u7;
            int u10 = (int)u8;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {u1, u2, u3, u4};
            v4si vec2 = {u5, u6, u9, u10};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {t5, t10, t14, u7};
            v4sf fvec2 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf fvec3 = fvec1 * fvec2;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to keep them alive */
            result += t1 + t2 + t8 + t12;
            result += (int)t3 + (int)t4;
            result += (int)t5 + (int)t6 + (int)t7;
            result += (int)t9 + (int)t10 + (int)t11;
            result += (int)t13 + (int)t14 + (int)t15;
            result += u1 + u2 + u3 + u4 + u9 + u10;
            
            #ifdef __SSE2__
            /* Use vector results */
            for (int i = 0; i < 4; i++) {
                result += vec3[i] + vec4[i] + vec5[i];
                result += (int)fvec3[i];
            }
            #endif
            
            /* --- BLOCK C: Use candidates again after high pressure --- */
            
            /* Force reuse of rematerialization candidates */
            result += cand1 * 2;      /* cand1 used again */
            result += cand2 - arg1;   /* cand2 used again */
            result += *cand3 + iter;  /* cand3 used again */
            result += cand4 >> 1;     /* cand4 used again */
            
            /* Additional uses in different contexts */
            if (cand1 > 0) {
                result += cand2;
            }
            
            result += (cand3 - local_array) * 2;
        }
        
        /* Alternate path to create control flow complexity */
        if (global_seed & 1) {
            /* Use candidates in different basic block */
            result -= cand1;
            result += cand4;
        }
    }
    
    return result;
}

/* Main function with loop */
int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile long total = 0;
    
    /* Create different argument patterns */
    for (int i = 0; i < iterations; i++) {
        volatile int arg1 = global_seed + i;
        volatile int arg2 = global_seed * 2 - i;
        volatile int arg3 = global_seed / (i + 1) + 7;
        volatile int arg4 = (global_seed ^ i) & 0xFF;
        
        /* Call test function */
        total += test_remat(arg1, arg2, arg3, arg4);
        
        /* Modify global seed to change recomputable expressions */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %ld\n", (long)total);
    
    /* Use result in system call to prevent optimization */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
