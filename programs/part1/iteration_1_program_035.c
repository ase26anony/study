#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
__attribute__((noinline))
static int func_many_args(int a, int b, int c, int d, int e, int f, int g, int h, 
                         int i, int j, int k, int l) {
    volatile int result = a + b - c + d - e + f - g + h - i + j - k + l;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    return result;
}

/* Function with pointer arguments */
__attribute__((noinline))
static float* func_pointer_ops(float* f1, float* f2, float* f3, float* f4) {
    volatile float temp = *f1 + *f2 - *f3 + *f4;
    *f1 = temp;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return f1;
}

/* Function that might be inlined (no static, no noinline) */
int func_inline_candidate(int x, int y) {
    volatile int z = x * y;
    __asm__ volatile ("" : : : "r10", "r11");
    return z + 1;
}

/* Function using alloca to affect frame pointer */
__attribute__((noinline))
static void func_with_alloca(int size) {
    volatile char* buf = alloca(size);
    for (int i = 0; i < size && i < 16; i++) {
        buf[i] = (char)(i * 3);
    }
    __asm__ volatile ("" : : : "rax", "rbx", "rcx");
}

/* Function with mixed float/int operations */
__attribute__((noinline))
static double func_mixed_types(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    __asm__ volatile ("" : : : "xmm4", "xmm5", "xmm6", "xmm7", "rax");
    return result * 2.0;
}

/* Main function creating maximum register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 5.5, d2 = 6.6;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Take addresses to affect frame pointer decisions */
    int local_for_address = 42;
    volatile int* addr_taker = &local_for_address;
    
    /* Complex control flow creating basic blocks */
    int checksum = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic block 1: computations before first call */
        v1 = v2 + v3;
        f1 = f2 * f3;
        d1 = (double)v4 + (double)f4;
        
        /* Call with many arguments - forces register pressure */
        int result1 = func_many_args(v1, v2, v3, v4, v5, v6, v7, v8, 
                                    v9, v10, v11, v12);
        
        /* Basic block 2: more computations between calls */
        if (result1 > 50) {
            v5 = v6 * v7;
            f2 = f3 / f4;
            
            /* Inline assembly clobbering specific registers */
            __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
            
            /* Call with pointer arguments */
            float* fp_result = func_pointer_ops((float*)&f1, (float*)&f2, 
                                               (float*)&f3, (float*)&f4);
            
            /* More computations */
            v8 = v9 - v10;
            d2 = d1 * 1.5;
            
            /* Call that might be inlined */
            int inline_result = func_inline_candidate(v11, v12);
            
            /* Use alloca to affect frame pointer */
            func_with_alloca(32 + iteration * 8);
            
            /* Mixed type function call */
            double mixed_result = func_mixed_types(v13, f1, d2, (int*)&v14, (float*)&f2);
            
            /* Update checksum with all results */
            checksum += result1 + (int)(*fp_result) + inline_result + (int)mixed_result;
        } else {
            /* Alternative path with different call pattern */
            v13 = v14 + v15;
            f3 = f4 * 2.0f;
            
            __asm__ volatile ("" : : : "r10", "r11", "r12");
            
            int result2 = func_many_args(v15, v14, v13, v12, v11, v10,
                                        v9, v8, v7, v6, v5, v4);
            
            checksum += result2 * 2;
        }
        
        /* Loop computations that keep values live across iterations */
        v2 = v3 ^ v4;
        f4 = f1 + f2;
        *addr_taker += iteration;
    }
    
    /* Final computations and output */
    volatile int final_check = checksum;
    for (int i = 0; i < 5; i++) {
        final_check = final_check * 3 + i;
        __asm__ volatile ("" : : : "rax", "rbx");
    }
    
    printf("Result checksum: %d\n", final_check);
    
    /* Additional test with nested function calls */
    {
        volatile int nest1 = 100, nest2 = 200, nest3 = 300;
        volatile float nest_f1 = 10.5f, nest_f2 = 20.5f;
        
        /* Chain of calls to create complex save/restore patterns */
        int r1 = func_many_args(nest1, nest2, nest3, v1, v2, v3, v4, v5,
                               v6, v7, v8, v9);
        
        __asm__ volatile ("" : : : "eax", "ecx", "edx", "esi", "edi");
        
        float* r2 = func_pointer_ops((float*)&nest_f1, (float*)&nest_f2,
                                    (float*)&f1, (float*)&f2);
        
        double r3 = func_mixed_types(r1, *r2, d1, &nest1, &nest_f1);
        
        printf("Nested result: %f\n", r3);
    }
    
    return final_check > 0 ? 0 : 1;
}

/* Additional functions to increase compilation complexity */
__attribute__((noinline))
void extra_func1(int a, int b, int c, int d, int e) {
    volatile int x = a + b + c + d + e;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
}

__attribute__((noinline))
float extra_func2(float a, float b, float c, float d, float e, float f) {
    volatile float y = a * b - c * d + e / f;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return y;
}
