/* early-remat-test.c - Test program for GCC early rematerialization coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function with many arguments to force register pressure */
__attribute__((noinline, noipa))
int use_values(int a, int b, float c, double d, 
               long e, short f, v4si v, v4sf vf) {
    volatile int result = 0;
    result += a * b;
    result += (int)(c * 100.0f);
    result += (int)(d * 1000.0);
    result += (int)(e % 100);
    result += f;
    
    /* Access vector elements */
    int* vp = (int*)&v;
    for (int i = 0; i < 4; i++) {
        result += vp[i];
    }
    
    float* fp = (float*)&vf;
    for (int i = 0; i < 4; i++) {
        result += (int)(fp[i] * 10.0f);
    }
    
    return result;
}

/* Another non-inline function for more register pressure */
__attribute__((noinline, noipa))
double compute_polynomial(double x, double y, double z,
                          double a, double b, double c,
                          double d, double e, double f) {
    /* Complex polynomial that uses many temporaries */
    double t1 = x * y + z;
    double t2 = a * b - c;
    double t3 = d * e / (f + 1.0);
    double t4 = sin(x) * cos(y);
    double t5 = exp(z) * log(fabs(a) + 1.0);
    double t6 = t1 * t2 + t3 * t4 - t5;
    
    return t6 * t1 - t2 * t3 + t4 * t5;
}

int main(void) {
    const int ITERATIONS = 100000;
    const int INNER_LOOPS = 50;
    
    /* Initialize arrays with volatile to prevent lifting */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 0.5f + 0.1f;
        array3[i] = i * 0.25 + 0.01;
    }
    
    int total_result = 0;
    
    /* Outer loop - creates many live ranges */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Use volatile variables to force memory operations */
        volatile int v1 = outer * 2;
        volatile float v2 = outer * 0.3f;
        volatile double v3 = outer * 0.7;
        
        /* Nested inner loops with many temporaries */
        for (int inner = 0; inner < INNER_LOOPS; inner++) {
            /* Create many independent computations with unique expressions */
            /* Each computation creates new temporaries */
            
            /* Integer computations */
            int t1 = v1 * inner + outer;
            int t2 = t1 * 3 - inner * 2;
            int t3 = t2 / (inner + 1) + outer % 7;
            int t4 = t3 << (inner & 3);
            int t5 = t4 ^ (t1 * t2);
            int t6 = (t5 + t3) * (t4 - t2);
            
            /* Floating-point computations */
            float f1 = v2 * inner + outer * 0.1f;
            float f2 = f1 * 3.14159f - inner * 0.5f;
            float f3 = f2 / (inner * 0.3f + 1.0f) + sinf(outer * 0.01f);
            float f4 = f3 * cosf(inner * 0.02f);
            float f5 = f4 + f1 * f2 - f3;
            float f6 = f5 * expf(f4 * 0.1f);
            
            /* Double precision computations */
            double d1 = v3 * inner + outer * 0.01;
            double d2 = d1 * 2.71828 - inner * 0.25;
            double d3 = d2 / (inner * 0.1 + 1.0) + cos(outer * 0.005);
            double d4 = d3 * sin(inner * 0.01);
            double d5 = d4 + d1 * d2 - d3;
            double d6 = d5 * log(fabs(d4) + 1.0);
            
            /* Vector operations */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, inner, outer};
            v4si vec3 = vec1 + vec2 * 3;
            v4si vec4 = vec3 - vec1 / 2;
            
            v4sf vecf1 = {f1, f2, f3, f4};
            v4sf vecf2 = {f5, f6, inner * 0.1f, outer * 0.01f};
            v4sf vecf3 = vecf1 + vecf2 * 2.5f;
            v4sf vecf4 = vecf3 - vecf1 / 1.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", "r8", "r9", "r10", "r11",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_values(t1, t2, f1, d1, 
                                        (long)t3 * t4, (short)t5,
                                        vec3, vecf3);
            
            /* More computations after function call */
            int t7 = func_result * inner + t6;
            float f7 = func_result * 0.01f + f6;
            double d7 = func_result * 0.001 + d6;
            
            /* Complex polynomial computation */
            double poly = compute_polynomial(d1, d2, d3, d4, d5, d6,
                                            f1, f2, f3);
            
            /* Volatile memory accesses to prevent optimization */
            global_sink = t7;
            float_sink = f7;
            double_sink = d7 + poly;
            
            /* Array accesses with volatile */
            int idx = (inner + outer) & 255;
            int arr_val1 = array1[idx];
            float arr_val2 = array2[idx];
            double arr_val3 = array3[idx];
            
            /* More computations using array values */
            int t8 = t7 * arr_val1 + inner;
            float f8 = f7 * arr_val2 * 0.5f;
            double d8 = d7 * arr_val3 * 0.25;
            
            /* Another inline assembly to break live ranges */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "memory");
            
            /* Accumulate to result with complex expression */
            total_result += t8 + (int)f8 + (int)d8 + 
                          (vec3[0] & 0xFF) + (int)(vecf3[0] * 10.0f);
            
            /* Force register pressure with many simultaneous values */
            int t9 = t8 * 2 - inner;
            int t10 = t9 + t7 / 2;
            int t11 = t10 ^ (t8 << 2);
            int t12 = t11 * 3 + t9;
            
            float f9 = f8 * 1.1f - inner * 0.01f;
            float f10 = f9 + sinf(f8 * 0.1f);
            float f11 = f10 * cosf(inner * 0.05f);
            float f12 = f11 / (fabsf(f9) + 1.0f);
            
            double d9 = d8 * 1.01 - inner * 0.001;
            double d10 = d9 + sin(d8 * 0.01);
            double d11 = d10 * cos(inner * 0.005);
            double d12 = d11 / (fabs(d9) + 1.0);
            
            /* Use all these temporaries in final expression */
            total_result += (t12 - t11) * (int)(f12 * 100.0f) + 
                          (int)(d12 * 1000.0);
        }
        
        /* Periodic check to prevent loop elimination */
        if (outer % 1000 == 0) {
            printf("Progress: %d/%d iterations, result: %d\n", 
                   outer, ITERATIONS, total_result & 0xFF);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return total_result & 0xFF;
}
