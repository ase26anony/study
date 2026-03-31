/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile arrays to force memory operations */
volatile int global_int_array[ARRAY_SIZE];
volatile double global_double_array[ARRAY_SIZE];
volatile float global_float_array[ARRAY_SIZE];

/* Helper function with many arguments to stress argument passing */
__attribute__((noinline))
double many_args_function(int a1, int a2, int a3, int a4,
                         float f1, float f2, float f3, float f4,
                         double d1, double d2, double d3, double d4,
                         int* p1, float* p2, double* p3) {
    /* Complex computation mixing all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result -= (double)(a3 + a4) / (d3 + d4);
    result += (*p1) * (*p2) * (*p3);
    return result;
}

/* Critical function with extreme register pressure */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
void compute_heavy(volatile int* input_int, volatile double* input_double,
                   volatile float* input_float, volatile int* output,
                   int size, int stride) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Floating point variables */
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Pointer/index variables */
    int i, j, k, idx1, idx2, idx3;
    volatile int* ptr1;
    volatile float* ptr2;
    volatile double* ptr3;
    
    /* Initialize some values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    
    ptr1 = input_int;
    ptr2 = input_float;
    ptr3 = input_double;
    
    /* Main computation loop with complex data dependencies */
    for (i = 0; i < size; i += stride) {
        /* Complex array indexing with multiple terms */
        idx1 = i * stride;
        idx2 = (i + 1) * stride - 1;
        idx3 = (i * 2 + stride / 2) % size;
        
        /* Load data with complex addressing */
        v6 = input_int[idx1] + input_int[idx2];
        v7 = input_int[idx3] - input_int[idx1];
        
        f6 = input_float[idx1] * input_float[idx2];
        f7 = input_float[idx3] / (input_float[idx1] + 1.0f);
        
        d6 = input_double[idx1] + input_double[idx2];
        d7 = input_double[idx3] - input_double[idx1];
        
        /* Long chain of mixed-type computations */
        /* This creates data dependencies preventing register reuse */
        v8 = (int)((float)v1 * f1 + (double)v2 * d1);
        v9 = (int)((float)v3 * f2 - (double)v4 * d2);
        v10 = v8 + v9 - v5;
        
        f8 = (float)((double)v6 * d3 + (int)f3 * v7);
        f9 = f4 * f5 + f6 - f7;
        f10 = f8 / f9 * 2.0f;
        
        d8 = (double)((float)v8 * f8 + (int)d4 * v9);
        d9 = d5 * d6 + d7 * d3;
        d10 = d8 / d9 * 2.0;
        
        /* More complex computations with type conversions */
        v11 = (int)(d10 * 100.0);
        v12 = (int)(f10 * 50.0f);
        v13 = v10 + v11 + v12;
        
        f10 = (float)v13 * 0.01f + f9;
        d10 = (double)v13 * 0.01 + d9;
        
        v14 = (int)(f10 * d10);
        v15 = v13 * v14 / (v12 + 1);
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
        #if defined(__x86_64__) || defined(_M_X64)
        __asm__ volatile (
            "# Clobber many registers\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            "mov $0, %%r12\n\t"
            "mov $0, %%r13\n\t"
            "mov $0, %%r14\n\t"
            "mov $0, %%r15\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory", "cc"
        );
        /* For AArch64 */
        #elif defined(__aarch64__) || defined(_M_ARM64)
        __asm__ volatile (
            "# Clobber many ARM registers\n\t"
            "mov x0, #0\n\t"
            "mov x1, #0\n\t"
            "mov x2, #0\n\t"
            "mov x3, #0\n\t"
            "mov x4, #0\n\t"
            "mov x5, #0\n\t"
            "mov x6, #0\n\t"
            "mov x7, #0\n\t"
            "mov x8, #0\n\t"
            "mov x9, #0\n\t"
            "mov x10, #0\n\t"
            "mov x11, #0\n\t"
            "mov x12, #0\n\t"
            "mov x13, #0\n\t"
            "mov x14, #0\n\t"
            "mov x15, #0\n\t"
            "mov x16, #0\n\t"
            "mov x17, #0\n\t"
            "mov x18, #0\n\t"
            "mov x19, #0\n\t"
            "mov x20, #0\n\t"
            "mov x21, #0\n\t"
            "mov x22, #0\n\t"
            "mov x23, #0\n\t"
            "mov x24, #0\n\t"
            "mov x25, #0\n\t"
            "mov x26, #0\n\t"
            "mov x27, #0\n\t"
            "mov x28, #0\n\t"
            "fmov d0, #0.0\n\t"
            "fmov d1, #0.0\n\t"
            "fmov d2, #0.0\n\t"
            "fmov d3, #0.0\n\t"
            "fmov d4, #0.0\n\t"
            "fmov d5, #0.0\n\t"
            "fmov d6, #0.0\n\t"
            "fmov d7, #0.0\n\t"
            "fmov d8, #0.0\n\t"
            "fmov d9, #0.0\n\t"
            "fmov d10, #0.0\n\t"
            "fmov d11, #0.0\n\t"
            "fmov d12, #0.0\n\t"
            "fmov d13, #0.0\n\t"
            "fmov d14, #0.0\n\t"
            "fmov d15, #0.0\n\t"
            "fmov d16, #0.0\n\t"
            "fmov d17, #0.0\n\t"
            "fmov d18, #0.0\n\t"
            "fmov d19, #0.0\n\t"
            "fmov d20, #0.0\n\t"
            "fmov d21, #0.0\n\t"
            "fmov d22, #0.0\n\t"
            "fmov d23, #0.0\n\t"
            "fmov d24, #0.0\n\t"
            "fmov d25, #0.0\n\t"
            "fmov d26, #0.0\n\t"
            "fmov d27, #0.0\n\t"
            "fmov d28, #0.0\n\t"
            "fmov d29, #0.0\n\t"
            "fmov d30, #0.0\n\t"
            "fmov d31, #0.0\n\t"
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory", "cc"
        );
        #else
        /* Generic clobber for other architectures */
        __asm__ volatile ("# Generic clobber" ::: "memory", "cc");
        #endif
        
        /* Continue computation after assembly clobber */
        v16 = v15 * 2 + v14;
        v17 = v16 - v13 + v12;
        v18 = v17 * 3 / 2;
        v19 = v18 + v11 - v10;
        v20 = v19 % 1000;
        
        f10 = f10 * 1.5f + (float)v20 * 0.01f;
        d10 = d10 * 1.5 + (double)v20 * 0.01;
        
        /* Call function with many arguments */
        double func_result = many_args_function(
            v1, v2, v3, v4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            (int*)&v5, (float*)&f5, (double*)&d5
        );
        
        /* Use function result in further computation */
        v20 += (int)(func_result * 100.0);
        f10 += (float)func_result;
        d10 += func_result;
        
        /* Complex store with addressing */
        output[i] = v20 + (int)f10 + (int)d10;
        
        /* Update variables to maintain dependencies */
        v1 = v20 % 256;
        v2 = v1 + 1;
        v3 = v2 * 2;
        v4 = v3 - 1;
        v5 = v4 % 128;
        
        f1 = (float)v1 * 0.1f;
        f2 = (float)v2 * 0.2f;
        f3 = (float)v3 * 0.3f;
        f4 = (float)v4 * 0.4f;
        f5 = (float)v5 * 0.5f;
        
        d1 = (double)v1 * 0.01;
        d2 = (double)v2 * 0.02;
        d3 = (double)v3 * 0.03;
        d4 = (double)v4 * 0.04;
        d5 = (double)v5 * 0.05;
        
        /* Structure-like access pattern */
        ptr1 = &output[i];
        ptr2 = &global_float_array[i % ARRAY_SIZE];
        ptr3 = &global_double_array[i % ARRAY_SIZE];
        
        *ptr1 = *ptr1 + (int)(*ptr2 * *ptr3);
    }
}

int main() {
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Output array */
    volatile int output[ARRAY_SIZE];
    
    /* Perform computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(global_int_array, global_double_array,
                     global_float_array, output,
                     ARRAY_SIZE, 4);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
