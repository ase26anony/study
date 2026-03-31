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
                                     v4si vi, v4sf vf) {
    /* Force computation to prevent inlining */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) : : "memory");
    return a + b + c + d + (int)e + (int)f + (int)g + (int)h + vi[0] + (int)vf[0];
}

/* Another non-inline function with different signature */
NOINLINE NOCLONE double compute_complex(double base, int iter,
                                        float f1, float f2,
                                        int i1, int i2, int i3) {
    /* Complex computation that can't be optimized away */
    double result = base;
    for (int i = 0; i < 3; i++) {
        result = result * f1 + f2 - i1 * i2 + i3;
        asm volatile("" : "+x"(result) : : "memory");
    }
    return result;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent optimization */
    volatile int array_int[256];
    volatile float array_float[256];
    volatile double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i;
        array_float[i] = i * 1.5f;
        array_double[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    v4si vec_acc_int = {0, 0, 0, 0};
    v4sf vec_acc_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Main computational kernel - designed for high register pressure */
    for (int outer = 0; outer < 1000; outer++) {
        for (int inner = 0; inner < 128; inner++) {
            /* Create many independent computations with different expressions */
            int idx1 = (inner * 3) % 256;
            int idx2 = (inner * 5) % 256;
            int idx3 = (inner * 7) % 256;
            int idx4 = (inner * 11) % 256;
            
            /* Force register pressure with many temporaries */
            int temp1 = array_int[idx1] * outer + inner;
            int temp2 = array_int[idx2] / (outer + 1) - inner;
            int temp3 = temp1 * temp2 + array_int[idx3];
            int temp4 = temp2 - temp1 * array_int[idx4];
            int temp5 = (temp3 << 3) | (temp4 & 0xFF);
            int temp6 = temp5 ^ (inner * outer);
            int temp7 = temp6 + (temp1 % (temp2 + 1));
            int temp8 = temp7 * 7 - temp3 / 3;
            
            /* Floating-point computations - different register class */
            float ftemp1 = array_float[idx1] * outer + inner;
            float ftemp2 = array_float[idx2] / (outer + 1.0f) - inner;
            float ftemp3 = ftemp1 * ftemp2 + array_float[idx3];
            float ftemp4 = ftemp2 - ftemp1 * array_float[idx4];
            float ftemp5 = ftemp3 * 3.14159f + ftemp4 / 2.71828f;
            float ftemp6 = ftemp5 * ftemp1 - ftemp2 * ftemp3;
            
            /* Double precision - more register pressure */
            double dtemp1 = array_double[idx1] * outer + inner;
            double dtemp2 = array_double[idx2] / (outer + 1.0) - inner;
            double dtemp3 = dtemp1 * dtemp2 + array_double[idx3];
            double dtemp4 = dtemp2 - dtemp1 * array_double[idx4];
            double dtemp5 = dtemp3 * 3.14159265358979 + dtemp4 / 2.71828182845904;
            double dtemp6 = dtemp5 * dtemp1 - dtemp2 * dtemp3;
            
            /* Vector operations - consume SIMD registers */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, temp7, temp8};
            v4si vec3 = vec1 + vec2 * (inner % 8);
            v4si vec4 = vec1 - vec2 / ((outer % 4) + 1);
            
            v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf fvec2 = {ftemp5, ftemp6, ftemp1, ftemp2};
            v4sf fvec3 = fvec1 + fvec2 * (inner * 0.1f);
            v4sf fvec4 = fvec1 - fvec2 / ((outer * 0.01f) + 1.0f);
            
            /* Inline assembly that clobbers registers */
            /* Clobber general purpose registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Call non-inline function with many arguments */
            int func_result = use_many_values(
                temp1, temp2, temp3, temp4,
                ftemp1, ftemp2, dtemp1, dtemp2,
                vec3, fvec3
            );
            
            /* Another function call with different arguments */
            double complex_result = compute_complex(
                dtemp3, inner,
                ftemp3, ftemp4,
                temp5, temp6, temp7
            );
            
            /* Volatile writes to prevent optimization */
            global_sink = func_result;
            float_sink = ftemp5;
            double_sink = complex_result;
            
            /* Accumulate results with complex expressions */
            total_int += temp8 + func_result + vec3[0] + vec3[1];
            total_float += ftemp6 + fvec3[0] + fvec4[1];
            total_double += dtemp6 + complex_result;
            
            vec_acc_int += vec3 + vec4;
            vec_acc_float += fvec3 + fvec4;
            
            /* More inline assembly to break live ranges */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "memory");
        }
        
        /* Additional computation between inner loops */
        if (outer % 100 == 0) {
            /* Force spill/reload opportunities */
            int spill_temp = total_int * 3 - total_int / 2;
            float fspill_temp = total_float * 1.5f - total_float / 3.0f;
            double dspill_temp = total_double * 2.5 - total_double / 4.0;
            
            asm volatile("" : "+r"(spill_temp), "+x"(fspill_temp), "+x"(dspill_temp) : : "memory");
            
            total_int = spill_temp % 1000;
            total_float = fspill_temp;
            total_double = dspill_temp;
        }
    }
    
    /* Final aggregation to prevent elimination */
    int final_result = total_int + (int)total_float + (int)total_double
                     + vec_acc_int[0] + vec_acc_int[1] + vec_acc_int[2] + vec_acc_int[3]
                     + (int)vec_acc_float[0] + (int)vec_acc_float[1]
                     + (int)vec_acc_float[2] + (int)vec_acc_float[3];
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
