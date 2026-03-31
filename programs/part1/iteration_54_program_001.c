/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector type to consume multiple registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments to force register pressure */
NOINLINE int use_many_values(int a, int b, int c, int d, 
                             float e, float f, double g, double h,
                             v4si vi, v4sf vf) {
    /* Force computation to prevent optimization */
    int sum_i = a + b + c + d;
    float sum_f = e + f;
    double sum_d = g + h;
    
    /* Extract vector elements */
    int vi_sum = vi[0] + vi[1] + vi[2] + vi[3];
    float vf_sum = vf[0] + vf[1] + vf[2] + vf[3];
    
    return sum_i + (int)sum_f + (int)sum_d + vi_sum + (int)vf_sum;
}

/* Another non-inline function with different signature */
NOINLINE double compute_complex(double a, double b, double c, double d,
                                double e, double f, double g, double h) {
    /* Complex expression with many temporaries */
    double t1 = a * b + c / (d + 1.0);
    double t2 = sin(e) * cos(f);
    double t3 = sqrt(g * g + h * h);
    double t4 = log(fabs(t1) + 1.0);
    double t5 = exp(-t2 * t2);
    double t6 = t3 * t4 * t5;
    
    return t1 + t2 + t3 + t4 + t5 + t6;
}

int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = (i * 37) % 101;
        array_float[i] = (i * 0.12345f) + 1.0f;
        array_double[i] = (i * 0.67891) + 2.0;
    }
    
    /* Accumulator to prevent optimization */
    int total = 0;
    double dtotal = 0.0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating short-lived temporaries */
            /* Each computation is slightly unique to avoid CSE */
            
            /* Integer computations */
            int t1 = array_int[i] * 3 + outer;
            int t2 = array_int[i+1] / 5 - outer;
            int t3 = array_int[i+2] % 7 + i;
            int t4 = (array_int[i+3] << 2) | (array_int[i] & 0xF);
            int t5 = t1 * t2 - t3 + t4;
            int t6 = (t2 << 3) + (t3 >> 1) * t4;
            int t7 = t5 ^ t6 ^ (i * outer);
            int t8 = (t7 * 1103515245 + 12345) & 0x7FFFFFFF;
            
            /* Floating-point computations */
            float f1 = array_float[i] * 1.234f + outer * 0.01f;
            float f2 = array_float[i+1] / 2.345f - i * 0.001f;
            float f3 = f1 * f2 + sinf(f1) - cosf(f2);
            float f4 = f3 * f3 - f1 * f2 + tanf(f3 * 0.1f);
            
            /* Double precision computations */
            double d1 = array_double[i] * 1.23456789 + outer * 0.001;
            double d2 = array_double[i+1] / 2.3456789 - i * 0.0001;
            double d3 = d1 * d2 + sin(d1) - cos(d2);
            double d4 = d3 * d3 - d1 * d2 + tan(d3 * 0.01);
            double d5 = d4 * 1.1 + d3 * 0.9 - d2 * 0.8 + d1 * 0.7;
            
            /* Vector operations */
            v4si vi1 = {t1, t2, t3, t4};
            v4si vi2 = {t5, t6, t7, t8};
            v4si vi3 = vi1 + vi2 * 2 - vi1 / 3;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = vf1 * 1.5f - vf1 / 2.0f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber commonly used registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* More computations after clobber */
            int t9 = t8 * 2 + (i % 13);
            float f5 = f4 * 3.14f + (outer % 17) * 0.01f;
            double d6 = d5 * 2.71828 + (i % 19) * 0.001;
            
            /* Call function with many arguments - forces register pressure */
            int result = use_many_values(t1, t2, t3, t9,
                                        f1, f5, d1, d6,
                                        vi3, vf2);
            
            /* Complex double computation */
            double dresult = compute_complex(d1, d2, d3, d4, d5, d6, 
                                           array_double[i], array_double[i+1]);
            
            /* Volatile writes to prevent optimization */
            global_sink = result;
            float_sink = f5;
            double_sink = dresult;
            
            /* Accumulate results */
            total += result + t9 + (int)f5;
            dtotal += dresult + d6;
            
            /* Another inline assembly barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Additional computations between outer loop iterations */
        int extra = (outer * 97) % 113;
        float fextra = sinf(outer * 0.1f) * 100.0f;
        double dextra = cos(outer * 0.01) * 200.0;
        
        total += extra + (int)fextra;
        dtotal += dextra;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Total: %d, Double total: %f\n", total, dtotal);
    
    return 0;
}
