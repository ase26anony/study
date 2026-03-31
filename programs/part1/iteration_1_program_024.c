#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    return result;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float* f1, float* f2, float* f3, float* f4, float* f5) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5;
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
    return sum;
}

/* Function that takes mixed types */
static double __attribute__((noinline))
mixed_calculation(int i1, int i2, float f1, double d1, int* ptr) {
    volatile double result = (double)i1 * i2 + f1 + d1 + *ptr;
    __asm__ volatile ("" : : : "rax", "rbx", "xmm0", "xmm1");
    return result;
}

/* Function with many arguments to force stack passing */
int __attribute__((noinline))
many_args(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8,
          int a9, int a10, int a11, int a12, int a13, int a14, int a15) {
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                      a11 + a12 + a13 + a14 + a15;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11");
    return sum;
}

/* Function that uses alloca to affect frame pointer */
static void* __attribute__((noinline))
allocate_temp(int size) {
    void* ptr = alloca(size);
    __asm__ volatile ("" : : : "rax", "rbx");
    return ptr;
}

/* Recursive function to create complex control flow */
static int __attribute__((noinline))
recursive_compute(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    
    volatile int temp = n * 2;
    __asm__ volatile ("" : : : "rax", "rbx");
    
    /* Call another function in the middle */
    int intermediate = compute_sum(n, acc, temp, n+1, acc+1, 
                                  temp+1, n+2, acc+2, temp+2, n+3);
    
    return recursive_compute(n - 1, acc + intermediate);
}

int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* ptr1 = &v1;
    volatile int* ptr2 = &v2;
    
    int result = 0;
    
    /* Complex control flow with multiple basic blocks */
    for (int i = 0; i < 10; i++) {
        /* First basic block with computations */
        v1 = v1 * 2 + i;
        v2 = v2 / 2 + i;
        v3 = v3 + v1 - v2;
        
        /* Inline assembly to clobber call-clobbered registers */
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", 
                          "rsi", "rdi", "r8", "r9", "r10", "r11",
                          "xmm0", "xmm1", "xmm2", "xmm3",
                          "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Function call in the middle of basic block */
        if (i % 2 == 0) {
            int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
            
            /* More computations between calls */
            f1 = f1 * 1.5f;
            f2 = f2 / 1.5f;
            
            /* Another function call */
            float sum2 = process_floats(&f1, &f2, &f3, &f4, &f5);
            
            v4 = v4 + (int)sum1 + (int)sum2;
            
            /* Use alloca to affect frame pointer */
            void* temp = allocate_temp(64);
            __asm__ volatile ("" : : "r"(temp) : "rax", "rbx");
        } else {
            /* Different path with different calls */
            double calc = mixed_calculation(v1, v2, f3, d1, ptr1);
            
            /* More register pressure */
            d2 = d2 * calc;
            d3 = d3 / calc;
            
            /* Function with many arguments */
            int many = many_args(v1, v2, v3, v4, v5, v6, v7, v8,
                                v9, v10, v1, v2, v3, v4, v5);
            
            v5 = v5 + many + (int)calc;
        }
        
        /* Additional computations to extend live ranges */
        v6 = v6 ^ v1;
        v7 = v7 | v2;
        v8 = v8 & v3;
        
        /* Another inline assembly barrier */
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1");
        
        /* Nested conditional to create more basic blocks */
        if (v1 > 100) {
            /* Call within nested block */
            int temp = compute_sum(v6, v7, v8, v9, v10, v1, v2, v3, v4, v5);
            v9 = v9 + temp;
            
            /* More register-intensive operations */
            for (int j = 0; j < 3; j++) {
                f3 = f3 + f4 - f5;
                f4 = f4 * 1.1f;
                f5 = f5 / 1.1f;
                
                __asm__ volatile ("" : : : "xmm2", "xmm3", "xmm4");
            }
        } else {
            v10 = v10 * 2 - v1;
        }
        
        /* Final computation in the loop */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)(f1 + f2 + f3 + f4 + f5);
        result += (int)(d1 + d2 + d3);
    }
    
    /* Recursive call to create more complex call graph */
    int recursive_result = recursive_compute(5, result);
    
    /* Take address of locals to affect frame pointer */
    int* addr_array[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Final computation using addressed variables */
    for (int i = 0; i < 10; i++) {
        result += *addr_array[i] * (i + 1);
    }
    
    printf("Final result: %d (recursive: %d)\n", result, recursive_result);
    
    return 0;
}
