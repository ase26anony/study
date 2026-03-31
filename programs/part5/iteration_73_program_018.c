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

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((intptr_t)p1 % 1000);
    result += (double)((intptr_t)p2 % 1000);
    
    /* Force memory access */
    result += volatile_global_double;
    result += (double)volatile_global_float;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(double* input, double* output, int size, int stride) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Pointer/index variables */
    int idx1, idx2, idx3, idx4;
    volatile int volatile_idx = 0; /* Volatile to prevent optimization */
    
    /* Initialize with pseudo-random values */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    i11 = 11; i12 = 12; i13 = 13; i14 = 14; i15 = 15;
    i16 = 16; i17 = 17; i18 = 18; i19 = 19; i20 = 20;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    f11 = 11.11f; f12 = 12.12f; f13 = 13.13f; f14 = 14.14f; f15 = 15.15f;
    f16 = 16.16f; f17 = 17.17f; f18 = 18.18f; f19 = 19.19f; f20 = 20.20f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    d16 = 16.16; d17 = 17.17; d18 = 18.18; d19 = 19.19; d20 = 20.20;
    
    /* Complex loop with data dependencies and mixed operations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = iter * stride;
        idx2 = (iter + 1) * stride;
        idx3 = (iter * 2) % size;
        idx4 = (iter * 3) % size;
        
        /* Load from input with complex addressing */
        d1 = input[idx1 + idx3 + volatile_idx];
        d2 = input[idx2 + idx4 + volatile_idx];
        f1 = (float)input[(idx1 * 2 + idx2) % size];
        f2 = (float)input[(idx3 * 3 + idx4) % size];
        
        /* Long chain of mixed-type computations creating data dependencies */
        /* This prevents register reuse due to dependencies */
        i1 = (int)(d1 * 100.0);
        f3 = (float)i1 * f1 + f2;
        d3 = (double)f3 * d2 / 3.14159;
        i2 = i1 + (int)d3;
        f4 = (float)i2 / 7.0f + f3;
        d4 = d3 + (double)f4 * 2.0;
        i3 = i2 * 3 - (int)d4;
        f5 = f4 * 1.5f - (float)i3;
        d5 = d4 / 1.618 + (double)f5;
        
        /* Continue the dependency chain */
        i4 = i3 ^ (int)(d5 * 1000);
        f6 = sqrtf(fabsf(f5)) + (float)i4;
        d6 = sin(d5) + (double)f6;
        i5 = (int)(d6 * 10000) % 1000;
        f7 = expf(f6 / 10.0f) * (float)i5;
        d7 = log(fabs(d6) + 1.0) * (double)f7;
        i6 = i5 + (int)(d7 * 100);
        f8 = (float)i6 * 0.01f + f7;
        d8 = d7 * 0.99 + (double)f8;
        
        /* More mixed operations */
        i7 = i6 | (int)(d8 * 1000000);
        f9 = (float)i7 / 1000.0f * f8;
        d9 = (double)f9 * d8 / 2.71828;
        i8 = i7 & (int)(d9 * 1000);
        f10 = f9 + (float)i8 * 0.001f;
        d10 = d9 - (double)f10 * 0.5;
        
        /* Use volatile variables in computation */
        i9 = i8 + volatile_global_int;
        f11 = f10 * volatile_global_float;
        d11 = d10 + volatile_global_double;
        
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
        /* For AArch64 */
#elif defined(__aarch64__) || defined(_M_ARM64)
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
        
        /* Continue computation after assembly clobber */
        /* This forces reloads of all variables */
        i10 = i9 * 2 + (int)(d11 * 100);
        f12 = f11 + (float)i10 * 0.1f;
        d12 = d11 * 2.0 - (double)f12;
        
        /* Call function with many arguments */
        /* This stresses argument passing registers */
        d13 = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&i5, (void*)&i6
        );
        
        /* More mixed operations */
        i11 = i10 + (int)d13;
        f13 = (float)i11 * 0.01f + f12;
        d14 = d12 + d13 * 0.5;
        
        i12 = i11 ^ (int)(d14 * 10000);
        f14 = f13 * 1.1f - (float)i12 * 0.001f;
        d15 = d14 / 1.1 + (double)f14;
        
        /* Final store with complex addressing */
        output[(idx1 + idx2 + idx3 + idx4) % size] = d15 + (double)i12 + (double)f14;
        
        /* Update volatile index */
        volatile_idx = (volatile_idx + 1) % 10;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                          d11 + d12 + d13 + d14 + d15 + (double)f1 + (double)f2 +
                          (double)f3 + (double)f4 + (double)f5 + (double)f6 +
                          (double)f7 + (double)f8 + (double)f9 + (double)f10 +
                          (double)f11 + (double)f12 + (double)f13 + (double)f14 +
                          (double)i1 + (double)i2 + (double)i3 + (double)i4 +
                          (double)i5 + (double)i6 + (double)i7 + (double)i8 +
                          (double)i9 + (double)i10 + (double)i11 + (double)i12;
    (void)sink; /* Suppress unused warning */
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
        input[i] = sin((double)i * 0.1) * 100.0;
        output[i] = 0.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ARRAY_SIZE, 16);
    
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
