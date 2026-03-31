/* early-remat-test.c - Program to trigger GCC's early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile double double_sink;

/* Non-inline function with many arguments */
__attribute__((noinline, optimize("O0")))
int use_values(int a, int b, float c, double d, 
               long e, short f, int g, double h) {
    return a + b + (int)c + (int)d + e + f + g + (int)h;
}

/* Another non-inline function for more pressure */
__attribute__((noinline))
float complex_math(float x, float y, float z, 
                   float w, float u, float v) {
    return (x * y) + (z / w) - (u * v) + sinf(x) * cosf(y);
}

int main(void) {
    /* Initialize arrays with volatile reads to force memory ops */
    volatile int array[256];
    volatile float farray[256];
    volatile double darray[256];
    
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
        farray[i] = i * 1.5f;
        darray[i] = i * 2.7;
    }
    
    /* Accumulator to prevent dead code elimination */
    int64_t total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Use vector operations to consume SIMD registers */
        v4si vec1 = {outer, outer+1, outer+2, outer+3};
        v4si vec2 = {outer*2, outer*3, outer*4, outer*5};
        v4si vec_result = vec1 + vec2 * vec1 - vec2;
        
        for (int inner = 0; inner < 50; inner++) {
            /* Many independent arithmetic operations creating temporaries */
            int t1 = outer * inner;
            int t2 = t1 + outer;
            int t3 = t2 * inner;
            int t4 = t3 - outer;
            int t5 = t4 / (inner + 1);
            int t6 = t5 ^ outer;
            int t7 = t6 & inner;
            int t8 = t7 | outer;
            int t9 = t8 << (inner & 3);
            int t10 = t9 >> (outer & 3);
            
            /* Floating-point operations using different register class */
            float f1 = farray[inner] * 1.1f;
            float f2 = f1 + farray[outer % 256];
            float f3 = f2 / (farray[(inner + outer) % 256] + 1.0f);
            float f4 = f3 - sinf(f1);
            float f5 = f4 * cosf(f2);
            float f6 = f5 + tanf(f3);
            
            /* Double precision for more pressure */
            double d1 = darray[inner] * 2.3;
            double d2 = d1 + darray[outer % 256];
            double d3 = d2 / (darray[(inner + outer) % 256] + 1.0);
            double d4 = d3 - sin(d1);
            double d5 = d4 * cos(d2);
            double d6 = d5 + tan(d3);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* For ARM/AArch64 (commented out, choose based on target)
            asm volatile("" : : : 
                "x0", "x1", "x2", "x3",
                "v0", "v1", "v2", "v3",
                "v4", "v5", "v6", "v7",
                "memory");
            */
            
            /* Call non-inline function with many arguments */
            int result = use_values(t1, t2, f1, d1, 
                                   (long)t3, (short)t4, t5, d2);
            
            /* More arithmetic to create more temporaries */
            int t11 = result + inner;
            int t12 = t11 * outer;
            int t13 = t12 - result;
            int t14 = t13 / (inner + 2);
            int t15 = t14 ^ result;
            int t16 = t15 & outer;
            
            /* Another function call with floating arguments */
            float fresult = complex_math(f1, f2, f3, f4, f5, f6);
            
            /* Volatile writes to force side effects */
            global_sink = t16;
            double_sink = d6 + fresult;
            
            /* Use array accesses with volatile to prevent optimization */
            array[(inner + outer) % 256] = t16;
            farray[inner % 256] = fresult;
            darray[outer % 256] = d6;
            
            /* Accumulate results with complex expression */
            total += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                    t11 + t12 + t13 + t14 + t15 + t16 +
                    (int)fresult + (int)d6 +
                    vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
            
            /* More inline assembly to break live ranges */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                         "xmm8", "xmm9", "xmm10", "xmm11", "memory");
        }
        
        /* Update vector with loop-dependent values */
        vec1 = (v4si){outer*3, outer*4, outer*5, outer*6};
        vec2 = (v4si){outer*7, outer*8, outer*9, outer*10};
        vec_result = vec1 * vec2 + vec1 - vec2;
    }
    
    /* Final computation to use all accumulated values */
    double final_result = (double)total / 1000000.0;
    
    /* Print result to prevent optimization */
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
