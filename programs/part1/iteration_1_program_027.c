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

/* Function with pointer arguments and float operations */
float __attribute__((noinline)) 
process_floats(float f1, float f2, float f3, float f4, 
               float f5, float f6, float* out) {
    volatile float temp = f1 * f2 + f3 * f4 - f5 / f6;
    *out = temp;
    /* Clobber floating point registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return temp;
}

/* Function that takes address of locals to force frame pointer */
static void __attribute__((noinline)) 
use_addresses(int* ptr1, float* ptr2, double* ptr3) {
    volatile int local1 = *ptr1 + 1;
    volatile float local2 = *ptr2 * 2.0f;
    volatile double local3 = *ptr3 / 3.0;
    
    /* Force register pressure with many operations */
    local1 = local1 * 2 - 3 + 4 / 5;
    local2 = local2 + 1.0f - 2.0f * 3.0f;
    local3 = local3 * 4.0 - 5.0 + 6.0;
    
    __asm__ volatile ("" : : : "rax", "r10", "r11", "xmm4", "xmm5");
}

/* Function with mixed types and alloca to affect stack frame */
void* __attribute__((noinline)) 
complex_operation(int count) {
    /* Use alloca to force frame pointer */
    int* dynamic = (int*)alloca(count * sizeof(int));
    volatile float f_temp = 0.0f;
    volatile double d_temp = 0.0;
    
    for (int i = 0; i < count; i++) {
        dynamic[i] = i * 2;
        f_temp += (float)dynamic[i];
        d_temp += (double)dynamic[i] * 1.5;
    }
    
    __asm__ volatile ("" : : : "r12", "r13", "r14", "r15", "xmm6", "xmm7");
    return dynamic; /* Address escapes, prevents optimization */
}

/* Recursive function to create more call sites */
static int __attribute__((noinline)) 
recursive_helper(int n, int* acc) {
    if (n <= 0) return *acc;
    
    volatile int local = n * 2;
    *acc += local;
    
    /* Create register pressure before recursive call */
    volatile float f1 = (float)local * 1.1f;
    volatile double d1 = (double)local * 1.2;
    volatile int i1 = local + 1;
    volatile int i2 = local - 1;
    
    __asm__ volatile ("" : : : "rbx", "rcx", "xmm8", "xmm9");
    
    return recursive_helper(n - 1, acc);
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33;
    volatile int* ptr1 = &v1;
    volatile float* ptr2 = &f1;
    volatile double* ptr3 = &d1;
    
    int checksum = 0;
    
    /* Control flow to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Basic block 1: Multiple computations between calls */
        v1 = v1 * 2 + v2;
        v3 = v3 - v4 * v5;
        f1 = f1 * f2 + f3;
        d1 = d1 / d2 - d3;
        
        /* Function call inside basic block */
        int sum = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += sum;
        
        /* More computations */
        v2 = v2 + v1 / 3;
        v4 = v4 * v5 - v6;
        f2 = f2 + f1 * 2.0f;
        d2 = d2 * 1.5 + d1;
        
        /* Inline assembly to clobber specific registers */
        __asm__ volatile ("" : : : "rax", "rbx", "rcx", "rdx", 
                                       "rsi", "rdi", "r8", "r9", "r10");
        
        /* Another function call */
        float f_result;
        float f_ret = process_floats(f1, f2, f3, f4, f5, f1, &f_result);
        checksum += (int)f_ret;
        
        /* Conditional to create new basic blocks */
        if (iteration % 2 == 0) {
            /* Basic block with address-taking operations */
            use_addresses((int*)ptr1, (float*)ptr2, (double*)ptr3);
            
            v6 = v6 * 3 - v7;
            v8 = v8 + v9 / 2;
            f3 = f3 * 1.5f - f4;
            d3 = d3 + d2 * 0.5;
            
            /* Function with alloca */
            complex_operation(10);
            
            v7 = v7 + v8 * v9;
            v10 = v10 - v1 + v2;
            f4 = f4 / f5 * 2.0f;
        } else {
            /* Alternative path with different operations */
            v5 = v5 * 2 + v6;
            v7 = v7 - v8 * v9;
            f5 = f5 * 3.0f - f1;
            
            /* More inline assembly */
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Recursive call */
            int acc = 5;
            recursive_helper(3, &acc);
            checksum += acc;
            
            v9 = v9 + v10 / 3;
            v2 = v2 * v3 - v4;
        }
        
        /* Loop-carried computations to maintain live values */
        v1 = v1 + iteration;
        v3 = v3 - iteration * 2;
        f1 = f1 + (float)iteration * 0.1f;
        d1 = d1 - (double)iteration * 0.01;
        
        /* Final function call in the loop */
        int final_sum = compute_sum(v10, v9, v8, v7, v6, v5, v4, v3, v2, v1);
        checksum += final_sum;
    }
    
    /* Compute final checksum using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)(f1 + f2 + f3 + f4 + f5);
    checksum += (int)(d1 + d2 + d3);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional test with nested loops and calls */
    {
        volatile int outer = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                /* Create register pressure in nested loop */
                volatile int temp1 = i * j + v1;
                volatile float temp2 = (float)i * f1 + (float)j * f2;
                volatile double temp3 = (double)i * d1 + (double)j * d2;
                
                /* Function call inside nested loop basic block */
                float f_out;
                process_floats(temp2, f1, f2, f3, f4, f5, &f_out);
                
                outer += temp1 + (int)temp2 + (int)temp3;
                
                /* Clobber registers between iterations */
                __asm__ volatile ("" : : : "rax", "rbx", "xmm10", "xmm11");
            }
        }
        checksum += outer;
    }
    
    printf("Final result: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
