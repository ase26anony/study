#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Helper functions with various attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline asm to clobber registers */
    __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return result;
}

/* Function with pointer arguments */
static float __attribute__((noinline))
process_floats(float *arr, int count, float scale, float offset) {
    volatile float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += arr[i] * scale + offset;
    }
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    return sum;
}

/* Function that uses alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    volatile int local = size * 2;
    void *buf = alloca(size + local % 16);
    __asm__ volatile ("" : : : "rbx", "r12", "r13", "r14", "r15");
    return buf;
}

/* Function with mixed types */
static double __attribute__((noinline))
mixed_calculation(int a, float b, double c, int *d, float *e) {
    volatile double result = (double)a + (double)b + c + (double)(*d) + (double)(*e);
    /* Force register clobbering */
    __asm__ volatile ("" : : : "rax", "rdx", "xmm0", "xmm1", "xmm2", "r10", "r11");
    return result;
}

/* Global function (not static) to affect inlining decisions */
int __attribute__((noinline))
global_computation(int x, int y, int z) {
    volatile int temp = x * y + z;
    __asm__ volatile ("" : : : "rcx", "rdx", "rsi", "rdi");
    return temp * 2 - 1;
}

/* Function that takes address of locals */
static void __attribute__((noinline))
use_addresses(int *ptr1, float *ptr2, double *ptr3) {
    volatile int local1 = 42;
    volatile float local2 = 3.14f;
    volatile double local3 = 2.71828;
    
    *ptr1 = local1 + (int)local2;
    *ptr2 = local2 * (float)local3;
    *ptr3 = local3 / (double)local1;
    
    /* Take address to force stack frame */
    int *addr1 = &local1;
    float *addr2 = &local2;
    double *addr3 = &local3;
    
    __asm__ volatile ("" : : "r"(addr1), "r"(addr2), "r"(addr3) : 
                     "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2");
}

int main(void) {
    /* Declare many local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1, *fp2 = &f2;
    volatile double result = 0.0;
    
    /* Create control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, 
                              iteration, iteration*2, iteration*3, 
                              iteration*4, iteration*5);
        
        /* Inline asm between computations to force live ranges */
        __asm__ volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", 
                         "r8", "r9", "r10", "r11", 
                         "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Conditional to create basic block boundaries */
        if (sum1 > 50) {
            /* Second basic block with different call */
            float arr[8] = {f1, f2, f3, f4, f5, 
                           (float)sum1, (float)iteration, 0.0f};
            float float_result = process_floats(arr, 8, 1.5f, 0.5f);
            
            /* More inline asm */
            __asm__ volatile ("" : : : "xmm6", "xmm7", "xmm8", "xmm9", 
                             "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Use alloca to affect frame pointer */
            void *buffer = create_buffer(64 + iteration * 16);
            memset(buffer, iteration, 32);
            
            /* Mixed type computation */
            double mixed = mixed_calculation(v1, f2, d3, &v4, &f5);
            
            /* Update result with all values */
            result += (double)sum1 + (double)float_result + mixed + 
                     (double)((intptr_t)buffer % 100);
        } else {
            /* Alternative path with different calls */
            int global_res = global_computation(v2, v3, v4);
            
            int addr_result1, float addr_result2;
            double addr_result3;
            use_addresses(&addr_result1, &addr_result2, &addr_result3);
            
            result += (double)global_res + addr_result3;
            
            /* More register clobbering */
            __asm__ volatile ("" : : : "rbx", "r12", "r13", "r14", "r15",
                             "xmm0", "xmm1", "xmm2", "xmm3");
        }
        
        /* Loop body continues with more computations */
        v1 = (v1 * 3) % 100;
        v2 = (v2 + iteration) % 100;
        v3 = (v3 * v2) % 100;
        f1 = f1 * 1.1f;
        f2 = f2 + 0.2f;
        d3 = d3 * 1.01;
        
        /* Final asm clobber in loop */
        __asm__ volatile ("" : : : "rax", "rcx", "rdx", "xmm0", "xmm1");
    }
    
    /* Additional complex control flow */
    for (int i = 0; i < 2; i++) {
        switch (i) {
            case 0: {
                int complex_sum = compute_sum(
                    v1, v2, v3, v4, v5,
                    i, i*10, i*20, i*30, i*40
                );
                result += complex_sum * 0.5;
                break;
            }
            case 1: {
                float local_arr[4] = {f1, f2, f3, f4};
                float float_res = process_floats(local_arr, 4, 2.0f, -1.0f);
                result += (double)float_res;
                
                /* Nested call sequence */
                int nested = global_computation(
                    (int)result, 
                    (int)(result * 2), 
                    (int)(result / 2)
                );
                result += nested * 0.25;
                break;
            }
        }
    }
    
    /* Final computation and output */
    printf("Result checksum: %f\n", result);
    printf("v1=%d, v2=%d, v3=%d, v4=%d, v5=%d\n", v1, v2, v3, v4, v5);
    printf("f1=%f, f2=%f, f3=%f, f4=%f, f5=%f\n", f1, f2, f3, f4, f5);
    
    return (int)(result * 1000) % 256;
}
