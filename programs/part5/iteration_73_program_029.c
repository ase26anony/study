#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile float global_volatile_float = 3.14159f;
volatile double global_volatile_double = 2.71828;

/* Helper function with many arguments to stress argument passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    void* p1, void* p2, long l1, long l2)
{
    /* Complex computation mixing all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)((int)(d3 * 100.0) + (int)(d4 * 100.0));
    result += (double)((uintptr_t)p1 % 1000 + (uintptr_t)p2 % 1000);
    result += (double)(l1 % 1000 - l2 % 1000);
    result += (double)a3 * (double)a4 * 0.01;
    result += (double)f3 * (double)f4 * 0.01;
    
    /* Force memory access */
    result += global_volatile_double;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline))
static void compute_heavy(const int* input, double* output, int size)
{
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int idx1, idx2, idx3, idx4;
    float* fp1, *fp2;
    double* dp1, *dp2;
    
    /* Volatile locals to inhibit optimizations */
    volatile int vi1 = 1, vi2 = 2;
    volatile float vf1 = 1.0f, vf2 = 2.0f;
    volatile double vd1 = 1.0, vd2 = 2.0;
    
    /* Initialize some variables */
    i1 = input[0] % 100;
    i2 = input[1] % 100;
    i3 = input[2] % 100;
    i4 = input[3] % 100;
    i5 = input[4] % 100;
    i6 = input[5] % 100;
    i7 = input[6] % 100;
    i8 = input[7] % 100;
    i9 = input[8] % 100;
    i10 = input[9] % 100;
    
    f1 = (float)i1 * 0.1f;
    f2 = (float)i2 * 0.2f;
    f3 = (float)i3 * 0.3f;
    f4 = (float)i4 * 0.4f;
    f5 = (float)i5 * 0.5f;
    
    d1 = (double)i6 * 0.1;
    d2 = (double)i7 * 0.2;
    d3 = (double)i8 * 0.3;
    d4 = (double)i9 * 0.4;
    d5 = (double)i10 * 0.5;
    
    l1 = (long)i1 * 1000L;
    l2 = (long)i2 * 2000L;
    l3 = (long)i3 * 3000L;
    l4 = (long)i4 * 4000L;
    
    /* Complex loop with data dependencies */
    for (int iter = 0; iter < ITERATIONS && iter < size; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % size;
        idx2 = (iter * 13) % size;
        idx3 = (iter * 17) % size;
        idx4 = (iter * 23) % size;
        
        /* Long chain of mixed-type computations */
        f6 = (float)input[idx1] * f1 + f2 - f3 * f4;
        d6 = (double)input[idx2] * d1 + d2 - d3 * d4;
        
        /* Mix integer and floating point */
        i1 = (int)(f6 * 10.0f) + i2 - i3 * i4;
        l5 = (long)(d6 * 100.0) + l1 - l2 * l3;
        
        /* More mixed computations */
        f7 = (float)i1 * 0.01f + (float)l5 * 0.0001f;
        d7 = (double)i2 * 0.01 + (double)l1 * 0.0001;
        
        /* Convert between types */
        i5 = (int)f7 + (int)d7;
        f8 = (float)i5 + (float)l2;
        d8 = (double)i6 + (double)l3;
        
        /* Use volatile variables in computation */
        f9 = f8 * vf1 + vf2;
        d9 = d8 * vd1 + vd2;
        i6 = i5 * vi1 + vi2;
        
        /* Even more variables to increase pressure */
        f10 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9;
        d10 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
        i7 = i1 + i2 + i3 + i4 + i5 + i6;
        l6 = l1 + l2 + l3 + l4 + l5;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__) || defined(_M_X64)
        __asm__ volatile (
            "# Clobber many registers\n"
            "mov $0, %%rax\n"
            "mov $0, %%rbx\n"
            "mov $0, %%rcx\n"
            "mov $0, %%rdx\n"
            "mov $0, %%rsi\n"
            "mov $0, %%rdi\n"
            "mov $0, %%r8\n"
            "mov $0, %%r9\n"
            "mov $0, %%r10\n"
            "mov $0, %%r11\n"
            "mov $0, %%r12\n"
            "mov $0, %%r13\n"
            "mov $0, %%r14\n"
            "mov $0, %%r15\n"
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
#elif defined(__aarch64__) || defined(__arm64__)
        /* For AArch64 */
        __asm__ volatile (
            "# Clobber many ARM registers\n"
            "mov x0, #0\n"
            "mov x1, #0\n"
            "mov x2, #0\n"
            "mov x3, #0\n"
            "mov x4, #0\n"
            "mov x5, #0\n"
            "mov x6, #0\n"
            "mov x7, #0\n"
            "mov x8, #0\n"
            "mov x9, #0\n"
            "mov x10, #0\n"
            "mov x11, #0\n"
            "mov x12, #0\n"
            "mov x13, #0\n"
            "mov x14, #0\n"
            "mov x15, #0\n"
            "mov x16, #0\n"
            "mov x17, #0\n"
            "mov x18, #0\n"
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces register/stack reloads */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            (void*)&input[idx3], (void*)&output[idx4],
            l5, l6
        );
        
        /* Use the result in further computation */
        f1 = f1 * 0.9f + (float)func_result * 0.1f;
        d1 = d1 * 0.9 + func_result * 0.1;
        i1 = i1 * 9 / 10 + (int)func_result % 10;
        
        /* Complex store with addressing mode */
        output[iter] = (double)f10 + d10 + (double)i7 + (double)l6 + func_result;
        
        /* Update variables to create dependencies for next iteration */
        f2 = f1 * 1.1f;
        d2 = d1 * 1.1;
        i2 = i1 + 1;
        l2 = l1 + 1000;
        
        /* Access volatile globals */
        f3 = f2 * global_volatile_float;
        d3 = d2 * global_volatile_double;
        i3 = i2 + global_volatile_int;
        
        /* More mixed operations */
        f4 = (float)i3 * 0.5f + f3;
        d4 = (double)i2 * 0.5 + d3;
        l3 = (long)(f4 * 1000.0f) + (long)(d4 * 1000.0);
        
        /* Cycle variables to keep them all live */
        float ftmp = f1; f1 = f2; f2 = f3; f3 = f4; f4 = f5; f5 = ftmp;
        double dtmp = d1; d1 = d2; d2 = d3; d3 = d4; d4 = d5; d5 = dtmp;
        int itmp = i1; i1 = i2; i2 = i3; i3 = i4; i4 = i5; i5 = itmp;
        long ltmp = l1; l1 = l2; l2 = l3; l3 = l4; l4 = l5; l5 = ltmp;
    }
    
    /* Final volatile store to prevent elimination */
    volatile double sink = d1 + d2 + d3 + d4 + d5 + (double)f1;
    (void)sink;
}

int main(void)
{
    /* Allocate and initialize arrays */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform heavy computation */
    compute_heavy(input_array, output_array, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE && i < ITERATIONS; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(input_array);
    free(output_array);
    
    return 0;
}
