#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex computation mixing all types */
    double sum = (double)a1 * b1 + (double)c1 * d1;
    sum += (double)a2 * b2 + (double)c2 * d2;
    sum += (double)a3 * b3 + (double)c3 * d3;
    sum += (double)a4 * b4 + (double)c4 * d4;
    sum += (double)a5 * b5 + (double)c5 * d5;
    sum += (double)a6 * b6 + (double)c6 * d6;
    
    /* Force register pressure inside helper too */
    volatile double v1 = sum * 1.1;
    volatile float v2 = (float)sum * 2.2f;
    volatile int v3 = (int)sum * 3;
    volatile long v4 = (long)sum * 4;
    
    return v1 + v2 + v3 + v4;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline))
static void compute_heavy(const int* input, double* output, int size) {
    /* Declare many local variables to consume registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int idx1, idx2, idx3, idx4;
    
    /* Initialize some variables */
    v1 = global_volatile_int;
    d1 = global_volatile_double;
    f1 = global_volatile_float;
    l1 = (long)v1 * 100;
    
    for (int i = 0; i < size; i++) {
        /* Complex array indexing with multiple terms */
        int base_idx = i * 2;
        idx1 = base_idx % 256;
        idx2 = (base_idx + 1) % 256;
        idx3 = (base_idx * 3) % 256;
        idx4 = (base_idx + idx1) % 256;
        
        /* Long chain of data-dependent computations mixing types */
        v2 = input[idx1] + v1;
        v3 = input[idx2] * v2;
        v4 = v3 - input[idx3];
        v5 = v4 / (input[idx4] + 1);
        
        l2 = (long)v2 * l1;
        l3 = l2 + (long)v3 * 7;
        l4 = l3 - (long)v4 * 3;
        l5 = l4 / ((long)v5 + 2);
        
        f2 = (float)v2 * f1;
        f3 = f2 + (float)v3 * 1.5f;
        f4 = f3 - (float)v4 * 0.5f;
        f5 = f4 / ((float)v5 + 0.1f);
        
        d2 = (double)l2 * d1;
        d3 = d2 + (double)l3 * 1.7;
        d4 = d3 - (double)l4 * 0.3;
        d5 = d4 / ((double)l5 + 0.01);
        
        /* More mixing between float and double */
        f6 = (float)d2 * f5;
        f7 = f6 + (float)d3 * 2.3f;
        f8 = f7 - (float)d4 * 1.2f;
        f9 = f8 / ((float)d5 + 0.05f);
        
        d6 = (double)f2 * d5;
        d7 = d6 + (double)f3 * 3.1;
        d8 = d7 - (double)f4 * 2.2;
        d9 = d8 / ((double)f5 + 0.02);
        
        /* Integer conversions */
        v6 = (int)f6 + v5;
        v7 = (int)d6 * v6;
        v8 = v7 - (int)f7;
        v9 = v8 / ((int)d7 + 1);
        v10 = v9 * (int)f8;
        
        l6 = (long)d8 + l5;
        l7 = (long)f9 * l6;
        l8 = l7 - (long)d9;
        l9 = l8 / ((long)f10 + 1);
        l10 = l9 * (long)v10;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__)
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
        /* For AArch64 */
#elif defined(__aarch64__)
        __asm__ volatile (
            "# Clobber many registers\n"
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
            "mov x19, #0\n"
            "mov x20, #0\n"
            "mov x21, #0\n"
            "mov x22, #0\n"
            "mov x23, #0\n"
            "mov x24, #0\n"
            "mov x25, #0\n"
            "mov x26, #0\n"
            "mov x27, #0\n"
            "mov x28, #0\n"
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
        
        /* Call function with many arguments - forces argument passing reloads */
        double func_result = many_args_function(
            v1, d1, f1, l1,
            v2, d2, f2, l2,
            v3, d3, f3, l3,
            v4, d4, f4, l4,
            v5, d5, f5, l5,
            v6, d6, f6, l6
        );
        
        /* More computations after function call */
        f10 = (float)func_result * f9;
        d10 = (double)func_result * d9;
        
        v1 = v10 + (int)func_result;
        l1 = l10 + (long)func_result;
        
        /* Complex store with multiple index calculations */
        int store_idx = (i * 3 + idx1 * 7 + idx2 * 11) % size;
        output[store_idx] = d10 + (double)f10 + (double)v1 + (double)l1;
        
        /* Update volatile globals to force memory stores */
        global_volatile_int = v1;
        global_volatile_double = d10;
        global_volatile_float = f10;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)calloc(ARRAY_SIZE, sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
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
    printf("Volatile globals: %d, %f, %f\n", 
           global_volatile_int, 
           global_volatile_double, 
           global_volatile_float);
    
    free(input_array);
    free(output_array);
    
    return 0;
}
