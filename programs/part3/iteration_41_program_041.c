/* test-early-remat.c
 * Program designed to trigger filter_old_remats logic in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat test-early-remat.c -o test-early-remat
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_iter = 100;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates that become "old remats" */
static volatile int test_remat(volatile int a, volatile int b, volatile int c) {
    /* Local variables for register pressure */
    int local_array[64];
    volatile int result = 0;
    
    /* Initialize local array */
    for (int i = 0; i < 64; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of candidates */
    for (int iter = 0; iter < vol_iter; iter++) {
        /* BLOCK A: Compute rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = a + 5;  /* arg + constant */
        int cand2 = b * 2;  /* arg * 2 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand3 = &local_array[c + 3];  /* &array[arg + constant] */
        
        /* Candidate 3: More complex but still recomputable expression */
        int cand4 = (a * b) + (c >> 1);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand4;
        
        /* Control flow: conditional jump to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + a;
            int t3 = t2 * b;
            int t4 = t3 - c;
            int t5 = t4 >> 2;
            int t6 = t5 * 3;
            int t7 = t6 + 100;
            int t8 = t7 - a;
            int t9 = t8 * b;
            int t10 = t9 / (c + 1);
            long t11 = t10 * 1000L;
            long t12 = t11 + 5000L;
            long t13 = t12 - 2000L;
            long t14 = t13 * 2L;
            long t15 = t14 / 3L;
            
            /* Floating point variables for FP register pressure */
            float f1 = t1 * 0.5f;
            float f2 = f1 + 1.5f;
            float f3 = f2 * 2.0f;
            float f4 = f3 - 0.25f;
            float f5 = f4 / 1.75f;
            double d1 = t11 * 0.01;
            double d2 = d1 + 3.14159;
            double d3 = d2 * 2.71828;
            double d4 = d3 - 1.41421;
            double d5 = d4 / 1.73205;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, 2.0f, 3.0f, 4.0f};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
            
            v2df dvec1 = {d1, d2};
            v2df dvec2 = {d3, d4};
            v2df dvec3 = dvec1 + dvec2;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent DCE */
            result += t15;
            result += (int)f5;
            result += (int)d5;
            #ifdef __SSE2__
            result += vec5[0] + vec5[1];
            result += (int)fvec3[0];
            result += (int)dvec3[0];
            #endif
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand2;  /* Use cand2 which may now be an "old remat" */
            result += cand1 * 2;
            result += *cand3 + 1;
            result += cand4 - 5;
            
            /* More operations to ensure values are live across blocks */
            int final1 = cand1 + cand2;
            int final2 = *cand3 + cand4;
            result += final1 + final2;
        }
        
        /* Alternate path to create more complex CFG */
        if (iter % 10 == 0) {
            /* Use candidates in different context */
            result -= cand1;
            result += cand2 * 3;
        }
    }
    
    return result;
}

/* Second function with different pattern to increase LTO opportunities */
static volatile int test_remat2(volatile int x, volatile int y) {
    int array[32];
    volatile int res = 0;
    
    for (int i = 0; i < 32; i++) {
        array[i] = i + x;
    }
    
    for (int i = 0; i < 50; i++) {
        /* More remat candidates */
        int rc1 = x + i;
        int *rc2 = &array[y + 2];
        int rc3 = (x * y) + i;
        
        res += rc1;
        
        if (vol_cond) {
            /* High pressure block */
            int p1 = rc1 * 2, p2 = p1 + 1, p3 = p2 * 3, p4 = p3 - 1;
            int p5 = p4 >> 2, p6 = p5 * 5, p7 = p6 + 10, p8 = p7 - 5;
            float pf1 = p1 * 0.1f, pf2 = pf1 + 0.2f, pf3 = pf2 * 0.3f;
            double pd1 = p2 * 0.01, pd2 = pd1 + 0.02, pd3 = pd2 * 0.03;
            
            asm volatile("" ::: "memory");
            
            res += p8 + (int)pf3 + (int)pd3;
            res += *rc2 + rc3;
        }
    }
    
    return res;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test functions multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly to prevent complete optimization */
        vol_arg1 = 10 + (i % 5);
        vol_arg2 = 20 + (i % 3);
        vol_arg3 = 30 + (i % 7);
        
        total += test_remat(vol_arg1, vol_arg2, vol_arg3);
        total += test_remat2(vol_arg1, vol_arg2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Also test with LTO-friendly compilation */
    #ifdef __GNUC__
    printf("Compiled with GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #endif
    
    return 0;
}
