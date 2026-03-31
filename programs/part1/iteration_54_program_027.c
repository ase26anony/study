/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))
#define NOCLONE __attribute__((noclone))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments */
NOINLINE NOCLONE int use_many_values(int a, int b, int c, float d, 
                                     double e, long f, short g, char h) {
    /* Complex computation to prevent inlining */
    int result = (a * b) + (c >> 2);
    result += (int)(d * 100.0f);
    result += (int)(e * 10.0);
    result += (int)(f % 1000);
    result += g * 2;
    result += h * 3;
    
    /* Force side effect */
    global_sink = result;
    
    return result & 0xFF;
}

/* Another non-inline function for floating point */
NOINLINE NOCLONE float fp_computation(float a, float b, float c, 
                                      float d, float e, float f) {
    /* Chain of FP operations */
    float t1 = a * b + c;
    float t2 = d / e - f;
    float t3 = t1 * t2;
    float t4 = t3 / (a + 1.0f);
    float t5 = t4 * b - c;
    
    float_sink = t5;
    return t5;
}

/* Vector operations function */
NOINLINE NOCLONE v4si vector_ops(v4si a, v4si b, v4si c) {
    v4si t1 = a + b;
    v4si t2 = b * c;
    v4si t3 = t1 - t2;
    v4si t4 = t3 << 1;
    v4si t5 = t4 >> 1;
    
    /* Force memory store */
    volatile v4si store;
    store = t5;
    
    return t5;
}

int main(void) {
    /* Initialize arrays with varying values */
    int array1[256];
    float array2[256];
    double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = (i * 37) % 101;
        array2[i] = (i * 0.37f);
        array3[i] = (i * 0.073);
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    float fp_total = 0.0f;
    double double_total = 0.0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Create many independent computations with unique expressions */
            /* Each computation uses different operations and constants */
            
            /* Integer computations - each creates temporaries */
            int t1 = array1[i] * 3 + outer;
            int t2 = array1[i+1] / 2 - outer;
            int t3 = t1 * t2 + (i % 7);
            int t4 = (t3 << 2) | (outer & 0xF);
            int t5 = t4 ^ (i * 11);
            int t6 = t5 + (t1 % 13);
            int t7 = t6 - (t2 * 3);
            int t8 = t7 & 0xFF;
            int t9 = t8 | (outer << 8);
            int t10 = t9 * 7 - (i * 5);
            
            /* Floating point computations - consume FP registers */
            float f1 = array2[i] * 1.5f + outer;
            float f2 = array2[i+1] / 1.3f - outer;
            float f3 = f1 * f2 + (i * 0.1f);
            float f4 = f3 / (f1 + 1.0f);
            float f5 = f4 * 2.7f - f2;
            float f6 = f5 + (outer * 0.01f);
            
            /* Double precision computations */
            double d1 = array3[i] * 1.7 + outer;
            double d2 = array3[i+1] / 1.9 - outer;
            double d3 = d1 * d2 + (i * 0.01);
            double d4 = d3 / (d1 + 1.0);
            double d5 = d4 * 3.1 - d2;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber commonly used registers */
            asm volatile(
                "# Force register clobber\n"
                "mov $0, %%eax\n"
                "mov $0, %%ebx\n"
                "mov $0, %%ecx\n"
                "mov $0, %%edx\n"
                "pxor %%xmm0, %%xmm0\n"
                "pxor %%xmm1, %%xmm1\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "xmm0", "xmm1", "memory"
            );
            
            /* More computations after clobber - forces rematerialization */
            int t11 = t10 + (t3 % 17);
            float f7 = f6 * 1.1f + (t11 * 0.01f);
            double d6 = d5 + (t11 * 0.001);
            
            /* Call function with many arguments - forces argument passing */
            int func_result = use_many_values(
                t11, t10, t9, f7, d6, 
                (long)t8 * 1000, (short)(t7 & 0xFFFF), (char)(t6 & 0xFF)
            );
            
            /* More FP computations */
            float f8 = fp_computation(f7, f6, f5, f4, f3, f2);
            
            /* Vector operations */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = {t9, t10, t11, func_result};
            v4si vec_result = vector_ops(vec1, vec2, vec3);
            
            /* Volatile memory access - prevents optimization */
            volatile int* volatile_ptr = &array1[i];
            int volatile_read = *volatile_ptr;
            
            /* Another assembly clobber */
            asm volatile(
                "# Another clobber\n"
                "mov $1, %%r8d\n"
                "mov $2, %%r9d\n"
                "mov $3, %%r10d\n"
                "mov $4, %%r11d\n"
                :
                :
                : "r8", "r9", "r10", "r11", "memory"
            );
            
            /* Final computations using all previous values */
            int final_int = (t11 + func_result + volatile_read) * 3;
            float final_float = f8 + f7 + (final_int * 0.01f);
            double final_double = d6 + d5 + (final_int * 0.001);
            
            /* Update accumulators */
            total += final_int;
            fp_total += final_float;
            double_total += final_double;
            
            /* Volatile write to prevent elimination */
            global_sink = final_int;
            float_sink = final_float;
            double_sink = final_double;
        }
        
        /* Additional computations between outer loop iterations */
        int loop_temp = outer * 7;
        for (int j = 0; j < 4; j++) {
            loop_temp = (loop_temp * 13 + j) % 97;
            
            /* More assembly to break up live ranges */
            asm volatile(
                "# Break live ranges\n"
                "add $1, %%eax\n"
                "add $1, %%ebx\n"
                :
                :
                : "eax", "ebx", "memory"
            );
        }
        total += loop_temp;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: total=%d, fp_total=%.2f, double_total=%.2f\n", 
           total, fp_total, double_total);
    
    return total & 0xFF;
}
