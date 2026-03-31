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

/* Non-inline function to force register usage for arguments */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int use_values(int a, int b, float c, double d, 
               v4si vec_int, v4sf vec_float, int extra) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + vec_int[0] + (int)vec_float[0] + extra;
    return sink;
}

/* Another non-inline function with different signature */
__attribute__((noinline))
double compute_more(double x, double y, double z, 
                    int i, int j, int k, float f1, float f2) {
    volatile double result;
    result = (x * y) / z + (double)(i * j) / k + f1 * f2;
    return result;
}

/* Main computational kernel */
int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int array[256];
    volatile float farray[256];
    volatile double darray[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
        farray[i] = i * 1.5f;
        darray[i] = i * 2.7;
    }
    
    /* Result accumulator */
    int total_result = 0;
    
    /* Nested loops to create high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating short-lived temporaries */
            int temp1 = array[i] * 3 + outer;
            int temp2 = array[i+1] / 2 - outer;
            int temp3 = temp1 * temp2 + i;
            int temp4 = temp2 - temp1 * i;
            int temp5 = (temp3 << 3) | (temp4 & 0xFF);
            int temp6 = temp5 ^ (i * outer);
            int temp7 = temp6 + (temp1 % 17);
            int temp8 = temp7 * 2 - temp3;
            
            /* Floating point computations - uses different register class */
            float ftemp1 = farray[i] * 3.14159f + outer;
            float ftemp2 = farray[i+1] / 2.71828f - outer;
            float ftemp3 = ftemp1 * ftemp2 + i;
            float ftemp4 = ftemp2 - ftemp1 * i;
            float ftemp5 = ftemp3 * 2.0f + ftemp4;
            float ftemp6 = ftemp5 / (ftemp1 + 1.0f);
            
            /* Double precision - more register pressure */
            double dtemp1 = darray[i] * 1.23456789 + outer;
            double dtemp2 = darray[i+1] / 9.87654321 - outer;
            double dtemp3 = dtemp1 * dtemp2 + i;
            double dtemp4 = dtemp2 - dtemp1 * i;
            double dtemp5 = dtemp3 * 3.1415926535 + dtemp4;
            double dtemp6 = dtemp5 / (dtemp1 + 1.0);
            
            /* Vector operations - consume multiple registers */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, temp7, temp8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf fvec2 = {ftemp5, ftemp6, ftemp1, ftemp2};
            v4sf fvec3 = fvec1 * fvec2;
            v4sf fvec4 = fvec1 + fvec2;
            v4sf fvec5 = fvec3 - fvec4;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* More computations after clobber */
            int temp9 = temp8 * 3 + (int)ftemp5;
            int temp10 = temp9 / (i + 1) + (int)dtemp5;
            float ftemp7 = ftemp6 * 2.0f + (float)temp10;
            double dtemp7 = dtemp6 * 1.5 + (double)ftemp7;
            
            /* Call non-inline function with many arguments */
            int func_result = use_values(temp9, temp10, ftemp7, dtemp7,
                                        vec5, fvec5, outer + i);
            
            /* Another function call with different arguments */
            double dresult = compute_more(dtemp1, dtemp2, dtemp3,
                                         temp1, temp2, temp3,
                                         ftemp1, ftemp2);
            
            /* More inline assembly to break up live ranges */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
            
            /* Complex expression with many temporaries */
            int final_temp = 
                (temp1 * temp2) + (temp3 / (temp4 + 1)) -
                (temp5 ^ temp6) + (temp7 & 0xFF) +
                (int)(ftemp1 * 100.0f) + (int)(dtemp1 * 10.0) +
                vec5[0] + vec5[1] + (int)fvec5[0] +
                func_result + (int)dresult;
            
            /* Volatile write to prevent elimination */
            array[i] = final_temp;
            
            /* Accumulate result */
            total_result += final_temp;
            
            /* More computations to increase pressure */
            for (int j = 0; j < 4; j++) {
                int inner_temp = temp1 + j * outer;
                float inner_ftemp = ftemp1 + j * 0.5f;
                double inner_dtemp = dtemp1 + j * 0.25;
                
                /* SSE intrinsics for x86 */
                __m128 sse_vec1 = _mm_set_ps(inner_ftemp, ftemp2, ftemp3, ftemp4);
                __m128 sse_vec2 = _mm_set_ps(ftemp5, ftemp6, ftemp7, inner_ftemp);
                __m128 sse_result = _mm_add_ps(sse_vec1, sse_vec2);
                
                volatile float sse_sink;
                sse_sink = ((float*)&sse_result)[0] + ((float*)&sse_result)[1];
                
                total_result += inner_temp + (int)inner_ftemp + (int)inner_dtemp;
            }
        }
        
        /* Additional outer loop computations */
        int outer_temp = outer * 7;
        for (int k = 0; k < 8; k++) {
            outer_temp = (outer_temp * 1103515245 + 12345) & 0x7fffffff;
            float outer_ftemp = (float)outer_temp / 1000.0f;
            double outer_dtemp = (double)outer_temp / 10000.0;
            
            /* More register pressure */
            int chain1 = outer_temp + k;
            int chain2 = chain1 * 3;
            int chain3 = chain2 / 5;
            int chain4 = chain3 - chain1;
            int chain5 = chain4 ^ chain2;
            int chain6 = chain5 & chain3;
            
            total_result += chain6 + (int)outer_ftemp + (int)outer_dtemp;
        }
    }
    
    printf("Total result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
