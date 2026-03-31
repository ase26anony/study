/* early-remat-test.c - Test program to trigger early rematerialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Prevent function inlining to force register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
int use_values(int a, int b, float c, double d, int e, int f, long g, short h) {
    volatile int sink;
    sink = a + b + (int)c + (int)d + e + f + g + h;
    return sink & 1;
}

/* Another noinline function with mixed types */
__attribute__((noinline))
double compute_pressure(double x, double y, float z, int i, int j) {
    volatile double result;
    result = (x * y) / (z + 1.0) + (i % 17) - (j & 0xFF);
    return result;
}

/* Vector type to consume SIMD registers */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global volatile to prevent optimizations */
volatile int global_sink;
volatile double global_double_sink;

int main(void) {
    /* Initialize arrays to feed computations */
    double arr_d[256];
    float arr_f[256];
    int arr_i[256];
    
    for (int i = 0; i < 256; i++) {
        arr_d[i] = sin(i * 0.1);
        arr_f[i] = cos(i * 0.05);
        arr_i[i] = i * 3;
    }
    
    /* Volatile pointer to force memory accesses */
    volatile int* volatile_ptr = arr_i;
    
    /* Main computational kernel with high register pressure */
    double total = 0.0;
    
    /* Outer loop to increase pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* Nested loops create many live ranges */
        for (int i = 0; i < 128; i++) {
            /* Many independent computations creating temporaries */
            double t1 = arr_d[i] * 1.2345;
            double t2 = arr_d[i+1] / 0.9876;
            float t3 = arr_f[i] + arr_f[i+1];
            float t4 = arr_f[i] - arr_f[i+1];
            int t5 = arr_i[i] * 3;
            int t6 = arr_i[i+1] / 2;
            long t7 = (long)arr_i[i] << 3;
            short t8 = (short)arr_i[i] & 0xFF;
            
            /* Complex expression with many intermediates */
            double expr1 = t1 * t2 + t3 / t4 - (t5 % 7) + (t6 & 0xF);
            double expr2 = sin(t1) * cos(t2) + exp(t3) - log(fabs(t4) + 1.0);
            float expr3 = t3 * t4 / (t5 + 1) - t6 * 0.5f;
            int expr4 = (t5 * t6 + i) ^ (outer << 2);
            
            /* Inline assembly that clobbers registers */
            /* For x86-64 */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                                       "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory");
            
            /* More computations after clobber */
            double t9 = expr1 * expr2 + expr3;
            int t10 = expr4 ^ (i * outer);
            float t11 = expr3 * 2.0f - expr1;
            double t12 = expr2 / (expr1 + 1.0) * t9;
            
            /* Call function with many arguments - forces register pressure */
            int ret = use_values(t5, t6, t3, t4, t10, expr4, t7, t8);
            
            /* Vector operations to consume SIMD registers */
            v4si vec_a = {t5, t6, t10, expr4};
            v4si vec_b = {i, outer, ret, t5 ^ t6};
            v4si vec_c = vec_a + vec_b * 2;
            
            v4sf vec_f1 = {t3, t4, t11, expr3};
            v4sf vec_f2 = vec_f1 * (v4sf){1.1f, 2.2f, 3.3f, 4.4f};
            
            /* More inline assembly between computations */
            asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9", 
                                       "xmm10", "xmm11", "xmm12", "memory");
            
            /* Additional function call with floating arguments */
            double pressure = compute_pressure(t1, t2, t3, t5, t6);
            
            /* Volatile memory access to prevent optimization */
            global_sink = vec_c[0] + vec_c[1];
            global_double_sink = pressure * t12;
            
            /* Force pointer dereference */
            int mem_val = volatile_ptr[i] + volatile_ptr[i+1];
            
            /* Complex final computation with many operands */
            total += t9 + t12 + pressure + vec_f2[0] + vec_f2[1] + 
                    (mem_val % 100) * 0.01 + sin(outer * 0.01) + 
                    cos(i * 0.02) * 0.5;
            
            /* Another register clobber */
            asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", 
                                       "r13", "r14", "r15", "memory");
        }
        
        /* Mix in some conditional computations */
        for (int j = 0; j < 64; j++) {
            double a = arr_d[j] * outer;
            double b = arr_d[j+64] / (outer + 1);
            float c = arr_f[j] * arr_f[j+64];
            int d = arr_i[j] ^ arr_i[j+64];
            
            /* Chain of dependent computations */
            double chain1 = a * b + c;
            double chain2 = chain1 / (a + 1.0) - b;
            double chain3 = chain2 * chain1 + sin(c) * cos(b);
            double chain4 = chain3 / (chain2 + 1.0) + a * b * c;
            
            total += chain4 + d * 0.001;
            
            /* Function call in inner loop */
            if (j % 8 == 0) {
                use_values(d, j, c, chain1, (int)chain2, (int)chain3, 
                          (long)chain4, (short)total);
            }
        }
    }
    
    printf("Result: %f\n", total);
    return (int)total & 0xFF;
}
