/* reload_stress.c - Program to stress GCC's reload pass */
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

/* Helper function with many arguments to stress argument passing */
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
    sum += (double)a5 * b5 + (double)c5 * (double)d5;
    sum += (double)a6 / b6 - (double)c6 + (double)d6;
    
    /* Force memory access */
    sum += volatile_global_double;
    sum *= volatile_global_float;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static double compute_heavy(const int* input_int, const double* input_double,
                           const float* input_float, double* output,
                           int stride, int offset)
{
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int vi1, vi2, vi3;  /* Volatile locals */
    volatile double vd1, vd2;
    
    /* Additional pointer/index variables */
    int idx1, idx2, idx3, idx4;
    const int* ptr1;
    const double* ptr2;
    const float* ptr3;
    
    double result = 0.0;
    
    /* Complex loop with data dependencies */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Initialize local variables with complex array indexing */
        idx1 = (outer * 7) % ARRAY_SIZE;
        idx2 = (outer * 13) % ARRAY_SIZE;
        idx3 = (outer * 23) % ARRAY_SIZE;
        idx4 = (outer * 37) % ARRAY_SIZE;
        
        /* Complex addressing modes with multiple terms */
        i1 = input_int[idx1 * stride + offset];
        i2 = input_int[idx2 * stride + (offset + 1) % stride];
        i3 = input_int[idx3 * stride + (offset + 2) % stride];
        i4 = input_int[idx4 * stride + (offset + 3) % stride];
        
        /* More complex array accesses */
        ptr1 = &input_int[(idx1 + idx2) * stride];
        i5 = ptr1[(idx3 + idx4) % stride];
        i6 = ptr1[(idx1 * 2 + idx2 * 3) % stride];
        
        /* Floating point loads with complex addressing */
        d1 = input_double[idx1 * stride + offset];
        d2 = input_double[idx2 * stride + (offset + 1) % stride];
        f1 = input_float[idx3 * stride + offset];
        f2 = input_float[idx4 * stride + (offset + 1) % stride];
        
        /* Long chain of mixed-type computations */
        l1 = (long)i1 * i2 + i3;
        l2 = (long)i4 * i5 - i6;
        
        f3 = (float)i1 * f1 + (float)l1;
        f4 = (float)i2 * f2 - (float)l2;
        
        d3 = (double)f3 * d1 + (double)f4;
        d4 = (double)f1 / d2 - (double)f2;
        
        i7 = (int)d3 + (int)d4;
        i8 = (int)(d3 * 100.0) - (int)(d4 * 50.0);
        
        f5 = (float)d3 * (float)i7;
        f6 = (float)d4 / (float)i8;
        
        l3 = (long)(f5 * 1000.0f);
        l4 = (long)(f6 * 500.0f);
        
        d5 = (double)l3 * d3 / (double)l4;
        d6 = (double)l4 + d4 * (double)l3;
        
        /* More computations creating data dependencies */
        f7 = (float)d5 * f5 + (float)d6 * f6;
        f8 = (float)d5 / f5 - (float)d6 / f6;
        
        i9 = (int)f7 * i7 + (int)f8 * i8;
        i10 = (int)f7 / i7 - (int)f8 / i8;
        
        l5 = (long)i9 * l3 + (long)i10 * l4;
        l6 = (long)i9 / l3 - (long)i10 / l4;
        
        d7 = (double)l5 * d5 + (double)l6 * d6;
        d8 = (double)l5 / d5 - (double)l6 / d6;
        
        f9 = (float)d7 * f7 + (float)d8 * f8;
        f10 = (float)d7 / f7 - (float)d8 / f8;
        
        l7 = (long)(f9 * 100.0f) + (long)(f10 * 50.0f);
        l8 = (long)(f9 / 100.0f) - (long)(f10 / 50.0f);
        
        d9 = (double)l7 * d7 / (double)l8;
        d10 = (double)l8 + d8 * (double)l7;
        
        /* Volatile operations to force memory accesses */
        vi1 = i1 + i2;
        vi2 = i3 * i4;
        vi3 = i5 - i6;
        vd1 = d1 * d2;
        vd2 = d3 + d4;
        
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
            "fmov d31, #0.0"
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
        
        /* Call function with many arguments - stresses argument passing */
        double func_result = many_args_function(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            i5, d5, f5, l5,
            i6, d6, f6, l6
        );
        
        /* Use volatile globals in computation */
        d9 += volatile_global_double;
        f9 *= volatile_global_float;
        i9 += volatile_global_int;
        
        /* Final computation mixing everything */
        double final_val = (double)i9 * d9 + (double)l7 * d10 +
                          (double)func_result * (double)vi1 +
                          vd1 - vd2;
        
        /* Complex array store with multiple index terms */
        int store_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7 + idx4 * 11) % ARRAY_SIZE;
        output[store_idx * stride + (offset + outer) % stride] = final_val;
        
        result += final_val;
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int* int_array = (int*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(double));
    float* float_array = (float*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(float));
    double* output_array = (double*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(double));
    
    if (!int_array || !double_array || !float_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        double_array[i] = (double)((i * 1664525 + 1013904223) & 0xffff) / 65536.0;
        float_array[i] = (float)((i * 214013 + 2531011) & 0x7fff) / 32768.0f;
        output_array[i] = 0.0;
    }
    
    /* Perform heavy computation */
    double total = 0.0;
    int stride = 16;
    
    for (int offset = 0; offset < stride; offset++) {
        total += compute_heavy(int_array, double_array, float_array,
                              output_array, stride, offset);
    }
    
    /* Use volatile sink to prevent optimization */
    volatile double sink = total;
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i += 257) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %.15f\n", checksum);
    printf("Total: %.15f\n", (double)sink);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(output_array);
    
    return 0;
}
