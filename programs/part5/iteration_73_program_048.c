#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((uintptr_t)p1 % 1024);
    result += (double)((uintptr_t)p2 % 512);
    
    /* More mixing */
    result = result * (double)a3 - (double)a4;
    result = result / (d3 + 1.0) * (d4 + 1.0);
    result = result + (double)(f3 * f4);
    
    /* Use volatile globals */
    result += global_volatile_double;
    result *= (double)global_volatile_int;
    result /= (double)global_volatile_float;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(double* input, double* output, int size) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Pointer variables */
    double* p1, *p2, *p3, *p4, *p5;
    
    /* Volatile locals to prevent optimization */
    volatile int vi1, vi2, vi3;
    volatile double vd1, vd2, vd3;
    volatile float vf1, vf2, vf3;
    
    /* Initialize some variables */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    i11 = 11; i12 = 12; i13 = 13; i14 = 14; i15 = 15;
    i16 = 16; i17 = 17; i18 = 18; i19 = 19; i20 = 20;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    f11 = 11.11f; f12 = 12.12f; f13 = 13.13f; f14 = 14.14f; f15 = 15.15f;
    f16 = 16.16f; f17 = 17.17f; f18 = 18.18f; f19 = 19.19f; f20 = 20.20f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    d16 = 16.16; d17 = 17.17; d18 = 18.18; d19 = 19.19; d20 = 20.20;
    
    p1 = input; p2 = output; p3 = input + size/2; p4 = output + size/2; p5 = input + size/4;
    
    /* Complex loop with data dependencies and mixed operations */
    for (int idx = 0; idx < size; idx++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (idx * 3) / 2;
        int idx2 = (idx + i1) * 2 - i2;
        int idx3 = idx1 + idx2 + i3;
        int idx4 = (idx * i4) % size;
        int idx5 = (idx + i5) & (size - 1);
        
        /* Load from input with complex addressing */
        double val1 = input[idx1];
        double val2 = input[idx2];
        double val3 = input[idx3];
        double val4 = input[idx4];
        double val5 = input[idx5];
        
        /* Long chain of arithmetic with mixed types */
        /* Stage 1: Integer and float mixing */
        f1 = (float)i1 * (float)val1 + f2;
        f3 = (float)i2 * (float)val2 + f4;
        f5 = (float)i3 * (float)val3 + f6;
        f7 = (float)i4 * (float)val4 + f8;
        f9 = (float)i5 * (float)val5 + f10;
        
        /* Stage 2: Float to double conversions */
        d1 = (double)f1 * val1 + d2;
        d3 = (double)f3 * val2 + d4;
        d5 = (double)f5 * val3 + d6;
        d7 = (double)f7 * val4 + d8;
        d9 = (double)f9 * val5 + d10;
        
        /* Stage 3: More mixing with longs */
        l1 = (long)(d1 * 1000) + l2;
        l2 = (long)(d3 * 1000) + l3;
        l3 = (long)(d5 * 1000) + l4;
        l4 = (long)(d7 * 1000) + l5;
        l5 = (long)(d9 * 1000) + l6;
        
        /* Stage 4: Back to floats and doubles */
        f11 = (float)l1 / 1000.0f + f12;
        f13 = (float)l2 / 1000.0f + f14;
        f15 = (float)l3 / 1000.0f + f16;
        f17 = (float)l4 / 1000.0f + f18;
        f19 = (float)l5 / 1000.0f + f20;
        
        d11 = (double)f11 * d12 + d13;
        d14 = (double)f13 * d15 + d16;
        d17 = (double)f15 * d18 + d19;
        d20 = (double)f17 * d1 + d2;
        
        /* Stage 5: Integer computations */
        i6 = (int)d11 + i7;
        i8 = (int)d14 + i9;
        i10 = (int)d17 + i11;
        i12 = (int)d20 + i13;
        i14 = (int)(d11 + d14) + i15;
        
        /* Use volatile variables */
        vi1 = i6;
        vi2 = i8;
        vi3 = i10;
        vd1 = d11;
        vd2 = d14;
        vd3 = d17;
        vf1 = f11;
        vf2 = f13;
        vf3 = f15;
        
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
            "mov x29, #0\n\t"
            "mov x30, #0\n\t"
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
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory", "cc"
        );
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("" : : : "memory");
#endif
        
        /* Call function with many arguments - forces register/stack reloads */
        double func_result = many_args_function(
            i6, i8, i10, i12,
            f11, f13, f15, f17,
            d11, d14, d17, d20,
            l1, l2, (void*)&input[idx], (void*)&output[idx]
        );
        
        /* More computations after function call */
        i16 = (int)func_result + i17;
        i18 = (int)(func_result * 2.0) + i19;
        i20 = (int)(func_result / 2.0) + i1;
        
        f18 = (float)func_result + f19;
        f20 = (float)(func_result * 0.5) + f1;
        
        d11 = func_result * d12 + d13;
        d14 = func_result / d15 + d16;
        
        /* Complex store with addressing */
        output[idx1] = d11 + (double)i16;
        output[idx2] = d14 + (double)i18;
        output[idx3] = (double)f18 + (double)i20;
        output[idx4] = (double)f20 + d11;
        output[idx5] = func_result;
        
        /* Update indices for next iteration */
        i1 = (i1 + 1) & 0xFF;
        i2 = (i2 + 2) & 0xFF;
        i3 = (i3 + 3) & 0xFF;
        i4 = (i4 + 4) & 0xFF;
        i5 = (i5 + 5) & 0xFF;
    }
    
    /* Final volatile store to prevent optimization */
    global_volatile_int = i20;
    global_volatile_double = d20;
    global_volatile_float = f20;
}

int main() {
    /* Allocate and initialize arrays */
    double* input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = (double)((i * 1234567) % 10000) / 100.0;
        output[i] = 0.0;
    }
    
    /* Perform heavy computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input, output, ARRAY_SIZE);
        
        /* Swap arrays to create data dependencies between iterations */
        double* temp = input;
        input = output;
        output = temp;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Volatile globals: int=%d, double=%f, float=%f\n",
           global_volatile_int, global_volatile_double, global_volatile_float);
    
    free(input);
    free(output);
    
    return 0;
}
