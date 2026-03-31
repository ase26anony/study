#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operands */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Large arrays to work with */
static double input_array[ARRAY_SIZE];
static float output_array[ARRAY_SIZE];

/* Helper function with many arguments to force argument passing complexity */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    int* p1, float* p2, double* p3, volatile int* p4)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result -= (double)a3 / d3;
    result += (*p1) * (*p2) * (*p3);
    *p4 += (int)result;
    
    /* Force memory access */
    volatile double temp = d4;
    result += temp;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(const double* input, float* output, int size)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    int i16 = 16, i17 = 17, i18 = 18, i19 = 19, i20 = 20;
    
    /* Long variables */
    long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    
    /* Float variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    /* Pointer variables */
    volatile int* volatile_ptr = &global_counter;
    const double* input_ptr = input;
    float* output_ptr = output;
    
    /* Complex array indexing variables */
    int stride = 16;
    int offset1 = 3, offset2 = 7, offset3 = 11;
    
    /* Main computation loop with unbroken dependency chains */
    for (int idx = 0; idx < size; idx++) {
        /* Complex array indexing with multiple terms */
        int complex_idx = (idx * stride + offset1) % ARRAY_SIZE;
        int complex_idx2 = (idx * 8 + offset2 * 3 + offset3) % ARRAY_SIZE;
        
        /* Load with complex addressing */
        volatile double load1 = input[complex_idx];
        volatile float load2 = (float)input[complex_idx2];
        
        /* Long chain of mixed-type computations */
        /* Each computation depends on previous results */
        d1 = (double)i1 * load1 + d2;
        f1 = (float)d1 * f2 + (float)i2;
        i1 = (int)f1 + i3 * (int)d1;
        d2 = d3 * (double)f1 - (double)i4;
        f2 = f3 * (float)d2 / (float)i5;
        i2 = i6 + (int)(f2 * 10.0f);
        
        d3 = sin(d1) * cos(d2) + (double)i7;
        f3 = expf(f1) * logf(f2) + (float)i8;
        i3 = i9 * i10 - (int)(d3 * f3);
        
        d4 = (double)l1 * d1 / (double)l2 + d5;
        f4 = (float)l3 * f1 - (float)l4 * f2 + f5;
        i4 = (int)l5 + i11 * i12 - (int)(d4 * f4);
        
        d5 = d6 * 2.0 - d7 / 3.0 + (double)i13;
        f5 = f6 * 1.5f + f7 * 0.5f - (float)i14;
        i5 = i15 + i16 - (int)(d5 + f5);
        
        d6 = (double)i17 * global_double + d8;
        f6 = (float)i18 * global_float - f8;
        i6 = i19 + (int)(global_double * 100.0);
        
        /* More mixed computations */
        d7 = d9 * (double)f9 + (double)i20;
        f7 = f10 * (float)d10 - (float)i1;
        d8 = (double)(i2 * i3) / d1;
        f8 = (float)(i4 + i5) * f2;
        d9 = d2 * d3 - d4 * d5;
        f9 = f3 * f4 + f5 * f6;
        d10 = (double)(l1 + l2) * (d6 + d7);
        
        /* Inline assembly that clobbers many registers */
        /* This forces spills and reloads */
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
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            &i5, &f5, &d5, volatile_ptr
        );
        
        /* More computations after function call */
        d10 = d10 + func_result * (double)i6;
        f10 = f10 + (float)func_result * f6;
        i10 = i10 + (int)func_result + i7;
        
        /* Store with complex addressing */
        output[complex_idx % size] = (float)(d1 + d2 + d3 + d4 + d5 + 
                                           (double)f1 + (double)f2 + (double)f3 + 
                                           (double)i1 + (double)i2 + (double)i3);
        
        /* Update indexing variables with complex expressions */
        offset1 = (offset1 * 3 + 7) % 13;
        offset2 = (offset2 * 5 + 11) % 17;
        offset3 = (offset3 * 7 + 13) % 19;
        stride = (stride + 1) % 32;
    }
    
    /* Volatile sink to prevent elimination */
    volatile float sink = f1 + f2 + f3 + f4 + f5;
    (void)sink;
}

int main(void)
{
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = sin((double)i * 0.1) * 100.0;
    }
    
    /* Run computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input_array, output_array, ARRAY_SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
