/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimization */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
__attribute__((noinline)) 
int use_many_values(int a, int b, int c, float d, double e, 
                    long f, short g, char h, v4si* vec) {
    /* Force side effects */
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return a + b + c + (int)d + (int)e + (int)f + g + h + (*vec)[0];
}

/* Another non-inline function for floating point */
__attribute__((noinline))
float fp_compute(float a, float b, float c, float d, 
                 float e, float f, float g, float h) {
    /* Complex FP expression */
    float t1 = a * b + c / d;
    float t2 = e - f * g + h;
    float t3 = sinf(t1) * cosf(t2);
    float t4 = tanf(t1 + t2) * expf(t3);
    return t3 + t4;
}

int main(void) {
    /* Initialize arrays with volatile to force loads */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.7;
    }
    
    /* Result accumulator */
    int total_result = 0;
    float fp_total = 0.0f;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Many independent computations with different expressions */
            /* Each computation uses unique temporaries */
            
            /* Integer computations - each creates new temps */
            int t1 = array1[inner] * 3 + outer;
            int t2 = array1[inner + 1] / 2 - outer;
            int t3 = t1 * t2 + inner * 7;
            int t4 = (t1 << 3) | (t2 & 0xFF);
            int t5 = t3 ^ t4 + (inner << 2);
            int t6 = t5 * 11 - t3 / 5;
            int t7 = (t6 + t4) * (t2 - t1);
            int t8 = t7 % 97 + t6 * 3;
            int t9 = (t8 << 1) + (t5 >> 2);
            int t10 = t9 * t8 - t7 / 3;
            
            /* Floating point computations - consumes FP registers */
            float f1 = array2[inner] * 2.3f + outer;
            float f2 = array2[inner + 1] / 1.7f - outer;
            float f3 = f1 * f2 + inner * 1.1f;
            float f4 = f3 / f1 - f2 * 0.7f;
            float f5 = sinf(f3) * cosf(f4);
            float f6 = f5 * 2.5f + f4 / 1.3f;
            float f7 = f6 * f5 - f3 * 0.9f;
            float f8 = f7 + f6 / f5 * 1.7f;
            float f9 = tanf(f8) * expf(f7);
            float f10 = f9 * 3.1f - f8 / 2.7f;
            
            /* Double computations - more register pressure */
            double d1 = array3[inner] * 1.11 + outer;
            double d2 = array3[inner + 1] / 2.22 - outer;
            double d3 = d1 * d2 + inner * 3.33;
            double d4 = d3 / d1 - d2 * 0.44;
            double d5 = sin(d3) * cos(d4);
            double d6 = d5 * 5.55 + d4 / 1.66;
            double d7 = d6 * d5 - d3 * 0.77;
            double d8 = d7 + d6 / d5 * 1.88;
            double d9 = tan(d8) * exp(d7);
            double d10 = d9 * 9.99 - d8 / 2.22;
            
            /* Vector operations - consume SIMD registers */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2 - vec3;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f6, f7, f8};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2 - fvec3;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Call non-inline function with many arguments */
            /* Forces values into argument registers */
            int func_result = use_many_values(
                t1, t2, t3, f1, d1, 
                (long)t4, (short)t5, (char)t6, &vec3);
            
            /* More computations after function call */
            /* These may need to rematerialize values */
            int t11 = t10 * 2 + func_result;
            int t12 = t11 - t9 / 4 + inner;
            int t13 = t12 * 3 + (t8 % 13);
            int t14 = t13 ^ t7 + outer * 5;
            int t15 = t14 * 7 - t6 / 3;
            
            float f11 = f10 * 1.5f + (float)func_result;
            float f12 = f11 - f9 / 2.0f + inner * 0.1f;
            float f13 = f12 * 3.3f + f8 * 0.7f;
            float f14 = sinf(f13) * cosf(f12);
            float f15 = f14 * 2.8f - f7 / 1.9f;
            
            /* Another function call with FP arguments */
            float fp_result = fp_compute(
                f1, f2, f3, f4, f5, f6, f7, f8);
            
            /* More inline assembly to break live ranges */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7", "memory");
            
            /* Volatile writes to prevent elimination */
            global_sink = t15;
            float_sink = f15;
            
            /* Accumulate results */
            total_result += t15 + func_result;
            fp_total += f15 + fp_result;
            
            /* Complex conditional with many operands */
            if ((t15 & 0xF) < (inner & 0x7)) {
                total_result -= t14;
                fp_total -= f14;
            } else if ((t13 % 5) > (outer % 3)) {
                total_result += t13 * 2;
                fp_total += f13 * 1.5f;
            }
        }
        
        /* Additional computations between inner loops */
        int loop_temp = outer * 37;
        for (int k = 0; k < 10; k++) {
            loop_temp = loop_temp * 3 + k;
            float loop_fp = sinf(loop_temp * 0.01f) * cosf(k * 0.1f);
            global_sink = loop_temp;
            float_sink = loop_fp;
        }
    }
    
    /* Final computation to use all results */
    double final_result = (double)total_result + (double)fp_total;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(total_result), "r"(fp_total) : "memory");
    
    printf("Result: %f\n", final_result);
    return (int)final_result % 256;
}
