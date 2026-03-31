/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fdump-rtl-all -fdump-rtl-early_remat -c early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>

/* Non-inline function to force register pressure */
__attribute__((noinline, optimize("no-ipa")))
int use_values(int a, int b, float c, double d, 
               int e, int f, float g, double h,
               int i, int j, float k, double l) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h + i + j + (int)k + (int)l;
    return sink;
}

/* Another non-inline function with different signature */
__attribute__((noinline, optimize("no-ipa")))
double compute_more(double x, double y, double z, 
                    float a, float b, float c,
                    int i, int j, int k) {
    volatile double vsink;
    vsink = x * y / z + a * b - c + i * j * k;
    return vsink;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile memory for anti-optimization */
volatile int global_sink;
volatile double global_double_sink;

int main(void) {
    /* Initialize arrays to feed computations */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i * 3;
        array_float[i] = i * 1.5f;
        array_double[i] = i * 2.7;
    }
    
    /* Accumulator to prevent dead code elimination */
    int64_t total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 100; inner++) {
            /* Many independent computations creating short-lived temporaries */
            /* Each computation is slightly unique to avoid CSE */
            
            /* Integer computations */
            int t1 = array_int[outer] * array_int[inner + 1] + outer;
            int t2 = array_int[outer + 1] / (inner + 2) - outer * inner;
            int t3 = t1 * t2 + (outer << 3) - (inner >> 2);
            int t4 = (t1 + t2) * (t3 - outer) / (inner + 1);
            int t5 = t3 ^ t4 | (outer & inner);
            int t6 = (t5 << 2) + (t4 >> 1) - t3 * 7;
            
            /* Floating-point computations */
            float f1 = array_float[outer] * 1.234f + inner * 0.567f;
            float f2 = array_float[inner] / 3.141f - outer * 2.718f;
            float f3 = f1 * f2 + outer * 0.123f - inner * 0.456f;
            float f4 = (f1 + f2) * (f3 - outer) / (inner + 1.0f);
            
            /* Double precision computations */
            double d1 = array_double[outer] * 1.234567 + inner * 0.987654;
            double d2 = array_double[inner] / 2.718281 - outer * 3.141592;
            double d3 = d1 * d2 + outer * 0.333333 - inner * 0.666666;
            double d4 = (d1 + d2) * (d3 - outer) / (inner + 1.0);
            
            /* More mixed computations */
            int t7 = (int)(f1 * 100.0f) + (int)(d1 * 10.0) + outer;
            int t8 = (int)(f2 * 50.0f) - (int)(d2 * 5.0) + inner;
            float f5 = (float)(t7 * t8) / 1000.0f + f3;
            double d5 = (double)(t7 - t8) * 0.01 + d3;
            
            /* Vector operations to consume SIMD registers */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, t7, t8};
            v4si vec3 = vec1 + vec2 * 2;
            
            v4sf vecf1 = {f1, f2, f3, f4};
            v4sf vecf2 = {f5, f1 * 2.0f, f2 * 3.0f, f3 * 4.0f};
            v4sf vecf3 = vecf1 + vecf2 * 1.5f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3",
                                     "xmm4", "xmm5", "xmm6", "xmm7",
                                     "memory");
            
            /* Call non-inline function with many arguments */
            int result1 = use_values(t1, t2, f1, d1,
                                    t3, t4, f2, d2,
                                    t5, t6, f3, d3);
            
            /* More computations after function call */
            int t9 = result1 * outer + inner * 11;
            float f6 = (float)result1 / 100.0f + f4;
            double d6 = (double)result1 / 1000.0 + d4;
            
            /* Another function call */
            double result2 = compute_more(d1, d2, d3,
                                         f1, f2, f3,
                                         t1, t2, t3);
            
            /* Volatile memory writes */
            global_sink = t9;
            global_double_sink = result2;
            
            /* More inline assembly */
            asm volatile("" : : : "r8", "r9", "r10", "r11",
                                     "xmm8", "xmm9", "xmm10", "xmm11",
                                     "memory");
            
            /* Even more computations */
            int t10 = t9 * 3 + (int)(result2 * 100.0);
            float f7 = f6 * 2.0f + (float)t10 * 0.01f;
            double d7 = d6 * 1.5 + (double)t10 * 0.001;
            
            int t11 = (t10 << 1) | (outer & 0xFF);
            float f8 = f7 / (inner + 1.0f) * outer;
            double d8 = d7 / (inner + 1.0) * outer;
            
            int t12 = t11 ^ inner + outer * 17;
            float f9 = f8 + array_float[inner % 256] * 0.5f;
            double d9 = d8 + array_double[outer % 256] * 0.25;
            
            /* Final aggregation */
            total += t10 + t11 + t12 + (int)f9 + (int)d9;
            
            /* Another volatile write */
            volatile int local_sink __attribute__((unused));
            local_sink = t12;
            
            /* More assembly to break live ranges */
            asm volatile("" : : : "r12", "r13", "r14", "r15",
                                     "xmm12", "xmm13", "xmm14", "xmm15",
                                     "memory");
        }
        
        /* Additional computations between inner loops */
        int loop_temp = outer * 7 + 13;
        float loop_float = (float)outer * 1.7f;
        double loop_double = (double)outer * 2.9;
        
        /* Use volatile pointer to force dereferencing */
        volatile int *volatile_ptr = &global_sink;
        *volatile_ptr = loop_temp;
        
        total += loop_temp + (int)loop_float + (int)loop_double;
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
