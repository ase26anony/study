/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile global variables to force memory operations */
volatile int volatile_global_int = 42;
volatile double volatile_global_double = 3.14159;
volatile float volatile_global_float = 2.71828f;

/* Helper function with many arguments to stress argument passing */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float b1, float b2, float b3, float b4,
    double c1, double c2, double c3, double c4,
    long d1, long d2, int* e1, float* e2
) {
    /* Complex mixing of types */
    double result = (double)a1 * c1 + (double)b1 * c2;
    result += (double)a2 * sin(c3) + (double)b2 * cos(c4);
    result += (double)d1 * 0.5 + (double)d2 * 0.25;
    *e1 = (int)result;
    *e2 = (float)result;
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
void compute_heavy(double* input, double* output, int size) {
    /* Declare many local variables to create register pressure */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile int vi6 = 6, vi7 = 7, vi8 = 8, vi9 = 9, vi10 = 10;
    
    volatile long vl1 = 1000, vl2 = 2000, vl3 = 3000, vl4 = 4000;
    volatile long vl5 = 5000, vl6 = 6000, vl7 = 7000, vl8 = 8000;
    
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f;
    volatile float vf5 = 5.5f, vf6 = 6.6f, vf7 = 7.7f, vf8 = 8.8f;
    volatile float vf9 = 9.9f, vf10 = 10.10f;
    
    volatile double vd1 = 1.01, vd2 = 2.02, vd3 = 3.03, vd4 = 4.04;
    volatile double vd5 = 5.05, vd6 = 6.06, vd7 = 7.07, vd8 = 8.08;
    volatile double vd9 = 9.09, vd10 = 10.10, vd11 = 11.11, vd12 = 12.12;
    
    /* Additional non-volatile variables for computation */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Complex array indexing variables */
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3;
    int stride1 = 16, stride2 = 32, stride3 = 64;
    
    /* Main computation loop with data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Load from input array with complex addressing */
        d1 = input[idx1 * stride1 + idx2 + iter];
        d2 = input[idx2 * stride2 + idx3 + iter * 2];
        d3 = input[idx3 * stride3 + idx4 + iter * 3];
        d4 = input[(idx1 + idx2) * 8 + iter];
        
        /* Long chain of mixed-type computations */
        f1 = (float)vi1 * vf1 + (float)vi2 * vf2;
        f2 = (float)vl1 * 0.001f + (float)vl2 * 0.002f;
        f3 = f1 * f2 + vf3 * vf4;
        
        i1 = (int)(f1 * 100.0f) + vi3;
        i2 = (int)(f2 * 200.0f) + vi4;
        i3 = i1 * i2 + vi5;
        
        d5 = (double)f3 * vd1 + (double)i3 * vd2;
        d6 = sin(d5) * cos(d1) + tan(d2);
        
        l1 = (long)(d5 * 1000.0) + vl3;
        l2 = (long)(d6 * 2000.0) + vl4;
        l3 = l1 * l2 / (vl5 + 1);
        
        f4 = (float)d3 * (float)d4 + (float)l3 * 0.001f;
        f5 = f4 * vf5 + vf6 * vf7;
        
        /* More computations creating data dependencies */
        d7 = (double)f5 * vd3 + (double)vi6 * vd4;
        d8 = d7 * d1 + d2 * d3 - d4 * d5;
        
        i4 = (int)d7 + (int)d8 + vi7;
        i5 = i4 * vi8 + vi9;
        
        f6 = (float)i5 * 0.01f + (float)i4 * 0.02f;
        f7 = f6 * vf8 + vf9 * vf10;
        
        d9 = (double)f7 * vd5 + (double)vi10 * vd6;
        d10 = d9 * d6 + d7 * d8 - d5 * d4;
        
        l4 = (long)d9 * (long)d10 + vl6;
        l5 = l4 / (vl7 + 1) * (vl8 + 1);
        
        f8 = (float)l4 * 0.0001f + (float)l5 * 0.0002f;
        f9 = f8 * vf1 + vf2 * vf3;
        f10 = f9 * vf4 + vf5 * vf6;
        
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
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        /* For AArch64 */
        #elif defined(__aarch64__) || defined(_M_ARM64)
        __asm__ volatile (
            "# Clobber many registers\n\t"
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
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
        );
        #endif
        
        /* Call function with many arguments */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2,
            &i6, &f8
        );
        
        /* More computations after function call */
        d10 = d10 * func_result + volatile_global_double;
        f10 = f10 * (float)func_result + volatile_global_float;
        i10 = i5 + (int)func_result + volatile_global_int;
        
        /* Store to output with complex addressing */
        int out_idx = (idx1 * stride1 + idx2 * stride2 + iter) % size;
        output[out_idx] = d10 + (double)f10 + (double)i10;
        
        /* Update indices with complex patterns */
        idx1 = (idx1 + 1) % 8;
        idx2 = (idx2 + 2) % 8;
        idx3 = (idx3 + 3) % 8;
        idx4 = (idx4 + 5) % 8;
        
        /* Use volatile globals in computation */
        vd1 += volatile_global_double * 0.01;
        vf1 += volatile_global_float * 0.02f;
        vi1 += volatile_global_int;
    }
    
    /* Final sink to volatile to prevent optimization */
    volatile double sink = d10 + f10 + i10;
    (void)sink; /* Prevent unused variable warning */
}

int main() {
    /* Allocate and initialize arrays */
    double* input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = sin(i * 0.1) * 100.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    
    return 0;
}
