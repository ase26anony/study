/* reload_stress.c - Designed to trigger GCC reload pass uncovered lines */

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

/* Helper with many mixed arguments - forces argument passing complexity */
__attribute__((noinline))
static double many_args_func(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex mixing of all argument types */
    double sum = (double)a1 + b1 + (double)c1 + (double)d1;
    sum += (double)a2 * b2 * (double)c2 / (double)(d2 + 1);
    sum += sqrt((double)a3) + sin(b3) + exp((double)c3) + log((double)labs(d3));
    sum += (double)(a4 & a5) * (b4 - b5) + (float)(c4 * c5) * (double)(d4 | d5);
    sum += (double)(a6 << 2) / (b6 + 1.0) + (double)((int)c6 % 7) * (double)(d6 >> 1);
    
    /* Force memory access */
    sum += volatile_global_double + (double)volatile_global_float;
    
    return sum;
}

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(const int* input_int, const double* input_double, 
                           const float* input_float, long* output) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = input_int[0];  /* volatile to prevent optimization */
    int i1 = v1 + 1, i2 = v1 * 2, i3 = v1 / 3, i4 = v1 - 4;
    int i5 = i1 ^ i2, i6 = i3 | i4, i7 = i1 & i2, i8 = i3 << 2;
    int i9 = i4 >> 1, i10 = i5 + i6, i11 = i7 - i8, i12 = i9 * i10;
    int i13 = i11 ^ i12, i14 = i13 | v1, i15 = i14 & 0xFF, i16 = i15 << 3;
    
    /* Long variables */
    long l1 = (long)i1 * i2, l2 = (long)i3 + i4, l3 = (long)i5 - i6;
    long l4 = (long)i7 | i8, l5 = (long)i9 ^ i10, l6 = (long)i11 & i12;
    long l7 = l1 * l2, l8 = l3 + l4, l9 = l5 - l6, l10 = l7 | l8;
    
    /* Float variables */
    float f1 = (float)i1 * 0.1f, f2 = (float)i2 * 0.2f, f3 = (float)i3 * 0.3f;
    float f4 = (float)i4 * 0.4f, f5 = f1 + f2, f6 = f3 - f4, f7 = f5 * f6;
    float f8 = f2 / f3, f9 = f4 + f5, f10 = f6 * f7, f11 = f8 - f9;
    float f12 = f10 / f11, f13 = f12 * 3.14f, f14 = f13 + 2.71f;
    
    /* Double variables */
    double d1 = (double)l1 * 0.01, d2 = (double)l2 * 0.02, d3 = (double)l3 * 0.03;
    double d4 = (double)l4 * 0.04, d5 = d1 + d2, d6 = d3 - d4, d7 = d5 * d6;
    double d8 = d2 / d3, d9 = d4 + d5, d10 = d6 * d7, d11 = d8 - d9;
    double d12 = d10 / d11, d13 = d12 * 3.14159, d14 = d13 + 2.71828;
    
    /* Pointer variables for complex addressing */
    const int* p1 = input_int;
    const double* p2 = input_double;
    const float* p3 = input_float;
    long* p4 = output;
    
    /* Complex array indexing variables */
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3;
    int stride1 = 8, stride2 = 16, stride3 = 32;
    
    double total = 0.0;
    
    /* Main computation loop with extreme register pressure */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array accesses with multiple index terms */
        int base_idx = (idx1 * stride1 + idx2 * stride2 + idx3) % ARRAY_SIZE;
        int offset_idx = (idx4 * stride3 + iter * 4) % ARRAY_SIZE;
        
        /* Load with complex addressing - forces MEM rtx generation */
        int load1 = input_int[base_idx];
        double load2 = input_double[offset_idx];
        float load3 = input_float[(base_idx + offset_idx) % ARRAY_SIZE];
        
        /* Long chain of mixed-type computations */
        i1 = load1 + i2;
        l1 = (long)i1 * l2 + (long)load1;
        f1 = (float)i1 * 0.5f + (float)l1 * 0.01f;
        d1 = (double)f1 * 1.5 + (double)load2;
        
        i3 = (int)f1 + i4;
        l3 = (long)i3 | l4;
        f3 = fabsf((float)l3 * 0.1f) + f2;
        d3 = sqrt(d1 * d1 + d2 * d2);
        
        i5 = i3 ^ i6;
        l5 = l3 & l6;
        f5 = f3 * f4 - f6;
        d5 = d3 / (d4 + 1.0);
        
        i7 = (int)d5 + i8;
        l7 = (long)i7 * l8;
        f7 = (float)d5 + f8;
        d7 = (double)f7 * d8;
        
        /* More computations to extend dependency chain */
        i9 = i7 << (i8 & 3);
        l9 = l7 >> (l8 & 7);
        f9 = f7 * f8 / f9;
        d9 = sin(d7) + cos(d8);
        
        i10 = i9 | i10;
        l10 = l9 ^ l10;
        f10 = f9 + f10 - f11;
        d10 = d9 * d10 - d11;
        
        /* Inline assembly that clobbers MANY registers */
        /* Forces spills and reloads around assembly block */
#if defined(__x86_64__)
        __asm__ volatile (
            "movq $0, %%rax\n"
            "movq $0, %%rbx\n"
            "movq $0, %%rcx\n"
            "movq $0, %%rdx\n"
            "movq $0, %%rsi\n"
            "movq $0, %%rdi\n"
            "movq $0, %%r8\n"
            "movq $0, %%r9\n"
            "movq $0, %%r10\n"
            "movq $0, %%r11\n"
            "movq $0, %%r12\n"
            "movq $0, %%r13\n"
            "movq $0, %%r14\n"
            "movq $0, %%r15\n"
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
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces argument passing reloads */
        double func_result = many_args_func(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            i5, d5, f5, l5,
            i6, d6, f6, l6
        );
        
        /* More mixed-type computations after function call */
        i11 = (int)func_result + i12;
        l11 = (long)i11 * l12;
        f11 = (float)func_result + f12;
        d11 = d10 * func_result;
        
        /* Store with complex addressing */
        int store_idx = (base_idx + offset_idx + iter) % ARRAY_SIZE;
        output[store_idx] = l11 + (long)i11 + (long)(f11 * 100.0f) + (long)(d11 * 1000.0);
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 7;
        idx2 = (idx2 + 3) % 11;
        idx3 = (idx3 + 5) % 13;
        idx4 = (idx4 + 7) % 17;
        
        /* Accumulate total for checksum */
        total += (double)i11 + (double)l11 + (double)f11 + d11 + func_result;
        
        /* Use volatile globals in computation */
        i12 += volatile_global_int;
        d12 += volatile_global_double;
        f12 += volatile_global_float;
    }
    
    /* Final complex computation mixing all local variables */
    double final_result = 
        (double)i1 + (double)i2 + (double)i3 + (double)i4 +
        (double)i5 + (double)i6 + (double)i7 + (double)i8 +
        (double)i9 + (double)i10 + (double)i11 + (double)i12 +
        (double)l1 + (double)l2 + (double)l3 + (double)l4 +
        (double)l5 + (double)l6 + (double)l7 + (double)l8 +
        (double)l9 + (double)l10 +
        (double)f1 + (double)f2 + (double)f3 + (double)f4 +
        (double)f5 + (double)f6 + (double)f7 + (double)f8 +
        (double)f9 + (double)f10 + (double)f11 + (double)f12 +
        d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 + d11 + d12 + d13 + d14;
    
    return final_result + total;
}

int main() {
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
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        double_array[i] = (double)int_array[i] / 1000.0;
        float_array[i] = (float)int_array[i] / 1000.0f;
        output_array[i] = 0;
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(int_array, double_array, float_array, output_array);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    /* Use volatile to ensure stores aren't optimized away */
    volatile double volatile_result = result;
    volatile long volatile_checksum = checksum;
    
    printf("Result: %f\n", (double)volatile_result);
    printf("Checksum: %ld\n", (long)volatile_checksum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(output_array);
    
    return 0;
}
