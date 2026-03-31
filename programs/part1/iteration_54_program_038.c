/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

/* Force function to not be inlined */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_many_values(int a, int b, int c, float d, double e, 
                    long f, short g, char h, int i, int j) {
    volatile int sink;
    sink = a + b + c + (int)d + (int)e + f + g + h + i + j;
    return sink & 1;
}

/* Another noinline function with different signature */
__attribute__((noinline))
float compute_float(float a, float b, float c, float d, float e) {
    volatile float result;
    result = a * b + c / d - e;
    return result;
}

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile float global_float_sink;

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
    
    /* Volatile pointer to force repeated dereferencing */
    volatile int* volatile_ptr = array_int;
    
    /* Result accumulator */
    int total_result = 0;
    
    /* Nested loops to create high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 256; inner++) {
            /* Many independent arithmetic operations creating temporaries */
            int idx = (inner + outer) & 255;
            
            /* Force register pressure with many unique computations */
            int temp1 = array_int[idx] * 3 + outer;
            int temp2 = array_int[(idx + 1) & 255] / 7 - inner;
            int temp3 = temp1 * temp2 + idx;
            int temp4 = (temp1 << 3) | (temp2 & 0xF);
            int temp5 = temp3 ^ temp4;
            int temp6 = (temp5 * 1103515245 + 12345) & 0x7FFFFFFF;
            
            /* Floating-point computations (uses different registers) */
            float ftemp1 = array_float[idx] * 2.0f + outer * 0.1f;
            float ftemp2 = array_float[(idx + 2) & 255] / 3.0f - inner * 0.01f;
            float ftemp3 = ftemp1 * ftemp2 + idx * 0.001f;
            float ftemp4 = ftemp1 / (ftemp2 + 1.0f) - ftemp3;
            float ftemp5 = ftemp3 * ftemp4 * (inner + 1);
            
            /* Double precision computations */
            double dtemp1 = array_double[idx] * 1.7 + outer * 0.3;
            double dtemp2 = array_double[(idx + 3) & 255] / 2.9 - inner * 0.07;
            double dtemp3 = dtemp1 * dtemp2 + idx * 0.001;
            double dtemp4 = dtemp1 / (dtemp2 + 1.0) - dtemp3;
            double dtemp5 = dtemp3 * dtemp4 * (inner + 1) * 0.5;
            
            /* Vector operations (consumes multiple registers) */
            v4si vec_a = {temp1, temp2, temp3, temp4};
            v4si vec_b = {temp2, temp3, temp4, temp5};
            v4si vec_c = vec_a + vec_b;
            v4si vec_d = vec_a * vec_b;
            v4si vec_e = vec_c - vec_d;
            
            v4sf vec_f = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf vec_g = {ftemp2, ftemp3, ftemp4, ftemp5};
            v4sf vec_h = vec_f + vec_g;
            v4sf vec_i = vec_f * vec_g;
            v4sf vec_j = vec_h - vec_i;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile(
                "# Clobber important registers\n"
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
            
            /* More computations after clobbering */
            int temp7 = temp6 * 2 + (vec_e[0] & 0xFF);
            int temp8 = temp7 ^ (int)(ftemp5 * 100.0f);
            int temp9 = temp8 + (int)(dtemp5 * 10.0);
            int temp10 = temp9 * 3 - (vec_j[0] > 0 ? 1 : 0);
            
            /* Call function with many arguments - forces moving values to registers */
            int func_result = use_many_values(
                temp1, temp2, temp3, ftemp1, dtemp1,
                (long)temp4, (short)temp5, (char)temp6,
                temp7, temp8
            );
            
            /* Another function call */
            float float_result = compute_float(ftemp1, ftemp2, ftemp3, ftemp4, ftemp5);
            
            /* Volatile memory operations */
            global_sink = temp10;
            global_float_sink = float_result;
            
            /* Force dereference through volatile pointer */
            int volatile_read = *volatile_ptr;
            
            /* More computations using volatile read */
            int temp11 = temp10 + func_result + volatile_read;
            int temp12 = temp11 * 31 + (int)(float_result * 100.0f);
            int temp13 = temp12 ^ (inner * 17);
            int temp14 = temp13 + outer * 13;
            
            /* Another inline assembly to clobber more registers */
            asm volatile(
                "# Clobber more registers\n"
                "mov $0, %%r8d\n"
                "mov $0, %%r9d\n"
                "mov $0, %%r10d\n"
                "mov $0, %%r11d\n"
                "pxor %%xmm2, %%xmm2\n"
                "pxor %%xmm3, %%xmm3\n"
                "pxor %%xmm4, %%xmm4\n"
                :
                :
                : "r8", "r9", "r10", "r11", "xmm2", "xmm3", "xmm4", "memory"
            );
            
            /* Final computation chain */
            int temp15 = temp14 + (vec_e[1] & 0xF);
            int temp16 = temp15 * 2 - (vec_e[2] >> 1);
            int temp17 = temp16 ^ (vec_e[3] << 2);
            int temp18 = temp17 + (int)(vec_j[1] * 50.0f);
            int temp19 = temp18 * 3 + (int)(dtemp5 * 20.0);
            int temp20 = temp19 & 0xFFFF;
            
            /* Accumulate result */
            total_result += temp20;
            
            /* Update volatile pointer to force different memory access */
            volatile_ptr = &array_int[(inner + 5) & 255];
        }
        
        /* Additional computation between outer loop iterations */
        int loop_temp = outer * 7 + total_result & 0xFF;
        total_result ^= loop_temp;
        
        /* Another function call with different arguments */
        use_many_values(
            outer, total_result & 0xFF, loop_temp,
            outer * 0.5f, outer * 1.5,
            (long)(total_result >> 8), (short)loop_temp, (char)outer,
            outer * 3, outer * 5
        );
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", total_result);
    
    return total_result & 1;
}
