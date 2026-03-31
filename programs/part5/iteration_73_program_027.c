#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile global variables to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to force register/stack pressure */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex computation mixing all arguments */
    double sum = (double)a1 * b1 + (double)c1 * d1;
    sum += (double)a2 * b2 + (double)c2 * d2;
    sum += (double)a3 * b3 + (double)c3 * d3;
    sum += (double)a4 * b4 + (double)c4 * d4;
    sum += (double)a5 * b5 + (double)c5 * d5;
    sum += (double)a6 * b6 + (double)c6 * d6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum += global_volatile_float;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline))
static double compute_heavy(const double* input, double* output, int size) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long l1 = 10, l2 = 20, l3 = 30, l4 = 40, l5 = 50;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    
    /* Additional variables for computation chains */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    float g1, g2, g3, g4, g5, g6, g7, g8, g9, g10;
    double h1, h2, h3, h4, h5, h6, h7, h8, h9, h10;
    
    /* Pointer variables */
    const double* p1 = input;
    double* p2 = output;
    volatile const double* vp = input;
    
    double total = 0.0;
    
    for (int idx = 0; idx < size; idx++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (idx * 7 + 3) % size;
        int idx2 = (idx * 13 + 5) % size;
        int idx3 = (idx * 17 + 11) % size;
        
        /* Load with complex addressing */
        double val1 = input[idx1 * 2 + idx % 8];
        double val2 = input[idx2 * 3 + idx % 16];
        double val3 = input[idx3 * 5 + idx % 32];
        
        /* Long chain of mixed-type computations */
        i1 = v1 + idx;
        i2 = v2 * i1;
        i3 = v3 + i2;
        i4 = v4 * i3;
        i5 = v5 + i4;
        
        j1 = l1 * i1;
        j2 = l2 + j1;
        j3 = l3 * i2;
        j4 = l4 + j3;
        j5 = l5 * i3;
        
        g1 = f1 * (float)i1;
        g2 = f2 + (float)j1;
        g3 = f3 * g1;
        g4 = f4 + g2;
        g5 = f5 * g3;
        
        h1 = d1 * val1;
        h2 = d2 + (double)g1;
        h3 = d3 * h1;
        h4 = d4 + h2;
        h5 = d5 * h3;
        
        /* More computation chains with data dependencies */
        i6 = i5 * 2 + i4;
        i7 = i6 / 3 + i3;
        i8 = i7 * 4 + i2;
        i9 = i8 / 5 + i1;
        i10 = i9 * 6 + idx;
        
        j6 = j5 * 7 + j4;
        j7 = j6 / 8 + j3;
        j8 = j7 * 9 + j2;
        j9 = j8 / 10 + j1;
        j10 = j9 * 11 + idx;
        
        g6 = g5 * 1.5f + g4;
        g7 = g6 / 2.5f + g3;
        g8 = g7 * 3.5f + g2;
        g9 = g8 / 4.5f + g1;
        g10 = g9 * 5.5f + (float)idx;
        
        h6 = h5 * 1.1 + h4;
        h7 = h6 / 2.2 + h3;
        h8 = h7 * 3.3 + h2;
        h9 = h8 / 4.4 + h1;
        h10 = h9 * 5.5 + val2;
        
        /* Inline assembly that clobbers many registers */
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
            "xorpd %%xmm0, %%xmm0\n"
            "xorpd %%xmm1, %%xmm1\n"
            "xorpd %%xmm2, %%xmm2\n"
            "xorpd %%xmm3, %%xmm3\n"
            "xorpd %%xmm4, %%xmm4\n"
            "xorpd %%xmm5, %%xmm5\n"
            "xorpd %%xmm6, %%xmm6\n"
            "xorpd %%xmm7, %%xmm7\n"
            "xorpd %%xmm8, %%xmm8\n"
            "xorpd %%xmm9, %%xmm9\n"
            "xorpd %%xmm10, %%xmm10\n"
            "xorpd %%xmm11, %%xmm11\n"
            "xorpd %%xmm12, %%xmm12\n"
            "xorpd %%xmm13, %%xmm13\n"
            "xorpd %%xmm14, %%xmm14\n"
            "xorpd %%xmm15, %%xmm15\n"
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
        
        /* Call function with many arguments */
        double func_result = many_args_function(
            i1, h1, g1, j1,
            i2, h2, g2, j2,
            i3, h3, g3, j3,
            i4, h4, g4, j4,
            i5, h5, g5, j5,
            i6, h6, g6, j6
        );
        
        /* More mixed-type computations after function call */
        double combined = (double)i10 * 0.1 + (double)j10 * 0.01;
        combined += (double)g10 + h10;
        combined += func_result;
        combined += val3;
        
        /* Complex store with indexing */
        output[(idx * 3 + idx1) % size] = combined;
        output[(idx * 5 + idx2) % size] = combined * 0.5;
        output[(idx * 7 + idx3) % size] = combined * 0.25;
        
        /* Update total with volatile memory access */
        total += combined + global_volatile_double;
        
        /* Force more register pressure with additional computations */
        v1 = (v1 * 3 + 1) % 100;
        v2 = (v2 * 5 + 2) % 100;
        v3 = (v3 * 7 + 3) % 100;
        l1 = (l1 * 11 + 4) % 1000;
        l2 = (l2 * 13 + 5) % 1000;
        f1 = f1 * 1.1f + 0.1f;
        f2 = f2 * 1.2f + 0.2f;
        d1 = d1 * 1.01 + 0.01;
        d2 = d2 * 1.02 + 0.02;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = total;
    return sink;
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
        input[i] = sin((double)i * 0.1) * 100.0;
        output[i] = 0.0;
    }
    
    /* Perform heavy computation */
    double result = 0.0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        result += compute_heavy(input, output, ARRAY_SIZE);
        
        /* Shuffle data to create different access patterns */
        for (int i = 0; i < ARRAY_SIZE / 2; i++) {
            double temp = input[i];
            input[i] = input[ARRAY_SIZE - 1 - i];
            input[ARRAY_SIZE - 1 - i] = temp;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i] * (i + 1);
    }
    
    printf("Result: %f\n", result);
    printf("Checksum: %f\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
