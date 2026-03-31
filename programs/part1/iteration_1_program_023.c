#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
float __attribute__((noinline)) 
process_floats(float* arr, int len, float scale, float offset) {
    volatile float sum = 0.0f;
    for (int i = 0; i < len; i++) {
        sum += arr[i] * scale + offset;
    }
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    return sum;
}

/* Function that takes mixed types and returns a pointer */
static void* __attribute__((noinline)) 
complex_operation(int a, float b, double c, void* ptr) {
    volatile double result = (double)a * b * c;
    __asm__ volatile ("" : : : "rax", "r10", "r11", "xmm3", "xmm4");
    
    if (ptr != NULL) {
        *(double*)ptr = result;
    }
    
    return ptr;
}

/* Function with variable arguments simulation */
int __attribute__((noinline)) 
multi_arg_func(int a, int b, int c, int d, int e, 
               int f, int g, int h, int i, int j,
               int k, int l, int m, int n, int o) {
    /* Force register pressure by using all arguments */
    volatile int sum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
    
    /* More inline assembly clobbering */
    __asm__ volatile ("" : : : "r8", "r9", "r12", "r13", "r14", "r15");
    
    return sum;
}

/* Function that uses alloca to affect frame pointer */
void* __attribute__((noinline)) 
use_alloca(size_t size) {
    void* buffer = alloca(size);
    volatile int* ptr = (int*)buffer;
    
    /* Force register usage with computation */
    for (size_t i = 0; i < size / sizeof(int); i++) {
        ptr[i] = (int)(i * 3.14159);
    }
    
    __asm__ volatile ("" : : : "rbx", "rbp", "rsp");
    
    return buffer;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile double* p3 = &d1;
    
    int result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Create control flow with basic blocks containing function calls */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Basic block 1: Multiple computations between calls */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        f1 = f1 * 1.5f + (float)iteration;
        f2 = f2 / 1.5f - (float)iteration;
        
        /* Call 1: Function with many arguments */
        if (iteration % 2 == 0) {
            result = compute_sum(v1, v2, v3, v4, v5, 
                                iteration, iteration*2, iteration*3,
                                iteration*4, iteration*5);
        } else {
            result = compute_sum(v5, v4, v3, v2, v1,
                                iteration*5, iteration*4, iteration*3,
                                iteration*2, iteration);
        }
        
        /* Basic block 2: More computations */
        d1 = d1 + (double)result * 0.01;
        d2 = d2 - (double)iteration * 0.02;
        
        /* Inline assembly to clobber specific registers */
        __asm__ volatile (
            "movl $0x12345678, %%eax\n\t"
            "movl $0x87654321, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            : : : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Call 2: Function with pointer arguments */
        float arr[8];
        for (int i = 0; i < 8; i++) {
            arr[i] = (float)(i + iteration) * 0.5f;
        }
        float_result = process_floats(arr, 8, f3, f4);
        
        /* Basic block 3: Conditional with function call */
        if (result > 100) {
            /* Call 3: Function with mixed types */
            double temp = d3;
            p3 = complex_operation(result, float_result, d4, &temp);
            d3 = temp;
            
            /* More register pressure */
            v3 = v3 ^ result;
            v4 = v4 | iteration;
            f3 = f3 * float_result;
            f4 = f4 / float_result;
        }
        
        /* Call 4: Function with many arguments (exceeding register limits) */
        int large_result = multi_arg_func(
            v1, v2, v3, v4, v5,
            result, iteration, iteration+1, iteration+2, iteration+3,
            iteration+4, iteration+5, iteration+6, iteration+7, iteration+8
        );
        
        /* Basic block 4: Loop with function call */
        for (int inner = 0; inner < 3; inner++) {
            /* Call 5: Function using alloca (affects frame pointer) */
            void* buffer = use_alloca(64);
            volatile int* buf_int = (int*)buffer;
            
            /* Use the allocated memory to prevent optimization */
            v5 = v5 + buf_int[inner];
            
            /* More computations to keep values live */
            d5 = d5 * (1.0 + (double)inner * 0.1);
            f5 = f5 + (float)large_result * 0.001f;
            
            /* Another inline assembly with clobber */
            __asm__ volatile (
                "movq $0x1122334455667788, %%r10\n\t"
                "movq $0x8877665544332211, %%r11\n\t"
                "addq %%r11, %%r10\n\t"
                : : : "r10", "r11", "r12", "r13", "memory"
            );
        }
        
        /* Final computation using all variables */
        double_result += (double)result + (double)float_result + d1 + d2 + d3 + d4 + d5;
        
        /* Take address of locals to affect frame pointer optimization */
        int* addr1 = &v1;
        float* addr2 = &f1;
        double* addr3 = &d1;
        
        /* Use the addresses to prevent dead store elimination */
        *addr1 += 1;
        *addr2 += 1.0f;
        *addr3 += 1.0;
    }
    
    /* Compute and print checksum */
    uint64_t checksum = (uint64_t)result + 
                       (uint64_t)float_result + 
                       (uint64_t)double_result +
                       (uint64_t)v1 + (uint64_t)v2 + (uint64_t)v3 +
                       (uint64_t)v4 + (uint64_t)v5;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Results: int=%d, float=%f, double=%f\n", 
           result, float_result, double_result);
    
    return 0;
}
