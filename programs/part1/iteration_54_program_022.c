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

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments */
NOINLINE NOCLONE int use_many_values(int a, int b, int c, int d,
                                     float e, float f, double g, double h,
                                     v4si *v1, v4sf *v2) {
    /* Force side effects */
    global_sink += a + b + c + d;
    float_sink += e + f;
    double_sink += g + h;
    
    /* Use vector arguments */
    v4si temp = *v1 + (v4si){a, b, c, d};
    v4sf tempf = *v2 + (v4sf){e, f, e * 2.0f, f * 3.0f};
    
    return temp[0] + (int)tempf[0];
}

/* Another non-inline function for different register class pressure */
NOINLINE NOCLONE double compute_complex(double base, int iterations) {
    double result = base;
    for (int i = 0; i < iterations; i++) {
        /* Complex floating point operations */
        result = result * 1.61803398875 - (double)i * 0.31415926536;
        result = result / (1.0 + result * result);
    }
    return result;
}

/* Main computational kernel */
int main(void) {
    /* Initialize arrays with volatile to prevent optimization */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.71828;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total_result = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Create many short-lived temporaries with complex expressions */
            int idx = (outer * 7 + inner * 13) % 256;
            
            /* Integer computations - each creates new temporaries */
            int t1 = array1[idx] * 3 + outer;
            int t2 = array1[(idx + 1) % 256] / 2 - inner;
            int t3 = t1 * t2 + (outer ^ inner);
            int t4 = (t1 << 3) | (t2 >> 2);
            int t5 = t3 * t4 - (t1 + t2) * (outer - inner);
            int t6 = t5 * 17 + (t3 % 31) - (t4 & 0xFF);
            
            /* Floating point computations - uses different register class */
            float f1 = array2[idx] * 2.5f + outer * 0.1f;
            float f2 = array2[(idx + 2) % 256] / 1.7f - inner * 0.05f;
            float f3 = f1 * f2 + (float)(outer * inner) * 0.01f;
            float f4 = f1 / (f2 + 1.0f) - f3 * 0.3f;
            float f5 = f3 * f4 + (f1 - f2) * (float)(outer + inner);
            
            /* Double precision computations */
            double d1 = array3[idx] * 1.41421356 + outer * 0.01;
            double d2 = array3[(idx + 3) % 256] / 2.71828 - inner * 0.005;
            double d3 = d1 * d2 + (double)(outer | inner) * 0.001;
            double d4 = d1 / (d2 + 1.0) - d3 * 0.25;
            double d5 = d3 * d4 + (d1 - d2) * (double)(outer & inner);
            
            /* Vector operations - consume multiple registers */
            v4si vec_int = {t1, t2, t3, t4};
            v4sf vec_float = {f1, f2, f3, f4};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose, SSE, and MMX registers */
            asm volatile(
                "# Force register pressure\n\t"
                "mov $0, %%eax\n\t"
                "mov $0, %%ebx\n\t"
                "pxor %%mm0, %%mm0\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                :
                :
                : "eax", "ebx", "ecx", "edx", "edi", "esi",
                  "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
                  "memory", "cc"
            );
            
            /* More computations after clobber - forces rematerialization */
            int t7 = t6 * 2 + (t5 >> 1);
            float f6 = f5 * 1.1f + (float)t7 * 0.01f;
            double d6 = d5 * 1.01 + (double)t7 * 0.001;
            
            /* Call function with many arguments - forces argument passing */
            int func_result = use_many_values(t1, t2, t3, t4,
                                             f1, f2, d1, d2,
                                             &vec_int, &vec_float);
            
            /* Complex double computation */
            double complex_d = compute_complex(d6, 3);
            
            /* More volatile memory operations */
            global_sink += t7;
            float_sink += f6;
            double_sink += complex_d;
            
            /* Final computation mixing all types */
            total_result += func_result + (int)f6 + (int)complex_d + t7;
            
            /* Another inline assembly to break live ranges */
            asm volatile(
                "# Break live ranges\n\t"
                "mov $0, %%r8d\n\t"
                "mov $0, %%r9d\n\t"
                "mov $0, %%r10d\n\t"
                "mov $0, %%r11d\n\t"
                :
                :
                : "r8", "r9", "r10", "r11", "memory", "cc"
            );
            
            /* Additional computations that might use rematerialized values */
            if ((outer + inner) % 7 == 0) {
                /* Use original values again - potential remat candidates */
                int t8 = t1 * t2 + t3 - t4;  /* t1-t4 might need rematerialization */
                float f7 = f1 + f2 * f3 - f4;
                total_result += t8 + (int)f7;
            }
        }
        
        /* Periodic memory access pattern */
        volatile int* ptr = (volatile int*)&array1[outer % 256];
        *ptr = total_result & 0xFF;
    }
    
    /* Print result to prevent optimization */
    printf("Final result: %d\n", total_result);
    printf("Global sinks: %d, %f, %f\n", global_sink, float_sink, double_sink);
    
    return total_result != 0 ? 0 : 1;
}
