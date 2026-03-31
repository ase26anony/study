#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and takes many arguments (exceeding register limits) */
int __attribute__((noinline)) many_args_func(int a, int b, int c, int d, int e,
                                             int f, int g, int h, int i, int j,
                                             float k, float l, float m) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    result += (int)(k + l + m);
    return result;
}

/* Static function that might be inlined or not depending on optimization */
static float __attribute__((noinline)) static_noinline_func(float a, float b, float c) {
    volatile float temp = a * b;
    __asm__ volatile ("" : : : "xmm0", "xmm1"); /* Clobber floating point regs */
    return temp / c;
}

/* Function using frame pointer (by taking address of locals) */
int __attribute__((noinline)) uses_frame_pointer(int x) {
    int local1 = x * 2;
    int local2 = x + 100;
    int local3 = x - 50;
    int *ptr1 = &local1;
    int *ptr2 = &local2;
    
    /* Use alloca to force frame pointer usage */
    void *dynamic = alloca(64);
    (void)dynamic;
    
    __asm__ volatile ("" : : "r"(ptr1), "r"(ptr2) : "memory");
    return local1 + local2 + local3;
}

/* Function with mixed types in arguments */
double __attribute__((noinline)) mixed_type_func(int a, float b, double c, 
                                                 int *d, float *e) {
    volatile double result = (double)a + (double)b + c;
    if (d) result += *d;
    if (e) result += *e;
    
    /* Clobber multiple call-clobbered registers */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "xmm0", "xmm1");
    
    return result;
}

/* Function that creates register pressure */
void __attribute__((noinline)) create_pressure(int *out) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 4.4, d2 = 5.5;
    
    /* Many computations keeping values in registers */
    v1 = v1 * v2 + v3;
    v2 = v2 / v4 - v5;
    f1 = f1 * f2 + f3;
    d1 = d1 / d2 * 2.0;
    
    /* Inline assembly that clobbers specific registers */
    __asm__ volatile (""
        : "=r"(v1), "=r"(v2), "=r"(v3)
        : "0"(v1), "1"(v2), "2"(v3)
        : "r10", "r11", "xmm2", "xmm3");
    
    *out = v1 + v2 + v3 + (int)f1 + (int)d1;
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile float k = 11.1f, l = 12.2f, m = 13.3f, n = 14.4f, o = 15.5f;
    volatile double p = 16.6, q = 17.7, r = 18.8;
    int *ptr1 = &a, *ptr2 = &b;
    float *fptr1 = &k, *fptr2 = &l;
    
    int result = 0;
    int loop_counter;
    
    /* Complex control flow creating basic blocks */
    for (loop_counter = 0; loop_counter < 3; loop_counter++) {
        if (loop_counter % 2 == 0) {
            /* First basic block with function calls */
            int temp1 = many_args_func(a, b, c, d, e, f, g, h, i, j, k, l, m);
            
            /* Computation between calls - keep values in registers */
            a = a + b * c - d;
            b = b / e + f;
            k = k * l + m;
            
            /* Inline assembly clobbering call-clobbered registers */
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1");
            
            float temp2 = static_noinline_func(k, l, m);
            
            /* More computations */
            c = c + d - e;
            l = l / m * n;
            
        } else {
            /* Second basic block with different function calls */
            int temp3 = uses_frame_pointer(a);
            
            /* Force register pressure with many operations */
            d = d * e / f;
            e = e + f - g;
            m = m + n - o;
            
            /* Clobber different registers */
            __asm__ volatile ("" : : : "rdx", "rsi", "rdi", "xmm4", "xmm5");
            
            double temp4 = mixed_type_func(b, k, p, ptr1, fptr1);
            
            /* Additional computations */
            f = f * g + h;
            n = n / o * 2.0f;
        }
        
        /* Third basic block inside loop */
        int pressure_result;
        create_pressure(&pressure_result);
        
        /* Mix in some pointer arithmetic */
        ptr1 = &a + loop_counter;
        fptr1 = &k + loop_counter;
        
        /* Complex condition creating another basic block */
        if (pressure_result > 100) {
            /* Call with many live values */
            int temp5 = many_args_func(a, b, c, d, e, 
                                      pressure_result, g, h, i, j,
                                      n, o, (float)pressure_result);
            result += temp5;
            
            /* Clobber around call */
            __asm__ volatile ("" : : : "r8", "r9", "r10", "xmm6", "xmm7");
        } else {
            float temp6 = static_noinline_func(n, o, (float)pressure_result);
            result += (int)temp6;
        }
        
        /* Loop-carried dependency to prevent optimization */
        a = result % 100;
        b = (result + loop_counter) % 50;
    }
    
    /* Final computation using all variables */
    int final_result = a + b + c + d + e + f + g + h + i + j;
    final_result += (int)(k + l + m + n + o);
    final_result += (int)(p + q + r);
    final_result += result;
    
    /* Use alloca in main to affect frame pointer */
    void *main_stack = alloca(32);
    (void)main_stack;
    
    /* Final function call with many live values */
    int verify = uses_frame_pointer(final_result);
    
    printf("Result: %d (checksum: %d)\n", final_result, verify);
    
    /* Return value based on verification */
    return (verify == final_result) ? 0 : 1;
}
