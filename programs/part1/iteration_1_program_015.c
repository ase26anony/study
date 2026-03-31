#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Force register pressure inside callee */
    volatile int x1 = result * 2;
    volatile int x2 = result / 3;
    volatile int x3 = result << 2;
    volatile int x4 = result ^ 0x55;
    return (int)(x1 + x2 + x3 + x4);
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float *arr, int count, float scale, float offset) {
    volatile float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += arr[i] * scale + offset;
    }
    /* More register pressure */
    volatile float t1 = sum * 2.0f;
    volatile float t2 = sum / 3.0f;
    volatile float t3 = sum + 1.0f;
    return t1 + t2 + t3;
}

/* Function that clobbers specific registers via inline asm */
void __attribute__((noinline))
force_register_clobber(void) {
    /* Explicitly clobber call-clobbered registers */
    __asm__ volatile (
        "# Force clobber\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

/* Function using alloca to affect frame pointer */
int __attribute__((noinline))
use_alloca(int size) {
    /* Taking address forces frame pointer usage */
    int local = 42;
    int *ptr = &local;
    
    /* Use alloca to force dynamic stack allocation */
    char *buffer = (char*)alloca(size);
    for (int i = 0; i < size; i++) {
        buffer[i] = (char)(i + *ptr);
    }
    
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += buffer[i];
    }
    
    return sum;
}

/* Function with mixed types */
double __attribute__((noinline))
mixed_type_computation(int a, float b, double c, long d) {
    volatile double result = (double)a + (double)b + c + (double)d;
    
    /* Create register pressure with different types */
    volatile float f1 = (float)result;
    volatile int i1 = (int)result;
    volatile long l1 = (long)result;
    volatile double d1 = result * 2.0;
    
    return d1 + (double)f1 + (double)i1 + (double)l1;
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile long l1 = 100, l2 = 200, l3 = 300;
    
    /* Array for pointer arguments */
    float float_arr[8];
    for (int i = 0; i < 8; i++) {
        float_arr[i] = (float)i * 1.5f;
    }
    
    int checksum = 0;
    
    /* Complex control flow creating multiple basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic Block 1: Computations before first call */
        v1 = v2 * v3 + v4;
        f1 = f2 * f3 - f4;
        d1 = (double)v5 + (double)f5;
        
        /* Inline asm to clobber registers between computations */
        __asm__ volatile (
            "# Clobber between computations\n"
            : 
            : 
            : "rax", "rcx", "rdx", "rsi", "rdi", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Call 1: Function with many arguments */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += sum1;
        
        /* Basic Block 2: More computations */
        if (iteration % 2 == 0) {
            v6 = v7 ^ v8 | v9;
            f2 = f3 / f4 * f5;
            d2 = d1 * 1.5;
            
            /* Call 2: Function with pointer arguments */
            float sum2 = process_floats(float_arr, 8, f1, f2);
            checksum += (int)sum2;
            
            /* Force register clobbering */
            force_register_clobber();
            
            /* More computations keeping values live */
            v7 = v8 + v9 - v10;
            f3 = f4 + f5 - f1;
        } else {
            v8 = v9 & v10 | v1;
            f4 = f5 * 2.0f - f2;
            d3 = d2 / 2.0;
            
            /* Call 3: Function using alloca */
            int sum3 = use_alloca(iteration + 16);
            checksum += sum3;
        }
        
        /* Basic Block 3: Mixed type computations */
        v9 = v10 << 2;
        f5 = f1 + f2 + f3 + f4;
        
        /* Call 4: Mixed type function */
        double sum4 = mixed_type_computation(v1, f1, d1, l1);
        checksum += (int)sum4;
        
        /* Inline asm creating live ranges across calls */
        __asm__ volatile (
            "# Another clobber point\n"
            : "+r" (v1), "+r" (v2), "+r" (v3)
            : 
            : "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Final computations in the loop */
        v10 = v1 + v2 + v3 + v4 + v5;
        l1 = (long)v6 * (long)v7;
        l2 = l1 + l3;
        
        /* Another call to increase call density */
        int sum5 = compute_sum(v10, v9, v8, v7, v6, v5, v4, v3, v2, v1);
        checksum += sum5;
    }
    
    /* Use computed values to prevent dead code elimination */
    volatile int final_result = 
        v1 + v2 + v3 + v4 + v5 + 
        v6 + v7 + v8 + v9 + v10 +
        (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
        (int)d1 + (int)d2 + (int)d3 +
        (int)l1 + (int)l2 + (int)l3 +
        checksum;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
