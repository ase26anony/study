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
process_floats(float *arr, int count) {
    volatile float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += arr[i];
        /* Force register pressure with inline assembly */
        __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    }
    return sum;
}

/* Function that takes mixed types */
static double __attribute__((noinline))
mixed_computation(int a, float b, double c, int *d, float *e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    /* Clobber more registers */
    __asm__ volatile ("" : : : "rax", "r10", "r11", "xmm3", "xmm4");
    return result;
}

/* Function with alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    /* alloca forces frame pointer usage */
    void *buf = alloca(size);
    volatile int marker = 42;
    __asm__ volatile ("" : : "r"(buf), "r"(marker) : "memory");
    return buf;
}

/* Function that returns a pointer */
static int* __attribute__((noinline))
get_pointer(int *base, int offset) {
    volatile int *result = base + offset;
    __asm__ volatile ("" : : : "rax", "rbx", "rcx");
    return result;
}

/* Main computation with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int *p1, *p2, *p3;
    volatile float *fp1, *fp2;
    volatile double *dp1;
    
    /* Take addresses to force stack usage and frame pointer */
    int local_array[20];
    float float_array[15];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        local_array[i] = i * 2;
    }
    for (int i = 0; i < 15; i++) {
        float_array[i] = i * 1.5f;
    }
    
    /* Create control flow with basic blocks containing calls */
    int checksum = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic block 1: Multiple computations between calls */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        f1 = f1 * 1.5f + (float)iteration;
        
        /* Call with many arguments - forces register pressure */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, 
                              iteration, iteration*2, iteration*3,
                              iteration*4, iteration*5);
        
        /* More computations keeping values live */
        v3 = v3 ^ sum1;
        v4 = v4 | (sum1 << 3);
        f2 = f2 + f1 * 2.0f;
        
        /* Inline assembly that clobbers specific registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            : 
            : "r"(v3), "r"(v4)
            : "eax", "ebx", "ecx", "edx"
        );
        
        /* Conditional to create basic block boundaries */
        if (iteration % 2 == 0) {
            /* Basic block 2: Another function call */
            float float_sum = process_floats(float_array, 10);
            f3 = f3 + float_sum;
            
            /* More computations */
            d1 = d1 + (double)f3;
            v5 = v5 * 3 - (int)d1;
            
            /* Another call with mixed types */
            p1 = local_array;
            fp1 = float_array;
            double mixed_result = mixed_computation(v5, f3, d1, p1, fp1);
            d2 = d2 + mixed_result;
            
            /* Use alloca to affect frame pointer */
            void *buffer = create_buffer(64);
            __asm__ volatile ("" : : "r"(buffer) : "memory");
        } else {
            /* Basic block 3: Different call pattern */
            p2 = get_pointer(local_array, iteration);
            v1 = v1 + *p2;
            
            /* More register pressure */
            f4 = f4 * 0.5f + (float)v1;
            d3 = d3 / 2.0 + (double)f4;
            
            /* Another call with many arguments */
            int sum2 = compute_sum(v1, *p2, v3, v4, v5,
                                  (int)f1, (int)f2, (int)f3,
                                  (int)d1, (int)d2);
            v2 = v2 ^ sum2;
        }
        
        /* Loop creates another basic block boundary */
        checksum += v1 + v2 + (int)f1 + (int)f2 + (int)d1 + (int)d2;
        
        /* Additional inline assembly between computations */
        __asm__ volatile (
            "addl %0, %%eax\n\t"
            "addsd %1, %%xmm0\n\t"
            : 
            : "r"(checksum), "r"(d3)
            : "eax", "xmm0", "xmm1", "memory"
        );
    }
    
    /* Final computation and output */
    int final_result = checksum 
                     + v1 + v2 + v3 + v4 + v5 
                     + (int)f1 + (int)f2 + (int)f3 + (int)f4
                     + (int)d1 + (int)d2 + (int)d3;
    
    printf("Result: %d\n", final_result);
    
    /* Additional test with nested loops and calls */
    {
        volatile int outer_var = 100;
        for (int i = 0; i < 5; i++) {
            volatile int inner_var = i * 10;
            for (int j = 0; j < 3; j++) {
                /* Function call inside nested loop creates complex basic blocks */
                int temp = compute_sum(outer_var, inner_var, j, i, 
                                      final_result, checksum, v1, v2, v3, v4);
                inner_var += temp;
                
                /* Inline assembly with clobber list */
                __asm__ volatile (
                    "movl %0, %%r10d\n\t"
                    "movl %1, %%r11d\n\t"
                    : 
                    : "r"(inner_var), "r"(temp)
                    : "r10", "r11", "r12", "memory"
                );
            }
            outer_var += inner_var;
        }
        printf("Nested result: %d\n", outer_var);
    }
    
    return final_result & 0xFF;
}

/* Additional helper functions to increase code density */
static void __attribute__((noinline))
side_effect_function(int *a, float *b, double *c) {
    *a = *a * 2 + 1;
    *b = *b * 1.5f;
    *c = *c / 2.0;
    __asm__ volatile ("" : : : "rax", "rbx", "xmm5", "xmm6", "xmm7");
}

/* Function with variable arguments (simulated) */
static int __attribute__((noinline))
varargs_like(int count, ...) {
    volatile int result = count;
    /* Simulate va_arg usage */
    int *args = &count;
    for (int i = 0; i < 5 && i < count; i++) {
        result += args[i];
    }
    __asm__ volatile ("" : : : "eax", "edx", "ecx", "r8", "r9");
    return result;
}
