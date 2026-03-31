/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Prevent optimization of helper functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile sink to prevent optimizations */
volatile int global_sink;
volatile float float_sink;
volatile double double_sink;

/* Vector types for increased register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Non-inline function with many arguments */
NOINLINE int use_values(int a, int b, float c, double d, 
                        int e, int f, float g, double h) {
    return (a * b) + (int)c + (int)d + e - f + (int)g + (int)h;
}

/* Another non-inline function for mixing things up */
NOINLINE float compute_more(float x, float y, double z, 
                           int i, int j, v4sf vec) {
    float_sink = x + y + (float)z;
    return vec[0] + vec[1] + i - j;
}

/* Complex computation that creates many temporaries */
NOINLINE int complex_calc(int base, int iter, float fval, double dval) {
    /* Many independent computations to create register pressure */
    int t1 = base * iter + 12345;
    int t2 = iter / (base + 1) * 6789;
    float t3 = fval * 2.71828f + (float)iter;
    double t4 = dval / 3.14159 + (double)base;
    int t5 = t1 ^ t2;
    float t6 = t3 * t3 - fval;
    double t7 = t4 * t4 + dval;
    int t8 = (int)t3 + (int)t4 + t5;
    
    /* Mix everything together */
    return t8 + (int)t6 + (int)t7 + base * iter;
}

int main(void) {
    const int N = 1000;
    int result = 0;
    
    /* Initialize some arrays to work with */
    int* int_array = (int*)malloc(N * sizeof(int));
    float* float_array = (float*)malloc(N * sizeof(float));
    double* double_array = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 3;
        float_array[i] = i * 1.5f;
        double_array[i] = i * 2.71828;
    }
    
    /* Main computational kernel - designed for high register pressure */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < N; inner++) {
            /* Volatile memory access to prevent optimization */
            volatile int* volatile_ptr = &int_array[inner];
            int load1 = *volatile_ptr;
            
            /* Many independent arithmetic operations */
            int a = load1 * 3 + outer;
            int b = inner / (a + 1) * 7;
            float c = float_array[inner] * 2.0f + (float)outer;
            double d = double_array[inner] / 1.414 + (double)inner;
            
            /* More computations creating short-lived temporaries */
            int e = (a * b) + (inner << 2);
            int f = (b * outer) - (inner >> 1);
            float g = c * 3.14159f + float_array[outer % N];
            double h = d * 2.71828 - double_array[inner % N];
            
            /* Even more temporaries with mixed operations */
            int i = e ^ f;
            int j = (e & f) | (inner * 255);
            float k = g + g - c;
            double l = h * h + d;
            
            /* Inline assembly that clobbers registers */
            /* For x86-64, clobber multiple registers */
            asm volatile("" 
                         : 
                         : 
                         : "rax", "rbx", "rcx", "rdx", 
                           "xmm0", "xmm1", "xmm2", "xmm3",
                           "xmm4", "xmm5", "memory");
            
            /* Vector operations to consume SIMD registers */
            v4si vec_int = {a, b, i, j};
            v4sf vec_float = {c, g, k, float_array[(inner + 1) % N]};
            
            vec_int = vec_int + (v4si){1, 2, 3, 4};
            vec_float = vec_float * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
            
            /* Call non-inline function with many arguments */
            int func_result = use_values(a, b, c, d, e, f, g, h);
            
            /* More computations after function call */
            int m = func_result * 2 + inner;
            float n = (float)func_result / 3.0f + k;
            double o = (double)func_result * 1.5 + l;
            
            /* Another inline assembly to break up live ranges */
            asm volatile("" : : : "rsi", "rdi", "r8", "r9",
                           "xmm6", "xmm7", "xmm8", "xmm9", "memory");
            
            /* Complex calculation creating more temporaries */
            int p = complex_calc(m, inner, n, o);
            
            /* Use vector results */
            int vec_sum = vec_int[0] + vec_int[1] + vec_int[2] + vec_int[3];
            float vec_float_sum = vec_float[0] + vec_float[1] + 
                                 vec_float[2] + vec_float[3];
            
            /* Final mixing */
            int final_val = p + vec_sum + (int)vec_float_sum;
            
            /* Volatile write to prevent elimination */
            global_sink = final_val;
            
            /* Accumulate result to prevent dead code elimination */
            result += final_val & 0xFF;
            
            /* More register pressure with conditional */
            if (inner % 16 == 0) {
                /* Additional computations in taken branch */
                double extra = (double)a * b * c * d;
                float extra_f = (float)(extra / (inner + 1));
                result += (int)extra_f;
                
                /* Another function call */
                float more = compute_more(c, g, h, i, j, vec_float);
                result += (int)more;
            }
            
            /* Periodic inline assembly */
            if (inner % 8 == 0) {
                asm volatile("" : : : "r10", "r11", "r12", "r13",
                               "xmm10", "xmm11", "xmm12", "xmm13", "memory");
            }
        }
    }
    
    /* Clean up and print result */
    free(int_array);
    free(float_array);
    free(double_array);
    
    printf("Result: %d\n", result);
    return result != 0;
}
