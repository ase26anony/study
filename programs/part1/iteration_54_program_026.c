/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function to force register usage for arguments */
__attribute__((noinline, noipa))
int use_values(int a, int b, float c, double d, 
               long e, short f, v4si v, v4sf vf) {
    /* Complex computation to prevent inlining */
    int result = a * b + (int)c + (int)d + e + f;
    result += v[0] + v[1] + v[2] + v[3];
    result += (int)vf[0] + (int)vf[1];
    global_sink = result;
    return result & 1;
}

/* Another non-inline function with different signature */
__attribute__((noinline, noipa))
double compute_more(double x, double y, double z,
                    int i, int j, int k,
                    float a, float b, float c) {
    /* Complex floating point operations */
    double t1 = x * y + z;
    double t2 = sin(x) * cos(y);
    double t3 = exp(z / 100.0);
    double t4 = t1 * t2 / (t3 + 1.0);
    
    /* Integer operations mixed in */
    t4 += (i * j - k) * 0.01;
    t4 += (a + b + c) * 0.1;
    
    float_sink = (float)t4;
    return t4;
}

int main(void) {
    /* Initialize arrays with varying values */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = (i * 37) & 0xFF;
        array_float[i] = (i * 0.1f) + 0.5f;
        array_double[i] = (i * 0.01) + 0.25;
    }
    
    /* Volatile pointer to force memory accesses */
    volatile int* volatile_ptr = array_int;
    
    /* Main computational kernel with high register pressure */
    long long total = 0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loops create many live ranges */
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating temporaries */
            int t1 = array_int[i] * 3;
            int t2 = array_int[i + 1] / 2;
            int t3 = t1 + t2 - i;
            int t4 = t3 * t3 - t1;
            int t5 = (t4 << 3) | (t3 & 0xF);
            
            float f1 = array_float[i] * 2.5f;
            float f2 = array_float[i + 1] / 1.5f;
            float f3 = f1 + f2 - i * 0.1f;
            float f4 = f3 * f3 - f1;
            float f5 = f4 * 0.75f + f3 * 0.25f;
            
            double d1 = array_double[i] * 1.7;
            double d2 = array_double[i + 1] / 0.9;
            double d3 = d1 + d2 - i * 0.01;
            double d4 = d3 * sin(d1) + cos(d2);
            double d5 = d4 * 0.5 + d3 * 0.5;
            
            /* More computations with mixed types */
            long l1 = t1 * t2 + i;
            long l2 = l1 * 7 - t3;
            long l3 = l2 / (t4 + 1) + t5;
            
            short s1 = (t1 + t2) & 0x7FFF;
            short s2 = (t3 - t4) & 0x7FFF;
            short s3 = s1 * s2 / 1000;
            
            /* Vector operations to consume SIMD registers */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, i, outer, t1 + t2};
            v4si vec3 = vec1 + vec2 * 2;
            v4si vec4 = vec3 - vec1 / 3;
            
            v4sf vecf1 = {f1, f2, f3, f4};
            v4sf vecf2 = {f5, i * 0.1f, outer * 0.01f, f1 + f2};
            v4sf vecf3 = vecf1 + vecf2 * 1.5f;
            v4sf vecf4 = vecf3 - vecf1 / 2.0f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", "r8", "r9", "r10",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "memory");
            
            /* Force memory access with volatile */
            global_sink = *volatile_ptr;
            volatile_ptr++;
            
            /* Call non-inline function with many arguments */
            int r1 = use_values(t1, t2, f3, d4, 
                               l3, s3, vec4, vecf4);
            
            /* More computations after call */
            int t6 = t5 + r1 * 7;
            float f6 = f5 + r1 * 0.1f;
            double d6 = d5 + r1 * 0.01;
            
            /* Another inline assembly to break live ranges */
            asm volatile("" : : : 
                "r12", "r13", "r14", "r15",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Call another function with different arguments */
            double r2 = compute_more(d1, d2, d3,
                                    t1, t2, t3,
                                    f1, f2, f3);
            
            /* Final computations aggregating results */
            total += t6 + (int)f6 + (int)d6 + (int)r2;
            total += vec3[0] + vec3[1] + vec3[2] + vec3[3];
            
            /* More volatile access */
            float_sink = f6;
            global_sink = t6;
            
            /* Additional arithmetic to create more temporaries */
            int t7 = (t6 * 11) % 97;
            int t8 = (t7 + i) * (outer + 1);
            float f7 = f6 * 1.1f + i * 0.01f;
            double d7 = d6 * 1.01 + outer * 0.001;
            
            total += t7 + t8 + (int)f7 + (int)d7;
            
            /* Another assembly clobber */
            asm volatile("" : : : "cc", "memory");
        }
        
        /* Additional computations between outer loop iterations */
        int outer_temp = outer * 37;
        float outer_float = outer * 0.37f;
        double outer_double = outer * 0.037;
        
        for (int j = 0; j < 4; j++) {
            outer_temp = outer_temp * 3 - j;
            outer_float = outer_float * 1.5f + j;
            outer_double = outer_double * 0.9 - j * 0.01;
        }
        
        total += outer_temp + (int)outer_float + (int)outer_double;
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
