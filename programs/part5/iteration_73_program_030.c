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
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * f1 + (double)a2 * f2;
    result += d1 * (double)l1 + d2 * (double)l2;
    result += (double)(*(int*)p1) * f3;
    result += (double)(*(int*)p2) * f4;
    result += (double)a3 * d3 + (double)a4 * d4;
    
    /* Force memory access */
    result += global_volatile_double;
    result *= global_volatile_float;
    result += global_volatile_int;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer", "no-schedule-insns", "no-schedule-insns2")))
static double compute_heavy(const int* input, double* output, int size)
{
    /* Declare many local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = input[0];
    int i1 = v1 + 1, i2 = v1 + 2, i3 = v1 + 3, i4 = v1 + 4;
    int i5 = v1 + 5, i6 = v1 + 6, i7 = v1 + 7, i8 = v1 + 8;
    int i9 = v1 + 9, i10 = v1 + 10, i11 = v1 + 11, i12 = v1 + 12;
    int i13 = v1 + 13, i14 = v1 + 14, i15 = v1 + 15, i16 = v1 + 16;
    
    /* Long variables */
    long l1 = (long)v1 * 2, l2 = (long)v1 * 3, l3 = (long)v1 * 4;
    long l4 = (long)v1 * 5, l5 = (long)v1 * 6, l6 = (long)v1 * 7;
    
    /* Float variables */
    float f1 = (float)v1 * 0.1f, f2 = (float)v1 * 0.2f;
    float f3 = (float)v1 * 0.3f, f4 = (float)v1 * 0.4f;
    float f5 = (float)v1 * 0.5f, f6 = (float)v1 * 0.6f;
    float f7 = (float)v1 * 0.7f, f8 = (float)v1 * 0.8f;
    float f9 = (float)v1 * 0.9f, f10 = (float)v1 * 1.0f;
    
    /* Double variables */
    double d1 = (double)v1 * 0.01, d2 = (double)v1 * 0.02;
    double d3 = (double)v1 * 0.03, d4 = (double)v1 * 0.04;
    double d5 = (double)v1 * 0.05, d6 = (double)v1 * 0.06;
    double d7 = (double)v1 * 0.07, d8 = (double)v1 * 0.08;
    double d9 = (double)v1 * 0.09, d10 = (double)v1 * 0.10;
    double d11 = (double)v1 * 0.11, d12 = (double)v1 * 0.12;
    
    /* Pointer variables */
    const int* p1 = input;
    double* p2 = output;
    volatile int* p3 = &global_volatile_int;
    
    /* Complex array indexing variables */
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3;
    int stride1 = 16, stride2 = 32, stride3 = 64;
    
    double total = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        int complex_idx = idx1 * stride1 + idx2 * stride2 + idx3 * stride3 + idx4;
        complex_idx = complex_idx % size;
        
        /* Load with complex addressing */
        int load_val = input[complex_idx];
        volatile int vol_load = load_val;
        
        /* Long chain of mixed-type computations with data dependencies */
        /* This prevents register reuse */
        f1 = (float)i1 * (float)vol_load + f2;
        d1 = (double)f1 * d2 + (double)i2;
        i3 = (int)d1 + i4 * load_val;
        f3 = (float)i3 * f4 - (float)l1;
        d3 = d1 * d2 + (double)f3 * d4;
        l2 = (long)d3 + l3 * (long)i5;
        f5 = (float)l2 * f6 + (float)i6;
        d5 = d3 * d4 + (double)f5 * d6;
        i7 = (int)d5 + i8 * (int)f6;
        f7 = (float)i7 * f8 - (float)l4;
        d7 = d5 * d6 + (double)f7 * d8;
        l5 = (long)d7 + l6 * (long)i9;
        f9 = (float)l5 * f10 + (float)i10;
        d9 = d7 * d8 + (double)f9 * d10;
        i11 = (int)d9 + i12 * (int)f10;
        
        /* More mixed computations */
        d11 = (double)i13 * d12 + (double)i14 * d1;
        f2 = (float)d11 * f3 + (float)i15;
        d2 = (double)f2 * d3 + (double)i16;
        i1 = (int)d2 + i2 * load_val;
        
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
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        /* For AArch64 */
#elif defined(__aarch64__) || defined(__ARM_ARCH_8A__)
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
        
        /* Continue computation after assembly clobber */
        /* This forces reloads of all variables */
        f4 = (float)i11 * f5 + (float)l3;
        d4 = d9 * d10 + (double)f4 * d11;
        l3 = (long)d4 + l4 * (long)i12;
        f6 = (float)l3 * f7 + (float)i13;
        d6 = d4 * d5 + (double)f6 * d7;
        i14 = (int)d6 + i15 * (int)f7;
        
        /* Call function with many arguments - stresses argument passing */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&i5, (void*)&i6
        );
        
        /* Use function result in computation */
        d8 = d6 * d7 + func_result;
        f8 = (float)d8 * f9 + (float)i16;
        
        /* Complex store with addressing */
        int store_idx = (idx2 * stride2 + idx3 * stride3 + idx4) % size;
        output[store_idx] = d8 + f8 + (double)i1;
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 8;
        idx2 = (idx2 + 2) % 8;
        idx3 = (idx3 + 3) % 8;
        idx4 = (idx4 + 5) % 8;
        
        /* Accumulate to total to prevent dead code elimination */
        total += d8 + f8 + (double)i1 + (double)i2 + (double)i3;
        
        /* Force memory access to volatile globals */
        total += global_volatile_double;
        total *= global_volatile_float;
        total += global_volatile_int;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = total;
    (void)sink;
    
    return total;
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
        input_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        output_array[i] = 0.0;
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(input_array, output_array, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    checksum += result;
    
    printf("Result: %.6f\n", result);
    printf("Checksum: %.6f\n", checksum);
    
    free(input_array);
    free(output_array);
    
    return 0;
}
