/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))
#define NOCLONE __attribute__((noclone))

/* Vector types to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;
volatile float float_sink = 0.0f;
volatile double double_sink = 0.0;

/* Non-inline function with many arguments */
NOINLINE NOCLONE int use_many_values(int a, int b, int c, int d,
                                      float e, float f, double g, double h,
                                      v4si vi, v4sf vf) {
    /* Force computation to prevent optimization */
    int sum = a + b + c + d;
    float fsum = e + f;
    double dsum = g + h;
    
    /* Extract vector elements */
    int vi_sum = vi[0] + vi[1] + vi[2] + vi[3];
    float vf_sum = vf[0] + vf[1] + vf[2] + vf[3];
    
    /* Use volatile to ensure side effects */
    global_sink = sum + vi_sum;
    float_sink = fsum + vf_sum;
    double_sink = dsum;
    
    return sum;
}

/* Another non-inline function with different signature */
NOINLINE NOCLONE double compute_complex(double base, int iterations) {
    double result = base;
    for (int i = 0; i < iterations; i++) {
        /* Complex computation to create register pressure */
        result = result * 1.61803398875 - (i * 0.31415926536);
        result = result / (1.0 + (i % 5));
        result = result + (result * result * 0.01);
    }
    double_sink = result;
    return result;
}

/* Main computational kernel */
int main(void) {
    /* Initialize arrays with volatile to prevent optimization */
    volatile int array1[256];
    volatile float array2[256];
    volatile double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5f;
        array3[i] = i * 2.5;
    }
    
    /* Accumulator to prevent dead code elimination */
    int total_int = 0;
    float total_float = 0.0f;
    double total_double = 0.0;
    
    /* Vector accumulators */
    v4si vec_int_acc = {0, 0, 0, 0};
    v4sf vec_float_acc = {0.0f, 0.0f, 0.0f, 0.0f};
    v2df vec_double_acc = {0.0, 0.0};
    
    /* Nested loops to create high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* MANY independent computations creating temporaries */
            
            /* Integer computations with unique expressions */
            int t1 = outer * 7 + inner * 3;
            int t2 = outer * 11 - inner * 5;
            int t3 = (outer << 3) | (inner & 0xF);
            int t4 = (outer % 17) * (inner % 13);
            int t5 = t1 * t2 - t3 + t4;
            int t6 = (t1 ^ t2) | (t3 & t4);
            int t7 = t5 * 2 - t6 / 3;
            int t8 = (t7 << 2) + (t6 >> 1);
            int t9 = t8 * 3 - t7 * 2 + t6;
            int t10 = (t9 % 31) * (t8 % 29);
            
            /* Floating-point computations */
            float f1 = outer * 1.2345f + inner * 0.9876f;
            float f2 = outer * 0.5432f - inner * 0.1234f;
            float f3 = f1 * 2.5f - f2 / 1.5f;
            float f4 = f3 * f3 - f2 * f2 + f1;
            float f5 = f4 / (1.0f + f3) * 2.0f;
            float f6 = f5 * 3.14159f - f4 * 2.71828f;
            
            /* Double precision computations */
            double d1 = outer * 2.34567 + inner * 1.23456;
            double d2 = outer * 0.98765 - inner * 0.45678;
            double d3 = d1 * 3.14159265358979 - d2 / 2.71828182845904;
            double d4 = d3 * d3 + d2 * d2 - d1;
            double d5 = d4 / (1.0 + d3 * 0.1) * 1.5;
            double d6 = d5 * 0.69314718056 - d4 * 0.4342944819;
            
            /* Vector computations */
            v4si vi1 = {t1, t2, t3, t4};
            v4si vi2 = {t5, t6, t7, t8};
            v4si vi3 = vi1 + vi2 * 2;
            v4si vi4 = vi1 - vi2 / 3;
            v4si vi5 = vi3 * vi4 + vi1;
            
            v4sf vf1 = {f1, f2, f3, f4};
            v4sf vf2 = {f5, f6, f1, f2};
            v4sf vf3 = vf1 * 1.5f - vf2 / 2.0f;
            v4sf vf4 = vf1 + vf2 * vf3;
            
            v2df vd1 = {d1, d2};
            v2df vd2 = {d3, d4};
            v2df vd3 = vd1 * 1.25 - vd2 / 1.75;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber multiple registers */
            asm volatile("" : : : 
                "rax", "rbx", "rcx", "rdx", 
                "rsi", "rdi", 
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "memory");
            
            /* More computations after clobbering */
            int t11 = t10 * 2 + outer;
            float f7 = f6 * 1.1f + inner * 0.1f;
            double d7 = d6 * 1.01 + outer * 0.01;
            
            /* Volatile memory accesses to force register reloading */
            global_sink = array1[inner & 0xFF];
            float_sink = array2[inner & 0xFF];
            double_sink = array3[inner & 0xFF];
            
            /* Call non-inline function with many arguments */
            int func_result = use_many_values(
                t1, t2, t3, t4,
                f1, f2, d1, d2,
                vi5, vf4
            );
            
            /* More computations using function result */
            int t12 = t11 + func_result;
            float f8 = f7 + func_result * 0.01f;
            double d8 = d7 + func_result * 0.001;
            
            /* Another inline assembly to clobber more registers */
            asm volatile("" : : : 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15",
                "memory");
            
            /* Complex function call */
            double complex_result = compute_complex(d8, 3);
            
            /* Accumulate results (prevents dead code elimination) */
            total_int += t12;
            total_float += f8;
            total_double += complex_result;
            
            vec_int_acc += vi5;
            vec_float_acc += vf4;
            vec_double_acc += vd3;
            
            /* Another volatile write */
            array1[inner & 0xFF] = t12;
            array2[inner & 0xFF] = f8;
            array3[inner & 0xFF] = complex_result;
        }
        
        /* Additional computation between outer loop iterations */
        int outer_temp = outer * 137;
        float outer_float = outer * 1.337f;
        double outer_double = outer * 3.14159;
        
        /* Mix in some conditional code */
        if (outer % 7 == 0) {
            total_int += outer_temp;
            total_float += outer_float;
            total_double += outer_double;
        }
    }
    
    /* Final aggregation and output */
    int final_int = total_int + vec_int_acc[0] + vec_int_acc[1] + 
                    vec_int_acc[2] + vec_int_acc[3];
    float final_float = total_float + vec_float_acc[0] + vec_float_acc[1] +
                       vec_float_acc[2] + vec_float_acc[3];
    double final_double = total_double + vec_double_acc[0] + vec_double_acc[1];
    
    printf("Results: int=%d, float=%f, double=%f\n", 
           final_int, final_float, final_double);
    
    return 0;
}
