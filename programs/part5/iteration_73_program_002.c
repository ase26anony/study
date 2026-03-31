#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile arrays to force memory operations */
volatile int global_int_array[ARRAY_SIZE];
volatile double global_double_array[ARRAY_SIZE];
volatile float global_float_array[ARRAY_SIZE];

/* Helper function with many arguments to force stack passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4, int a5, int a6,
    float f1, float f2, float f3, float f4, float f5, float f6,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2 * d3;
    result += (double)l1 / (double)l2;
    result += (double)((intptr_t)p1 % 1000) * 0.001;
    result += (double)((intptr_t)p2 % 1000) * 0.001;
    
    /* More mixing */
    result = sin(result) * cos((double)f3);
    result += tanh((double)a3 / 100.0);
    
    return result;
}

/* Critical function with extreme register pressure */
__attribute__((noinline))
static double compute_heavy(volatile int* input_int, 
                           volatile double* input_double,
                           volatile float* input_float,
                           int stride)
{
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Pointer variables */
    volatile int* p1;
    volatile double* p2;
    volatile float* p3;
    
    /* Index variables for complex addressing */
    int idx1, idx2, idx3, idx4, idx5;
    
    /* Initialize some values */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    d16 = 16.16; d17 = 17.17; d18 = 18.18; d19 = 19.19; d20 = 20.20;
    
    p1 = input_int;
    p2 = input_double;
    p3 = input_float;
    
    /* Complex loop with data dependencies */
    double accumulator = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = iter * stride;
        idx2 = (iter + 1) * stride;
        idx3 = (iter * 2) % ARRAY_SIZE;
        idx4 = (iter * 3) % ARRAY_SIZE;
        idx5 = (iter * 5) % ARRAY_SIZE;
        
        /* Load from volatile arrays - forces memory operations */
        i1 = p1[idx1];
        i2 = p1[idx2];
        i3 = p1[idx3];
        i4 = p1[idx4];
        i5 = p1[idx5];
        
        d1 = p2[idx1];
        d2 = p2[idx2];
        d3 = p2[idx3];
        d4 = p2[idx4];
        d5 = p2[idx5];
        
        f1 = p3[idx1];
        f2 = p3[idx2];
        f3 = p3[idx3];
        f4 = p3[idx4];
        f5 = p3[idx5];
        
        /* Long chain of mixed-type computations with dependencies */
        /* Each result depends on previous to prevent register reuse */
        
        /* Integer to float conversions */
        f6 = (float)i1 * f1 + (float)i2 * f2;
        f7 = (float)i3 * f3 + (float)i4 * f4;
        f8 = (float)i5 * f5 + f6 * 0.5f;
        
        /* Float to double conversions */
        d6 = (double)f6 * d1 + (double)f7 * d2;
        d7 = (double)f8 * d3 + d6 * 0.25;
        
        /* Double to integer conversions */
        i6 = (int)(d6 * 100.0) + i1;
        i7 = (int)(d7 * 100.0) + i2;
        
        /* More mixed operations */
        d8 = (double)i6 * d4 + (double)i7 * d5;
        d9 = sin(d8) * cos(d6);
        
        /* Integer arithmetic */
        l1 = (long)i6 * i7 + l2;
        l2 = (long)i8 * i9 + l3;
        l3 = (long)i10 * i1 + l4;
        
        /* Float operations */
        f9 = f6 * f7 - f8 * 0.3f;
        f10 = f9 * 2.0f + f1 * f2;
        
        /* Double operations */
        d10 = d8 * d9 - d6 * d7;
        d11 = d10 * 1.5 + d1 * d2;
        d12 = d11 * 0.8 + d3 * d4;
        d13 = d12 * 1.2 + d5 * d6;
        d14 = d13 * 0.9 + d7 * d8;
        d15 = d14 * 1.1 + d9 * d10;
        
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
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("# Dummy assembly" ::: "memory");
#endif
        
        /* Call function with many arguments - forces argument passing */
        d16 = many_args_function(
            i1, i2, i3, i4, i5, i6,
            f1, f2, f3, f4, f5, f6,
            d1, d2, d3, d4,
            l1, l2, (void*)&i1, (void*)&i2
        );
        
        /* More computations after the call */
        d17 = d16 * d15 + d14 * 0.3;
        d18 = sin(d17) * cos(d16);
        d19 = d18 * 2.0 + d17 * 0.5;
        d20 = d19 * 1.1 + d18 * 0.9;
        
        /* Update accumulator with complex expression */
        accumulator += d20 + (double)f10 + (double)i10 + (double)l10;
        
        /* Store results back to volatile arrays */
        p1[idx1] = i6;
        p1[idx2] = i7;
        p2[idx3] = d15;
        p2[idx4] = d16;
        p3[idx5] = f10;
        
        /* Create data dependencies for next iteration */
        i1 = i6;
        i2 = i7;
        f1 = f10;
        d1 = d20;
        l1 = (long)(d20 * 1000.0);
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = accumulator;
    (void)sink;
    
    return accumulator;
}

int main(void)
{
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    /* Call the compute-heavy function */
    double result = compute_heavy(
        global_int_array,
        global_double_array,
        global_float_array,
        4  /* stride */
    );
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %.15f\n", result);
    
    return 0;
}
