/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent function inlining to force register pressure */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               int i, int j, float k, double l) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h + i + j + (int)k + (int)l;
    return sink & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline))
float compute_more(float x, float y, double z, int w, 
                   float a, float b, double c, int d) {
    volatile float vsink;
    vsink = x * y + (float)z * w + a / b + (float)c * d;
    return vsink;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile globals to prevent optimizations */
volatile int global_counter = 0;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

int main(void) {
    /* Initialize arrays with volatile elements */
    volatile int array_int[256];
    volatile float array_float[256];
    volatile double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i;
        array_float[i] = i * 0.5f;
        array_double[i] = i * 0.25;
    }
    
    /* Result accumulator */
    int total_result = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Volatile variables to force memory operations */
        volatile int v1 = outer;
        volatile float v2 = outer * 1.5f;
        volatile double v3 = outer * 2.5;
        
        for (int inner = 0; inner < 100; inner++) {
            /* MANY independent arithmetic operations creating temporaries */
            /* Each computation is slightly unique to avoid CSE */
            int t1 = v1 + inner * 3;
            int t2 = inner * 5 - v1;
            float t3 = v2 * inner + 1.0f;
            double t4 = v3 / (inner + 1) + 2.0;
            int t5 = t1 * t2 + inner;
            float t6 = t3 * 2.0f - (float)t4;
            double t7 = t4 * 3.0 + t2;
            int t8 = t5 - (int)t6 * 4;
            float t9 = t6 / (float)t7 + t3;
            double t10 = t7 - (double)t9 * 5.0;
            
            /* More computations with mixed types */
            int t11 = (t1 ^ t2) | t8;
            float t12 = t3 + t6 - t9;
            double t13 = t4 * t7 / t10;
            int t14 = t11 * inner % 17;
            float t15 = t12 * global_float;
            double t16 = t13 + global_double;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose and xmm registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* Vector operations to consume SIMD registers */
            v4si vec_a = {t1, t2, t5, t8};
            v4si vec_b = {inner, outer, t14, t11};
            v4si vec_c = vec_a + vec_b * 2;
            
            v4sf vec_f1 = {t3, t6, t9, t12};
            v4sf vec_f2 = {global_float, 2.0f, 3.0f, 4.0f};
            v4sf vec_f3 = vec_f1 * vec_f2;
            
            /* More arithmetic with the results */
            int t17 = vec_c[0] + vec_c[1] - vec_c[2] + vec_c[3];
            float t18 = vec_f3[0] + vec_f3[1] * vec_f3[2] - vec_f3[3];
            
            /* Function call with many arguments - forces register/stack use */
            int func_result = use_values(
                t1, t2, t3, t4,           /* First 4 in registers (x86-64 SysV) */
                t5, t6, t7, t8,           /* Next 4 may be in registers/stack */
                t11, t14, t15, t16        /* Remaining on stack */
            );
            
            /* Another function call */
            float float_result = compute_more(
                t3, t6, t7, t8,
                t9, t12, t13, t14
            );
            
            /* Volatile memory access to prevent optimization */
            array_int[inner & 255] = t17 + func_result;
            array_float[inner & 255] = t18 + float_result;
            array_double[inner & 255] = t16 + t13;
            
            /* Another assembly clobber */
            asm volatile("" : : : 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* More computations after clobber - forces rematerialization */
            int t19 = t1 * 2 + t2 / 3 - inner;
            float t20 = t3 * 3.0f + t6 / 2.0f - outer;
            double t21 = t4 * 1.5 + t7 / 3.0 - v3;
            int t22 = t5 ^ t8 | t11 & t14;
            float t23 = t9 + t12 * t15 - global_float;
            double t24 = t10 * t13 / t16 + global_double;
            
            /* Use SSE intrinsics for additional register pressure */
            __m128 sse_a = _mm_set_ps(t20, t23, t18, float_result);
            __m128 sse_b = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 sse_c = _mm_add_ps(sse_a, sse_b);
            
            /* Accumulate results (prevents dead code elimination) */
            total_result += t17 + t19 + t22 + (int)t18 + (int)t20 + 
                           (int)(t21 + t24) + func_result + 
                           vec_c[0] + (int)sse_c[0];
            
            /* Update volatile global */
            global_counter++;
        }
        
        /* Additional computation between outer loop iterations */
        volatile int gap_var = outer * 37;
        volatile float gap_float = gap_var * 0.123f;
        asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
    }
    
    printf("Result: %d\n", total_result);
    return total_result & 0xFF;
}
