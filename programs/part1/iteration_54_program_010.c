/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent inlining to force register pressure at call sites */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               int i, int j, float k, double l) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h + i + j + (int)k + (int)l;
    return sink;
}

/* Another noinline function with different signature */
__attribute__((noinline))
double compute_heavy(double x, double y, double z, 
                     float a, float b, float c,
                     int i, int j, int k) {
    volatile double result;
    result = (x * y + z) / (a - b * c) + (i % 7) - (j << 2) + (k & 0xFF);
    return result;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_sink;
volatile double global_double_sink;

int main(void) {
    /* Initialize arrays with varying values */
    double array_d[256];
    float array_f[256];
    int array_i[256];
    
    for (int idx = 0; idx < 256; idx++) {
        array_d[idx] = (idx * 1.2345) / (idx + 1.0);
        array_f[idx] = (idx * 0.9876f) / (idx + 2.0f);
        array_i[idx] = idx * 3 + 7;
    }
    
    /* Volatile variables to prevent optimization */
    volatile int v1 = 42;
    volatile float v2 = 3.14159f;
    volatile double v3 = 2.71828;
    
    /* Main computational kernel with high register pressure */
    double total = 0.0;
    
    /* Outer loop to increase iteration count */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with complex computations */
        for (int i = 0; i < 256; i++) {
            /* Create many independent temporary values */
            double t1 = array_d[i] * v3 + (i * 0.5);
            double t2 = array_d[(i + 1) % 256] / (v3 + 1.0) - (i * 0.25);
            float t3 = array_f[i] * v2 - (i * 0.125f);
            float t4 = array_f[(i + 2) % 256] + v2 / (i + 3.0f);
            int t5 = array_i[i] * v1 + (i << 3);
            int t6 = array_i[(i + 3) % 256] - v1 * (i & 0xF);
            
            /* More temporaries with mixed operations */
            double t7 = t1 * t2 - t3 + t4;
            float t8 = t3 / t4 + (float)t1 - (float)t2;
            int t9 = t5 ^ t6 + (int)t1 * (int)t2;
            
            /* Vector operations to consume SIMD registers */
            v4si vec_int = {t5, t6, t9, i};
            v4si vec_int2 = {i, i*2, i*3, i*4};
            v4si vec_result = vec_int + vec_int2 * 7;
            
            v4sf vec_float = {t3, t4, t8, (float)i};
            v4sf vec_float2 = {(float)i, (float)(i+1), (float)(i+2), (float)(i+3)};
            v4sf vec_fresult = vec_float * vec_float2 - 2.0f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber commonly used registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* More computations after clobber */
            double t10 = t7 * 2.0 + (double)vec_result[0] / 100.0;
            float t11 = t8 * 3.0f + (float)vec_fresult[1];
            int t12 = t9 + vec_result[2] - (int)t10;
            
            /* Call function with many arguments - forces register pressure */
            int func_result = use_values(
                t5, t6, t3, t1,          /* First 4 args */
                t12, i, t4, t2,          /* Next 4 args */
                vec_result[0], vec_result[1], t11, t10  /* Last 4 args */
            );
            
            /* Another function call with different arguments */
            double heavy_result = compute_heavy(
                t1, t2, t7,
                t3, t4, t8,
                t5, t6, t9
            );
            
            /* Volatile memory access to prevent optimization */
            global_sink = func_result;
            global_double_sink = heavy_result;
            
            /* Complex expression that uses all temporaries */
            total += (t1 + t2 + t7 + t10) * 0.5 
                   - (t3 + t4 + t8 + t11) * 0.3 
                   + (t5 + t6 + t9 + t12) * 0.2
                   + heavy_result * 0.1
                   + func_result * 0.05;
            
            /* Another inline assembly barrier */
            asm volatile("" : : : 
                "r8", "r9", "r10", "r11", 
                "r12", "r13", "r14", "r15",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Additional computations to increase pressure */
            for (int j = 0; j < 4; j++) {
                /* Small inner loop creates more short-lived values */
                double inner_tmp = array_d[(i + j) % 256] * j;
                float inner_ftmp = array_f[(i + j) % 256] + j;
                int inner_itmp = array_i[(i + j) % 256] ^ j;
                
                total += inner_tmp * inner_ftmp - inner_itmp;
                
                /* Another assembly clobber */
                if (j % 2 == 0) {
                    asm volatile("" : : : "rax", "rdx", "xmm0", "xmm1", "memory");
                }
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Additional test with switch statement to create complex control flow */
    {
        double switch_total = 0.0;
        for (int i = 0; i < 1000; i++) {
            switch (i % 7) {
                case 0:
                    switch_total += i * 1.1 + array_d[i % 256];
                    break;
                case 1:
                    switch_total += i * 2.2 - array_f[i % 256];
                    break;
                case 2:
                    switch_total += i * 3.3 * (i & 0xF);
                    break;
                case 3:
                    switch_total += compute_heavy(i, i*2, i*3, i*4, i*5, i*6, i, i+1, i+2);
                    break;
                case 4:
                    /* More register pressure in this case */
                    {
                        double a = i * 1.234;
                        double b = i * 5.678;
                        double c = a * b - i;
                        double d = b / a + i;
                        float e = (float)a + (float)b;
                        float f = (float)c * (float)d;
                        switch_total += use_values(i, i+1, e, a, i+2, i+3, f, b, (int)c, (int)d, e*2, c+d);
                    }
                    break;
                default:
                    switch_total += i;
                    break;
            }
        }
        printf("Switch result: %f\n", switch_total);
    }
    
    return (int)(total + global_sink + global_double_sink) & 0xFF;
}
