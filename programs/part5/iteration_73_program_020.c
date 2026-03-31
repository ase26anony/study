#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile global variables to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to force stack passing */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6)
{
    /* Complex computation mixing all types */
    double sum = (double)a1 + b1 + (double)c1 + (double)d1;
    sum += (double)a2 + b2 + (double)c2 + (double)d2;
    sum += (double)a3 + b3 + (double)c3 + (double)d3;
    sum += (double)a4 + b4 + (double)c4 + (double)d4;
    sum += (double)a5 + b5 + (double)c5 + (double)d5;
    sum += (double)a6 + b6 + (double)c6 + (double)d6;
    
    /* Use volatile globals */
    sum += global_volatile_double + (double)global_volatile_float;
    
    return sum * 0.5;
}

/* Structure for complex memory access patterns */
struct DataPoint {
    float f;
    double d;
    int i;
    long l;
    volatile int vi;
};

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(struct DataPoint* input, struct DataPoint* output, 
                           int size, int stride, volatile double* sink)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    int i11 = 110, i12 = 120, i13 = 130, i14 = 140, i15 = 150;
    
    /* Long variables */
    long l1 = 1000L, l2 = 2000L, l3 = 3000L, l4 = 4000L, l5 = 5000L;
    long l6 = 6000L, l7 = 7000L, l8 = 8000L, l9 = 9000L, l10 = 10000L;
    
    /* Float variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    
    /* Double variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    double d11 = 11.11, d12 = 12.12, d13 = 13.13, d14 = 14.14, d15 = 15.15;
    
    /* Pointer variables */
    int* p1 = &i1;
    float* p2 = &f1;
    double* p3 = &d1;
    volatile int* p4 = &v1;
    
    /* Index variables for complex array access */
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3, idx5 = 4;
    int idx6 = 5, idx7 = 6, idx8 = 7, idx9 = 8, idx10 = 9;
    
    double total = 0.0;
    
    /* Main computation loop with extreme register pressure */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        int array_idx = (iter * stride + idx1 * idx2 + idx3 - idx4) % size;
        int array_idx2 = (iter * 2 * stride + idx5 * idx6 + idx7 - idx8) % size;
        int array_idx3 = (iter * 3 * stride + idx9 * idx10 + idx1 - idx2) % size;
        
        /* Load from input with complex addressing */
        f1 = input[array_idx].f + (float)iter;
        d1 = input[array_idx].d * (double)iter;
        i1 = input[array_idx].i ^ iter;
        l1 = input[array_idx].l + iter;
        
        f2 = input[array_idx2].f * 2.0f;
        d2 = input[array_idx2].d / 2.0;
        i2 = input[array_idx2].i << 1;
        l2 = input[array_idx2].l * 2;
        
        /* Long chain of mixed-type computations */
        /* This creates dependencies preventing register reuse */
        f3 = (float)i1 * f1 + f2;
        d3 = (double)f3 * d1 - d2;
        i3 = (int)d3 + i2 * i1;
        l3 = (long)f3 * l1 + l2;
        
        f4 = f3 * 1.5f + (float)d3;
        d4 = d3 * 0.75 + (double)f4;
        i4 = i3 / 2 + (int)f4;
        l4 = l3 >> 1 + (long)d4;
        
        f5 = sqrtf(fabsf(f4)) + (float)i4;
        d5 = sqrt(fabs(d4)) + (double)l4;
        i5 = (int)(sin(d5) * 100.0) + i4;
        l5 = (long)(cos(d5) * 1000.0) + l4;
        
        f6 = (float)l5 * 0.01f + f5;
        d6 = (double)i5 * 0.001 + d5;
        i6 = (int)(f6 * 10.0f) + i5;
        l6 = (long)(d6 * 100.0) + l5;
        
        f7 = f6 * f5 - f4;
        d7 = d6 * d5 - d4;
        i7 = i6 * i5 - i4;
        l7 = l6 * l5 - l4;
        
        f8 = (float)(i7 % 100) + f7;
        d8 = (double)(l7 % 1000) + d7;
        i8 = (int)(f8 * 100.0f) ^ i7;
        l8 = (long)(d8 * 1000.0) | l7;
        
        f9 = f8 / (f7 + 1.0f);
        d9 = d8 / (d7 + 1.0);
        i9 = i8 / (i7 + 1);
        l9 = l8 / (l7 + 1);
        
        f10 = (float)(i9 & 0xFF) * f9;
        d10 = (double)(l9 & 0xFFFF) * d9;
        i10 = (int)(f10 * 2.0f) | i9;
        l10 = (long)(d10 * 2.0) & l9;
        
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
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
#elif defined(__aarch64__) || defined(_M_ARM64)
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
            : /* no outputs */
            : /* no inputs */
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
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("" : : : "memory");
#endif
        
        /* Continue computation after assembly clobber */
        f11 = f10 * 3.14159f + (float)global_volatile_int;
        d11 = d10 * 2.71828 + global_volatile_double;
        i11 = i10 + global_volatile_int;
        l11 = l10 + (long)global_volatile_double;
        
        f12 = (float)(i11 | 0xAA) * f11;
        d12 = (double)(l11 | 0x5555) * d11;
        i12 = (int)(f12 * 1.234f) & i11;
        l12 = (long)(d12 * 5.678) | l11;
        
        /* Call function with many arguments - forces register/stack spills */
        double func_result = many_args_function(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            i5, d5, f5, l5,
            i6, d6, f6, l6
        );
        
        /* Use function result in further computation */
        f13 = f12 * (float)func_result;
        d13 = d12 * func_result;
        i13 = i12 + (int)func_result;
        l13 = l12 + (long)func_result;
        
        /* Final computation mixing all accumulated values */
        double final_d = d13 + (double)f13 + (double)i13 + (double)l13;
        total += final_d;
        
        /* Store to output with complex addressing */
        int out_idx = (array_idx3 + iter) % size;
        output[out_idx].f = f13;
        output[out_idx].d = d13;
        output[out_idx].i = i13;
        output[out_idx].l = l13;
        output[out_idx].vi = v1 + v2 + v3 + v4 + v5;  /* Use volatile */
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 10;
        idx2 = (idx2 * 2) % 10;
        idx3 = (idx3 + 3) % 10;
        idx4 = (idx4 * 4) % 10;
        idx5 = (idx5 + 5) % 10;
        idx6 = (idx6 * 6) % 10;
        idx7 = (idx7 + 7) % 10;
        idx8 = (idx8 * 8) % 10;
        idx9 = (idx9 + 9) % 10;
        idx10 = (idx10 * 10) % 10;
    }
    
    /* Store final result to volatile sink */
    *sink = total;
    
    return total;
}

int main(void) {
    /* Allocate and initialize arrays */
    struct DataPoint* input = (struct DataPoint*)malloc(ARRAY_SIZE * sizeof(struct DataPoint));
    struct DataPoint* output = (struct DataPoint*)malloc(ARRAY_SIZE * sizeof(struct DataPoint));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].f = (float)(i * 1.234f);
        input[i].d = (double)(i * 2.345);
        input[i].i = i * 3;
        input[i].l = i * 4L;
        input[i].vi = i * 5;
        
        output[i].f = 0.0f;
        output[i].d = 0.0;
        output[i].i = 0;
        output[i].l = 0L;
        output[i].vi = 0;
    }
    
    volatile double sink = 0.0;
    
    /* Perform heavy computation */
    double result = compute_heavy(input, output, ARRAY_SIZE, 7, &sink);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].f + output[i].d + (double)output[i].i + (double)output[i].l;
    }
    
    printf("Result: %f\n", result);
    printf("Checksum: %f\n", checksum);
    printf("Sink value: %f\n", (double)sink);
    
    free(input);
    free(output);
    
    return 0;
}
