/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Non-inline function to force register pressure */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
int use_values(int a, int b, float c, double d, 
               long e, short f, unsigned g, char h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + g + h;
    return sink & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline))
double complex_op(double x, double y, int scale, float factor) {
    volatile double result;
    result = (x * y * scale) / (factor + 1.0);
    return result;
}

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

int main(void) {
    /* Initialize arrays to feed computations */
    double array1[256], array2[256];
    float farray1[256], farray2[256];
    int iarray1[256], iarray2[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = sin(i * 0.1);
        array2[i] = cos(i * 0.05);
        farray1[i] = i * 0.25f;
        farray2[i] = i * 0.33f;
        iarray1[i] = i * 3;
        iarray2[i] = i * 7;
    }
    
    /* Volatile pointer to force memory operations */
    volatile double *volatile ptr1 = array1;
    volatile float *volatile ptr2 = farray1;
    
    /* Main computational kernel with high register pressure */
    double total = 0.0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested inner loops with many temporaries */
        for (int i = 0; i < 128; i++) {
            /* Create many distinct temporary variables */
            double t1 = ptr1[i] * 1.2345;
            double t2 = ptr1[i+1] * 2.3456;
            double t3 = t1 + t2 * 3.4567;
            double t4 = sin(t3) * cos(t2);
            
            float f1 = ptr2[i] * 1.5f;
            float f2 = ptr2[i+1] * 2.5f;
            float f3 = f1 * f2 - f1 / (f2 + 0.001f);
            float f4 = f3 * f3 - sqrtf(fabsf(f2));
            
            int i1 = iarray1[i] * 11;
            int i2 = iarray2[i] * 13;
            int i3 = i1 ^ i2 + (i1 & i2) * 17;
            int i4 = i3 * i3 - (i3 >> 3) + (i << 2);
            
            /* More temporaries with mixed operations */
            double t5 = t4 * f4 + i4 * 0.01;
            double t6 = t5 * t5 - t4 * t4 + f4 * 0.5;
            float f5 = (float)t6 * 0.3f + (float)i4 * 0.7f;
            int i5 = (int)(t6 * 100.0) ^ (int)(f5 * 10.0);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", "r8", "r9", "r10",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "memory", "cc");
            
            /* Call non-inline function with many arguments */
            int r1 = use_values(i1, i2, f1, t1, 
                               (long)i3, (short)i4, 
                               (unsigned)i5, (char)i);
            
            /* More computations after call (forces rematerialization) */
            double t7 = t6 * (r1 + 1) * 0.5;
            float f6 = f5 * (r1 % 7 + 1) * 0.3f;
            int i6 = i5 * (r1 | 1) + (i & 0xFF);
            
            /* Vector operations to consume SIMD registers */
            v4si vec1 = {i1, i2, i3, i4};
            v4si vec2 = {i5, i6, i, outer};
            v4si vec3 = vec1 + vec2 * vec1 - vec2;
            
            v4sf vecf1 = {f1, f2, f3, f4};
            v4sf vecf2 = {f5, f6, (float)t1, (float)t2};
            v4sf vecf3 = vecf1 * vecf2 + vecf1 / (vecf2 + 0.1f);
            
            /* Another assembly clobber */
            asm volatile("" : : : 
                "xmm12", "xmm13", "xmm14", "xmm15",
                "r11", "r12", "r13", "r14", "r15",
                "memory");
            
            /* Call another function */
            double r2 = complex_op(t7, t6, i6, f6);
            
            /* Final computations mixing all values */
            double final_val = t7 * r2 + vecf3[0] * 0.1 + vec3[0] * 0.01;
            
            /* Volatile write to prevent elimination */
            global_accumulator += final_val;
            
            /* Accumulate to total (prevents dead code elimination) */
            total += final_val * (i % 8 + 1) * (outer % 4 + 1);
            
            /* More register pressure with chain of dependencies */
            double chain1 = total * 0.99;
            float chain2 = (float)chain1 * 1.01f;
            int chain3 = (int)chain2 ^ i;
            double chain4 = chain1 * sin(chain2) + cos(chain3);
            total = chain4 * 0.5 + total * 0.5;
            
            /* Update volatile global */
            global_counter += i & 1;
        }
        
        /* Additional computations between outer loop iterations */
        double outer_temp = sin(outer * 0.1) * cos(outer * 0.05);
        float outer_ftemp = (float)outer_temp * outer * 0.1f;
        int outer_itemp = (int)(outer_temp * 1000) ^ outer;
        
        /* Force spill/reload opportunities */
        for (int j = 0; j < 4; j++) {
            double loop_temp = outer_temp * j;
            float loop_ftemp = outer_ftemp * j;
            int loop_itemp = outer_itemp * j;
            
            /* Many independent expressions */
            double a = loop_temp * 1.1 + loop_ftemp * 0.9;
            double b = a * a - loop_temp * loop_temp;
            double c = b * 0.8 + loop_itemp * 0.2;
            double d = c * sin(b) + cos(a);
            
            total += d * 0.01;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %f (counter: %d)\n", total, global_counter);
    
    return (int)(total * 1000) & 0xFF;
}
