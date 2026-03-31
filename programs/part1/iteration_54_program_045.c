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
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("no-ipa")))
int use_many_values(int a, int b, int c, float d, double e, 
                    long f, short g, v4si h, v4sf i) {
    /* Force side effects */
    asm volatile("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return (a + b + c) ^ (int)d ^ (int)e ^ (int)f ^ g;
}

/* Another non-inline function for floating point pressure */
__attribute__((noinline, optimize("no-ipa")))
double compute_pressure(double x, double y, double z, 
                        float a, float b, float c) {
    /* Complex expression that can't be easily optimized */
    double t1 = x * y + z;
    double t2 = x / (y + 1.0);
    double t3 = t1 * t2 - (x + y + z);
    double t4 = (a + b + c) * t3;
    
    /* Force register usage */
    asm volatile("" : : "x"(t1), "x"(t2), "x"(t3), "x"(t4) : 
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    return t4;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent lifting */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 0.5f;
        array3[i] = i * 0.25;
    }
    
    /* Result accumulator */
    int total_result = 0;
    double fp_result = 0.0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Many independent computations with unique expressions */
            int idx = (outer * 7 + inner * 3) & 255;
            
            /* Integer computations - each creates temporaries */
            int temp1 = array1[idx] * 3 + outer;
            int temp2 = array1[(idx + 1) & 255] / (inner + 2);
            int temp3 = temp1 - temp2 + (outer ^ inner);
            int temp4 = (temp1 * temp2) >> (inner & 3);
            int temp5 = (temp3 + temp4) * (outer - inner);
            int temp6 = temp5 ^ (array1[(idx + 2) & 255] << 1);
            int temp7 = (temp6 * 13) / (inner + 1);
            int temp8 = temp7 + (array1[(idx + 3) & 255] & 0xFF);
            int temp9 = (temp8 << 2) | (temp5 & 0xF);
            int temp10 = temp9 * temp3 - temp4 + temp2;
            
            /* Floating point computations - consume FP registers */
            float ftemp1 = array2[idx] * 2.5f + outer;
            float ftemp2 = array2[(idx + 4) & 255] / (inner + 3.0f);
            float ftemp3 = ftemp1 - ftemp2 * 0.7f;
            float ftemp4 = (ftemp1 * ftemp2) + (outer * 0.1f);
            float ftemp5 = ftemp3 * ftemp4 - (inner * 0.3f);
            
            /* Double precision computations */
            double dtemp1 = array3[idx] * 1.7 + outer;
            double dtemp2 = array3[(idx + 5) & 255] / (inner + 4.0);
            double dtemp3 = dtemp1 * dtemp2 - (outer * 0.25);
            double dtemp4 = (dtemp1 + dtemp2) * (inner * 0.5);
            double dtemp5 = dtemp3 / (dtemp4 + 1.0);
            
            /* Vector operations - consume SIMD registers */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, temp7, temp8};
            v4si vec3 = vec1 + vec2 * 2;
            v4si vec4 = vec1 - vec2;
            v4si vec5 = vec3 * vec4;
            
            v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf fvec2 = {ftemp5, ftemp1 * 0.5f, ftemp2 * 0.3f, ftemp3 * 0.7f};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2 - fvec3;
            
            /* Inline assembly that clobbers registers */
            /* Clobber integer registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            /* Clobber floating point registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3", 
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_many_values(
                temp1, temp2, temp3, ftemp1, dtemp1,
                (long)temp4 * inner, (short)temp5, vec5, fvec4);
            
            /* More computations after function call */
            int temp11 = func_result * 17 + temp10;
            float ftemp6 = ftemp5 * 2.0f + (float)func_result;
            double dtemp6 = compute_pressure(dtemp2, dtemp3, dtemp4, 
                                            ftemp2, ftemp3, ftemp4);
            
            /* Volatile writes to prevent elimination */
            global_sink = temp11;
            float_sink = ftemp6;
            
            /* More register pressure with complex addressing */
            int addr1 = (idx + outer) & 255;
            int addr2 = (idx + inner) & 255;
            int addr3 = (idx + outer + inner) & 255;
            
            int mem_temp1 = array1[addr1] + array1[addr2];
            int mem_temp2 = array1[addr3] * array1[(addr1 + 1) & 255];
            int mem_temp3 = mem_temp1 - mem_temp2 + array1[(addr2 + 2) & 255];
            
            /* Final accumulation with mixing */
            total_result ^= temp11 + mem_temp3;
            fp_result += dtemp6 + (double)ftemp6;
            
            /* Another round of clobbering */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
        }
        
        /* Periodic complex computation to break patterns */
        if (outer % 7 == 0) {
            double complex_temp = 0.0;
            for (int k = 0; k < 8; k++) {
                complex_temp += (array3[(outer + k) & 255] * 
                               (1.0 + k * 0.125)) / (outer + 1.0);
            }
            fp_result *= (1.0 + complex_temp * 0.01);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Integer result: %d\n", total_result);
    printf("Floating result: %f\n", fp_result);
    
    return (total_result & 0xFF) | ((int)fp_result & 0xFF);
}
