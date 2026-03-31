/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int volatile_global_int = 42;
volatile double volatile_global_double = 3.14159;
volatile float volatile_global_float = 2.71828f;

/* Helper function with many arguments - forces register/stack pressure */
__attribute__((noinline))
static double many_args_func(
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
    sum += volatile_global_double;
    sum += (double)volatile_global_float;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(const int* input_int, const double* input_double,
                           const float* input_float, long* output,
                           int stride, int offset)
{
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Additional non-volatile variables for computation */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    float g1, g2, g3, g4, g5, g6, g7, g8, g9, g10;
    double h1, h2, h3, h4, h5, h6, h7, h8, h9, h10;
    
    /* Pointer variables */
    const int* p1 = input_int;
    const double* p2 = input_double;
    const float* p3 = input_float;
    long* p4 = output;
    
    double total_sum = 0.0;
    
    /* Complex loop with data dependencies */
    for (int idx = 0; idx < ITERATIONS; idx++) {
        /* Complex array indexing with multiple terms */
        int index1 = (idx * stride + offset) % ARRAY_SIZE;
        int index2 = (idx * (stride + 1) + offset * 2) % ARRAY_SIZE;
        int index3 = (idx * (stride - 1) + offset / 2) % ARRAY_SIZE;
        
        /* Load data with complex addressing */
        i1 = input_int[index1];
        i2 = input_int[index2];
        i3 = input_int[index3];
        
        d1 = input_double[index1];
        d2 = input_double[index2];
        d3 = input_double[index3];
        
        g1 = input_float[index1];
        g2 = input_float[index2];
        g3 = input_float[index3];
        
        /* Long chain of mixed-type computations */
        /* Each computation depends on previous results */
        h1 = (double)i1 * d1 + (double)g1;
        f1 = (float)h1 * g2 + (float)i2;
        
        i4 = (int)f1 + i3 * 2;
        d4 = d2 * (double)i4 - h1;
        
        g4 = g3 * (float)d4 + (float)i1;
        h2 = (double)g4 * d3 + h1;
        
        i5 = i2 + (int)h2 - i4;
        f2 = (float)i5 * g1 + f1;
        
        d5 = d1 * h2 + d4;
        g5 = (float)d5 * g2 + g4;
        
        i6 = (int)g5 * i3 + i5;
        h3 = (double)i6 * d2 + h2;
        
        f3 = (float)h3 * g3 + f2;
        d6 = d3 * (double)f3 + d5;
        
        g6 = g1 * (float)d6 + g5;
        i7 = (int)g6 + i6 * 3;
        
        h4 = (double)i7 * d1 + h3;
        f4 = (float)h4 * g2 + f3;
        
        /* More computation chains */
        l1 = (long)i1 * (long)i2 + (long)i3;
        l2 = (long)d1 + (long)f1 * l1;
        l3 = l1 * l2 + (long)i4;
        l4 = (long)h1 + l2 * l3;
        
        /* Store to volatile variables to force memory ops */
        v1 = i1; v2 = i2; v3 = i3; v4 = i4; v5 = i5;
        v6 = i6; v7 = i7;
        
        l5 = l1; l6 = l2; l7 = l3; l8 = l4;
        
        f5 = f1; f6 = f2; f7 = f3; f8 = f4;
        d7 = d1; d8 = d2; d9 = d3; d10 = d4;
        
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
              "memory"
        );
#endif
        
        /* Continue computation after assembly clobber */
        i8 = (int)d6 + i7 - (int)f4;
        h5 = (double)i8 * d7 + h4;
        
        f5 = (float)h5 * g6 + f4;
        d8 = d9 * (double)f5 + d6;
        
        /* Call function with many arguments - forces argument passing reloads */
        double func_result = many_args_func(
            i1, d1, g1, l1,
            i2, d2, g2, l2,
            i3, d3, g3, l3,
            i4, d4, g4, l4,
            i5, d5, g5, l5,
            i6, d6, g6, l6
        );
        
        /* Use function result in computation */
        h6 = h5 * 0.5 + func_result;
        i9 = (int)h6 + i8;
        
        /* Complex store with indexing */
        int store_idx = (idx * 3 + offset) % ARRAY_SIZE;
        output[store_idx] = (long)i9 + (long)h6 + l4;
        
        /* Update total sum */
        total_sum += h6 + d8 + (double)f5 + (double)i9;
        
        /* More volatile stores */
        v8 = i8; v9 = i9;
        f9 = f5; f10 = (float)h6;
        d9 = d8; d10 = h6;
    }
    
    return total_sum;
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* output_array = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!int_array || !double_array || !float_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        double_array[i] = (double)(i * 1664525 + 1013904223) / 1000000000.0;
        float_array[i] = (float)((i * 1103515245 + 12345) & 0x7fff) / 32768.0f;
        output_array[i] = 0;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double result_sink = 0.0;
    
    /* Perform computation with different strides/offsets */
    for (int run = 0; run < 5; run++) {
        double result = compute_heavy(int_array, double_array, float_array,
                                     output_array, run * 7 + 3, run * 11 + 5);
        result_sink += result;
        
        /* Print checksum to prevent dead code elimination */
        long checksum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            checksum += output_array[i];
        }
        printf("Run %d: checksum = %ld, result = %f\n", run, checksum, result);
    }
    
    /* Final print to ensure all computations are used */
    printf("Final sink value: %f\n", (double)result_sink);
    
    free(int_array);
    free(double_array);
    free(float_array);
    free(output_array);
    
    return 0;
}
