/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))

/* Vector type to increase register pressure */
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

/* Another non-inline function for more pressure */
NOINLINE double compute_pressure(double base, int iter, 
                                 float f1, float f2, 
                                 long l1, long l2) {
    double result = base;
    for (int i = 0; i < 4; i++) {
        result += sin(base + i) * cos(f1 + f2 * i);
        result += (l1 % (iter + 1)) - (l2 % (iter + 2));
    }
    float_sink = (float)result;
    return result;
}

int main(void) {
    /* Initialize arrays to feed computations */
    int array_int[256];
    float array_float[256];
    double array_double[256];
    
    for (int i = 0; i < 256; i++) {
        array_int[i] = i * 3 + 1;
        array_float[i] = i * 0.5f + 2.0f;
        array_double[i] = i * 0.25 + 1.5;
    }
    
    /* Volatile pointer to force memory accesses */
    volatile int* volatile_ptr = array_int;
    
    /* Main computational kernel with high register pressure */
    long long total = 0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner loop with many temporaries */
        for (int i = 0; i < 100; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int t1 = array_int[i & 255] * 3 + outer;
            int t2 = array_int[(i + 1) & 255] / 7 - i;
            float t3 = array_float[i & 255] * 2.5f + outer * 0.3f;
            double t4 = array_double[i & 255] / 1.7 + i * 0.1;
            long t5 = (long)t1 * t2 + i * 7L;
            short t6 = (short)(t1 + t2) & 0xFF;
            
            /* More temporaries with unique expressions */
            int t7 = t1 * t2 - t1 / (t2 + 1) + i;
            float t8 = t3 * t3 - sinf(t3) + i * 0.01f;
            double t9 = t4 * t4 - cos(t4) + i * 0.001;
            long t10 = t5 * 3 - t5 / 2 + outer * 11L;
            
            /* Vector operations for wide register usage */
            v4si vec1 = {t1, t2, t7, i};
            v4si vec2 = {t2, t7, t1, outer};
            v4si vec3 = vec1 + vec2 * 3 - vec1 / 2;
            
            v4sf vecf1 = {t3, t8, t3 * 0.5f, t8 * 2.0f};
            v4sf vecf2 = {t8, t3, t8 * 1.5f, t3 * 0.75f};
            v4sf vecf3 = vecf1 + vecf2 * vecf1 - vecf2 / 3.0f;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3", "memory");
            
            /* Call function with many arguments */
            int r1 = use_values(t1, t7, t3, t9, t10, t6, vec3, vecf3);
            
            /* More computations after call (forces re-materialization) */
            int t11 = t1 * 2 + r1 * 3;
            float t12 = t3 * 1.5f + r1 * 0.5f;
            double t13 = t4 * 2.0 + r1 * 0.25;
            
            /* Another function call with different arguments */
            double r2 = compute_pressure(t13, i, t12, t8, t10, t5);
            
            /* More temporaries using results */
            int t14 = t11 * 3 - (int)(r2 * 100);
            float t15 = t12 * 2.0f + (float)r2;
            double t16 = t13 * 1.5 - r2;
            
            /* Volatile memory access to prevent optimization */
            global_sink = t14;
            float_sink = t15;
            
            /* Another inline assembly clobber */
            /* For x86-64 SSE/AVX registers */
            asm volatile("" : : : "xmm4", "xmm5", "xmm6", "xmm7",
                         "xmm8", "xmm9", "xmm10", "xmm11", "memory");
            
            /* Complex expression with many operands */
            total += (long long)(t1 * t2 + t7 * t11 - t14 / (t2 + 1)) 
                   + (long long)(t3 * 100 + t8 * 50 + t12 * 25 + t15 * 10)
                   + (long long)(t4 * 1000 + t9 * 500 + t13 * 250 + t16 * 100)
                   + (long long)(t5 + t10 + outer * 1000 + i * 100);
            
            /* Force volatile pointer dereference */
            volatile_ptr[i & 255] = t14;
        }
        
        /* Additional pressure between outer loop iterations */
        for (int j = 0; j < 10; j++) {
            double temp = sin(outer * 0.1 + j * 0.01);
            float ftemp = cosf(outer * 0.05f + j * 0.02f);
            total += (long long)(temp * 1000 + ftemp * 100);
            
            /* Clobber more registers */
            asm volatile("" : : : "rsi", "rdi", "r8", "r9", 
                         "r10", "r11", "xmm12", "xmm13", "memory");
        }
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
