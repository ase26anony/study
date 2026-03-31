/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments */
NOINLINE int use_many_values(int a, int b, int c, int d,
                             float e, float f, double g, double h,
                             v4si vi, v4sf vf) {
    /* Force register usage for all arguments */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d),
                      "r"(e), "r"(f), "r"(g), "r"(h),
                      "x"(vi), "x"(vf) : "memory");
    return a + b + c + d + (int)e + (int)f + (int)g + (int)h;
}

/* Another non-inline function to force register pressure */
NOINLINE double complex_calculation(double a, double b, double c,
                                    double d, double e, double f) {
    /* Complex expression that can't be easily optimized */
    double t1 = a * b + c / (d + 1.0);
    double t2 = e * f - a / (b + 1.0);
    double t3 = c * d + e / (f + 1.0);
    double t4 = a * f + b / (c + 1.0);
    
    /* Force all temporaries to be used */
    asm volatile("" : : "x"(t1), "x"(t2), "x"(t3), "x"(t4) : "memory");
    
    return t1 + t2 - t3 * t4;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent optimization */
    volatile int array_int[256];
    volatile float array_float[256];
    volatile double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i;
        array_float[i] = i * 0.1f;
        array_double[i] = i * 0.01;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    
    /* Nested loops to create high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 100; i++) {
            /* Create many independent computations with different expressions */
            /* Each computation uses slightly different expressions to avoid CSE */
            
            /* Integer computations */
            int a = array_int[i & 255] + outer;
            int b = array_int[(i + 1) & 255] * outer;
            int c = array_int[(i + 2) & 255] / (outer + 1);
            int d = array_int[(i + 3) & 255] - outer;
            int e = array_int[(i + 4) & 255] % (outer + 2);
            int f = array_int[(i + 5) & 255] ^ outer;
            int g = array_int[(i + 6) & 255] | (outer << 2);
            int h = array_int[(i + 7) & 255] & (outer + 3);
            
            /* Floating-point computations */
            float fa = array_float[i & 255] * 1.1f + outer;
            float fb = array_float[(i + 1) & 255] / 2.3f - outer;
            float fc = array_float[(i + 2) & 255] * 3.5f + outer * 0.1f;
            float fd = array_float[(i + 3) & 255] / 4.7f - outer * 0.2f;
            
            /* Double computations */
            double da = array_double[i & 255] * 1.01 + outer * 0.01;
            double db = array_double[(i + 1) & 255] / 2.02 - outer * 0.02;
            double dc = array_double[(i + 2) & 255] * 3.03 + outer * 0.03;
            double dd = array_double[(i + 3) & 255] / 4.04 - outer * 0.04;
            double de = array_double[(i + 4) & 255] * 5.05 + outer * 0.05;
            double df = array_double[(i + 5) & 255] / 6.06 - outer * 0.06;
            
            /* Vector computations */
            v4si vi1 = {a, b, c, d};
            v4si vi2 = {e, f, g, h};
            v4si vi3 = vi1 + vi2 * 2;
            v4si vi4 = vi1 - vi2 / 3;
            
            v4sf vf1 = {fa, fb, fc, fd};
            v4sf vf2 = {fb, fc, fd, fa};
            v4sf vf3 = vf1 * vf2 + 1.0f;
            v4sf vf4 = vf1 / vf2 - 0.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose and SSE registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15", "memory");
            
            /* Complex expression that creates many temporaries */
            /* This is where rematerialization opportunities arise */
            int temp1 = a * b + c / (d + 1) - e % (f + 2);
            int temp2 = b * c + d / (e + 1) - f % (g + 2);
            int temp3 = c * d + e / (f + 1) - g % (h + 2);
            int temp4 = d * e + f / (g + 1) - h % (a + 2);
            int temp5 = e * f + g / (h + 1) - a % (b + 2);
            int temp6 = f * g + h / (a + 1) - b % (c + 2);
            int temp7 = g * h + a / (b + 1) - c % (d + 2);
            int temp8 = h * a + b / (c + 1) - d % (e + 2);
            
            /* More floating-point temporaries */
            float ftemp1 = fa * 1.234f + fb / 2.345f - fc * 3.456f;
            float ftemp2 = fb * 2.345f + fc / 3.456f - fd * 4.567f;
            float ftemp3 = fc * 3.456f + fd / 4.567f - fa * 5.678f;
            float ftemp4 = fd * 4.567f + fa / 5.678f - fb * 6.789f;
            
            /* Double temporaries with complex expressions */
            double dtemp1 = da * db + dc / (dd + 1.0) - de * df;
            double dtemp2 = db * dc + dd / (de + 1.0) - df * da;
            double dtemp3 = dc * dd + de / (df + 1.0) - da * db;
            double dtemp4 = dd * de + df / (da + 1.0) - db * dc;
            double dtemp5 = de * df + da / (db + 1.0) - dc * dd;
            double dtemp6 = df * da + db / (dc + 1.0) - dd * de;
            
            /* Call non-inline function with many arguments */
            /* This forces values into argument registers */
            int func_result = use_many_values(
                temp1, temp2, temp3, temp4,
                ftemp1, ftemp2, dtemp1, dtemp2,
                vi3, vf3
            );
            
            /* Another function call with different arguments */
            double complex_result = complex_calculation(
                dtemp3, dtemp4, dtemp5, dtemp6,
                da + db, dc + dd
            );
            
            /* Volatile writes to prevent optimization */
            global_sink = func_result;
            float_sink = ftemp3 + ftemp4;
            double_sink = complex_result;
            
            /* Accumulate results */
            total_int += temp1 + temp2 + temp3 + temp4 + 
                        temp5 + temp6 + temp7 + temp8;
            total_float += ftemp1 + ftemp2 + ftemp3 + ftemp4;
            total_double += dtemp1 + dtemp2 + dtemp3 + dtemp4 + 
                           dtemp5 + dtemp6 + complex_result;
            
            /* Another inline assembly to break live ranges */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "memory");
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: int=%d float=%f double=%lf\n", 
           total_int, total_float, total_double);
    
    return 0;
}
