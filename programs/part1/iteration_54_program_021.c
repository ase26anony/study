/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        long e, short f, v4si v, v4sf w) {
    /* Force computation to prevent elimination */
    int result = a + b + (int)c + (int)d + (int)e + f;
    for (int i = 0; i < 4; i++) {
        result += v[i];
        result += (int)w[i];
    }
    global_sink = result;
    return result & 1;
}

/* Another non-inline function for different register class pressure */
NOINLINE double compute_pressure(double x, double y, double z,
                                 int i, int j, int k) {
    /* Complex floating point computation */
    double t1 = x * y + z;
    double t2 = x / (y + 1.0);
    double t3 = t1 * t2 - z;
    double t4 = t3 + (double)i + (double)j + (double)k;
    
    /* Inline assembly to clobber registers */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                 "xmm4", "xmm5", "rax", "rbx", "rcx", "memory");
    
    return t4 * 0.5;
}

/* Main computational kernel */
int main(void) {
    /* Initialize arrays with volatile to prevent hoisting */
    volatile int array[256];
    volatile float farray[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
        farray[i] = i * 1.5f;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Create many short-lived temporaries */
        for (int i = 0; i < 128; i++) {
            /* Integer computations with unique expressions */
            int t1 = array[i] * 3 + outer;
            int t2 = array[i+1] / 2 - outer;
            int t3 = t1 * t2 + i;
            int t4 = (t1 + t2) * (t3 - i);
            int t5 = t4 ^ (t1 | t2) & t3;
            int t6 = t5 * 7 - t4 / 3;
            
            /* Floating point computations */
            float f1 = farray[i] * 2.3f + outer;
            float f2 = farray[i+1] / 1.7f - outer;
            float f3 = f1 * f2 + i * 0.1f;
            float f4 = (f1 + f2) * (f3 - i * 0.5f);
            float f5 = f4 * 3.14f / (f3 + 1.0f);
            
            /* Double precision computations */
            double d1 = (double)f1 * 1.23456789;
            double d2 = (double)f2 * 9.87654321;
            double d3 = d1 * d2 + (double)i * 0.01;
            double d4 = d3 / (d1 + d2) - (double)outer * 0.001;
            
            /* Vector operations */
            v4si vec1 = {t1, t2, t3, t4};
            v4si vec2 = {t5, t6, i, outer};
            v4si vec3 = vec1 + vec2 * 2;
            v4si vec4 = vec1 - vec2 / 3;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f1*2.0f, f2*3.0f, f3*4.0f};
            v4sf fvec3 = fvec1 + fvec2 * 1.5f;
            
            /* Inline assembly to clobber specific registers */
            /* Clobber integer registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "memory");
            
            /* Clobber floating point/vector registers */
            asm volatile("" : : : 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Call function with many arguments - forces register moves */
            int func_result = use_values(t1, t2, f3, d4, 
                                        (long)t5 * i, (short)t6,
                                        vec3, fvec3);
            
            /* More computations after function call */
            int t7 = t6 * 11 + func_result;
            float f6 = f5 * 2.71828f + (float)func_result;
            double d5 = compute_pressure(d1, d2, d3, t7, i, outer);
            
            /* Volatile memory access to prevent optimization */
            global_sink = t7;
            float_sink = f6;
            
            /* Complex expression with many temporaries */
            total += (int)(t1 * 0.3 + t2 * 0.7 + 
                          t3 * 1.2 - t4 * 0.8 +
                          t5 * 2.1 + t6 * 0.4 +
                          (int)f1 + (int)f2 * 2 +
                          (int)(d5 * 100.0) +
                          vec3[0] + vec3[1] + vec3[2] + vec3[3] +
                          (int)fvec3[0] + (int)fvec3[1]);
            
            /* Additional pressure with bit operations */
            uint64_t u1 = (uint64_t)t1 * t2;
            uint64_t u2 = u1 ^ ((uint64_t)t3 << 16);
            uint64_t u3 = u2 + ((uint64_t)t4 * t5);
            uint64_t u4 = u3 * 6364136223846793005ULL;
            
            total ^= (int)(u4 & 0xFFFFFFFF);
            
            /* Another assembly barrier */
            asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
        }
        
        /* Outer loop computations */
        int outer_tmp = outer * 37;
        float outer_float = (float)outer * 1.6180339887f;
        double outer_double = (double)outer * 3.141592653589793;
        
        total += outer_tmp + (int)outer_float + (int)outer_double;
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
