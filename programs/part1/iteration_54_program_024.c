/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>

/* Prevent function inlining to force argument passing */
__attribute__((noinline, used))
int use_values(int a, int b, float c, double d, 
               long e, short f, unsigned char g, int h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + g + h;
    return sink & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline, used))
float process_vector(float a, float b, float c, float d,
                     float e, float f, float g, float h) {
    volatile float result;
    result = a * b - c / d + e - f * g + h;
    return result;
}

/* Vector type using GCC extensions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Volatile globals to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

int main(void) {
    /* Initialize arrays with varying data */
    float fa[256];
    double da[256];
    int ia[256];
    
    for (int i = 0; i < 256; i++) {
        fa[i] = (i * 1.234f) / (i + 1);
        da[i] = (i * 3.456) / (i + 2);
        ia[i] = i * 7 - 13;
    }
    
    /* Volatile pointer to force repeated dereferencing */
    volatile float* volatile_fa = fa;
    volatile double* volatile_da = da;
    volatile int* volatile_ia = ia;
    
    /* Accumulator to prevent dead code elimination */
    long long total = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations creating temporaries */
            float t1 = volatile_fa[i] * 1.1f + outer * 0.01f;
            float t2 = volatile_fa[i+1] / 2.3f - outer * 0.02f;
            double t3 = volatile_da[i] * 3.14 + i * 0.001;
            double t4 = volatile_da[i+1] / 2.71 - i * 0.002;
            
            /* More temporaries with mixed operations */
            int t5 = volatile_ia[i] * 3 + outer;
            int t6 = volatile_ia[i+1] / 2 - outer;
            long t7 = (long)t5 * t6 + i;
            short t8 = (short)(t5 & 0xFFFF) + (short)(t6 & 0xFFFF);
            
            /* Even more temporaries with unique expressions */
            float t9 = t1 * t2 - (float)t3 + (float)t4;
            double t10 = (double)t1 + (double)t2 * t3 - t4;
            int t11 = t5 ^ t6 | (i << 3);
            int t12 = (t5 * 7) / (t6 + 1) + (i % 16);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* Function call with many arguments - forces register/stack use */
            int r1 = use_values(t5, t6, t1, t3, t7, t8, 
                               (unsigned char)(i & 0xFF), t11);
            
            /* More temporaries after call */
            float t13 = t9 * 2.0f + (float)r1;
            double t14 = t10 / 1.5 + (double)r1;
            int t15 = t11 * 2 - t12 + r1;
            
            /* Another function call with floating-point arguments */
            float r2 = process_vector(t1, t2, t9, t13,
                                     (float)t3, (float)t4, (float)t10, (float)t14);
            
            /* Vector operations using GCC extensions */
            v4sf vec1 = {t1, t2, t9, t13};
            v4sf vec2 = {t2, t9, t13, t1};
            v4sf vec3 = vec1 + vec2 * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
            
            /* Extract and use vector elements */
            float vec_elems[4];
            memcpy(vec_elems, &vec3, sizeof(vec_elems));
            
            /* More arithmetic with vector results */
            float t16 = vec_elems[0] * vec_elems[1] - vec_elems[2] + vec_elems[3];
            double t17 = (double)vec_elems[0] + (double)vec_elems[1] * 
                        (double)vec_elems[2] / (double)vec_elems[3];
            
            /* Integer vector operations */
            v4si ivec1 = {t5, t6, t11, t12};
            v4si ivec2 = {t6, t11, t12, t5};
            v4si ivec3 = ivec1 * ivec2 + (v4si){i, outer, i+outer, i-outer};
            
            int ivec_elems[4];
            memcpy(ivec_elems, &ivec3, sizeof(ivec_elems));
            
            /* Final computations mixing all types */
            double final1 = (double)t16 * t17 + (double)ivec_elems[0];
            float final2 = (float)t17 * t16 + (float)ivec_elems[1];
            int final3 = ivec_elems[2] * (int)t16 + (int)t17;
            
            /* Another inline assembly to clobber more registers */
            asm volatile("" : : : 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm8", "xmm9", "xmm10", "xmm11", 
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Use volatile writes to prevent optimization */
            global_sink = final3;
            float_sink = final2;
            double_sink = final1;
            
            /* Accumulate to total (prevents dead code elimination) */
            total += (long long)final3 + (long long)final2 + (long long)final1;
            
            /* More temporaries in next iteration to increase pressure */
            float t18 = t13 * 0.99f + (float)i * 0.01f;
            double t19 = t14 * 0.999 + (double)i * 0.001;
            int t20 = t15 * 2 + (i & 31);
            
            /* Use these in next iteration's computations */
            t1 = t18 * 1.01f;
            t3 = t19 * 1.001;
            t5 = t20 ^ (outer & 0xFF);
        }
        
        /* Additional computations between outer loop iterations */
        float loop_temp1 = (float)outer * 0.5f;
        double loop_temp2 = (double)outer * 0.25;
        int loop_temp3 = outer * 3;
        
        /* Complex expression with many operands */
        float complex1 = loop_temp1 * 2.0f + 
                        (float)loop_temp2 / 3.0f - 
                        (float)loop_temp3 * 0.1f +
                        (float)(outer % 10) * 0.01f;
        
        /* Another function call */
        use_values(loop_temp3, outer, complex1, loop_temp2,
                  (long)loop_temp3 * 2, (short)(outer & 0x7FFF),
                  (unsigned char)(complex1 * 100.0f), outer * 7);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
