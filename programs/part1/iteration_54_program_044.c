/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Non-inline function to force register usage for arguments */
__attribute__((noinline, noipa))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               v4si vi, v4sf vf) {
    /* Force computation to prevent elimination */
    int sum = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    for (int i = 0; i < 4; i++) {
        sum += vi[i];
        sum += (int)vf[i];
    }
    global_sink = sum;
    return sum & 0xFF;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
double compute_polynomial(double x, double y, double z,
                          int i, int j, int k,
                          float a, float b, float c) {
    /* Complex polynomial that uses many temporaries */
    double t1 = x * x + y * y;
    double t2 = z * z * z;
    double t3 = t1 * t2;
    double t4 = (double)i * x + (double)j * y + (double)k * z;
    double t5 = (double)a * b * c;
    double t6 = t3 * t4 + t5;
    
    /* Force side effect */
    double_sink = t6;
    return t6;
}

int main(void) {
    /* Initialize arrays with volatile to prevent lifting */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.7;
    }
    
    int total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating short-lived values */
            int idx1 = i + outer;
            int idx2 = i * 2 + outer;
            int idx3 = i * 3 - outer;
            
            /* Force register pressure with mixed computations */
            int a = array1[idx1 & 255] * 3;
            int b = array1[idx2 & 255] / 7;
            int c = a + b * 5;
            int d = c - array1[idx3 & 255];
            int e = d * d + a;
            int f = e / (b + 1) + c;
            
            /* Floating point computations */
            float fa = array2[idx1 & 255] * 1.1f;
            float fb = array2[idx2 & 255] / 3.3f;
            float fc = fa + fb * 2.5f;
            float fd = fc - array2[idx3 & 255];
            float fe = fd * fd + fa;
            float ff = fe / (fb + 1.0f) + fc;
            
            /* Double precision computations */
            double da = array3[idx1 & 255] * 1.7;
            double db = array3[idx2 & 255] / 4.9;
            double dc = da + db * 3.2;
            double dd = dc - array3[idx3 & 255];
            double de = dd * dd + da;
            double df = de / (db + 1.0) + dc;
            
            /* Vector operations */
            v4si vi = {a, b, c, d};
            v4si vj = {e, f, a + b, c + d};
            v4si vk = vi + vj * 2;
            
            v4sf vfa = {fa, fb, fc, fd};
            v4sf vfb = {fe, ff, fa + fb, fc + fd};
            v4sf vfc = vfa + vfb * 1.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile(
                "# Clobber important registers\n\t"
                "mov $0, %%eax\n\t"
                "mov $0, %%ebx\n\t"
                "mov $0, %%ecx\n\t"
                "mov $0, %%edx\n\t"
                "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                :
                :
                : "eax", "ebx", "ecx", "edx", 
                  "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7",
                  "memory"
            );
            
            /* Call function with many arguments - forces register allocation */
            int result1 = use_values(a, b, fa, da,
                                     c, d, fb, db,
                                     vk, vfc);
            
            /* More computations between calls */
            int g = result1 * 11 + i;
            int h = g / 3 + outer;
            float fg = (float)result1 * 0.7f + fa;
            float fh = fg / 2.3f + fb;
            double dg = (double)result1 * 0.9 + da;
            double dh = dg / 1.8 + db;
            
            /* Another assembly clobber */
            asm volatile(
                "# Clobber more registers\n\t"
                "mov $0, %%r8d\n\t"
                "mov $0, %%r9d\n\t"
                "mov $0, %%r10d\n\t"
                "mov $0, %%r11d\n\t"
                "pxor %%xmm8, %%xmm8\n\t"
                "pxor %%xmm9, %%xmm9\n\t"
                :
                :
                : "r8", "r9", "r10", "r11",
                  "xmm8", "xmm9", "xmm10", "xmm11",
                  "memory"
            );
            
            /* Second function call with different arguments */
            double result2 = compute_polynomial(dg, dh, da + db,
                                                g, h, result1,
                                                fg, fh, fc);
            
            /* Volatile writes to prevent elimination */
            global_sink = g + h;
            float_sink = fg + fh;
            double_sink = result2;
            
            /* Accumulate result with complex expression */
            total += (result1 & 0xF) + ((int)result2 & 0xF) + 
                     (g & 3) + ((int)fg & 3) + i + outer;
            
            /* Additional pressure with conditional */
            if ((i & 15) == 0) {
                /* More computations in cold path */
                int extra = a * b - c * d + e * f;
                float fextra = fa * fb - fc * fd + fe * ff;
                double deextra = da * db - dc * dd + de * df;
                
                asm volatile(
                    "# Conditional clobber\n\t"
                    "mov %0, %%eax\n\t"
                    "mov %1, %%xmm0\n\t"
                    :
                    : "r"(extra), "x"(fextra)
                    : "eax", "xmm0", "memory"
                );
                
                double_sink = deextra;
            }
        }
        
        /* Outer loop computation to prevent invariant motion */
        int outer_temp = outer * outer + total;
        float outer_float = (float)outer * 0.123f + (float)total * 0.456f;
        
        asm volatile(
            "# Outer loop clobber\n\t"
            "mov %0, %%esi\n\t"
            "movss %1, %%xmm15\n\t"
            :
            : "r"(outer_temp), "m"(outer_float)
            : "esi", "xmm15", "memory"
        );
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
