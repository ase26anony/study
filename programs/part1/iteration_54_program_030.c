/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Complex computation functions that cannot be inlined */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h) {
    /* Force register usage for all arguments */
    int result = (a * b) + (int)(c * 100.0f) + (int)(d * 200.0) + 
                 (e ^ f) + (int)(g * 300.0f) + (int)(h * 400.0);
    global_sink = result;
    return result & 0xFF;
}

NOINLINE float vector_operation(v4sf vec1, v4sf vec2, v4sf vec3) {
    /* Vector operations consume multiple registers */
    v4sf result = vec1 * vec2 + vec3;
    float_sink = result[0] + result[1] + result[2] + result[3];
    return result[0];
}

NOINLINE double mixed_computation(int i, float f, double d, 
                                  v2df vec1, v2df vec2) {
    /* Mixed-type computation forcing conversions */
    double result = (double)i * 1.5 + (double)f * 2.5 + d * 3.5;
    result += vec1[0] * vec2[0] + vec1[1] * vec2[1];
    double_sink = result;
    return result;
}

/* Main computational kernel designed for high register pressure */
int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = (i * 37) & 0xFF;
        array_float[i] = (float)(i * 0.12345f);
        array_double[i] = (double)(i * 0.6789);
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating short-lived temporaries */
            
            /* Integer computations with unique expressions */
            int temp1 = array_int[i] * 3 + outer;
            int temp2 = array_int[i+1] / 7 - outer;
            int temp3 = array_int[i+2] ^ (i * 13);
            int temp4 = array_int[i+3] | (outer << 3);
            int temp5 = temp1 * temp2 - temp3 + temp4;
            
            /* Floating-point computations */
            float ftemp1 = array_float[i] * 2.5f + (float)outer;
            float ftemp2 = array_float[i+1] / 1.7f - (float)i;
            float ftemp3 = ftemp1 * ftemp2 * (float)(i % 17);
            float ftemp4 = ftemp1 + ftemp2 - ftemp3;
            
            /* Double precision computations */
            double dtemp1 = array_double[i] * 3.14159 + (double)outer;
            double dtemp2 = array_double[i+1] / 2.71828 - (double)i;
            double dtemp3 = dtemp1 * dtemp2 * (double)((i + outer) % 23);
            double dtemp4 = dtemp1 + dtemp2 - dtemp3;
            
            /* Vector operations */
            v4sf vec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf vec2 = {(float)i, (float)(i+1), (float)(i+2), (float)(i+3)};
            v4sf vec3 = {(float)outer, (float)(outer*2), 
                        (float)(outer*3), (float)(outer*4)};
            
            /* Double vector */
            v2df dvec1 = {dtemp1, dtemp2};
            v2df dvec2 = {dtemp3, dtemp4};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber multiple registers */
            asm volatile(
                "# Force register pressure\n\t"
                "mov $0, %%eax\n\t"
                "mov $0, %%ebx\n\t"
                "mov $0, %%ecx\n\t"
                "mov $0, %%edx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ebx", "ecx", "edx", "memory"
            );
            
            /* Call function with many arguments - forces argument passing */
            int func_result = use_values(
                temp1, temp2, ftemp1, dtemp1,
                temp3, temp4, ftemp2, dtemp2
            );
            
            /* More computations between calls */
            int temp6 = temp5 * func_result + i;
            float ftemp5 = ftemp3 * (float)func_result - (float)temp6;
            double dtemp5 = dtemp3 * (double)func_result / (double)(temp6 + 1);
            
            /* Another inline assembly to break live ranges */
            asm volatile(
                "# Clobber more registers\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2\n\t"
                : /* no outputs */
                : /* no inputs */
                : "xmm0", "xmm1", "xmm2", "memory"
            );
            
            /* Call vector function */
            float vec_result = vector_operation(vec1, vec2, vec3);
            
            /* Mixed computation call */
            double mixed_result = mixed_computation(
                temp6, ftemp5, dtemp5,
                dvec1, dvec2
            );
            
            /* Volatile memory access to prevent optimization */
            global_sink = temp6;
            float_sink = vec_result;
            double_sink = mixed_result;
            
            /* Complex expression combining all results */
            total += (int)((temp6 & 0xFF) + 
                          (int)(vec_result * 10.0f) + 
                          (int)(mixed_result * 5.0));
            
            /* Another inline assembly to force rematerialization */
            asm volatile(
                "# Final clobber\n\t"
                "mov $0, %%r8d\n\t"
                "mov $0, %%r9d\n\t"
                "mov $0, %%r10d\n\t"
                "mov $0, %%r11d\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "r10", "r11", "memory"
            );
        }
        
        /* Additional computations in outer loop */
        int outer_temp = outer * 7;
        for (int j = 0; j < 4; j++) {
            outer_temp = (outer_temp * 11 + j) & 0xFFF;
            /* More inline assembly */
            asm volatile(
                "# Outer loop clobber\n\t"
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "memory"
            );
        }
        total += outer_temp;
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
