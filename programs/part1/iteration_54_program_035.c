/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Prevent inlining to force register pressure at call sites */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_values(int a, int b, float c, double d, int e, int f, long g, short h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + h;
    return sink & 1;
}

/* Another noinline function with different signature */
__attribute__((noinline))
float compute_more(float x, float y, double z, int i, int j) {
    volatile float vsink;
    vsink = x * y + (float)z + i - j;
    return vsink;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile float float_sink;

int main(void) {
    /* Initialize arrays with varying data */
    int array1[256];
    float array2[256];
    double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = sinf(i * 0.1f);
        array3[i] = cos(i * 0.05);
    }
    
    /* Volatile pointer to force repeated dereferencing */
    volatile int* vptr = array1;
    volatile float* vfptr = array2;
    
    /* Accumulator to prevent dead code elimination */
    long long total = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Create many short-lived temporaries in inner loop */
        for (int i = 0; i < 128; i++) {
            /* Force register pressure with many independent computations */
            int t1 = array1[i] * 3 + outer;
            int t2 = array1[i + 1] / 2 - outer;
            int t3 = t1 * t2 + i;
            int t4 = t2 - t1 * i;
            int t5 = (t3 << 3) | (t4 & 0xFF);
            int t6 = t5 ^ (i * 7 + outer);
            
            /* Floating point computations to use FP registers */
            float f1 = array2[i] * 2.5f + outer;
            float f2 = array2[i + 1] / 1.7f - outer;
            float f3 = f1 * f2 * i;
            float f4 = sinf(f3) * cosf(f2);
            
            /* Double precision for more register pressure */
            double d1 = array3[i] * 1.234567;
            double d2 = array3[i + 1] / 0.987654;
            double d3 = d1 * d2 + tan(d1);
            double d4 = exp(d3 * 0.01) * log(fabs(d2) + 1.0);
            
            /* Vector operations to consume SIMD registers */
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, i, outer};
            v4si v3 = v1 + v2 * 2;
            v4si v4 = v1 - v2 / 3;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {f2, f1, f4, f3};
            v4sf vf3 = vf1 * vf2 + vf1;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose and some SSE registers */
            asm volatile("" 
                : /* no outputs */
                : /* no inputs */ 
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "memory", "cc");
            
            /* Call function with many arguments - forces moving values to registers */
            int result = use_values(t1, t2, f3, d4, t5, t6, (long)v3[0], (short)v4[1]);
            
            /* More computations after call (forces rematerialization of values) */
            int t7 = t1 + t2 * result;
            int t8 = t3 - t4 / (result + 1);
            float f5 = f1 * result + f2;
            float f6 = compute_more(f3, f4, d3, t5, t6);
            
            /* Volatile memory writes to prevent optimization */
            global_sink = t7 + t8;
            float_sink = f5 + f6;
            
            /* Another assembly clobber to break live ranges */
            asm volatile("" ::: "r8", "r9", "r10", "r11", "r12", 
                         "xmm8", "xmm9", "xmm10", "xmm11", "memory");
            
            /* Complex expression with many temporaries */
            double d5 = (d1 * d2) + (d3 / d4) * result;
            double d6 = sin(d5) * cos(d4) + tan(d3);
            int t9 = (int)(d5 * 1000) ^ (int)(d6 * 1000);
            
            /* Use volatile pointer to force load/store */
            int loaded1 = *vptr;
            float loaded2 = *vfptr;
            *vptr = t9;
            vptr++;
            vfptr++;
            
            /* Final accumulation with mixing */
            total += t7 + t8 + t9 + (int)f5 + (int)f6 + (int)d5 + loaded1 + (int)loaded2;
            
            /* More arithmetic to create more virtual registers */
            for (int j = 0; j < 3; j++) {
                int tmp1 = (t7 << j) | (t8 >> j);
                int tmp2 = tmp1 * (i + j + outer);
                float tmp3 = f5 * j + f6 / (j + 1);
                total += tmp2 + (int)tmp3;
                
                /* Small inline assembly between computations */
                asm volatile("" ::: "r13", "r14", "r15", "xmm12", "xmm13", "memory");
            }
        }
        
        /* Additional computations between outer loop iterations */
        double outer_d = sin(outer * 0.1) * cos(outer * 0.05);
        float outer_f = outer * 0.7f;
        int outer_i = (int)(outer_d * 1000) + (int)(outer_f * 100);
        
        /* Call with expressions that need computation */
        use_values(outer_i, outer, outer_f, outer_d, 
                  outer_i * 2, outer_i / 3, total & 0xFFFF, (short)outer);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
