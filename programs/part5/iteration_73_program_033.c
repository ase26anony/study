#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile float global_volatile_float = 3.14159f;
volatile double global_volatile_double = 2.71828;

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(int a1, int a2, int a3, int a4,
                          float f1, float f2, float f3, float f4,
                          double d1, double d2, double d3, double d4,
                          void* p1, void* p2) {
    /* Complex mixing of types to require different register classes */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)((int)d3 + a3) * (double)f3;
    result += (double)((intptr_t)p1 % 1000) * 0.001;
    result += (double)((intptr_t)p2 % 1000) * 0.001;
    return result * (double)f4 * d4 * (double)a4;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(const int* input_int, const float* input_float, 
                   const double* input_double, int* output_int,
                   float* output_float, double* output_double,
                   int stride, int offset) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    int k1, k2, k3, k4, k5, k6, k7, k8, k9, k10;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float g1, g2, g3, g4, g5, g6, g7, g8, g9, g10;
    
    /* Double precision variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double e1, e2, e3, e4, e5, e6, e7, e8, e9, e10;
    
    /* Pointer/index variables */
    int idx1, idx2, idx3, idx4, idx5;
    volatile int sink_volatile; /* Force memory operations */
    
    /* Initialize with volatile reads to force loads */
    i1 = global_volatile_int;
    f1 = global_volatile_float;
    d1 = global_volatile_double;
    
    /* Complex loop with data dependencies preventing register reuse */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * stride + offset) % ARRAY_SIZE;
        idx2 = (iter * stride * 2 + offset * 3) % ARRAY_SIZE;
        idx3 = (iter * stride / 2 + offset * 5) % ARRAY_SIZE;
        idx4 = (idx1 * 3 + idx2 * 7 + idx3 * 11) % ARRAY_SIZE;
        idx5 = (idx1 + idx2 * 13 + idx3 * 17) % ARRAY_SIZE;
        
        /* Long chain of interdependent computations mixing types */
        /* Load from arrays with complex addressing */
        i2 = input_int[idx1] + input_int[idx2];
        i3 = input_int[idx3] * 2 - input_int[idx4];
        
        f2 = input_float[idx1] * 1.5f;
        f3 = input_float[idx2] + input_float[idx3];
        
        d2 = input_double[idx1] * 1.7;
        d3 = input_double[idx2] / 2.3;
        
        /* Chain 1: Integer to float to double conversions */
        f4 = (float)i2 * f2 + (float)i3;
        d4 = (double)f3 * d2 + (double)f4;
        i4 = (int)d3 + (int)d4 * 3;
        
        /* Chain 2: More mixed operations */
        f5 = f2 * f3 - (float)i4;
        d5 = d2 / d3 + (double)f5;
        i5 = i2 * i3 - (int)f5 + (int)d5;
        
        /* Chain 3: Pointer arithmetic mixed with computations */
        f6 = (float)((intptr_t)&input_int[idx1] % 100) * 0.01f;
        d6 = (double)((intptr_t)&input_float[idx2] % 100) * 0.01;
        i6 = ((intptr_t)&input_double[idx3] % 100) + i5;
        
        /* Chain 4: Trigonometric operations */
        f7 = sinf(f6 * 3.14159f / 180.0f);
        d7 = cos(d6 * 3.14159 / 180.0);
        i7 = (int)(f7 * 1000.0f) + (int)(d7 * 1000.0);
        
        /* Chain 5: Exponential operations */
        f8 = expf(f7 * 0.1f);
        d8 = exp(d7 * 0.1);
        i8 = (int)(logf(f8) * 100.0f) + (int)(log(d8) * 100.0);
        
        /* Chain 6: More complex dependencies */
        f9 = f4 * f5 - f6 * f7 + f8;
        d9 = d4 * d5 - d6 * d7 + d8;
        i9 = i4 * i5 - i6 * i7 + i8;
        
        /* Chain 7: Final mixing */
        f10 = (float)i9 * 0.5f + f9;
        d10 = (double)i9 * 0.5 + d9;
        i10 = (int)f10 + (int)d10 + i9;
        
        /* Inline assembly that clobbers many registers */
        /* This forces spills and reloads around the asm block */
#if defined(__x86_64__)
        __asm__ volatile (
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
        
        /* Call function with many arguments - stresses calling convention */
        double func_result = many_args_function(
            i1, i2, i3, i4, 
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            (void*)&input_int[idx5],
            (void*)&output_int[iter % ARRAY_SIZE]
        );
        
        /* Use the result in further computations */
        f10 += (float)func_result * 0.1f;
        d10 += func_result * 0.1;
        i10 += (int)func_result;
        
        /* Store results with complex array indexing */
        output_int[(iter * 3 + offset) % ARRAY_SIZE] = i10;
        output_float[(iter * 5 + offset * 2) % ARRAY_SIZE] = f10;
        output_double[(iter * 7 + offset * 3) % ARRAY_SIZE] = d10;
        
        /* Volatile sink to prevent optimization */
        sink_volatile = i10 + (int)f10 + (int)d10;
        
        /* Rotate values to create dependencies for next iteration */
        i1 = i10;
        f1 = f10;
        d1 = d10;
    }
    
    /* Final volatile store */
    global_volatile_int = sink_volatile;
}

int main() {
    /* Allocate and initialize arrays with pseudo-random data */
    int* input_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* input_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* input_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    int* output_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* output_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* output_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_int[i] = rand() % 1000;
        input_float[i] = (float)(rand() % 1000) / 10.0f;
        input_double[i] = (double)(rand() % 1000) / 10.0;
        output_int[i] = 0;
        output_float[i] = 0.0f;
        output_double[i] = 0.0;
    }
    
    /* Perform heavy computation with different strides/offsets */
    for (int run = 0; run < 10; run++) {
        compute_heavy(input_int, input_float, input_double,
                     output_int, output_float, output_double,
                     run * 3 + 1, run * 7);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_int[i];
        checksum += (long long)output_float[i];
        checksum += (long long)output_double[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(input_int);
    free(input_float);
    free(input_double);
    free(output_int);
    free(output_float);
    free(output_double);
    
    return 0;
}
