/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper with many arguments to force stack passing */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6)
{
    /* Complex mixing of types */
    double sum = (double)a1 * b1 + (double)c1 * d1;
    sum += (double)a2 * b2 + (double)c2 * d2;
    sum += (double)a3 * b3 + (double)c3 * d3;
    sum += (double)a4 * b4 + (double)c4 * d4;
    sum += (double)a5 * b5 + (double)c5 * d5;
    sum += (double)a6 * b6 + (double)c6 * d6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum *= global_volatile_float;
    sum += global_volatile_int;
    
    return sum;
}

/* Structure for complex memory access */
struct ComplexData {
    int ints[8];
    double doubles[4];
    float floats[8];
    long longs[4];
};

/* Main computation with extreme register pressure */
__attribute__((noinline))
static void compute_heavy(const int* input, double* output, int size) {
    /* Many local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int idx1, idx2, idx3, idx4, idx5;
    
    /* Additional pointer variables */
    const int* ptr1;
    const int* ptr2;
    double* out_ptr;
    
    /* Initialize some values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    l1 = 10L; l2 = 20L; l3 = 30L; l4 = 40L; l5 = 50L;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    
    /* Complex array indexing variables */
    int stride = 16;
    int offset = 8;
    
    /* Main computation loop with data dependencies */
    for (int i = 0; i < size; i++) {
        /* Complex array indexing with multiple terms */
        idx1 = i * stride + (i & 7);
        idx2 = idx1 + offset;
        idx3 = idx2 * 2 - i;
        idx4 = (idx3 + i) % ARRAY_SIZE;
        idx5 = (idx4 * 3 + 1) % ARRAY_SIZE;
        
        /* Load with complex addressing */
        v6 = input[idx1] + input[idx2];
        v7 = input[idx3] - input[idx4];
        v8 = input[idx5] * 2;
        
        /* Mixed type computations creating register pressure */
        f6 = (float)v6 * f1 + (float)v7 * f2;
        f7 = (float)v8 * f3 - (float)v6 * f4;
        
        d6 = (double)f6 * d1 + (double)f7 * d2;
        d7 = (double)v6 * d3 - (double)v7 * d4;
        
        l6 = (long)d6 * l1 + (long)d7 * l2;
        l7 = (long)v6 * l3 - (long)v8 * l4;
        
        /* More mixed computations */
        f8 = (float)l6 * f5 + (float)l7 * f1;
        d8 = (double)f8 * d5 + d6 * d7;
        
        v9 = (int)d8 + v6 - v7;
        v10 = (int)f8 * v8 + v9;
        
        l8 = (long)v10 * l5 + l6;
        l9 = (long)v9 * l7 - l8;
        
        f9 = (float)l8 * f2 + (float)l9 * f3;
        f10 = (float)v10 * f4 - f9;
        
        d9 = (double)f9 * d1 + (double)f10 * d2;
        d10 = d9 * d3 - d8 * d4;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__)
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
              "memory"
        );
#elif defined(__aarch64__)
        /* For AArch64 */
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
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces register/stack reloads */
        double func_result = many_args_function(
            v1, d1, f1, l1,
            v2, d2, f2, l2,
            v3, d3, f3, l3,
            v4, d4, f4, l4,
            v5, d5, f5, l5,
            v6, d6, f6, l6
        );
        
        /* Use the result in further computation */
        d10 += func_result;
        f10 += (float)func_result;
        
        /* Complex store with indexing */
        output[i] = d10 + (double)f10 + (double)v10 + (double)l9;
        
        /* Update variables to create dependencies for next iteration */
        v1 = v10 + 1;
        v2 = v9 - 1;
        l1 = l9 >> 1;
        l2 = l8 << 1;
        f1 = f10 * 0.5f;
        f2 = f9 * 2.0f;
        d1 = d10 * 0.5;
        d2 = d9 * 2.0;
        
        /* Access volatile globals to force memory operations */
        v3 += global_volatile_int;
        d3 += global_volatile_double;
        f3 += global_volatile_float;
    }
}

int main() {
    /* Allocate and initialize arrays */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Perform heavy computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input_array, output_array, ARRAY_SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(input_array);
    free(output_array);
    
    return 0;
}
