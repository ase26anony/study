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

/* Helper function with many arguments to force stack passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((intptr_t)p1 % 1000);
    result += (double)((intptr_t)p2 % 1000);
    result += d3 * d4;
    result += (double)a3 * (double)a4;
    result += (double)f3 * (double)f4;
    
    /* Force memory access */
    result += global_volatile_double;
    result += (double)global_volatile_int;
    result += (double)global_volatile_float;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline))
static void compute_heavy(const int* input, double* output, int size)
{
    /* Declare many local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int vi1, vi2, vi3;
    volatile double vd1, vd2, vd3;
    volatile float vf1, vf2, vf3;
    
    /* Initialize with values to prevent optimization */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    
    vi1 = 0; vi2 = 0; vi3 = 0;
    vd1 = 0.0; vd2 = 0.0; vd3 = 0.0;
    vf1 = 0.0f; vf2 = 0.0f; vf3 = 0.0f;
    
    /* Complex loop with data dependencies and mixed types */
    for (int idx = 0; idx < size; idx++) {
        /* Complex array indexing with multiple terms */
        int index1 = (idx * 7 + 3) % size;
        int index2 = (idx * 13 + 5) % size;
        int index3 = (idx * 17 + 11) % size;
        
        /* Load from input with complex addressing */
        int val1 = input[index1];
        int val2 = input[index2];
        int val3 = input[index3];
        
        /* Long chain of mixed-type computations */
        f1 = (float)val1 * f2 + f3 - f4 * (float)val2;
        d1 = (double)f1 * d2 + d3 / (double)val3;
        i1 = (int)d1 + i2 * i3 - i4 / (val1 + 1);
        l1 = (long)i1 * l2 + l3 - l4;
        f3 = (float)l1 * f4 + f5;
        d3 = (double)f3 * d4 + d5;
        i3 = (int)d3 + i4 * i5;
        f5 = (float)i3 * f6 + f7;
        d5 = (double)f5 * d6 + d7;
        i5 = (int)d5 + i6 * i7;
        f7 = (float)i5 * f8 + f9;
        d7 = (double)f7 * d8 + d9;
        i7 = (int)d7 + i8 * i9;
        f9 = (float)i7 * f10 + f1;
        d9 = (double)f9 * d10 + d1;
        
        /* More computations to increase pressure */
        l2 = l1 * 2 + l3;
        l4 = l2 / 3 + l5;
        l6 = l4 * 4 - l7;
        l8 = l6 / 5 + l9;
        
        f2 = f1 * 1.5f + f3;
        f4 = f2 / 2.0f + f5;
        f6 = f4 * 3.0f - f7;
        f8 = f6 / 4.0f + f9;
        
        d2 = d1 * 1.5 + d3;
        d4 = d2 / 2.0 + d5;
        d6 = d4 * 3.0 - d7;
        d8 = d6 / 4.0 + d9;
        
        i2 = i1 + i3 * 2;
        i4 = i2 - i5 / 3;
        i6 = i4 * 4 + i7;
        i8 = i6 - i9 / 5;
        
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
#elif defined(__aarch64__) || defined(__arm64__)
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
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("" : : : "memory");
#endif
        
        /* Call function with many arguments */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&input[idx], (void*)&output[idx]
        );
        
        /* More computations after function call */
        i10 = (int)func_result + i9;
        l10 = (long)func_result * l9;
        f10 = (float)func_result + f9;
        d10 = func_result * d9;
        
        /* Store to volatile variables to force memory writes */
        vi1 = i1 + i2;
        vi2 = i3 + i4;
        vi3 = i5 + i6;
        
        vf1 = f1 + f2;
        vf2 = f3 + f4;
        vf3 = f5 + f6;
        
        vd1 = d1 + d2;
        vd2 = d3 + d4;
        vd3 = d5 + d6;
        
        /* Complex store with addressing */
        output[idx] = d10 + (double)vi1 + (double)vf1 + vd1;
        
        /* Update indices for next iteration */
        i1 = (i1 + 1) % 100;
        i2 = (i2 + 2) % 100;
        i3 = (i3 + 3) % 100;
    }
    
    /* Use volatile results to prevent optimization */
    global_volatile_int = vi1 + vi2 + vi3;
    global_volatile_float = vf1 + vf2 + vf3;
    global_volatile_double = vd1 + vd2 + vd3;
}

int main(void)
{
    /* Allocate and initialize arrays */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Perform heavy computation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input_array, output_array, ARRAY_SIZE);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Global volatile values: %d, %f, %f\n", 
           global_volatile_int, 
           global_volatile_float, 
           global_volatile_double);
    
    /* Cleanup */
    free(input_array);
    free(output_array);
    
    return 0;
}
