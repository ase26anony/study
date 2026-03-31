/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))
#define NOCLONE __attribute__((noclone))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function with many arguments */
NOINLINE NOCLONE int use_many_values(int a, int b, int c, float d, 
                                     double e, long f, short g, char h) {
    return a + b + c + (int)d + (int)e + (int)f + g + h;
}

/* Another non-inline function for floating point */
NOINLINE NOCLONE float use_floats(float a, float b, float c, 
                                  float d, float e, float f) {
    return a * b + c * d - e / f;
}

/* Function to create complex expressions */
NOINLINE NOCLONE double complex_expr(double base, int iter) {
    /* Many intermediate computations */
    double t1 = base * 1.1 + iter * 0.5;
    double t2 = base / 1.3 - iter * 0.7;
    double t3 = t1 * t2 + base;
    double t4 = t2 / t1 - base;
    double t5 = t3 * t4 + iter * 0.3;
    double t6 = t4 / t3 - iter * 0.2;
    double t7 = t5 * t6 + base * 0.9;
    double t8 = t6 / t5 - base * 0.8;
    
    return t7 + t8;
}

int main(void) {
    /* Initialize arrays with volatile to prevent lifting */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    v4si vec_acc = {0, 0, 0, 0};
    
    /* Main computational kernel - designed for high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Load volatile values - forces memory ops */
            int v1 = array1[i];
            float v2 = array2[i];
            double v3 = array3[i];
            
            /* Many independent arithmetic operations creating temporaries */
            /* Each expression is slightly different to avoid CSE */
            int t1 = v1 * 3 + outer * 7;
            int t2 = v1 / 2 - outer * 3;
            int t3 = t1 * t2 + i * 5;
            int t4 = t2 / (t1 + 1) - i * 2;
            int t5 = t3 * t4 + outer * 11;
            int t6 = t4 / (t3 + 1) - outer * 13;
            int t7 = t5 * t6 + i * 17;
            int t8 = t6 / (t5 + 1) - i * 19;
            
            float f1 = v2 * 1.1f + outer * 0.7f;
            float f2 = v2 / 1.3f - outer * 0.3f;
            float f3 = f1 * f2 + i * 0.5f;
            float f4 = f2 / (f1 + 0.1f) - i * 0.2f;
            float f5 = f3 * f4 + outer * 0.9f;
            float f6 = f4 / (f3 + 0.1f) - outer * 0.4f;
            float f7 = f5 * f6 + i * 0.8f;
            float f8 = f6 / (f5 + 0.1f) - i * 0.6f;
            
            double d1 = v3 * 1.01 + outer * 0.07;
            double d2 = v3 / 1.03 - outer * 0.03;
            double d3 = d1 * d2 + i * 0.05;
            double d4 = d2 / (d1 + 0.01) - i * 0.02;
            double d5 = d3 * d4 + outer * 0.09;
            double d6 = d4 / (d3 + 0.01) - outer * 0.04;
            double d7 = d5 * d6 + i * 0.08;
            double d8 = d6 / (d5 + 0.01) - i * 0.06;
            
            /* Vector operations - consume SIMD registers */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            vec_acc += vec5;
            
            v4sf vecf1 = {f1, f2, f3, f4};
            v4sf vecf2 = {f5, f6, f7, f8};
            v4sf vecf3 = vecf1 + vecf2;
            v4sf vecf4 = vecf1 * vecf2;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 - clobber commonly used registers */
            asm volatile(
                "# Force register pressure\n"
                "movl $0, %%eax\n"
                "movl $0, %%ebx\n"
                "movl $0, %%ecx\n"
                "movl $0, %%edx\n"
                "movl $0, %%esi\n"
                "movl $0, %%edi\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
            
            /* Call non-inline function with many arguments */
            int result1 = use_many_values(t1, t2, t3, f1, d1, 
                                         (long)t4, (short)t5, (char)t6);
            
            /* More computations after function call */
            int t9 = t7 * t8 + result1;
            int t10 = t8 / (t7 + 1) - result1;
            
            float result2 = use_floats(f2, f3, f4, f5, f6, f7);
            float f9 = f7 * f8 + result2;
            float f10 = f8 / (f7 + 0.1f) - result2;
            
            double result3 = complex_expr(d2, i);
            double d9 = d7 * d8 + result3;
            double d10 = d8 / (d7 + 0.01) - result3;
            
            /* Another inline assembly to break live ranges */
            asm volatile(
                "# Clobber more registers\n"
                "pxor %%xmm0, %%xmm0\n"
                "pxor %%xmm1, %%xmm1\n"
                "pxor %%xmm2, %%xmm2\n"
                "pxor %%xmm3, %%xmm3\n"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* More independent computations */
            int t11 = t9 * t10 + outer * 23;
            int t12 = t10 / (t9 + 1) - outer * 29;
            int t13 = t11 * t12 + i * 31;
            int t14 = t12 / (t11 + 1) - i * 37;
            
            float f11 = f9 * f10 + outer * 1.3f;
            float f12 = f10 / (f9 + 0.1f) - outer * 1.7f;
            float f13 = f11 * f12 + i * 2.3f;
            float f14 = f12 / (f11 + 0.1f) - i * 2.9f;
            
            double d11 = d9 * d10 + outer * 1.03;
            double d12 = d10 / (d9 + 0.01) - outer * 1.07;
            double d13 = d11 * d12 + i * 1.13;
            double d14 = d12 / (d11 + 0.01) - i * 1.17;
            
            /* Write to volatile sinks */
            global_sink = t13 + t14;
            float_sink = f13 + f14;
            double_sink = d13 + d14;
            
            /* Accumulate results */
            total_int += t13 + t14;
            total_float += f13 + f14;
            total_double += d13 + d14;
            
            /* Another function call with different arguments */
            use_many_values(t13, t14, result1, f13, d13,
                           (long)global_sink, (short)outer, (char)i);
        }
        
        /* Additional computation between outer loop iterations */
        asm volatile(
            "# Reset registers between outer iterations\n"
            "xor %%r8d, %%r8d\n"
            "xor %%r9d, %%r9d\n"
            "xor %%r10d, %%r10d\n"
            "xor %%r11d, %%r11d\n"
            :
            :
            : "r8", "r9", "r10", "r11", "memory"
        );
    }
    
    /* Final aggregation to prevent optimization */
    int vec_sum = vec_acc[0] + vec_acc[1] + vec_acc[2] + vec_acc[3];
    total_int += vec_sum;
    
    printf("Results: int=%d, float=%f, double=%f\n", 
           total_int, total_float, total_double);
    
    return 0;
}
