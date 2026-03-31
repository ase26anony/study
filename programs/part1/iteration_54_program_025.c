/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* No-inline function with many arguments */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
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
    return sum & 1; /* Return something non-trivial */
}

/* Another no-inline function for variety */
__attribute__((noinline))
double compute_polynomial(double x, double y, int n) {
    double result = 0.0;
    double term = 1.0;
    for (int i = 0; i < n; i++) {
        result += term;
        term *= x * y / (i + 1);
    }
    double_sink = result;
    return result;
}

int main(void) {
    /* Initialize arrays with volatile reads to force memory ops */
    volatile int init_array[256];
    volatile float float_array[256];
    for (int i = 0; i < 256; i++) {
        init_array[i] = i * 3 + 7;
        float_array[i] = i * 0.5f + 1.0f;
    }
    
    /* Main computational kernel with high register pressure */
    long long total_sum = 0;
    
    /* Outer loops to create many live ranges */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loops increase pressure */
        for (int i = 0; i < 128; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int a = init_array[i] * 3 + 7;
            int b = init_array[i + 1] / 5 - 2;
            int c = a * b + i;
            int d = b * b - a * a + outer;
            int e = (c << 3) | (d >> 2);
            int f = e ^ (i * 0x5A827999);
            
            /* Floating-point operations use different registers */
            float fa = float_array[i] * 1.5f + 2.3f;
            float fb = float_array[i + 1] / 0.7f - 1.2f;
            float fc = fa * fb + i * 0.1f;
            float fd = fb * fb - fa * fa + outer * 0.01f;
            
            /* Double precision for more register pressure */
            double da = (double)a * 1.23456789;
            double db = (double)b / 9.87654321;
            double dc = da * db + i * 0.0001;
            double dd = db * db - da * da;
            
            /* Vector operations consume multiple registers */
            v4si vi = {a, b, c, d};
            v4si vj = {e, f, i, outer};
            v4si vk = vi + vj * 2 - (vj >> 1);
            
            v4sf vfa = {fa, fb, fc, fd};
            v4sf vfb = vfa * 1.5f + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple registers */
            asm volatile("# Force register pressure\n\t"
                         "mov %%eax, %%ebx\n\t"
                         : : : "eax", "ebx", "ecx", "edx", 
                                "xmm0", "xmm1", "xmm2", "xmm3",
                                "memory");
            
            /* Call function with many arguments - forces argument passing */
            int r1 = use_values(a, b, fa, da, 
                               c, d, fb, db,
                               vk, vfb);
            
            /* More computations using results */
            int g = (r1 * 17 + i) & 0xFF;
            float fg = fc * 0.3f + g * 0.01f;
            double dg = compute_polynomial(dc, dd, g % 8 + 1);
            
            /* Another inline assembly with different clobbers */
            asm volatile("# More clobbering\n\t"
                         : : : "rax", "rbx", "rcx", "rdx",
                                "r8", "r9", "r10", "r11",
                                "xmm4", "xmm5", "xmm6", "xmm7",
                                "xmm8", "xmm9", "xmm10", "xmm11",
                                "memory");
            
            /* Complex expression with many temporaries */
            int h = (a * b + c * d - e * f + g * 11) / (i + 1);
            float fh = (fa * fb - fc * fd + fg * 1.7f) / (i + 2.0f);
            double dh = (da * db - dc * dd + dg * 0.987) / (i + 3.0);
            
            /* Volatile writes to prevent elimination */
            global_sink = h;
            float_sink = fh;
            double_sink = dh;
            
            /* Accumulate results with complex dependency chain */
            total_sum += h + (int)fh + (int)dh + vk[0] + vk[1] + vk[2] + vk[3];
            
            /* More arithmetic to extend live ranges */
            int tmp1 = (h << 1) | (g >> 1);
            int tmp2 = tmp1 ^ (i * 0x9E3779B9);
            float tmp3 = fh * 1.234f + tmp2 * 0.001f;
            double tmp4 = dh * 0.987654321 + tmp3;
            
            /* Final assembly barrier */
            asm volatile("# Final barrier\n\t"
                         : : : "rax", "rbx", "xmm0", "xmm1", "memory");
            
            /* Use all temporaries one more time */
            total_sum += tmp2 + (int)tmp3 + (int)tmp4;
        }
        
        /* Cross-iteration dependency to prevent loop invariant motion */
        init_array[outer % 256] = total_sum & 0xFFFF;
        float_array[outer % 256] = (total_sum & 0xFF) * 0.01f;
    }
    
    printf("Result: %lld\n", total_sum);
    return (total_sum > 0) ? 0 : 1;
}
