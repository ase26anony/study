/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent inlining to force register pressure at call sites */
__attribute__((noinline, noclone))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               int i, int j, float k, double l) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h + i + j + (int)k + (int)l;
    return sink;
}

/* Another noinline function with different signature */
__attribute__((noinline, noclone))
double compute_more(double x, double y, double z, 
                    float a, float b, float c,
                    int i, int j, int k) {
    volatile double vsink;
    vsink = x * y + z / a - b * c + i - j + k;
    return vsink;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to force memory operations */
volatile int global_counter = 0;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

int main(void) {
    /* Declare many variables to increase register pressure */
    int i, j, k, m, n, p, q, r, s, t;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int result = 0;
    
    /* Initialize with different values to prevent constant propagation */
    volatile int seed = 12345;
    int base = seed;
    
    /* Nested loops to create complex control flow */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 25; k++) {
                /* Create many independent computations with unique expressions */
                /* Each computation uses loop indices differently to avoid CSE */
                
                /* Integer computations */
                int t1 = (i * j + k) * (base + 1);
                int t2 = (j * k - i) * (base + 2);
                int t3 = (k * i + j) * (base + 3);
                int t4 = (i + j * k) * (base + 4);
                int t5 = (j + k * i) * (base + 5);
                int t6 = (k + i * j) * (base + 6);
                int t7 = (i - j + k) * (base + 7);
                int t8 = (j - k + i) * (base + 8);
                int t9 = (k - i + j) * (base + 9);
                int t10 = (i * 2 + j * 3 - k) * (base + 10);
                
                /* Floating-point computations */
                f1 = (i * 1.1f) + (j * 2.2f) - (k * 3.3f);
                f2 = (j * 4.4f) / (i + 1.0f) + (k * 5.5f);
                f3 = (k * 6.6f) * (i * 0.5f) - (j * 7.7f);
                f4 = (i + j) * 8.8f / (k + 1.0f);
                f5 = (j + k) * 9.9f * (i * 0.25f);
                f6 = (k + i) * 10.10f - (j * 0.75f);
                f7 = f1 * f2 - f3 + f4;
                f8 = f5 / f6 + f2 * f3;
                f9 = f4 * f5 - f6 / f1;
                f10 = f7 + f8 - f9 * 2.0f;
                
                /* Double precision computations */
                d1 = (double)i * 1.111 + (double)j * 2.222;
                d2 = (double)j * 3.333 / ((double)i + 1.111);
                d3 = (double)k * 4.444 - (double)i * 5.555;
                d4 = d1 * d2 + d3 / 6.666;
                d5 = d2 - d3 * d1 + 7.777;
                d6 = d3 + d1 / d2 - 8.888;
                d7 = d4 * d5 - d6;
                d8 = d5 / d6 + d4;
                d9 = d6 * d7 - d8 / d5;
                d10 = d7 + d8 - d9 * 1.5;
                
                /* Vector operations to consume SIMD registers */
                v4si v1 = {i, j, k, base};
                v4si v2 = {j, k, i, base + 1};
                v4si v3 = {k, i, j, base + 2};
                v4si v4 = v1 + v2 * v3 - v1;
                v4si v5 = v2 - v3 + v4 * v1;
                
                v4sf vf1 = {f1, f2, f3, f4};
                v4sf vf2 = {f5, f6, f7, f8};
                v4sf vf3 = vf1 * vf2 + vf1 / 2.0f;
                
                /* Inline assembly that clobbers registers */
                /* For x86-64, clobber commonly used registers */
                asm volatile(
                    "# Force register pressure\n"
                    : 
                    : 
                    : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                      "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                      "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                      "xmm12", "xmm13", "xmm14", "xmm15", "memory"
                );
                
                /* Call function with many arguments - forces register allocation */
                int ret1 = use_values(
                    t1, t2, f1, d1,
                    t3, t4, f2, d2,
                    t5, t6, f3, d3
                );
                
                /* More computations between calls */
                int t11 = t1 * t2 - t3 + t4;
                int t12 = t5 / (t6 + 1) * t7 - t8;
                float f11 = f4 * f5 - f6 / f7;
                double d11 = d4 + d5 * d6 - d7 / d8;
                
                /* Another assembly clobber */
                asm volatile(
                    "# Another clobber\n"
                    : 
                    : 
                    : "rax", "rbx", "rcx", "rdx", 
                      "xmm0", "xmm1", "xmm2", "xmm3", "memory"
                );
                
                /* Second function call with different arguments */
                double ret2 = compute_more(
                    d1, d2, d3,
                    f1, f2, f3,
                    t1, t2, t3
                );
                
                /* Volatile memory operations to prevent optimization */
                volatile int* volatile_ptr = &global_counter;
                *volatile_ptr += i + j + k;
                
                volatile float vf = global_float;
                f1 += vf;
                
                volatile double vd = global_double;
                d1 -= vd;
                
                /* Complex expression that uses many temporaries */
                result += (t1 * t2) / (t3 + 1) 
                        + (int)(f1 * 100.0f) 
                        + (int)(d1 * 10.0) 
                        + ret1 
                        + (int)(ret2 * 5.0)
                        + v4[0] + v5[1]
                        + (int)(vf3[0] * 10.0f);
                
                /* Another assembly to break live ranges */
                asm volatile("# Break live ranges" ::: "memory");
            }
        }
        
        /* Outer loop computation to vary patterns */
        base += i;
        global_counter++;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
