#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float f1, float f2, float f3, float f4, 
               float f5, float f6, float f7, float f8) {
    volatile float sum = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
    /* Force register clobbering */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return sum * 0.5f;
}

/* Function that takes mixed types */
static double __attribute__((noinline))
mixed_computation(int i1, int i2, float f1, float f2, 
                  double d1, double d2, int* ptr) {
    volatile double result = (double)i1 * i2 + (double)f1 * f2 + d1 * d2;
    *ptr += (int)result;
    __asm__ volatile ("" : : : "rax", "r10", "xmm4", "xmm5");
    return result;
}

/* Function using alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    void* buffer = alloca(size + 16);
    /* Access buffer to ensure it's used */
    volatile char* p = (char*)buffer;
    p[0] = 1;
    __asm__ volatile ("" : : : "r12", "r13", "r14", "r15");
    return buffer;
}

/* Function with variable arguments (simulated) */
static int __attribute__((noinline))
varargs_like(int count, ...) {
    volatile int sum = 0;
    /* Simulate variable argument processing */
    for (int i = 0; i < count; i++) {
        sum += i * 2;
    }
    __asm__ volatile ("" : : : "esi", "edi", "r8", "r9");
    return sum;
}

/* Main computation function with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    volatile int* ptr1 = &v1;
    volatile float* ptr2 = &f1;
    volatile int result = 0;
    volatile float fresult = 0.0f;
    volatile double dresult = 0.0;
    
    /* Take addresses to affect frame pointer decisions */
    int local_addr = (int)(intptr_t)&v1;
    
    /* First basic block with computations and calls */
    if (local_addr > 0) {
        /* Computation before call */
        v1 = v2 + v3 * v4 - v5;
        f1 = f2 * f3 + f4 / f5;
        
        /* Function call with many arguments */
        result = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* More computations between calls */
        v6 = v7 * v8 + result;
        f2 = f3 - f4 * result;
        
        /* Inline assembly to clobber specific registers */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx"
            : : : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Second function call */
        fresult = process_floats(f1, f2, f3, f4, f5, 6.6f, 7.7f, 8.8f);
        
        /* Complex computation using results */
        v7 = (int)(fresult * 100) + v6;
    }
    
    /* Loop to create more basic blocks with calls inside */
    for (int i = 0; i < 3; i++) {
        volatile int loop_var = i * 10;
        
        /* Different computation in each iteration */
        if (i == 0) {
            dresult = mixed_computation(v1, v2, f1, f2, d1, d2, (int*)&v3);
            
            /* More register pressure */
            v8 = v9 * v10 + (int)dresult;
            f3 = f4 + f5 * (float)dresult;
            
            /* Another call */
            void* buffer = create_buffer(64);
            volatile char* p = (char*)buffer;
            p[0] = (char)v8;
        } 
        else if (i == 1) {
            /* Different call pattern */
            int sum = varargs_like(5, v1, v2, v3, v4, v5);
            
            /* Computation using the result */
            v9 = v8 + sum * 2;
            f4 = f3 * 2.0f + (float)sum;
            
            /* Force more register clobbering */
            __asm__ volatile (
                "pxor %%xmm0, %%xmm0\n\t"
                "pxor %%xmm1, %%xmm1\n\t"
                "pxor %%xmm2, %%xmm2"
                : : : "xmm0", "xmm1", "xmm2", "memory"
            );
        } 
        else {
            /* Nested calls and computations */
            result = compute_sum(v6, v7, v8, v9, v10, v1, v2, v3, v4, v5);
            fresult = process_floats(f3, f4, f5, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
            
            /* Complex expression with many live values */
            dresult = (double)result * 0.5 + (double)fresult * 2.0 + d3 + d4;
            
            /* Use alloca in loop to affect stack frame */
            if (i == 2) {
                void* temp_buf = alloca(32);
                volatile int* ip = (int*)temp_buf;
                *ip = result;
            }
        }
        
        /* Loop-carried dependency to prevent optimization */
        v10 += loop_var + result;
    }
    
    /* Final computations and output */
    volatile int final_result = 
        v1 + v2 + v3 + v4 + v5 + 
        v6 + v7 + v8 + v9 + v10 + 
        (int)fresult + (int)dresult;
    
    printf("Result: %d\n", final_result);
    
    /* Additional call at the end */
    int checksum = varargs_like(3, final_result, final_result / 2, final_result % 100);
    printf("Checksum: %d\n", checksum);
    
    return final_result > 0 ? 0 : 1;
}

/* Additional functions to increase code density */
static void __attribute__((noinline))
side_effect_function(int* a, float* b, double* c) {
    *a += 100;
    *b *= 1.5f;
    *c /= 2.0;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "xmm6", "xmm7");
}

/* Function that might be inlined (no noinline attribute) */
static int simple_multiply(int x, int y) {
    return x * y;
}

/* Another function with register pressure */
void __attribute__((noinline)) 
external_call_sim(int a, int b, int c, int d, int e) {
    volatile int sum = a + b + c + d + e;
    /* Clobber many registers */
    __asm__ volatile (
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12"
        : : : "r10", "r11", "r12", "memory"
    );
    
    /* Call another function to create call chain */
    int result = simple_multiply(sum, 2);
    
    /* Use the result */
    volatile int* mem = (int*)malloc(sizeof(int));
    if (mem) {
        *mem = result;
        free(mem);
    }
}
