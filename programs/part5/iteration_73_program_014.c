/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for memory addressing */
struct DataBlock {
    int ints[32];
    double doubles[32];
    float floats[32];
    long longs[32];
};

/* Argument-heavy helper function */
__attribute__((noinline))
static double heavy_args_func(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex mixing of types */
    double sum = (double)a1 * b1 + (double)c1 * (double)d1;
    sum += (double)a2 / b2 - (double)c2 * (double)d2;
    sum += (double)a3 + b3 + (double)c3 - (double)d3;
    sum += (double)a4 * b4 * (double)c4 / (double)d4;
    sum += (double)a5 - b5 + (double)c5 * (double)d5;
    sum += (double)a6 + b6 - (double)c6 / (double)d6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum += (double)global_volatile_float;
    sum += (double)global_volatile_int;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static double compute_heavy(struct DataBlock* input, struct DataBlock* output, 
                           int stride, int iterations) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int idx1, idx2, idx3, idx4, idx5;
    volatile double accumulator = 0.0;
    
    /* Initialize locals with complex expressions */
    v1 = global_volatile_int;
    v2 = v1 * 2;
    v3 = v2 + 17;
    v4 = v3 - v1;
    v5 = v4 * 3;
    v6 = v5 / 2;
    v7 = v6 + v2;
    v8 = v7 - v3;
    v9 = v8 * v4;
    v10 = v9 / v5;
    
    l1 = (long)v1 * 1000L;
    l2 = l1 + (long)v2;
    l3 = l2 - (long)v3;
    l4 = l3 * (long)v4;
    l5 = l4 / (long)v5;
    l6 = l5 + l1;
    l7 = l6 - l2;
    l8 = l7 * l3;
    l9 = l8 / l4;
    l10 = l9 + l5;
    
    f1 = (float)v1 * 1.1f;
    f2 = f1 + (float)v2 * 2.2f;
    f3 = f2 - (float)v3 * 3.3f;
    f4 = f3 * (float)v4 * 4.4f;
    f5 = f4 / (float)v5 * 5.5f;
    f6 = f5 + f1;
    f7 = f6 - f2;
    f8 = f7 * f3;
    f9 = f8 / f4;
    f10 = f9 + f5;
    
    d1 = (double)l1 * 1.111;
    d2 = d1 + (double)l2 * 2.222;
    d3 = d2 - (double)l3 * 3.333;
    d4 = d3 * (double)l4 * 4.444;
    d5 = d4 / (double)l5 * 5.555;
    d6 = d5 + d1;
    d7 = d6 - d2;
    d8 = d7 * d3;
    d9 = d8 / d4;
    d10 = d9 + d5;
    
    idx1 = 0;
    idx2 = stride;
    idx3 = stride * 2;
    idx4 = stride * 3;
    idx5 = stride * 4;
    
    /* Main computation loop with complex addressing */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        int base_idx = (iter * 7) % 16;
        int offset1 = (base_idx + idx1) % 32;
        int offset2 = (base_idx + idx2 + iter) % 32;
        int offset3 = (base_idx + idx3 + iter * 2) % 32;
        int offset4 = (base_idx + idx4 + iter * 3) % 32;
        int offset5 = (base_idx + idx5 + iter * 4) % 32;
        
        /* Load data with complex addressing */
        v1 = input->ints[offset1];
        v2 = input->ints[offset2];
        v3 = input->ints[offset3];
        v4 = input->ints[offset4];
        v5 = input->ints[offset5];
        
        l1 = input->longs[offset1];
        l2 = input->longs[offset2];
        l3 = input->longs[offset3];
        l4 = input->longs[offset4];
        l5 = input->longs[offset5];
        
        f1 = input->floats[offset1];
        f2 = input->floats[offset2];
        f3 = input->floats[offset3];
        f4 = input->floats[offset4];
        f5 = input->floats[offset5];
        
        d1 = input->doubles[offset1];
        d2 = input->doubles[offset2];
        d3 = input->doubles[offset3];
        d4 = input->doubles[offset4];
        d5 = input->doubles[offset5];
        
        /* Long chain of mixed-type computations */
        f6 = (float)v1 * (float)d1 + f1;
        d6 = (double)v2 * d2 + (double)f2;
        l6 = (long)v3 * l3 + (long)f3;
        
        f7 = f6 * 2.0f - (float)l4 / 1000.0f;
        d7 = d6 / 3.0 + (double)v4 * 0.01;
        l7 = l6 + (long)(d4 * 1000.0);
        
        f8 = (float)d5 * f5 + (float)l5 * 0.001f;
        d8 = (double)f4 * d4 + (double)v5 * 0.0001;
        l8 = l4 + (long)(f3 * 100.0f);
        
        f9 = f7 + f8 - f2 * f3;
        d9 = d7 * d8 / d2 + d3;
        l9 = l7 - l8 + l2 * l3;
        
        f10 = (float)l9 * 0.01f + f9 * f4;
        d10 = (double)l9 * 0.001 + d9 * d5;
        
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
              "memory", "cc"
        );
#elif defined(__aarch64__)
        /* For ARM64 */
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
#endif
        
        /* Call argument-heavy function with mixed types */
        double func_result = heavy_args_func(
            v1, d1, f1, l1,
            v2, d2, f2, l2,
            v3, d3, f3, l3,
            v4, d4, f4, l4,
            v5, d5, f5, l5,
            iter, d10, f10, l10
        );
        
        /* More mixed-type computations */
        f1 = (float)func_result + f6 * 0.5f;
        d1 = func_result * 2.0 + d6;
        l1 = (long)(func_result * 1000.0) + l6;
        
        v1 = (int)func_result + v2 + v3;
        v2 = v1 * 2 - v4;
        v3 = v2 + v5 - iter;
        
        /* Store results with complex addressing */
        output->ints[offset1] = v1;
        output->ints[offset2] = v2;
        output->ints[offset3] = v3;
        output->ints[offset4] = v4;
        output->ints[offset5] = v5;
        
        output->longs[offset1] = l1;
        output->longs[offset2] = l2;
        output->longs[offset3] = l3;
        output->longs[offset4] = l4;
        output->longs[offset5] = l5;
        
        output->floats[offset1] = f1;
        output->floats[offset2] = f2;
        output->floats[offset3] = f3;
        output->floats[offset4] = f4;
        output->floats[offset5] = f5;
        
        output->doubles[offset1] = d1;
        output->doubles[offset2] = d2;
        output->doubles[offset3] = d3;
        output->doubles[offset4] = d4;
        output->doubles[offset5] = d5;
        
        /* Update accumulator with volatile access */
        accumulator += d1 + d2 + d3 + d4 + d5;
        accumulator += (double)f1 + (double)f2 + (double)f3 + (double)f4 + (double)f5;
        accumulator += (double)v1 + (double)v2 + (double)v3 + (double)v4 + (double)v5;
        accumulator += (double)l1 + (double)l2 + (double)l3 + (double)l4 + (double)l5;
        
        /* Force memory access to volatile globals */
        accumulator += global_volatile_double;
        accumulator += (double)global_volatile_float;
        accumulator += (double)global_volatile_int;
        
        /* Update indices with complex expressions */
        idx1 = (idx1 + iter * 3) % 32;
        idx2 = (idx2 + iter * 5) % 32;
        idx3 = (idx3 + iter * 7) % 32;
        idx4 = (idx4 + iter * 11) % 32;
        idx5 = (idx5 + iter * 13) % 32;
    }
    
    return accumulator;
}

int main(void) {
    struct DataBlock input_data;
    struct DataBlock output_data;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < 32; i++) {
        input_data.ints[i] = rand() % 1000;
        input_data.longs[i] = (long)rand() * rand();
        input_data.floats[i] = (float)rand() / (float)RAND_MAX * 100.0f;
        input_data.doubles[i] = (double)rand() / (double)RAND_MAX * 1000.0;
    }
    
    /* Clear output */
    for (int i = 0; i < 32; i++) {
        output_data.ints[i] = 0;
        output_data.longs[i] = 0;
        output_data.floats[i] = 0.0f;
        output_data.doubles[i] = 0.0;
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(&input_data, &output_data, 5, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < 32; i++) {
        checksum += output_data.ints[i];
        checksum += output_data.longs[i];
        checksum += output_data.floats[i];
        checksum += output_data.doubles[i];
    }
    
    checksum += result;
    
    printf("Result: %f\n", result);
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
