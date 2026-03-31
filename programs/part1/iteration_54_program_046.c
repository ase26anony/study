/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("no-ipa")))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               v4si vi, v4sf vf) {
    /* Force side effects */
    asm volatile("" : : "r"(a), "r"(b), "r"(e), "r"(f) : "memory");
    
    /* Use vector types */
    v4si vi2 = vi + (v4si){1, 2, 3, 4};
    v4sf vf2 = vf * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
    
    /* Prevent dead code elimination */
    asm volatile("" : : "x"(vi2), "x"(vf2) : "memory");
    
    return a + b + (int)c + (int)d + e + f + (int)g + (int)h;
}

/* Another non-inline function to force register shuffling */
__attribute__((noinline, optimize("no-ipa")))
double compute_polynomial(double x, double y, double z,
                          int i, int j, int k,
                          float a, float b, float c) {
    /* Complex polynomial with many temporaries */
    double t1 = x * x + y * y;
    double t2 = z * z * 2.5;
    double t3 = t1 / (t2 + 1.0);
    double t4 = (double)i * (double)j * (double)k;
    double t5 = (double)a * (double)b * (double)c;
    double t6 = t3 * t4 * t5;
    
    /* Force register pressure with inline asm */
    asm volatile("" 
                 : "+x"(t1), "+x"(t2), "+x"(t3), "+x"(t4), "+x"(t5), "+x"(t6)
                 : 
                 : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
    
    return t6;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent hoisting */
    volatile int array_int[1024];
    volatile float array_float[1024];
    volatile double array_double[1024];
    
    for (int i = 0; i < 1024; i++) {
        array_int[i] = i;
        array_float[i] = i * 1.5f;
        array_double[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    int64_t total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Create many short-lived temporaries with complex expressions */
            int idx1 = outer * 50 + inner;
            int idx2 = (outer + inner) % 1024;
            int idx3 = (outer * inner) % 1024;
            
            /* Force register pressure with independent computations */
            int temp1 = array_int[idx1] * 3 + array_int[idx2] / 7;
            int temp2 = array_int[idx2] * 5 - array_int[idx3] / 11;
            int temp3 = temp1 * temp2 + idx1 - idx2 + idx3;
            int temp4 = temp2 * temp3 / (temp1 + 1) + outer * inner;
            int temp5 = temp3 * temp4 - temp2 * temp1 + idx1 * idx2;
            int temp6 = temp4 * 5 + temp5 * 3 - temp1 * 2 + idx3;
            
            /* Floating-point computations to use FP registers */
            float ftemp1 = array_float[idx1] * 1.234f + array_float[idx2] / 5.678f;
            float ftemp2 = array_float[idx2] * 9.012f - array_float[idx3] / 3.456f;
            float ftemp3 = ftemp1 * ftemp2 + (float)idx1 - (float)idx2;
            float ftemp4 = ftemp2 * ftemp3 / (ftemp1 + 1.0f) + (float)outer;
            
            /* Double precision for more register pressure */
            double dtemp1 = array_double[idx1] * 1.234567 + array_double[idx2] / 5.678901;
            double dtemp2 = array_double[idx2] * 9.012345 - array_double[idx3] / 3.456789;
            double dtemp3 = dtemp1 * dtemp2 + (double)idx1 * 0.5;
            double dtemp4 = dtemp2 * dtemp3 / (dtemp1 + 1.0) + (double)inner * 0.25;
            
            /* Vector operations */
            v4si vec_int = {temp1, temp2, temp3, temp4};
            v4sf vec_float = {ftemp1, ftemp2, ftemp3, ftemp4};
            
            /* Inline assembly that clobbers registers */
            asm volatile("" 
                         : 
                         : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4), 
                           "r"(temp5), "r"(temp6),
                           "x"(ftemp1), "x"(ftemp2), "x"(ftemp3), "x"(ftemp4),
                           "x"(dtemp1), "x"(dtemp2), "x"(dtemp3), "x"(dtemp4)
                         : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                           "xmm0", "xmm1", "xmm2", "xmm3", 
                           "xmm4", "xmm5", "xmm6", "xmm7",
                           "xmm8", "xmm9", "xmm10", "xmm11",
                           "memory");
            
            /* Call function with many arguments - forces register shuffling */
            int result1 = use_values(temp1, temp2, ftemp1, dtemp1,
                                     temp3, temp4, ftemp2, dtemp2,
                                     vec_int, vec_float);
            
            /* More computations between function calls */
            int temp7 = result1 * 7 + idx1;
            float ftemp5 = (float)result1 * 0.123f + ftemp3;
            double dtemp5 = (double)result1 * 0.456 + dtemp3;
            
            /* Another function call with different arguments */
            double result2 = compute_polynomial(dtemp1, dtemp2, dtemp3,
                                                temp1, temp2, temp3,
                                                ftemp1, ftemp2, ftemp3);
            
            /* Volatile writes to prevent optimization */
            global_sink = temp7;
            float_sink = ftemp5;
            double_sink = dtemp5 + result2;
            
            /* Accumulate results with complex expression */
            total += (int64_t)temp7 + (int64_t)result1 + (int64_t)result2
                   + (int64_t)ftemp5 + (int64_t)dtemp5;
            
            /* More inline asm to break live ranges */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3",
                         "xmm4", "xmm5", "xmm6", "xmm7");
        }
        
        /* Additional computations in outer loop */
        int outer_temp = outer * 17 + 23;
        float outer_ftemp = (float)outer * 3.14159f;
        double outer_dtemp = (double)outer * 2.71828;
        
        /* Vector operation in outer loop */
        v4si outer_vec = {outer_temp, outer_temp + 1, outer_temp + 2, outer_temp + 3};
        v4sf outer_vf = {outer_ftemp, outer_ftemp * 2.0f, 
                         outer_ftemp * 3.0f, outer_ftemp * 4.0f};
        
        /* Use vectors */
        v4si outer_vec2 = outer_vec + (v4si){10, 20, 30, 40};
        v4sf outer_vf2 = outer_vf * (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        
        asm volatile("" : : "x"(outer_vec2), "x"(outer_vf2) : "memory");
        
        total += outer_temp + (int64_t)outer_ftemp + (int64_t)outer_dtemp;
    }
    
    printf("Result: %ld\n", (long)total);
    return 0;
}
