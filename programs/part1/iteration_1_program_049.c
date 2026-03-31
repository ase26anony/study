#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create varied call sites */

/* Function that returns value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return result;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float* arr, int count, float scale, float bias) {
    volatile float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += arr[i] * scale + bias;
    }
    /* Force register clobbering */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return sum;
}

/* Function that takes address of locals */
static void __attribute__((noinline))
manipulate_pointers(int* p1, int* p2, int* p3, int* p4, int* p5) {
    volatile int temp = *p1 + *p2;
    *p3 = temp * 2;
    *p4 = temp / 2;
    *p5 = temp % 17;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
}

/* Function with mixed types */
static double __attribute__((noinline))
mixed_calculation(int a, float b, double c, long d, short e) {
    volatile double result = (double)a + (double)b + c + (double)d + (double)e;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "rax", "rbx");
    return result;
}

/* Function using alloca to affect frame pointer */
static int __attribute__((noinline))
use_alloca(int size) {
    char* buffer = (char*)alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 100; i++) {
        buffer[i] = (char)(i % 256);
        sum += buffer[i];
    }
    /* Take address to force frame pointer usage */
    volatile char* addr = &buffer[0];
    (void)addr;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx");
    return sum;
}

/* Main function with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4, *p5 = &v5;
    volatile float farray[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Take addresses to inhibit optimization and affect frame pointer */
    volatile int* addr1 = &v1;
    volatile int* addr2 = &v2;
    (void)addr1; (void)addr2;
    
    int result = 0;
    float fresult = 0.0f;
    double dresult = 0.0;
    
    /* Create control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* Block 1: Multiple computations between calls */
            v1 = v1 * 2 + v2;
            v2 = v2 / 2 + v3;
            
            /* Call with many arguments - forces register pressure */
            int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
            
            /* More computations keeping values live */
            f1 = f1 * 1.5f + f2;
            f2 = f2 / 1.5f + f3;
            
            /* Inline assembly that clobbers call-clobbered registers */
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                                            "r8", "r9", "r10", "r11", "xmm0", "xmm1");
            
            /* Another call with different signature */
            fresult += process_floats((float*)farray, 8, f1, f2);
            
            /* Keep using the computed values */
            v3 = sum1 % 100;
            v4 = v3 * 2;
        } else {
            /* Block 2: Different sequence of calls and computations */
            v5 = v5 + v6 - v7;
            v6 = v6 * v7 / 2;
            
            /* Call that takes addresses */
            manipulate_pointers(&v1, &v2, &v3, &v4, &v5);
            
            /* Mixed type computations */
            d1 = d1 + 0.5;
            d2 = d2 * 1.1;
            
            /* Another inline assembly clobber */
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2");
            
            /* Call with mixed types */
            dresult += mixed_calculation(v1, f1, d1, (long)v2, (short)v3);
            
            /* Use alloca to affect stack frame */
            int alloca_result = use_alloca(50 + iteration * 10);
            v7 = alloca_result % 100;
        }
        
        /* Additional computations between iterations */
        v8 = v1 + v2 + v3;
        v9 = v4 * v5 - v6;
        v10 = (v7 + v8 + v9) % 256;
        
        /* More calls in the loop body */
        if (iteration < 2) {
            int sum2 = compute_sum(v8, v9, v10, v1, v2, v3, v4, v5, v6, v7);
            v1 = (v1 + sum2) % 1000;
            
            __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi");
            
            fresult += process_floats((float*)farray, 4, f3, f4);
        }
    }
    
    /* Final computation using all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)fresult;
    result += (int)dresult;
    
    /* Use all variables one more time to keep them live */
    volatile int final_check = 
        *p1 + *p2 + *p3 + *p4 + *p5 + 
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3;
    
    printf("Result: %d (check: %d)\n", result, final_check);
    
    return result == 0 ? 0 : 1;
}
