/* reload_stress.c - Designed to trigger GCC's reload pass uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper with many arguments to stress calling convention */
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
    double sum = 0.0;
    sum += (double)a1 * b1 + (double)c1;
    sum += (double)a2 / b2 - (double)c2;
    sum += (double)a3 + b3 * (double)c3;
    sum += (double)a4 - b4 / (double)c4;
    sum += (double)d1 * 0.01 + (double)d2 * 0.02;
    sum += (double)d3 * 0.03 - (double)d4 * 0.04;
    sum += (double)d5 * b5 + (double)d6 / b6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum += (double)global_volatile_float;
    
    return sum;
}

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static void compute_heavy(const int* input_int, const double* input_double,
                         const float* input_float, long* output,
                         int stride, int offset)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15;
    
    /* Pointer/Index variables */
    int idx1, idx2, idx3, idx4;
    volatile int vol_idx; /* Volatile index to prevent optimization */
    
    /* Initialize some values */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    l1 = 1000L; l2 = 2000L; l3 = 3000L;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f;
    d1 = 1.01; d2 = 2.02; d3 = 3.03;
    
    vol_idx = offset;
    
    /* Main computation loop with complex data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = iter * stride + offset;
        idx2 = (iter + 1) * stride - offset;
        idx3 = iter * (stride / 2) + offset * 2;
        idx4 = (iter * 3) % stride + offset;
        
        /* Load from arrays with complex addressing */
        i6 = input_int[idx1] + global_volatile_int;
        i7 = input_int[idx2] - global_volatile_int;
        i8 = input_int[idx3] * 2;
        i9 = input_int[idx4] / 3;
        
        d4 = input_double[idx1] + global_volatile_double;
        d5 = input_double[idx2] - global_volatile_double;
        d6 = input_double[idx3] * 2.0;
        d7 = input_double[idx4] / 3.0;
        
        f4 = input_float[idx1] + global_volatile_float;
        f5 = input_float[idx2] - global_volatile_float;
        f6 = input_float[idx3] * 2.0f;
        f7 = input_float[idx4] / 3.0f;
        
        /* Long chain of mixed-type computations */
        /* Creates demand for both integer and FP registers */
        f8 = (float)i6 * (float)d4 + f4;
        d8 = (double)i7 * d5 + (double)f5;
        i10 = (int)f8 + (int)d8 + i8;
        
        f9 = f6 * (float)i9 + (float)d6;
        d9 = d7 * (double)i10 + (double)f7;
        i11 = (int)f9 * (int)d9 + i1;
        
        /* More mixed computations */
        l4 = (long)(f8 * 100.0f) + (long)(d8 * 100.0);
        l5 = (long)(f9 * 200.0f) - (long)(d9 * 200.0);
        
        f10 = (float)l4 * 0.01f + (float)l5 * 0.02f;
        d10 = (double)l4 * 0.001 + (double)l5 * 0.002;
        
        i12 = (int)(f10 * 10.0f) + (int)(d10 * 20.0);
        i13 = i12 * i11 - i10;
        
        /* Continue the dependency chain */
        f11 = (float)i13 / 7.0f + f10;
        d11 = (double)i12 / 11.0 + d10;
        
        l6 = (long)(f11 * 1000.0f) + (long)(d11 * 1000.0);
        l7 = l6 * l4 - l5;
        
        f12 = (float)l6 * 0.0001f + (float)l7 * 0.0002f;
        d12 = (double)l6 * 0.00001 + (double)l7 * 0.00002;
        
        i14 = (int)(f12 * 10000.0f) + (int)(d12 * 20000.0);
        i15 = i14 + i13 * 3;
        
        /* Even more variables to increase pressure */
        f13 = f11 * f12 - f10;
        d13 = d11 * d12 + d10;
        
        l8 = (long)(f13 * 50.0f) * (long)(d13 * 60.0);
        l9 = l8 / (l7 + 1);
        
        f14 = (float)l8 * 0.5f + (float)l9 * 0.25f;
        d14 = (double)l8 * 0.05 + (double)l9 * 0.025;
        
        i16 = (int)f14 * (int)d14 + i15;
        i17 = i16 - i14 + i13;
        
        f15 = (float)i16 * (float)i17 + f14;
        d15 = (double)i16 * (double)i17 - d14;
        
        l10 = (long)(f15 * 100.0f) + (long)(d15 * 100.0);
        
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
              "memory", "cc"
        );
#elif defined(__aarch64__)
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
              "x24", "x25", "x26", "x27", "x28",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory", "cc"
        );
#endif
        
        /* Call function with many arguments - forces register/stack spills */
        double func_result = many_args_function(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            i5, d5, f5, l5,
            i6, d6, f6, l6
        );
        
        /* Use the result in further computations */
        d15 += func_result * 0.5;
        f15 += (float)func_result * 0.25f;
        i17 += (int)func_result;
        
        /* Complex store with indexing */
        int store_idx = (idx1 + idx2 + idx3 + idx4) % ARRAY_SIZE;
        output[store_idx] = l10 + (long)(d15 * 1000.0) + (long)(f15 * 100.0f) + i17;
        
        /* Update variables to maintain dependencies across iterations */
        i1 = i17 % 100;
        d1 = d15 * 0.9;
        f1 = f15 * 0.8f;
        l1 = l10 / 2;
        
        /* More updates to keep variables live */
        i2 = i16;
        d2 = d14;
        f2 = f14;
        l2 = l9;
        
        i3 = i15;
        d3 = d13;
        f3 = f13;
        l3 = l8;
    }
    
    /* Volatile sink to prevent elimination */
    volatile long sink = l10;
    (void)sink;
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int* input_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* input_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* input_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* output = (long*)calloc(ARRAY_SIZE, sizeof(long));
    
    if (!input_int || !input_double || !input_float || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_int[i] = rand() % 1000;
        input_double[i] = (double)(rand() % 10000) / 100.0;
        input_float[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    /* Perform heavy computation */
    compute_heavy(input_int, input_double, input_float, output, 16, 4);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum ^= (checksum << 13);
        checksum ^= (checksum >> 17);
        checksum ^= (checksum << 5);
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(input_int);
    free(input_double);
    free(input_float);
    free(output);
    
    return 0;
}
