#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to force argument passing complexity */
__attribute__((noinline))
double many_args_function(
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
    
    /* Use volatile globals */
    sum += global_volatile_int + global_volatile_double + global_volatile_float;
    
    return sum;
}

/* Structure for complex memory accesses */
struct ComplexData {
    int ints[16];
    double doubles[8];
    float floats[12];
    long longs[4];
};

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(struct ComplexData* input, struct ComplexData* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile int32_t i32_1, i32_2, i32_3, i32_4, i32_5;
    volatile int64_t i64_1, i64_2, i64_3, i64_4, i64_5;
    
    /* Floating point variables */
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f11, f12, f13, f14, f15;
    volatile double d11, d12, d13, d14, d15;
    
    /* Pointer/index variables */
    volatile int idx1, idx2, idx3, idx4;
    volatile int stride1, stride2;
    volatile int* ptr1;
    volatile double* ptr2;
    volatile float* ptr3;
    
    /* Initialize some values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    f11 = 11.11f; f12 = 12.12f; f13 = 13.13f; f14 = 14.14f; f15 = 15.15f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    
    stride1 = 8;
    stride2 = 4;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % 16;
        idx2 = (iter * 3) % 8;
        idx3 = (iter * 5) % 12;
        idx4 = (iter * 2) % 4;
        
        /* Load from input with complex addressing */
        v1 = input->ints[idx1 * stride1 + idx2 + iter];
        v2 = input->ints[idx2 * stride2 + idx3 + iter + 1];
        v3 = input->ints[idx3 * 3 + idx4 + iter * 2];
        v4 = input->ints[idx4 * 5 + idx1 + iter * 3];
        
        l1 = input->longs[idx1 % 4] + iter;
        l2 = input->longs[idx2 % 4] + iter * 2;
        l3 = input->longs[idx3 % 4] + iter * 3;
        l4 = input->longs[idx4 % 4] + iter * 4;
        
        f1 = input->floats[idx1 * 2 + idx2];
        f2 = input->floats[idx2 * 3 + idx3];
        f3 = input->floats[idx3 * 4 + idx4];
        f4 = input->floats[idx4 * 5 + idx1];
        
        d1 = input->doubles[idx1 + idx2];
        d2 = input->doubles[idx2 + idx3];
        d3 = input->doubles[idx3 + idx4];
        d4 = input->doubles[idx4 + idx1];
        
        /* LONG chain of mixed-type computations with data dependencies */
        /* This prevents register reuse */
        f5 = (float)v1 * f1 + (float)v2 * f2;
        d5 = (double)l1 * d1 + (double)l2 * d2;
        v5 = (int)(f5 * 100.0f) + (int)(d5 * 10.0);
        l5 = (long)(v5 * 3) + (long)(f5 * 2.0f);
        
        f6 = f3 * (float)v3 + f4 * (float)v4;
        d6 = d3 * (double)l3 + d4 * (double)l4;
        v6 = (int)(f6 * 50.0f) - (int)(d6 * 5.0);
        l6 = (long)(v6 * 2) - (long)(f6 * 3.0f);
        
        f7 = (float)(v5 + v6) * 0.5f;
        d7 = (double)(l5 + l6) * 0.25;
        v7 = (int)(f7 * d7 * 100.0);
        l7 = (long)(f7 / d7 * 1000.0);
        
        f8 = f5 + f6 - f7;
        d8 = d5 + d6 - d7;
        v8 = v5 + v6 - v7;
        l8 = l5 + l6 - l7;
        
        f9 = f1 * f2 * f3 * f4;
        d9 = d1 * d2 * d3 * d4;
        v9 = v1 * v2 * v3 * v4;
        l9 = l1 * l2 * l3 * l4;
        
        f10 = (float)(v8 + v9) / (f8 + f9);
        d10 = (double)(l8 + l9) / (d8 + d9);
        v10 = (int)(f10 * d10 * 1000.0f);
        l10 = (long)(f10 / d10 * 10000.0);
        
        f11 = sqrtf(fabsf(f10));
        d11 = sqrt(fabs(d10));
        i32_1 = (int32_t)(f11 * 100.0f);
        i64_1 = (int64_t)(d11 * 1000.0);
        
        f12 = sinf(f11) + cosf(f10);
        d12 = sin(d11) + cos(d10);
        i32_2 = (int32_t)(f12 * 100.0f);
        i64_2 = (int64_t)(d12 * 1000.0);
        
        f13 = expf(f12 * 0.1f);
        d13 = exp(d12 * 0.1);
        i32_3 = (int32_t)(f13 * 100.0f);
        i64_3 = (int64_t)(d13 * 1000.0);
        
        f14 = logf(fabsf(f13) + 1.0f);
        d14 = log(fabs(d13) + 1.0);
        i32_4 = (int32_t)(f14 * 100.0f);
        i64_4 = (int64_t)(d14 * 1000.0);
        
        f15 = f14 * 2.0f - f13;
        d15 = d14 * 2.0 - d13;
        i32_5 = i32_4 * 2 - i32_3;
        i64_5 = i64_4 * 2 - i64_3;
        
        /* Inline assembly that clobbers MANY registers */
        /* This forces spills and reloads */
#if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile (
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
            "pxor %%xmm0, %%xmm0\n"
            "pxor %%xmm1, %%xmm1\n"
            "pxor %%xmm2, %%xmm2\n"
            "pxor %%xmm3, %%xmm3\n"
            "pxor %%xmm4, %%xmm4\n"
            "pxor %%xmm5, %%xmm5\n"
            "pxor %%xmm6, %%xmm6\n"
            "pxor %%xmm7, %%xmm7\n"
            "pxor %%xmm8, %%xmm8\n"
            "pxor %%xmm9, %%xmm9\n"
            "pxor %%xmm10, %%xmm10\n"
            "pxor %%xmm11, %%xmm11\n"
            "pxor %%xmm12, %%xmm12\n"
            "pxor %%xmm13, %%xmm13\n"
            "pxor %%xmm14, %%xmm14\n"
            "pxor %%xmm15, %%xmm15\n"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
#elif defined(__aarch64__)
        __asm__ volatile (
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
            "mov x29, #0\n"
            "mov x30, #0\n"
            "fmov d0, #0.0\n"
            "fmov d1, #0.0\n"
            "fmov d2, #0.0\n"
            "fmov d3, #0.0\n"
            "fmov d4, #0.0\n"
            "fmov d5, #0.0\n"
            "fmov d6, #0.0\n"
            "fmov d7, #0.0\n"
            "fmov d8, #0.0\n"
            "fmov d9, #0.0\n"
            "fmov d10, #0.0\n"
            "fmov d11, #0.0\n"
            "fmov d12, #0.0\n"
            "fmov d13, #0.0\n"
            "fmov d14, #0.0\n"
            "fmov d15, #0.0\n"
            "fmov d16, #0.0\n"
            "fmov d17, #0.0\n"
            "fmov d18, #0.0\n"
            "fmov d19, #0.0\n"
            "fmov d20, #0.0\n"
            "fmov d21, #0.0\n"
            "fmov d22, #0.0\n"
            "fmov d23, #0.0\n"
            "fmov d24, #0.0\n"
            "fmov d25, #0.0\n"
            "fmov d26, #0.0\n"
            "fmov d27, #0.0\n"
            "fmov d28, #0.0\n"
            "fmov d29, #0.0\n"
            "fmov d30, #0.0\n"
            "fmov d31, #0.0\n"
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces complex argument passing */
        double result = many_args_function(
            v1, d1, f1, l1,
            v2, d2, f2, l2,
            v3, d3, f3, l3,
            v4, d4, f4, l4,
            v5, d5, f5, l5,
            v6, d6, f6, l6
        );
        
        /* More mixed computations using the function result */
        f15 += (float)result * 0.1f;
        d15 += result * 0.01;
        v10 += (int)(result * 100.0);
        l10 += (long)(result * 1000.0);
        
        /* Store results with complex addressing */
        output->ints[idx1 * stride1 + idx2 + iter] = v10;
        output->ints[idx2 * stride2 + idx3 + iter + 1] = i32_5;
        output->ints[idx3 * 3 + idx4 + iter * 2] = i32_4;
        output->ints[idx4 * 5 + idx1 + iter * 3] = i32_3;
        
        output->longs[idx1 % 4] = l10 + iter;
        output->longs[idx2 % 4] = i64_5 + iter * 2;
        output->longs[idx3 % 4] = i64_4 + iter * 3;
        output->longs[idx4 % 4] = i64_3 + iter * 4;
        
        output->floats[idx1 * 2 + idx2] = f15;
        output->floats[idx2 * 3 + idx3] = f14;
        output->floats[idx3 * 4 + idx4] = f13;
        output->floats[idx4 * 5 + idx1] = f12;
        
        output->doubles[idx1 + idx2] = d15;
        output->doubles[idx2 + idx3] = d14;
        output->doubles[idx3 + idx4] = d13;
        output->doubles[idx4 + idx1] = d12;
        
        /* Use volatile globals in computation */
        v1 += global_volatile_int;
        d1 += global_volatile_double;
        f1 += global_volatile_float;
    }
    
    /* Volatile sink to prevent elimination */
    volatile double sink = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                          d11 + d12 + d13 + d14 + d15;
    (void)sink;
}

int main() {
    /* Allocate and initialize data */
    struct ComplexData* input = (struct ComplexData*)malloc(sizeof(struct ComplexData));
    struct ComplexData* output = (struct ComplexData*)malloc(sizeof(struct ComplexData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < 16; i++) {
        input->ints[i] = rand() % 1000;
    }
    for (int i = 0; i < 8; i++) {
        input->doubles[i] = (double)rand() / RAND_MAX * 100.0;
    }
    for (int i = 0; i < 12; i++) {
        input->floats[i] = (float)rand() / RAND_MAX * 50.0f;
    }
    for (int i = 0; i < 4; i++) {
        input->longs[i] = (long)rand() * 1000L;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += output->ints[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (long)output->doubles[i];
    }
    for (int i = 0; i < 12; i++) {
        checksum += (long)output->floats[i];
    }
    for (int i = 0; i < 4; i++) {
        checksum += output->longs[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
