#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
process_floats(float f1, float f2, float f3, float f4, float *out) {
    volatile float temp = f1 * f2 + f3 / f4;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    *out = temp;
    return temp;
}

/* Function that takes variable arguments to force stack usage */
int __attribute__((noinline))
var_args_func(int count, ...) {
    volatile int sum = 0;
    /* Force register pressure with local computations */
    int a = count * 2;
    int b = count + 5;
    int c = a ^ b;
    __asm__ volatile ("" : : : "r10", "r11", "r12");
    return sum + a + b + c;
}

/* Function using alloca to affect frame pointer */
void* __attribute__((noinline))
use_alloca(size_t size) {
    void *ptr = alloca(size);
    volatile int marker = 42;
    __asm__ volatile ("" : : : "rbp", "rsp");
    return ptr;
}

/* Function that clobbers many registers */
void __attribute__((noinline))
clobber_registers(void) {
    __asm__ volatile ("" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15");
}

/* Main computation with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1, *fp2 = &f2;
    
    int result = 0;
    float float_result = 0.0f;
    
    /* Take addresses to affect frame pointer decisions */
    int local_addr_test = 42;
    int *addr_ptr = &local_addr_test;
    
    /* Control flow to create basic blocks */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            /* Block with function calls and computations */
            v1 = v1 * 2 + i;
            v2 = v2 / 2 - i;
            
            /* Call with many arguments - forces register/stack decisions */
            int sum = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
            
            /* Inline assembly between computations and calls */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx");
            
            v3 = v3 ^ sum;
            v4 = v4 | (sum << 2);
            
            /* Call function with float arguments */
            float temp;
            process_floats(f1, f2, f3, f4, &temp);
            float_result += temp;
            
            /* More register-intensive computations */
            f1 = f1 * 1.1f + (float)i;
            f2 = f2 / 1.1f - (float)i;
            
            /* Use alloca to affect stack frame */
            if (i % 3 == 0) {
                void *dynamic = use_alloca(64);
                *(int*)dynamic = i;
            }
        } else {
            /* Different basic block with different call pattern */
            v5 = v5 + v6 * i;
            v6 = v6 - v5 / (i + 1);
            
            /* Call varargs function */
            int var_res = var_args_func(5, v1, v2, v3, v4, v5);
            
            __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
            
            v7 = v7 & ~var_res;
            v8 = v8 | var_res;
            
            /* Function that clobbers many registers */
            clobber_registers();
            
            /* More float computations */
            f3 = f3 * f4 / (f5 + 1.0f);
            f4 = f4 + f3 * 0.5f;
            
            /* Pointer arithmetic to keep values live */
            *p1 = *p1 + *p2;
            *p2 = *p2 - *p1;
        }
        
        /* Additional computations between loop iterations */
        v9 = v9 * 3 + i;
        v10 = v10 / 3 - i;
        
        /* Mix in more inline assembly */
        __asm__ volatile ("" : : : "rax", "rdx", "xmm0", "xmm1");
        
        f5 = f5 * 2.0f - f1;
        
        /* Conditional call within the block */
        if (v9 > 100) {
            int extra = compute_sum(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            v9 = v9 % extra;
        }
    }
    
    /* Final computation using all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(f1 + f2 + f3 + f4 + f5);
    result += *addr_ptr;
    
    /* One more call at the end */
    int final_sum = compute_sum(result, result/2, result/3, result/4, 
                               result/5, result/6, result/7, result/8,
                               result/9, result/10);
    
    printf("Result: %d (checksum: %u)\n", final_sum, 
           (unsigned int)(final_sum ^ (uintptr_t)&result));
    
    return final_sum > 0 ? 0 : 1;
}
