/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h,
                        v4si vi, v4sf vf) {
    /* Force computation to prevent removal */
    int sum = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    for (int i = 0; i < 4; i++) sum += vi[i];
    for (int i = 0; i < 4; i++) sum += (int)vf[i];
    global_sink = sum;
    return sum & 1;
}

/* Another non-inline function with different signature */
NOINLINE double compute_pressure(double base, int iter, 
                                 float f1, float f2,
                                 int i1, int i2, int i3) {
    /* Complex computation to force register usage */
    double t1 = base * iter + f1 * f2;
    double t2 = (i1 * i2) / (double)(i3 + 1);
    double t3 = t1 * t2 - base / (iter + 1.0);
    
    /* Use inline assembly to clobber registers */
    asm volatile("" 
                 : 
                 : "r"(t1), "r"(t2), "r"(t3)
                 : "xmm0", "xmm1", "xmm2", "xmm3", 
                   "rax", "rbx", "rcx", "rdx", "memory");
    
    return t3;
}

/* Main computational kernel */
int main(int argc, char **argv) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int iterations = argc > 2 ? atoi(argv[2]) : 100000;
    
    /* Arrays to create memory pressure */
    volatile int array[256];
    volatile float farray[256];
    for (int i = 0; i < 256; i++) {
        array[i] = (seed + i) % 100;
        farray[i] = (seed + i) * 0.1f;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total = 0;
    
    /* Nested loops with high register pressure */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many temporary variables with different types */
        int t1 = outer * 3;
        int t2 = outer / 2;
        float f1 = outer * 0.5f;
        float f2 = outer * 0.25f;
        double d1 = outer * 0.125;
        double d2 = outer * 0.0625;
        
        /* Vector operations to consume SIMD registers */
        v4si vec_int = {t1, t2, outer, seed};
        v4sf vec_float = {f1, f2, f1 * 2.0f, f2 * 3.0f};
        
        /* Complex arithmetic chain creating many intermediates */
        for (int inner = 0; inner < 16; inner++) {
            /* Each iteration creates new temporaries */
            int a = t1 + inner * 7;
            int b = t2 - inner * 3;
            float c = f1 + inner * 0.7f;
            float d = f2 - inner * 0.3f;
            double e = d1 * (inner + 1);
            double f = d2 / (inner + 1);
            
            /* More intermediate computations */
            int x1 = a * b + inner;
            int x2 = a / (b + 1) - inner;
            float y1 = c * d - inner * 0.1f;
            float y2 = c / (d + 0.1f) + inner * 0.2f;
            double z1 = e + f * 2.0;
            double z2 = e - f / 2.0;
            
            /* Use inline assembly to clobber specific registers */
            /* This forces the compiler to re-materialize values */
            asm volatile("# Force register clobbering\n\t"
                         : 
                         : "r"(x1), "r"(x2), "r"(y1), "r"(y2), 
                           "r"(z1), "r"(z2)
                         : "rax", "rbx", "rcx", "rdx",
                           "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "xmm6", "xmm7",
                           "memory");
            
            /* Volatile memory access to prevent optimization */
            global_sink = array[(a + b) & 255];
            float_sink = farray[(inner + outer) & 255];
            
            /* Call function with many arguments - forces register moves */
            int result = use_values(x1, x2, y1, y2, 
                                   a, b, c, d,
                                   vec_int, vec_float);
            
            /* Another computation using the result */
            double pressure = compute_pressure(z1, inner, y1, y2, 
                                              x1, x2, result);
            
            /* Accumulate with complex expression */
            total += (int)(pressure * 100.0) + result + 
                    array[inner & 255] + (int)farray[inner & 255];
            
            /* Modify vectors to create new values */
            vec_int += (v4si){inner, result, a, b};
            vec_float *= (v4sf){1.01f, 0.99f, 1.02f, 0.98f};
            
            /* More intermediate computations */
            int t3 = x1 * x2 + a * b;
            int t4 = x1 / (x2 + 1) + a / (b + 1);
            float t5 = y1 * y2 * c * d;
            float t6 = y1 / (y2 + 0.1f) + c / (d + 0.1f);
            
            /* Use these temporaries */
            total += t3 + t4 + (int)t5 + (int)t6;
            
            /* Another inline assembly barrier */
            asm volatile("# Another clobber point\n\t"
                         : 
                         : "r"(t3), "r"(t4), "r"(t5), "r"(t6)
                         : "r8", "r9", "r10", "r11",
                           "xmm8", "xmm9", "xmm10", "xmm11",
                           "memory");
        }
        
        /* Additional computation between outer loop iterations */
        if (outer % 100 == 0) {
            /* Force spill/reload around this point */
            asm volatile("# Loop boundary clobber\n\t"
                         : 
                         : 
                         : "rax", "rbx", "rcx", "rdx",
                           "xmm0", "xmm1", "xmm2", "xmm3",
                           "memory");
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
