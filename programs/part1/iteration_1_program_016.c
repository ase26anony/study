#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with varying attributes to create diverse call sites */

/* Function that returns a value and uses many registers */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b + c + d + e + f + g + h;
    /* Force register pressure inside the function too */
    int t1 = a * b;
    int t2 = c * d;
    int t3 = e * f;
    int t4 = g * h;
    return result + t1 + t2 + t3 + t4;
}

/* Function with pointer arguments that may alias */
float __attribute__((noinline))
process_floats(float* f1, float* f2, float* f3, float* f4, 
               float* f5, float* f6, float* f7) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5 + *f6 + *f7;
    
    /* Create register pressure with float operations */
    float prod = *f1 * *f2 * *f3;
    float div = *f4 / *f5;
    float sub = *f6 - *f7;
    
    return sum + prod + div + sub;
}

/* Function that uses alloca to affect frame pointer */
void* __attribute__((noinline))
create_buffer(int size) {
    /* alloca forces frame pointer usage */
    void* buf = alloca(size);
    volatile int* p = (int*)buf;
    for (int i = 0; i < size / sizeof(int); i++) {
        p[i] = i * 2;
    }
    return buf;
}

/* Function with mixed types in arguments */
double __attribute__((noinline))
mixed_calculation(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    
    /* Complex computation to use more registers */
    double t1 = c * 3.14159;
    double t2 = (double)a / (b + 1.0f);
    double t3 = (double)(*d) * (*e);
    
    return result + t1 + t2 + t3;
}

/* Static function that might be inlined in some compilation modes */
static int __attribute__((always_inline))
inline_helper(int x, int y) {
    volatile int temp = x ^ y;
    return temp * 2;
}

/* Function with variable arguments to test different calling conventions */
int __attribute__((noinline))
var_args_computation(int count, ...) {
    volatile int sum = 0;
    /* Simulate va_list usage */
    int* args = &count;
    for (int i = 0; i < count && i < 8; i++) {
        sum += args[i];
    }
    return sum;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    
    int result = 0;
    
    /* Control flow to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Multiple function calls with register pressure between them */
            
            /* Computation before call */
            v1 = v2 * v3 + v4;
            f1 = f2 * f3 - f4;
            
            /* Inline assembly to clobber specific registers */
            /* For x86_64 - clobber common call-clobbered registers */
            __asm__ volatile (
                "mov $0x12345678, %%eax\n\t"
                "mov $0x87654321, %%ecx\n\t"
                "mov $0x11111111, %%edx\n\t"
                "mov $0x22222222, %%r10\n\t"
                "mov $0x33333333, %%r11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ecx", "edx", "r10", "r11", "memory"
            );
            
            /* Function call with many arguments - will exceed register passing on most ABIs */
            int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8);
            
            /* More computations keeping values live in registers */
            v2 = v3 ^ v4 | v5;
            f2 = f3 / f4 * f5;
            
            /* Another inline assembly to clobber different registers */
            __asm__ volatile (
                "mov $0xAAAAAAAA, %%r8\n\t"
                "mov $0xBBBBBBBB, %%r9\n\t"
                "mov $0xCCCCCCCC, %%rsi\n\t"
                "mov $0xDDDDDDDD, %%rdi\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "rsi", "rdi", "memory"
            );
            
            /* Call function with pointer arguments */
            float float_result = process_floats(&f1, &f2, &f3, &f4, &f5, &f2, &f3);
            
            /* Use alloca to affect frame pointer decisions */
            void* buffer = create_buffer(64);
            volatile int* buf_int = (int*)buffer;
            v3 = buf_int[0] + buf_int[1];
            
        } else {
            /* Block 2: Different sequence of calls and computations */
            
            /* Complex computation chain */
            v6 = (v7 << 2) | (v8 >> 1);
            f3 = f4 * 2.0f - f5 / 3.0f;
            d1 = d2 * 1.5 + d3;
            
            /* Mixed type function call */
            double mixed_result = mixed_calculation(v6, f3, d1, &v7, &f4);
            
            /* Inline assembly between computations */
            __asm__ volatile (
                "mov $0x44444444, %%rax\n\t"
                "mov $0x55555555, %%rbx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "rax", "rbx", "memory"
            );
            
            /* Use inline helper (might be inlined) */
            v8 = inline_helper(v9, v10);
            
            /* More register pressure */
            v9 = v10 * v1 - v2;
            f4 = f5 + f1 - f2;
            
            /* Call with what looks like variable arguments */
            int var_sum = var_args_computation(5, v1, v2, v3, v4, v5);
            
            /* Final computation in this block */
            v10 = (v1 + v2 + v3 + v4 + v5) & 0xFF;
        }
        
        /* Cross-iteration computations to create longer live ranges */
        result += v1 + v2 + v3 + v4 + v5 + (int)f1 + (int)f2 + (int)d1;
        
        /* Additional inline assembly at loop end */
        __asm__ volatile (
            "mov $0x66666666, %%r12\n\t"
            "mov $0x77777777, %%r13\n\t"
            "mov $0x88888888, %%r14\n\t"
            : /* no outputs */
            : /* no inputs */
            : "r12", "r13", "r14", "memory"
        );
    }
    
    /* Final checksum computation using all variables */
    int final_result = result + v6 + v7 + v8 + v9 + v10 + 
                      (int)(f3 * 100) + (int)(f4 * 100) + (int)(f5 * 100) +
                      (int)(d1 * 100) + (int)(d2 * 100) + (int)(d3 * 100);
    
    printf("Result: %d\n", final_result);
    
    /* Additional test: nested loops with calls */
    {
        volatile int outer = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                /* Function call inside nested loop - creates interesting BB structure */
                int temp = compute_sum(i, j, v1, v2, v3, v4, v5, v6);
                outer += temp;
                
                /* Conditional with call inside */
                if (temp > 10) {
                    float ftemp = process_floats(&f1, &f2, &f3, &f4, &f5, &f1, &f2);
                    outer += (int)ftemp;
                }
            }
        }
        printf("Nested result: %d\n", outer);
    }
    
    return final_result > 1000 ? 0 : 1;
}
