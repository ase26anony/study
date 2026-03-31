#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with various attributes to create diverse call sites */

/* Function that returns a value and takes many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, 
            int i, int j, float k, float l, void* m, void* n) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    result += (int)(k + l);
    result += (int)((intptr_t)m ^ (intptr_t)n);
    return result;
}

/* Function with pointer arguments and returns */
static void* __attribute__((noinline)) 
manipulate_pointers(void* p1, void* p2, void* p3, void* p4, 
                    void* p5, void* p6) {
    volatile intptr_t sum = (intptr_t)p1 + (intptr_t)p2 + (intptr_t)p3;
    sum ^= (intptr_t)p4 ^ (intptr_t)p5 ^ (intptr_t)p6;
    return (void*)(sum & 0xFFFFFF);
}

/* Function that uses alloca to affect frame pointer */
static int __attribute__((noinline)) 
use_alloca(int size) {
    volatile int* arr = (int*)alloca(size * sizeof(int));
    int sum = 0;
    for (int i = 0; i < size && i < 8; i++) {
        arr[i] = i * 2;
        sum += arr[i];
    }
    return sum;
}

/* Function with mixed float/int operations */
static float __attribute__((noinline))
float_operations(float a, float b, float c, float d, 
                 float e, float f, float g, float h) {
    volatile float result = a * b + c * d - e * f + g / h;
    /* Force register pressure with intermediate calculations */
    float t1 = a + b;
    float t2 = c - d;
    float t3 = e * f;
    float t4 = g * h;
    result += t1 * t2 - t3 / t4;
    return result;
}

/* Non-static function to prevent inlining */
int __attribute__((noinline)) 
external_computation(int x, int y, float z, void* ptr) {
    volatile int result = x * y + (int)z;
    result ^= (intptr_t)ptr;
    return result;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile void* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4, *p5 = &v5;
    
    int result = 0;
    
    /* Create control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Multiple function calls with live values */
            
            /* Keep values live across calls */
            int temp1 = v1 + v2 + v3;
            float temp2 = f1 * f2 + f3;
            
            /* Inline assembly to clobber specific registers */
            /* Force eax/rax clobber on x86 */
            __asm__ volatile (
                "movl $0x12345678, %%eax\n\t"
                "addl $0x11111111, %%eax\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%eax", "memory"
            );
            
            /* Call with many arguments - will exceed register passing */
            int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8,
                                  v9, v10, f1, f2, p1, p2);
            
            /* More computations keeping values live */
            temp1 += v4 * v5;
            temp2 -= f4 / f5;
            
            /* Another inline assembly clobber */
            /* Force r10 clobber on x86-64 */
            __asm__ volatile (
                "movq $0xFFFFFFFF, %%r10\n\t"
                "xorq %%r10, %%r10\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%r10", "memory"
            );
            
            /* Call that returns pointer */
            void* ptr_result = manipulate_pointers(p1, p2, p3, p4, p5, &v6);
            
            /* Use results in further computations */
            temp1 += (intptr_t)ptr_result;
            result += temp1 + (int)temp2 + sum1;
            
        } else {
            /* Block 2: Different pattern of calls */
            
            /* Create more register pressure */
            float temp3 = f1 + f2 + f3 + f4 + f5;
            int temp4 = v6 * v7 - v8 * v9 + v10;
            
            /* Call with alloca to affect frame pointer */
            int alloca_result = use_alloca(temp4 % 16 + 4);
            
            /* Force register clobber */
            __asm__ volatile (
                "movl $0xAAAAAAAA, %%ebx\n\t"
                "movl $0x55555555, %%ecx\n\t"
                "addl %%ebx, %%ecx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "%ebx", "%ecx", "memory"
            );
            
            /* Float-intensive call */
            float float_res = float_operations(f1, f2, f3, f4, f5, f1, f2, f3);
            
            /* Mix results */
            temp3 += float_res;
            temp4 += alloca_result;
            
            /* External call */
            int ext_res = external_computation(temp4, v1, temp3, p3);
            
            result += ext_res + (int)temp3 + temp4;
        }
        
        /* Loop creates basic block boundaries */
        if (iteration < 2) {
            /* Additional computation between iterations */
            v1 += 1; v2 += 2; v3 += 3;
            f1 += 0.5f; f2 += 1.0f; f3 += 1.5f;
            
            /* Another call in loop body */
            int loop_sum = compute_sum(v1, v2, v3, v4, v5, 
                                      v6, v7, v8, v9, v10,
                                      f1, f2, p1, p2);
            result += loop_sum;
        }
    }
    
    /* Final computation using all variables */
    int final_check = 0;
    final_check += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    final_check += (int)(f1 + f2 + f3 + f4 + f5);
    final_check += (intptr_t)p1 + (intptr_t)p2 + (intptr_t)p3 + 
                   (intptr_t)p4 + (intptr_t)p5;
    
    result += final_check;
    
    /* Print to prevent optimization and verify */
    printf("Result: %d\n", result);
    
    return 0;
}
