/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Non-inline function to force register usage for arguments */
__attribute__((noinline)) 
int use_values(int a, int b, float c, double d, int e, int f, float g, double h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    return sink;
}

/* Another non-inline function with mixed types */
__attribute__((noinline))
float complex_op(int x, float y, double z, int w) {
    volatile float result;
    result = (x * y) + (float)z - w;
    return result;
}

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

int main(void) {
    /* Initialize arrays with volatile elements to force memory ops */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    long long total = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int t1 = array1[i] * 3;
            int t2 = array1[(i + 1) % 256] / 2;
            float t3 = array2[i] * 1.7f;
            double t4 = array3[i] / 3.14159;
            int t5 = t1 + t2 - i;
            float t6 = t3 + (float)t4 * 2.0f;
            double t7 = (double)t5 * 0.5 + t4;
            int t8 = (int)(t6 * 10.0f) + (int)t7;
            
            /* More temporaries with unique expressions using loop indices */
            int t9 = (t1 * i) + (t2 % (i + 1));
            float t10 = t3 * (i % 10 + 1) - t6;
            double t11 = t4 * (outer + 1) + t7 * 0.3;
            int t12 = t8 ^ t9;
            float t13 = t10 + (float)t11;
            double t14 = t11 * 2.0 - (double)t13;
            
            /* Even more temporaries - each is a unique computation */
            int t15 = t12 * 7 + outer;
            float t16 = t13 / (float)(abs(i - 128) + 1);
            double t17 = t14 * 3.14159;
            int t18 = t15 & 0xFF;
            float t19 = t16 * t16 - t13;
            double t20 = t17 + t14 / 2.0;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple integer and floating-point registers */
            asm volatile("" 
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "memory");
            
            /* Function call with many arguments - forces register usage */
            int result1 = use_values(t1, t2, t3, t4, t5, t6, t7, t8);
            
            /* Another function call with different arguments */
            float result2 = complex_op(t9, t10, t11, t12);
            
            /* More arithmetic mixing results */
            int t21 = result1 + (int)result2 + t15;
            float t22 = (float)result1 * 0.5f + result2;
            double t23 = (double)result2 * 2.0 + (double)t21;
            
            /* Vector operations using GCC extensions */
            v4si vec1 = {t1, t2, t5, t8};
            v4si vec2 = {t9, t12, t15, t18};
            v4si vec3 = vec1 + vec2 * 2;
            
            v4sf fvec1 = {t3, t6, t10, t13};
            v4sf fvec2 = {t16, t19, (float)t22, result2};
            v4sf fvec3 = fvec1 * fvec2 + fvec1;
            
            /* Extract elements from vectors */
            int vecelem1 = vec3[0] + vec3[1];
            float vecelem2 = fvec3[2] - fvec3[3];
            
            /* Volatile memory writes to prevent optimization */
            global_sink = t21;
            float_sink = t22;
            double_sink = t23;
            
            /* More inline assembly between computations */
            asm volatile("" ::: "r8", "r9", "r10", "r11", 
                         "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Additional computations using vector results */
            int t24 = vecelem1 * 3 + (int)vecelem2;
            float t25 = (float)vecelem1 * 0.25f + vecelem2;
            double t26 = (double)t24 * 0.1 + (double)t25;
            
            /* Final accumulation with all temporaries */
            total += t1 + t2 + (int)t3 + (int)t4 + t5 + (int)t6 + (int)t7 + t8 +
                     t9 + (int)t10 + (int)t11 + t12 + (int)t13 + (int)t14 + t15 +
                     (int)t16 + (int)t17 + t18 + (int)t19 + (int)t20 + t21 +
                     (int)t22 + (int)t23 + t24 + (int)t25 + (int)t26 +
                     vecelem1 + (int)vecelem2;
            
            /* Another function call to break live ranges */
            if (i % 32 == 0) {
                use_values(t24, (int)t25, t25, t26, vecelem1, (int)vecelem2, 
                          (float)total, (double)total);
            }
        }
        
        /* Outer loop computations to add more pressure */
        int outer_temp1 = outer * 37;
        float outer_temp2 = outer * 1.618f;
        double outer_temp3 = outer * 3.14159;
        
        asm volatile("" ::: "r12", "r13", "r14", "r15", 
                     "xmm0", "xmm1", "xmm2", "memory");
        
        int outer_result = use_values(outer_temp1, outer, outer_temp2, 
                                     outer_temp3, total % 1000, 
                                     (total / 1000) % 1000,
                                     (float)(total % 10000), 
                                     (double)(total / 10000));
        
        total += outer_result;
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
