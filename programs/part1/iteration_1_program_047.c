#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function with many arguments to exceed register passing limits */
__attribute__((noinline))
static int many_args_func(int a, int b, int c, int d, int e, 
                         int f, int g, int h, int i, int j,
                         float k, float l, double m) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    result += (int)(k + l + m);
    return result;
}

/* Function returning a value, potentially using different registers */
__attribute__((noinline))
static float float_ops_func(float a, float b, float c, float d) {
    volatile float res = a * b + c / d;
    /* Force register pressure with local computations */
    float t1 = res * 2.0f;
    float t2 = res / 3.0f;
    float t3 = t1 + t2;
    float t4 = t3 - res;
    return t4;
}

/* Function using pointer arguments */
__attribute__((noinline))
static void pointer_func(int *p1, int *p2, float *p3, double *p4) {
    volatile int temp = *p1 + *p2;
    *p3 = (float)temp * 0.5f;
    *p4 = (double)temp * 0.25;
}

/* Function that might be inlined (no static, no noinline) */
int maybe_inlined(int x, int y) {
    return x * y + (x >> 3) - (y << 2);
}

/* Function using alloca to affect frame pointer */
__attribute__((noinline))
static void use_alloca_func(int size) {
    volatile char *buf = alloca(size);
    for (int i = 0; i < size && i < 16; i++) {
        buf[i] = (char)(i * 3);
    }
}

/* Function with mixed types */
__attribute__((noinline))
static double mixed_types_func(int a, float b, double c, int *d) {
    volatile double result = (double)a + (double)b + c;
    *d = (int)result;
    return result * 2.0;
}

/* Main function creating maximum register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 5.5, d2 = 6.6;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile double *dp1 = &d1;
    
    int result = 0;
    
    /* Control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Sequence of operations and calls */
            
            /* Computation keeping values in registers */
            v1 = v2 + v3;
            v4 = v5 * v6;
            f1 = f2 * f3;
            d1 = d2 + 1.0;
            
            /* Inline assembly to clobber specific registers */
            /* Force eax/rax clobber on x86 */
            __asm__ volatile (
                "movl $0x12345678, %%eax\n\t"
                "addl $0x11111111, %%eax\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%eax", "memory"
            );
            
            /* Function call with many arguments */
            int r1 = many_args_func(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
                                   f1, f2, d1);
            
            /* More computations between calls */
            v2 = v1 ^ v3;
            v5 = v4 | v6;
            f2 = f1 + f3;
            
            /* Inline assembly clobbering another register */
            /* Force r10 clobber on x86-64 */
            __asm__ volatile (
                "movq $0xAAAAAAAA, %%r10\n\t"
                "xorq %%r10, %%r10\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%r10", "memory"
            );
            
            /* Call function returning float */
            float r2 = float_ops_func(f1, f2, f3, f4);
            
            /* Use result in computation */
            v7 = (int)r2 + v1;
            
        } else {
            /* Block 2: Different sequence */
            
            /* Take addresses to affect frame pointer */
            int local_array[8];
            for (int i = 0; i < 8; i++) {
                local_array[i] = i * iteration;
            }
            
            /* Pointer manipulation */
            p1 = &local_array[0];
            p2 = &local_array[4];
            
            /* Function with pointer arguments */
            pointer_func((int *)p1, (int *)p2, (float *)fp1, (double *)dp1);
            
            /* Computation using results */
            v8 = *p1 + *p2;
            v9 = (int)(*fp1 * 10.0f);
            
            /* Inline assembly with multiple clobbers */
            __asm__ volatile (
                "movl $0x33333333, %%ebx\n\t"
                "movl $0x44444444, %%ecx\n\t"
                "addl %%ebx, %%ecx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%ebx", "%ecx", "memory"
            );
            
            /* Call function using alloca */
            use_alloca_func(32 + iteration * 8);
            
            /* Mixed type function call */
            int out_val;
            double r3 = mixed_types_func(v8, f3, d2, &out_val);
            v10 = out_val;
            
            /* Potentially inlined function */
            v3 = maybe_inlined(v9, v10);
        }
        
        /* Loop body computation - creates another basic block */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)(f1 + f2 + f3 + f4);
        result += (int)(d1 + d2);
        
        /* Conditional call inside loop */
        if (result % 7 == 0) {
            /* Another call with register pressure */
            int r4 = many_args_func(result, v2, v3, v4, v5, 
                                   iteration, iteration*2, iteration*3,
                                   iteration*4, iteration*5,
                                   1.5f, 2.5f, 3.5);
            result ^= r4;
        }
    }
    
    /* Final computation and output */
    printf("Result checksum: %d\n", result);
    
    /* Verify execution */
    if (result != 0) {
        printf("Program executed successfully with non-zero result\n");
    }
    
    return 0;
}
