/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               v4si v) {
    int sum = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    sum += v[0] + v[1] + v[2] + v[3];
    global_sink = sum;
    return sum & 1;
}

/* Another non-inline function for more pressure */
__attribute__((noinline))
double compute_pressure(double base, int iter, float factor) {
    volatile double temp = base;
    for (int i = 0; i < 3; i++) {
        temp = temp * factor + iter * 0.5;
        /* Inline asm that clobbers registers */
        asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx");
    }
    return temp;
}

int main(void) {
    /* Initialize arrays to feed computations */
    int array1[256];
    float array2[256];
    double array3[256];
    v4si vectors[64];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 0.25f;
        array3[i] = i * 0.125;
    }
    
    for (int i = 0; i < 64; i++) {
        vectors[i] = (v4si){i, i*2, i*3, i*4};
    }
    
    /* Volatile pointer to force repeated dereferencing */
    volatile int* volatile_ptr = array1;
    
    /* Main computational kernel with high register pressure */
    int total = 0;
    
    /* Outer loop - creates many live ranges */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loops for more pressure */
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations with unique expressions */
            /* Each computation uses different combinations of variables */
            
            /* Group 1: Integer computations */
            int temp1 = array1[i] * 3 + outer / (i + 1);
            int temp2 = array1[i+1] * 7 - outer % (i + 2);
            int temp3 = temp1 * temp2 + i * outer;
            int temp4 = (temp1 + temp2) * (temp3 - i);
            int temp5 = temp3 * temp4 / (outer + 1);
            int temp6 = (temp4 << 3) | (temp5 & 0xFF);
            int temp7 = temp6 * 13 - temp5 * 11;
            int temp8 = (temp7 ^ temp6) + (temp5 & temp4);
            
            /* Group 2: Floating-point computations */
            float ftemp1 = array2[i] * 2.5f + outer * 0.1f;
            float ftemp2 = array2[i+1] * 1.75f - i * 0.05f;
            float ftemp3 = ftemp1 * ftemp2 / (i + 1.0f);
            float ftemp4 = (ftemp1 + ftemp2) * (ftemp3 - outer * 0.01f);
            float ftemp5 = ftemp3 * ftemp4 * 0.5f;
            float ftemp6 = ftemp4 / (ftemp5 + 1.0f) + i * 0.25f;
            
            /* Group 3: Double precision computations */
            double dtemp1 = array3[i] * 1.25 + outer * 0.125;
            double dtemp2 = array3[i+1] * 2.75 - i * 0.0625;
            double dtemp3 = dtemp1 * dtemp2 / (outer + 1.0);
            double dtemp4 = (dtemp1 + dtemp2) * (dtemp3 - i * 0.03125);
            double dtemp5 = dtemp3 * dtemp4 * 0.75;
            double dtemp6 = dtemp4 / (dtemp5 + 1.0) + outer * 0.015625;
            
            /* Group 4: Vector operations */
            v4si vtemp1 = vectors[i % 64] + (v4si){i, outer, i*outer, i+outer};
            v4si vtemp2 = vectors[(i+1) % 64] * (v4si){2, 3, 4, 5};
            v4si vtemp3 = vtemp1 + vtemp2;
            v4si vtemp4 = vtemp3 * (v4si){outer, i, outer+i, outer-i};
            
            /* Inline assembly that clobbers specific registers */
            /* Force compiler to save/restore or recompute values */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* More computations after clobber */
            int temp9 = temp8 * 17 + (i * outer) % 256;
            float ftemp7 = ftemp6 * 3.14f + outer * 0.123f;
            double dtemp7 = dtemp6 * 2.71828 + i * 0.456;
            
            /* Volatile memory access to prevent optimization */
            global_sink = temp9;
            float_sink = ftemp7;
            double_sink = dtemp7;
            
            /* Force repeated dereferencing through volatile pointer */
            int volatile_read = *volatile_ptr;
            volatile_ptr = &array1[(i + outer) % 256];
            
            /* Call function with many arguments - forces values into registers */
            int func_result = use_values(
                temp1, temp2, ftemp1, dtemp1,
                temp3, temp4, ftemp2, dtemp2,
                vtemp4
            );
            
            /* More computations using function result */
            int temp10 = temp9 + func_result * 19;
            float ftemp8 = ftemp7 * (func_result + 1) * 0.5f;
            double dtemp8 = dtemp7 * (func_result + 1) * 0.25;
            
            /* Another function call for more pressure */
            double pressure_result = compute_pressure(dtemp8, i, ftemp8);
            
            /* Final aggregation with unique expression per iteration */
            total += temp10 + (int)ftemp8 + (int)pressure_result + 
                    volatile_read + vtemp4[0] + vtemp4[1];
            
            /* Another asm clobber to break live ranges */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                       "xmm8", "xmm9", "xmm10", "xmm11");
            
            /* Additional loop to create more temporary values */
            for (int j = 0; j < 4; j++) {
                int inner_temp = (temp10 * j + i * outer) % 1024;
                float inner_ftemp = ftemp8 * j * 0.333f;
                double inner_dtemp = pressure_result * j * 0.666;
                
                /* Small computation chain */
                inner_temp = inner_temp * 3 + j;
                inner_ftemp = inner_ftemp * 1.5f - j;
                inner_dtemp = inner_dtemp / (j + 1.0) + 1.0;
                
                total += inner_temp + (int)inner_ftemp + (int)inner_dtemp;
            }
        }
        
        /* Periodic memory barrier */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
