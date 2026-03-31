#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_counter = 0;
volatile double global_double = 0.0;
volatile float global_float_array[ARRAY_SIZE];

/* Argument-heavy helper function */
__attribute__((noinline))
static double heavy_calculation(
    int a1, int a2, int a3, int a4,
    long b1, long b2, long b3, long b4,
    float c1, float c2, float c3, float c4,
    double d1, double d2, double d3, double d4,
    int* p1, float* p2, double* p3, volatile int* p4)
{
    /* Complex mixed-type computation */
    double result = 0.0;
    result += (double)a1 * (double)a2 / (double)a3;
    result += (double)b1 - (double)b2 + (double)b3;
    result += (double)c1 * (double)c2 + (double)c3;
    result += d1 * d2 / d3 * d4;
    result += *p1 + (double)*p2 + *p3 + (double)*p4;
    
    /* Force memory access */
    *p4 = (int)result;
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
static void compute_heavy(const int* input, double* output, int size)
{
    /* Declare many local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Additional pointer variables */
    volatile int* volatile_ptr = &global_counter;
    const int* input_ptr = input;
    double* output_ptr = output;
    
    /* Initialize with values from input array */
    i1 = input[0]; i2 = input[1]; i3 = input[2]; i4 = input[3];
    i5 = input[4]; i6 = input[5]; i7 = input[6]; i8 = input[7];
    i9 = input[8]; i10 = input[9];
    
    l1 = (long)i1 * 2; l2 = (long)i2 * 3; l3 = (long)i3 * 4;
    l4 = (long)i4 * 5; l5 = (long)i5 * 6; l6 = (long)i6 * 7;
    l7 = (long)i7 * 8; l8 = (long)i8 * 9; l9 = (long)i9 * 10;
    l10 = (long)i10 * 11;
    
    f1 = (float)i1 * 0.1f; f2 = (float)i2 * 0.2f;
    f3 = (float)i3 * 0.3f; f4 = (float)i4 * 0.4f;
    f5 = (float)i5 * 0.5f; f6 = (float)i6 * 0.6f;
    f7 = (float)i7 * 0.7f; f8 = (float)i8 * 0.8f;
    f9 = (float)i9 * 0.9f; f10 = (float)i10 * 1.0f;
    
    d1 = (double)f1 * 1.1; d2 = (double)f2 * 1.2;
    d3 = (double)f3 * 1.3; d4 = (double)f4 * 1.4;
    d5 = (double)f5 * 1.5; d6 = (double)f6 * 1.6;
    d7 = (double)f7 * 1.7; d8 = (double)f8 * 1.8;
    d9 = (double)f9 * 1.9; d10 = (double)f10 * 2.0;
    
    /* Complex loop with data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Long chain of mixed-type computations */
        d1 = (double)i1 * d1 + (double)l1;
        f1 = (float)d1 * f1 + (float)i2;
        i1 = (int)f1 + i3 + (int)d1;
        
        d2 = (double)i2 * d2 - (double)l2;
        f2 = (float)d2 * f2 - (float)i3;
        i2 = (int)f2 + i4 - (int)d2;
        
        d3 = (double)i3 * d3 * (double)l3;
        f3 = (float)d3 * f3 * (float)i4;
        i3 = (int)f3 * i5 * (int)d3;
        
        d4 = (double)i4 / d4 + (double)l4;
        f4 = (float)d4 / f4 + (float)i5;
        i4 = (int)f4 / i6 + (int)d4;
        
        d5 = sin(d5) * (double)i5 + cos((double)l5);
        f5 = sinf(f5) * (float)i6 + cosf((float)l6);
        i5 = (int)(d5 * 1000) + (int)(f5 * 1000);
        
        /* Complex array indexing with multiple terms */
        int idx1 = (i1 * 17 + i2 * 13 + iter * 7) % size;
        int idx2 = (i3 * 19 + i4 * 11 + iter * 5) % size;
        int idx3 = (i5 * 23 + i6 * 29 + iter * 3) % size;
        
        /* Memory operations with complex addressing */
        volatile_ptr = (volatile int*)&input[idx1];
        f6 = global_float_array[idx2] * 2.0f;
        d6 = (double)input[idx3] * 3.0;
        
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
            "pxor %%xmm15, %%xmm15"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        /* For AArch64 */
#elif defined(__aarch64__)
        __asm__ volatile (
            "# Clobber many registers\n\t"
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
            "fmov d31, #0.0"
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
#endif
        
        /* More mixed-type computations after assembly */
        d7 = d6 * d1 + d2 * d3 - d4 / d5;
        f7 = f6 * f1 + f2 * f3 - f4 / f5;
        i6 = i1 * i2 + i3 * i4 - i5 / (i1 + 1);
        
        l7 = l1 * l2 + l3 * l4 - l5 / (l6 + 1);
        
        /* Call argument-heavy function with many parameters */
        double func_result = heavy_calculation(
            i1, i2, i3, i4,
            l1, l2, l3, l4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            &i5, &f5, &d5, volatile_ptr
        );
        
        /* Use function result in further computation */
        d8 = d7 * func_result + d9;
        f8 = f7 * (float)func_result + f9;
        i7 = i6 * (int)func_result + i8;
        
        /* Store results with complex indexing */
        int out_idx = (i1 + i2 * 3 + i3 * 5 + iter * 11) % size;
        output[out_idx] = d8 + (double)f8 + (double)i7 + (double)l7;
        
        /* Update global volatile variables */
        global_counter++;
        global_double += d8;
        global_float_array[out_idx % ARRAY_SIZE] = f8;
        
        /* Create data dependencies for next iteration */
        i1 = i7 % 256;
        i2 = (i1 + i3) % 256;
        i3 = (i2 + i4) % 256;
        i4 = (i3 + i5) % 256;
        i5 = (i4 + i6) % 256;
        
        l1 = l7;
        l2 = l1 * 2;
        l3 = l2 * 3;
        
        f1 = f8;
        f2 = f1 * 1.1f;
        f3 = f2 * 1.2f;
        
        d1 = d8;
        d2 = d1 * 1.01;
        d3 = d2 * 1.02;
    }
    
    /* Final store to volatile to prevent optimization */
    *volatile_ptr = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
}

int main(void)
{
    /* Initialize arrays with pseudo-random data */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        global_float_array[i] = (float)(input_array[i] % 1000) * 0.001f;
    }
    
    /* Perform heavy computation */
    compute_heavy(input_array, output_array, ARRAY_SIZE);
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    checksum += global_double;
    checksum += (double)global_counter;
    
    printf("Checksum: %f\n", checksum);
    
    free(input_array);
    free(output_array);
    
    return 0;
}
