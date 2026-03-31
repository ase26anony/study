/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent function inlining to force argument passing */
__attribute__((noinline)) 
int use_values(int a, int b, float c, double d, int e, int f, float g, double h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    return sink;
}

/* Another noinline function with mixed types */
__attribute__((noinline))
double complex_calc(double x, double y, float z, int w, 
                    long a, short b, char c, unsigned d) {
    volatile double result;
    result = (x * y) / (z + 1.0) + (w * a) / (b + c - d);
    return result;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile memory for anti-optimization */
volatile int global_sink;
volatile double global_double_sink;

int main(void) {
    /* Initialize arrays to feed computations */
    double arr_d[256];
    float arr_f[256];
    int arr_i[256];
    
    for (int i = 0; i < 256; i++) {
        arr_d[i] = (i * 1.5) / (i + 1);
        arr_f[i] = (i * 0.75f) / (i + 2);
        arr_i[i] = i * 3 - 7;
    }
    
    /* Variables to accumulate results */
    double total_d = 0.0;
    float total_f = 0.0f;
    int total_i = 0;
    
    /* Vector accumulators */
    v4si vec_acc_i = {0, 0, 0, 0};
    v4sf vec_acc_f = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Nested loops to maximize register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 256; inner++) {
            /* Create many independent temporary computations */
            /* Each computation uses different expressions to avoid CSE */
            
            /* Integer computations */
            int t1 = arr_i[inner] * outer + inner;
            int t2 = arr_i[(inner + 1) % 256] / (outer + 1) - inner;
            int t3 = t1 * t2 + (inner << 2);
            int t4 = (t1 ^ t2) | (inner & 0xFF);
            int t5 = t3 - t4 * (outer % 16);
            int t6 = (t5 << 3) / (inner + 1) + (outer * 7);
            
            /* Floating-point computations */
            float f1 = arr_f[inner] * outer * 0.5f;
            float f2 = arr_f[(inner + 3) % 256] / (outer + 2.0f);
            float f3 = f1 + f2 * (inner * 0.25f);
            float f4 = (f1 - f2) / (inner + 3.0f) * outer;
            float f5 = f3 * f4 - (inner * 0.1f);
            float f6 = f5 / (outer + 1.0f) + arr_f[(inner + 5) % 256];
            
            /* Double precision computations */
            double d1 = arr_d[inner] * outer * 0.75;
            double d2 = arr_d[(inner + 7) % 256] / (outer * 0.5 + 1.0);
            double d3 = d1 * d2 + inner * 0.25;
            double d4 = (d1 - d2) / (inner * 0.125 + 1.0);
            double d5 = d3 * d4 - outer * 0.333;
            double d6 = d5 / (inner + 2.0) + arr_d[(inner + 11) % 256];
            
            /* Vector operations */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, inner, outer};
            v4si vec3 = vec1 + vec2 * (inner % 8);
            v4si vec4 = vec1 - vec2 / ((outer % 4) + 1);
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {f5, f6, arr_f[inner], arr_f[(inner + 1) % 256]};
            v4sf vf3 = vf1 * vf2 + (inner * 0.01f);
            v4sf vf4 = vf1 / (vf2 + 0.001f) * outer;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber commonly used registers */
            asm volatile(
                "# Clobber some registers\n"
                "mov $0, %%eax\n"
                "mov $0, %%ebx\n"
                "mov $0, %%ecx\n"
                "mov $0, %%edx\n"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ebx", "ecx", "edx", "memory"
            );
            
            /* More computations after assembly to force rematerialization */
            int t7 = t3 * t4 + t5 - t6;
            float f7 = f3 * f4 - f5 + f6;
            double d7 = d3 * d4 / d5 + d6;
            
            /* Call function with many arguments - forces use of argument registers */
            int func_result = use_values(t1, t2, f1, d1, t3, t4, f2, d2);
            
            /* Another function call with different arguments */
            double d8 = complex_calc(d3, d4, f3, t5, 
                                    (long)inner * outer, 
                                    (short)(inner % 256), 
                                    (char)(outer % 128), 
                                    (unsigned)(inner + outer));
            
            /* Volatile memory operations to prevent optimization */
            global_sink = t7 + func_result;
            global_double_sink = d7 + d8;
            
            /* More assembly with different clobbered registers */
            asm volatile(
                "# Clobber more registers\n"
                "pxor %%xmm0, %%xmm0\n"
                "pxor %%xmm1, %%xmm1\n"
                "pxor %%xmm2, %%xmm2\n"
                : /* no outputs */
                : /* no inputs */
                : "xmm0", "xmm1", "xmm2", "memory"
            );
            
            /* Final computations mixing all types */
            int t8 = (t7 * inner) / (outer + 1) + func_result;
            float f8 = (f7 * inner * 0.01f) / (outer + 1.0f) + (float)func_result;
            double d9 = (d7 * inner * 0.01) / (outer + 1.0) + d8;
            
            /* Update accumulators */
            total_i += t8 + (inner % 8);
            total_f += f8 + arr_f[inner % 256];
            total_d += d9 + arr_d[inner % 256];
            
            /* Update vector accumulators */
            vec_acc_i += vec3 + vec4;
            vec_acc_f += vf3 * vf4;
            
            /* Another volatile write */
            volatile int local_sink;
            local_sink = t8 + (int)f8 + (int)d9;
        }
        
        /* Additional computation between inner loops */
        if (outer % 10 == 0) {
            /* Force some register spilling with a complex expression */
            double temp = total_d * total_f * total_i;
            for (int j = 0; j < 4; j++) {
                temp += arr_d[outer % 256] * arr_f[j] * arr_i[j * 2];
            }
            total_d += temp;
        }
    }
    
    /* Extract results from vectors to prevent optimization */
    int vec_sum = 0;
    float vec_fsum = 0.0f;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_acc_i[i];
        vec_fsum += vec_acc_f[i];
    }
    
    /* Final result computation and output */
    double final_result = total_d + total_f + total_i + vec_sum + vec_fsum;
    printf("Result: %f\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
