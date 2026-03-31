/* early-remat-test.c - Program to trigger early rematerialization coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Non-inline function with many arguments */
NOINLINE int use_many_values(int a, int b, int c, float d, double e, 
                             long f, short g, char h, v4si vi, v4sf vf) {
    /* Force computation to prevent elimination */
    int result = a + b - c + (int)d + (int)e + (int)f + g + h;
    
    /* Use vector components */
    result += vi[0] + vi[1] + vi[2] + vi[3];
    result += (int)vf[0] + (int)vf[1] + (int)vf[2] + (int)vf[3];
    
    global_sink = result;
    return result;
}

/* Another non-inline function for floating point pressure */
NOINLINE double compute_pressure(double a, double b, double c, double d,
                                 double e, double f, double g, double h) {
    /* Complex expression that can't be easily optimized */
    double t1 = a * b + c / (d + 1.0);
    double t2 = e * f - g * h;
    double t3 = t1 * t2 / (a + b + c + d);
    double t4 = (e - f) * (g - h) / (t1 + 1.0);
    
    double_sink = t3 + t4;
    return t3 * t4;
}

/* Main computational kernel */
int main(void) {
    /* Initialize arrays with volatile to prevent lifting */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    long long total = 0;
    
    /* Nested loops for maximum register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Create many independent computations with unique expressions */
            /* Each computation uses different operations and constants */
            
            /* Integer computations - each creates temporaries */
            int t1 = array1[i] * 3 + outer;
            int t2 = array1[(i + 1) % 256] / 2 - outer;
            int t3 = t1 * t2 + i * 7;
            int t4 = (t1 - t2) * (i + outer);
            int t5 = t3 % (t4 + 1) + array1[(i + 2) % 256];
            int t6 = (t5 << 3) | (t4 >> 2);
            int t7 = t6 ^ t3 ^ t2;
            int t8 = t7 * 0x9e3779b9 + i;
            
            /* Floating point computations - consume FP registers */
            float f1 = array2[i] * 1.2345f + outer * 0.9876f;
            float f2 = array2[(i + 3) % 256] / 0.5432f - i * 0.1234f;
            float f3 = f1 * f2 + array2[(i + 5) % 256];
            float f4 = (f1 - f2) * (f3 + 1.0f);
            float f5 = f3 / (f4 + 0.001f) * array2[(i + 7) % 256];
            
            /* Double precision - more register pressure */
            double d1 = array3[i] * 2.34567 + outer * 1.23456;
            double d2 = array3[(i + 4) % 256] / 0.45678 - i * 0.34567;
            double d3 = d1 * d2 + array3[(i + 6) % 256];
            double d4 = (d1 - d2) * (d3 + 1.0);
            double d5 = d3 / (d4 + 0.00001) * array3[(i + 8) % 256];
            double d6 = d4 * d5 - d1 / (d2 + 0.5);
            double d7 = d6 * 3.14159 + d3 * 2.71828;
            
            /* Vector operations - consume SIMD registers */
            v4si vi1 = {t1, t2, t3, t4};
            v4si vi2 = {t5, t6, t7, t8};
            v4si vi3 = vi1 + vi2 * 2;
            v4si vi4 = vi1 - vi2 / 3;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {f5, f1 * 2.0f, f2 / 3.0f, f3 * 4.0f};
            v4sf vf3 = vf1 + vf2 * 1.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 - clobber general purpose, SSE, and memory */
            asm volatile("" 
                : /* no outputs */
                : /* no inputs */ 
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_many_values(t1, t2, t3, f1, d1, 
                                             (long)t4, (short)t5, (char)t6,
                                             vi3, vf3);
            
            /* More computations after function call */
            int t9 = t8 * func_result + i * 11;
            float f6 = f5 * (float)func_result + i * 0.333f;
            double d8 = d7 * (double)func_result + i * 0.777;
            
            /* Another inline assembly to break live ranges */
            asm volatile("" ::: "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3", "memory");
            
            /* Call another function with many FP arguments */
            double fp_result = compute_pressure(d1, d2, d3, d4, d5, d6, d7, d8);
            
            /* More independent computations */
            int t10 = t9 ^ (int)(f6 * 100.0f) ^ (int)(fp_result * 10.0);
            float f7 = f6 + (float)t10 * 0.01f + (float)fp_result;
            double d9 = fp_result * 0.5 + d8 * 0.25 + (double)t10 * 0.125;
            
            /* Vector operations mixed with scalar */
            v4si vi5 = vi3 + vi4 * (i % 16);
            v4sf vf4 = vf3 * (float)(i % 8) + vf1;
            
            /* Volatile memory writes - prevent optimization */
            global_sink = t10;
            float_sink = f7;
            double_sink = d9;
            
            /* Accumulate results with complex expression */
            total += (long long)t10 + (long long)(f7 * 100.0f) + 
                    (long long)(d9 * 10.0) + vi5[0] + vi5[1] + 
                    (int)(vf4[0] * 10.0f);
            
            /* Additional pressure: compute address with complex indexing */
            int idx = (i * 13 + outer * 17) % 256;
            volatile int *ptr1 = &array1[idx];
            volatile float *ptr2 = &array2[(idx + 5) % 256];
            volatile double *ptr3 = &array3[(idx + 10) % 256];
            
            /* Use pointer values in computations */
            int t11 = *ptr1 + t10;
            float f8 = *ptr2 + f7;
            double d10 = *ptr3 + d9;
            
            /* Final assembly clobber */
            asm volatile("" ::: "rax", "xmm0", "memory");
            
            /* Use computed values */
            global_sink = t11;
            float_sink = f8;
            double_sink = d10;
            
            total += t11 + (int)(f8 * 10.0f) + (int)(d10 * 5.0);
        }
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
