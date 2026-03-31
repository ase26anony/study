#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments and returns pointer */
static float* __attribute__((noinline))
process_floats(float* arr, int size, float factor) {
    volatile float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
        sum += arr[i];
        /* Force register pressure with inline asm */
        __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    }
    return arr;
}

/* Function that takes mixed types and uses alloca to affect frame pointer */
int __attribute__((noinline))
complex_calculation(int base, float multiplier, char* desc) {
    /* Use alloca to force frame pointer usage */
    int* dynamic = (int*)alloca(sizeof(int) * 8);
    volatile float result = 0.0f;
    
    for (int i = 0; i < 8; i++) {
        dynamic[i] = base + i;
        result += dynamic[i] * multiplier;
    }
    
    /* Clobber multiple registers */
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
    
    return (int)result;
}

/* Variadic-like function using many registers */
static double __attribute__((noinline))
mixed_operations(int a, float b, double c, int d, float e, double f) {
    volatile double res = (double)a + (double)b + c + (double)d + (double)e + f;
    /* Force floating point register pressure */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return res * 2.0;
}

/* Function that returns through pointer parameter */
void __attribute__((noinline))
compute_stats(int* data, int size, int* sum, float* avg) {
    volatile int local_sum = 0;
    for (int i = 0; i < size; i++) {
        local_sum += data[i];
        /* Create artificial live ranges */
        __asm__ volatile ("" : : : "esi", "edi");
    }
    *sum = local_sum;
    *avg = (float)local_sum / size;
}

/* Main function with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile char buffer[32];
    volatile int checksum = 0;
    
    /* Array to create more pressure */
    volatile float float_arr[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    volatile int int_arr[12];
    
    /* Initialize arrays */
    for (int i = 0; i < 12; i++) {
        int_arr[i] = i * 2;
    }
    
    /* Control flow to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Multiple computations between calls */
            v1 = compute_sum(v1, v2, v3, v4, v5, 
                            iteration, iteration+1, iteration+2, 
                            iteration+3, iteration+4);
            
            /* Use inline assembly to clobber call-clobbered registers */
            __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
            
            /* Keep values live in registers between calls */
            f1 = f1 * 1.5f + (float)v1;
            d1 = d1 * 1.1 + (double)f1;
            
            p2 = process_floats((float*)float_arr, 8, 1.1f);
            
            /* More computations keeping values live */
            v2 = v2 + v1 * 2;
            f2 = f2 + f1 / 2.0f;
            
            __asm__ volatile ("" : : : "xmm6", "xmm7", "xmm8", "xmm9");
            
            d2 = mixed_operations(v1, f1, d1, v2, f2, d2);
        } else {
            /* Block 2: Different sequence of calls and computations */
            v3 = complex_calculation(v3, f3, (char*)buffer);
            
            /* Clobber different registers */
            __asm__ volatile ("" : : : "r11", "r12", "r13", "r14", "r15");
            
            f3 = f3 * 2.0f - (float)v3;
            d3 = d3 + (double)f3 * 0.5;
            
            int sum_result;
            float avg_result;
            compute_stats((int*)int_arr, 12, &sum_result, &avg_result);
            
            v4 = v4 + sum_result;
            f4 = f4 * avg_result;
            
            /* Force spill/fill around this call */
            __asm__ volatile ("" : : : "rbx", "rbp", "r12", "r13", "r14", "r15");
            
            v5 = compute_sum(v4, v3, v2, v1, iteration,
                            (int)f1, (int)f2, (int)f3, (int)f4, sum_result);
        }
        
        /* Loop-carried dependencies to keep values live across iterations */
        checksum += v1 + v2 + v3 + v4 + v5 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
        checksum += (int)d1 + (int)d2 + (int)d3;
        
        /* Conditional call inside loop to create more basic blocks */
        if (checksum % 7 == 0) {
            /* Another call site with different characteristics */
            float* result_ptr = process_floats((float*)float_arr, 8, 0.9f);
            f5 = *result_ptr + f5;
            
            __asm__ volatile ("" : : : "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        }
    }
    
    /* Final computation using all variables */
    volatile int final_result = 
        v1 * 2 + v2 * 3 + v3 * 4 + v4 * 5 + v5 * 6 +
        (int)(f1 * 10.0f) + (int)(f2 * 20.0f) + 
        (int)(f3 * 30.0f) + (int)(f4 * 40.0f) + (int)(f5 * 50.0f) +
        (int)(d1 * 100.0) + (int)(d2 * 200.0) + (int)(d3 * 300.0) +
        checksum;
    
    printf("Result: %d\n", final_result);
    
    /* Additional test with nested control flow */
    {
        volatile int a = 10, b = 20, c = 30;
        volatile float x = 1.5f, y = 2.5f, z = 3.5f;
        
        for (int i = 0; i < 2; i++) {
            if (i == 0) {
                a = compute_sum(a, b, c, i, i+1, i+2, i+3, i+4, i+5, i+6);
                __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
                x = x * (float)a;
            } else {
                b = complex_calculation(b, y, "test");
                __asm__ volatile ("" : : : "r8", "r9", "r10", "r11");
                y = y + (float)b;
            }
            
            /* Call within the loop with live values */
            z = (float)mixed_operations(a, x, (double)y, b, y, (double)z);
        }
    }
    
    return final_result > 0 ? 0 : 1;
}
