/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("no-ipa")))
int use_values(int a, int b, float c, double d, 
               long e, short f, char g, int h) {
    return a + b + (int)c + (int)d + e + f + g + h;
}

/* Another non-inline function for more pressure */
__attribute__((noinline, optimize("no-ipa")))
float compute_more(float a, float b, float c, float d,
                   float e, float f, float g, float h) {
    return a * b + c * d - e * f + g / h;
}

int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i * 3 + 1;
        array_float[i] = i * 1.5f + 0.5f;
        array_double[i] = i * 2.7 + 1.3;
    }
    
    /* Variables to accumulate results */
    int total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    
    /* Vector variables for register pressure */
    v4si vec_acc = {0, 0, 0, 0};
    v4sf vec_float_acc = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Main computational kernel with high register pressure */
    for (int outer = 0; outer < 1000; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Create many independent computations with unique expressions */
            /* Each computation creates temporary values that need registers */
            
            /* Integer computations - each creates unique temporaries */
            int temp1 = array_int[i] * 3 + outer;
            int temp2 = array_int[(i + 1) % 256] / 2 - outer;
            int temp3 = temp1 * temp2 + i;
            int temp4 = temp2 - temp1 * i;
            int temp5 = (temp3 << 3) | (temp4 & 0xFF);
            int temp6 = temp5 ^ (i * 7 + outer);
            
            /* Floating-point computations - uses FP registers */
            float ftemp1 = array_float[i] * 2.3f + outer * 0.1f;
            float ftemp2 = array_float[(i + 2) % 256] / 1.7f - outer * 0.2f;
            float ftemp3 = ftemp1 * ftemp2 + i * 0.5f;
            float ftemp4 = ftemp2 - ftemp1 * i * 0.3f;
            float ftemp5 = ftemp3 * ftemp4 / (i + 1);
            float ftemp6 = ftemp5 + ftemp1 - ftemp2;
            
            /* Double precision computations */
            double dtemp1 = array_double[i] * 1.9 + outer * 0.01;
            double dtemp2 = array_double[(i + 3) % 256] / 1.3 - outer * 0.02;
            double dtemp3 = dtemp1 * dtemp2 + i * 0.1;
            double dtemp4 = dtemp2 - dtemp1 * i * 0.05;
            double dtemp5 = dtemp3 * dtemp4 / (i + 1.0);
            double dtemp6 = dtemp5 + dtemp1 - dtemp2;
            
            /* Vector operations - consume multiple registers */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, i, outer};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            vec_acc += vec5;
            
            v4sf vf1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf vf2 = {ftemp5, ftemp6, i * 0.1f, outer * 0.1f};
            v4sf vf3 = vf1 + vf2;
            v4sf vf4 = vf1 * vf2;
            v4sf vf5 = vf3 - vf4;
            vec_float_acc += vf5;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple general purpose and floating point registers */
            asm volatile(
                "# Force register pressure\n"
                "mov %%eax, %%eax\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", 
                  "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "memory"
            );
            
            /* Call non-inline function with many arguments */
            /* Forces values into argument registers */
            int func_result = use_values(
                temp1, temp2, ftemp3, dtemp4,
                (long)temp5 * i, (short)temp6, (char)i, outer
            );
            
            /* Another function call with floating-point arguments */
            float float_result = compute_more(
                ftemp1, ftemp2, ftemp3, ftemp4,
                ftemp5, ftemp6, array_float[i], array_float[(i + 1) % 256]
            );
            
            /* Volatile memory operations to prevent optimization */
            global_sink = func_result;
            float_sink = float_result;
            
            /* More computations using function results */
            int combined1 = func_result * 2 + temp3;
            float combined2 = float_result * 1.5f + ftemp4;
            double combined3 = dtemp5 * 2.0 + dtemp6;
            
            /* Additional arithmetic to create more temporaries */
            int extra1 = combined1 ^ (i * 11);
            int extra2 = extra1 * 3 - outer;
            int extra3 = extra2 / 7 + temp4;
            int extra4 = extra3 << 2;
            
            float fextra1 = combined2 * 0.7f + i;
            float fextra2 = fextra1 / 1.3f - outer;
            float fextra3 = fextra2 * fextra1;
            float fextra4 = fextra3 + array_float[i];
            
            /* Accumulate results to prevent dead code elimination */
            total_int += extra4;
            total_float += fextra4;
            total_double += combined3;
            
            /* Another inline assembly to break live ranges */
            asm volatile(
                "# Break live ranges\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11",
                  "xmm6", "xmm7", "xmm8", "xmm9",
                  "xmm10", "xmm11", "xmm12", "xmm13", "memory"
            );
            
            /* More independent computations */
            int chain1 = i * 17 + outer;
            int chain2 = chain1 * 3 - array_int[i];
            int chain3 = chain2 / 5 + temp6;
            int chain4 = chain3 ^ 0xABCD;
            
            float fchain1 = i * 2.7f + outer;
            float fchain2 = fchain1 * 1.3f - array_float[i];
            float fchain3 = fchain2 / 2.5f + ftemp5;
            float fchain4 = fchain3 * 0.9f;
            
            /* Use all these values in a complex expression */
            int final_int = chain4 + extra4 + (int)fchain4;
            total_int += final_int;
        }
        
        /* Periodic reduction to prevent overflow */
        if (outer % 100 == 0) {
            total_int %= 1000000;
            total_float = fmodf(total_float, 1000.0f);
            total_double = fmod(total_double, 10000.0);
        }
    }
    
    /* Extract and sum vector elements */
    int vec_sum = 0;
    float vec_float_sum = 0.0f;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_acc[i];
        vec_float_sum += vec_float_acc[i];
    }
    
    /* Final aggregation */
    int final_result = total_int + (int)total_float + (int)total_double 
                     + vec_sum + (int)vec_float_sum;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 100;
}
