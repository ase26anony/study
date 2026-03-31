/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline))

/* Volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Vector type to increase register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h,
                        v4si *vec_int, v4sf *vec_float) {
    /* Force side effects */
    global_sink = a + b + (int)c + (int)d + e + f + (int)g + (int)h;
    if (vec_int) global_sink += (*vec_int)[0];
    if (vec_float) global_sink += (int)(*vec_float)[0];
    return global_sink;
}

/* Another non-inline function with different signature */
NOINLINE double compute_polynomial(double x, double y, double z,
                                   int i, int j, int k) {
    /* Complex polynomial that can't be easily optimized */
    double result = x * x * x + 2.7 * y * y + 3.14159 * z;
    result += (i % 17) * 0.01 + (j % 23) * 0.001 + (k % 29) * 0.0001;
    
    /* Volatile memory access */
    volatile double* mem = (volatile double*)&global_sink;
    *mem = result;
    
    return result;
}

/* Main computational kernel */
int main(void) {
    /* Initialize arrays with volatile to prevent optimization */
    volatile int array_a[256];
    volatile float array_b[256];
    volatile double array_c[256];
    
    for (int i = 0; i < 256; i++) {
        array_a[i] = (i * 37) % 101;
        array_b[i] = (i * 1.618f) - 3.14159f;
        array_c[i] = (i * 2.71828) / 1.41421;
    }
    
    /* Accumulator to prevent dead code elimination */
    double total = 0.0;
    
    /* Nested loops to create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int i = 0; i < 256; i++) {
            /* Many independent arithmetic operations creating temporaries */
            int temp1 = array_a[i] * 3;
            int temp2 = array_a[(i + 1) % 256] / 7;
            float temp3 = array_b[i] * 2.5f;
            float temp4 = array_b[(i + 2) % 256] + 1.234f;
            double temp5 = array_c[i] - 0.98765;
            double temp6 = array_c[(i + 3) % 256] * 1.41421;
            
            /* More temporaries with complex expressions */
            int temp7 = temp1 + temp2 * (i % 13);
            int temp8 = temp7 - (outer % 11) * 5;
            float temp9 = temp3 / (temp4 + 0.001f);
            float temp10 = temp9 * (i * 0.01f + outer * 0.001f);
            double temp11 = temp5 * temp6 / (i + 1);
            double temp12 = temp11 + (temp5 - temp6) * 0.5;
            
            /* Even more temporaries - each slightly different */
            int temp13 = temp8 * temp7 + (i << 2);
            int temp14 = temp13 ^ (temp8 >> 1);
            float temp15 = temp10 - temp9 * (outer % 7);
            float temp16 = temp15 + (temp10 / (abs(i - 128) + 1));
            double temp17 = temp12 * 3.14159;
            double temp18 = temp17 / (temp12 + 0.00001);
            
            /* Vector operations to consume SIMD registers */
            v4si vec_int = {temp13, temp14, temp7, temp8};
            v4sf vec_float = {temp15, temp16, temp9, temp10};
            
            /* Inline assembly that clobbers registers */
            /* For x86-64: clobber general purpose and xmm registers */
            asm volatile(
                "# Force register pressure\n"
                "movq %%rax, %%rbx\n"
                "movq %%rcx, %%rdx\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "memory"
            );
            
            /* Another assembly block clobbering XMM registers */
            asm volatile(
                "# Clobber floating point registers\n"
                "xorps %%xmm0, %%xmm0\n"
                "xorps %%xmm1, %%xmm1\n"
                :
                :
                : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
            );
            
            /* Call non-inline function with many arguments */
            /* This forces values into argument registers */
            int result1 = use_values(
                temp13, temp14, temp15, temp16,
                temp7, temp8, temp9, temp10,
                &vec_int, &vec_float
            );
            
            /* More computations between function calls */
            double temp19 = temp17 * 2.0 + temp18;
            double temp20 = temp19 / (outer + 1.0);
            
            /* Another function call with different arguments */
            double result2 = compute_polynomial(
                temp17, temp18, temp19,
                temp13, temp14, i
            );
            
            /* Volatile memory access to prevent optimization */
            volatile double* sink_ptr = (volatile double*)&global_sink;
            *sink_ptr = result2;
            
            /* Use all temporaries in final computation */
            total += temp1 * 0.001 + temp2 * 0.0001 +
                    temp3 * 0.01 + temp4 * 0.001 +
                    temp5 * 1.0 + temp6 * 0.1 +
                    temp7 * 0.01 + temp8 * 0.001 +
                    temp9 * 0.1 + temp10 * 0.01 +
                    temp11 * 1.5 + temp12 * 0.5 +
                    temp13 * 0.001 + temp14 * 0.0001 +
                    temp15 * 0.01 + temp16 * 0.001 +
                    temp17 * 2.0 + temp18 * 0.2 +
                    temp19 * 0.3 + temp20 * 0.03 +
                    result1 * 0.00001 + result2 * 0.001;
            
            /* More assembly to break up live ranges */
            asm volatile(
                "# Break live ranges\n"
                "movl $0, %%eax\n"
                :
                :
                : "eax", "memory"
            );
        }
        
        /* Additional outer loop computations */
        double outer_temp = total * (outer % 19) * 0.01;
        volatile double* outer_sink = (volatile double*)&global_sink;
        *outer_sink = outer_temp;
        
        /* More register pressure in outer loop */
        for (int j = 0; j < 10; j++) {
            int j_temp1 = outer * j * 7;
            int j_temp2 = j_temp1 + (j << 3);
            float j_temp3 = j_temp1 * 0.123f;
            float j_temp4 = j_temp2 * 0.456f;
            
            asm volatile(
                "# Outer loop clobber\n"
                :
                :
                : "r8", "r9", "r10", "r11", "memory"
            );
            
            total += j_temp1 + j_temp2 + j_temp3 + j_temp4;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %f\n", total);
    
    return (int)total % 256;
}
