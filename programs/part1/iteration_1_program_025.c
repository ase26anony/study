#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Force register pressure inside callee */
    volatile int x1 = result * 2;
    volatile int x2 = result / 3;
    volatile int x3 = x1 ^ x2;
    volatile int x4 = x3 << 2;
    return x4;
}

/* Function with pointer arguments */
float __attribute__((noinline)) 
process_floats(float* f1, float* f2, float* f3, float* f4, float* f5) {
    volatile float sum = *f1 + *f2 + *f3 + *f4 + *f5;
    /* More register pressure */
    volatile float t1 = sum * 1.5f;
    volatile float t2 = sum / 2.0f;
    volatile float t3 = t1 - t2;
    return t3;
}

/* Function that clobbers specific registers via inline asm */
void __attribute__((noinline))
force_register_clobber(void) {
    /* Explicitly clobber call-clobbered registers */
    __asm__ volatile (
        "nop"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

/* Function using alloca to affect frame pointer */
int* __attribute__((noinline))
create_dynamic_array(int size) {
    /* Using alloca forces frame pointer usage */
    int* arr = (int*)alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        arr[i] = i * i;
    }
    volatile int* ptr = arr;
    return ptr;
}

/* Mixed type function */
double __attribute__((noinline))
mixed_computation(int a, float b, double c, int* d, float* e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    
    /* Inline asm that clobbers specific registers */
    __asm__ volatile (
        "mov $0x12345678, %%eax\n\t"
        "mov $0x9ABCDEF0, %%ecx\n\t"
        "add %%ecx, %%eax"
        : 
        : 
        : "eax", "ecx", "memory"
    );
    
    return result * 2.0;
}

/* Main function with complex control flow and register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* ptr1 = &v1;
    volatile float* ptr2 = &f1;
    
    /* Take addresses to inhibit optimization and affect frame pointer */
    int local_for_address = 42;
    volatile int* addr_taker = &local_for_address;
    
    /* Result accumulator */
    volatile double final_result = 0.0;
    
    /* Control flow to create basic block boundaries */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        v3 = v3 ^ v1;
        v4 = v4 | v2;
        
        /* Force register clobbering between computations */
        force_register_clobber();
        
        /* Function call with many arguments - will exceed register passing limits */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        
        /* More computations keeping values live in registers */
        f1 = f1 * 1.1f + (float)sum1;
        f2 = f2 / 1.2f + (float)iteration;
        
        /* Conditional to create basic block split */
        if (sum1 > 100) {
            /* Another basic block with different operations */
            v5 = v5 << 2;
            v6 = v6 >> 1;
            
            /* Inline asm that clobbers specific registers */
            __asm__ volatile (
                "mov $0xDEADBEEF, %%r10d\n\t"
                "mov $0xCAFEBABE, %%r11d\n\t"
                "xor %%r10d, %%r11d"
                : 
                : 
                : "r10", "r11", "memory"
            );
            
            /* Call function with pointer arguments */
            float float_result = process_floats(&f1, &f2, &f3, &f4, &f5);
            final_result += (double)float_result;
        } else {
            /* Alternative path with alloca usage */
            int* dynamic = create_dynamic_array(5);
            v7 = dynamic[2] + v7;
            
            /* More register-intensive computations */
            d1 = d1 * 1.5;
            d2 = d2 + 0.5;
            d3 = d3 - 0.25;
            
            /* Mixed type function call */
            double mixed = mixed_computation(v8, f3, d1, &v9, &f4);
            final_result += mixed;
        }
        
        /* Loop body continues with more computations */
        v8 = v8 + v1;
        v9 = v9 - v2;
        v10 = v10 * v3;
        
        /* Another forced clobber */
        __asm__ volatile (
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2"
            : 
            : 
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        /* Final computation in loop */
        f5 = f5 + f1 + f2 + f3 + f4;
    }
    
    /* Additional complex control flow outside loop */
    {
        volatile int temp = 0;
        for (int i = 0; i < 5; i++) {
            if (i % 2 == 0) {
                temp += compute_sum(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9);
                force_register_clobber();
            } else {
                temp -= mixed_computation(i, i*1.0f, i*1.0, &i, &f1);
            }
        }
        final_result += temp;
    }
    
    /* Print result to prevent optimization */
    printf("Final result: %f\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
