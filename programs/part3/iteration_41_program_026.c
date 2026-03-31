/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force values to be recomputable but not constant-folded */
static volatile int global_volatile = 0;

/* Vector extensions for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of candidates */
    volatile int loop_counter = global_volatile ? 0 : 100;
    for (int iter = 0; iter < loop_counter; iter++) {
        /* --- BLOCK A: Create rematerialization candidates --- */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 10;  /* arg1 + constant */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 3 - 5;
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 2];  /* &array[arg + const] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg4 << 2) | 0xF;
        
        /* Immediate use of candidates in block A */
        result += cand1;
        result += *cand3;
        result += cand2 ^ cand4;
        
        /* --- Conditional jump to split live ranges --- */
        /* Use volatile condition to prevent optimization */
        if (global_volatile == 0) {  /* Always true at runtime */
            /* --- BLOCK B: High register pressure region --- */
            /* Many distinct local variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            int t3 = t2 - arg2;
            int t4 = t3 * arg3;
            int t5 = t4 / (arg4 + 1);
            long t6 = t5 * 1000L;
            long t7 = t6 + 12345L;
            long t8 = t7 - 6789L;
            float f1 = t8 * 0.5f;
            float f2 = f1 + 3.14f;
            float f3 = f2 * 2.0f;
            double d1 = f3 * 1.618;
            double d2 = d1 / 3.14159;
            double d3 = d2 + 2.71828;
            int t9 = (int)d3;
            int t10 = t9 ^ 0xABCDEF;
            
            /* More variables for additional pressure */
            int t11 = t10 << 3;
            int t12 = t11 >> 1;
            int t13 = t12 | 0xFF;
            int t14 = t13 & 0x7F;
            int t15 = t14 + iter;
            int t16 = t15 * 3;
            int t17 = t16 - 7;
            int t18 = t17 % 13;
            int t19 = t18 ^ t1;
            int t20 = t19 | t2;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {f1, f2, f3, (float)d1};
            v4sf fvec2 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf fvec3 = fvec1 * fvec2;
            v4sf fvec4 = fvec1 + fvec2;
            
            /* Use vector results */
            int *vp = (int*)&vec5;
            t20 += vp[0] + vp[1] + vp[2] + vp[3];
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            result += t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
            
            /* --- BLOCK C: Use candidates again after pressure --- */
            /* This forces compiler to reconsider rematerialization */
            result += cand1 * 2;          /* cand1 used again */
            result += cand2 >> 1;         /* cand2 used again */
            result += *cand3 + 5;         /* cand3 used again */
            result += cand4 & 0xAA;       /* cand4 used again */
            
            /* Additional use with different computation */
            result += (cand1 + cand2) * (cand4 - *cand3);
        } else {
            /* Never executed but needed for control flow */
            result += arg1 + arg2 + arg3 + arg4;
        }
    }
    
    return result;
}

/* Main function with loop */
int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int final_result = 0;
    
    /* Call test function multiple times with different volatile args */
    for (int i = 0; i < iterations; i++) {
        /* Use different arguments each iteration */
        volatile int arg1 = i * 3 + 1;
        volatile int arg2 = i * 5 + 2;
        volatile int arg3 = i * 7 + 3;
        volatile int arg4 = i * 11 + 4;
        
        final_result += test_remat(arg1, arg2, arg3, arg4);
        
        /* Modify global volatile to affect control flow */
        if (i % 7 == 0) {
            global_volatile = i;
        }
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", final_result);
    
    return 0;
}
