/* early-remat-test.c - Test case for GCC early rematerialization coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent function inlining to force argument passing */
__attribute__((noinline, used))
int use_values(int a, int b, float c, double d, int e, float f, double g, int h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + (int)f + (int)g + h;
    return sink & 1;
}

/* Another noinline function with different signature */
__attribute__((noinline, used))
float process_data(float a, float b, float c, float d, 
                   float e, float f, float g, float h) {
    volatile float sink;
    sink = a * b - c / d + e - f * g + h;
    return sink;
}

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_counter = 0;
volatile float global_float = 0.0f;
volatile double global_double = 0.0;

int main(void) {
    /* Initialize arrays with volatile to prevent optimization */
    volatile int array_int[256];
    volatile float array_float[256];
    volatile double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i;
        array_float[i] = i * 0.5f;
        array_double[i] = i * 0.25;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    float ftotal = 0.0f;
    double dtotal = 0.0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int t1 = array_int[i] * 3;
            int t2 = t1 + outer * 7;
            int t3 = t2 - i * 5;
            int t4 = t3 / (outer + 1);
            int t5 = t4 ^ (i << 2);
            int t6 = t5 | (outer & 0xFF);
            int t7 = t6 + t1 - t2;
            int t8 = t7 * t3 / (t4 + 1);
            
            /* Floating point operations - different register class */
            float f1 = array_float[i] * 1.5f;
            float f2 = f1 + outer * 0.3f;
            float f3 = f2 - i * 0.7f;
            float f4 = f3 / (outer * 0.1f + 1.0f);
            float f5 = f4 * f1 - f2;
            float f6 = f5 + f3 / f4;
            
            /* Double precision - more register pressure */
            double d1 = array_double[i] * 2.5;
            double d2 = d1 + outer * 0.9;
            double d3 = d2 - i * 1.3;
            double d4 = d3 / (outer * 0.2 + 1.0);
            double d5 = d4 * d1 - d2;
            double d6 = d5 + d3 / d4;
            
            /* Vector operations - consume SIMD registers */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2 - vec3;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {f5, f6, f1, f2};
            v4sf vf3 = vf1 * vf2 - vf1 / vf2;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7",
                                       "memory");
            
            /* More computations after clobber */
            int t9 = t8 + (t7 >> 3);
            int t10 = t9 * (i + 1) - outer;
            float f7 = f6 * 2.0f + i * 0.1f;
            float f8 = f7 - f5 / (outer + 1.0f);
            double d7 = d6 * 1.7 + i * 0.05;
            double d8 = d7 - d5 / (outer * 0.3 + 1.0);
            
            /* Call function with many arguments - forces register moves */
            int result = use_values(t1, t2, f1, d1, t3, f2, d2, t4);
            
            /* Another function call with floating arguments */
            float fresult = process_data(f3, f4, f5, f6, f7, f8, f1, f2);
            
            /* Volatile memory access to prevent optimization */
            global_counter = t5;
            global_float = f5;
            global_double = d5;
            
            /* Use array elements to force address calculations */
            int idx = (i + outer) & 0xFF;
            volatile int mem1 = array_int[idx];
            volatile float mem2 = array_float[idx];
            volatile double mem3 = array_double[idx];
            
            /* More inline assembly with different clobbers */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                       "xmm8", "xmm9", "xmm10", "xmm11",
                                       "xmm12", "xmm13", "xmm14", "xmm15",
                                       "memory");
            
            /* Final computations mixing everything */
            int final_int = t10 + result + mem1 + (int)fresult;
            float final_float = f8 + fresult + mem2 + result;
            double final_double = d8 + mem3 + result + fresult;
            
            /* Accumulate results */
            total += final_int + (int)final_float + (int)final_double;
            ftotal += final_float;
            dtotal += final_double;
            
            /* Another clobber to break live ranges */
            asm volatile("" : : : "r12", "r13", "r14", "r15",
                                       "xmm0", "xmm1", "xmm2", "memory");
        }
        
        /* Additional computations between outer loop iterations */
        int outer_temp = outer * 17;
        float outer_float = outer * 3.14f;
        double outer_double = outer * 6.28;
        
        for (int j = 0; j < 4; j++) {
            outer_temp = outer_temp * 3 - j;
            outer_float = outer_float / (j + 1.0f) + outer;
            outer_double = outer_double * 1.1 - j * 0.5;
            
            /* Use vector operations here too */
            v4si outer_vec = {outer_temp, j, outer, outer_temp + j};
            v4si outer_vec2 = outer_vec * 2 - (v4si){1, 2, 3, 4};
            
            total += outer_vec2[0] + outer_vec2[1];
        }
        
        total += (int)outer_float + (int)outer_double;
    }
    
    /* Print results to prevent optimization */
    printf("Total: %d, Float: %f, Double: %lf\n", total, ftotal, dtotal);
    
    return total > 0 ? 0 : 1;
}
