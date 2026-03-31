#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

// Volatile global variables to force memory operations
volatile int volatile_global_int = 42;
volatile double volatile_global_double = 3.14159;
volatile float volatile_global_float = 2.71828f;

// Helper function with many arguments to stress argument passing
__attribute__((noinline))
static double many_args_function(
    int a1, double a2, float a3, long a4,
    int a5, double a6, float a7, long a8,
    int a9, double a10, float a11, long a12,
    int a13, double a14, float a15, long a16,
    volatile int* a17, volatile double* a18)
{
    // Complex computation mixing all arguments
    double sum = (double)a1 + a2 + (double)a3 + (double)a4 +
                 (double)a5 + a6 + (double)a7 + (double)a8 +
                 (double)a9 + a10 + (double)a11 + (double)a12 +
                 (double)a13 + a14 + (double)a15 + (double)a16;
    
    // Force memory accesses through volatile pointers
    sum += *a17 + *a18;
    
    // Mix integer and floating point operations
    sum = sum * (double)(a1 & 0xFF) / (double)(a4 | 1);
    sum = sum + sin((double)a3) + cos(a6);
    
    return sum;
}

// Main computation function with extreme register pressure
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(double* input, double* output, int size, int stride)
{
    // Declare MANY local variables to create register pressure
    // Integer variables
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    long l6 = 600, l7 = 700, l8 = 800, l9 = 900, l10 = 1000;
    
    // Floating point variables
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    // Pointer variables
    double* p1 = input;
    double* p2 = output;
    volatile double* vp1 = &volatile_global_double;
    volatile float* vp2 = &volatile_global_float;
    
    // Index variables for complex array access
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3;
    
    // Volatile locals to prevent optimization
    volatile int vi1 = 0;
    volatile double vd1 = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Complex array indexing with multiple terms
        int base_idx = (iter * stride) % size;
        int offset1 = (idx1 * 3 + idx2 * 7) % 16;
        int offset2 = (idx3 * 5 + idx4 * 11) % 16;
        int read_idx = (base_idx + offset1 + offset2) % size;
        int write_idx = (base_idx * 2 + offset1 - offset2 + size) % size;
        
        // Load data with complex addressing
        double load1 = input[read_idx];
        double load2 = input[(read_idx + stride) % size];
        float load3 = (float)input[(read_idx * 2) % size];
        
        // LONG chain of mixed-type computations with data dependencies
        // Each result depends on previous computations to prevent register reuse
        
        // Integer computations
        i1 = i2 + i3 * i4 - i5;
        i2 = i1 ^ i6 | i7 & i8;
        i3 = i2 * i9 / (i10 | 1);
        i4 = i3 << 2 + i1 >> 1;
        i5 = i4 % 17 + i2 * 3;
        
        // Long integer computations
        l1 = l2 * l3 + l4 - l5;
        l2 = l1 ^ l6 | l7;
        l3 = l2 * l8 / (l9 | 1);
        l4 = l3 << 3 + l1 >> 2;
        l5 = l4 % 23 + l2 * 5;
        
        // Float computations with mixing
        f1 = f2 * (float)i1 + f3 / (float)l1;
        f2 = f1 + (float)d1 * f4 - f5;
        f3 = sinf(f2) * cosf(f1) + f6;
        f4 = f3 * (float)i2 + (float)load1;
        f5 = f4 / (f7 + 1.0f) * (float)l2;
        
        // Double computations with type conversions
        d1 = d2 * (double)f1 + d3 / (double)i3;
        d2 = d1 + (double)load2 * d4 - d5;
        d3 = sin(d2) * cos(d1) + d6;
        d4 = d3 * (double)l3 + (double)load3;
        d5 = d4 / (d7 + 1.0) * (double)i4;
        
        // More mixed computations
        f6 = (float)d1 * f8 + (float)i5;
        f7 = (float)l4 * f9 - f10;
        d6 = (double)f6 * d8 + (double)l5;
        d7 = (double)i6 * d9 - d10;
        
        // Complex computation involving all types
        double complex_result = 
            (double)i1 * d1 + 
            (double)l1 * d2 - 
            (double)f1 * d3 + 
            (double)f2 * d4 +
            (double)i2 * (double)l2 / (d5 + 1.0);
        
        // Inline assembly that clobbers many registers
        // This forces the compiler to spill/reload around it
#if defined(__x86_64__)
        __asm__ volatile (
            "movq $0x123456789ABCDEF0, %%rax\n\t"
            "movq $0xFEDCBA9876543210, %%rbx\n\t"
            "movq $0x1111111111111111, %%rcx\n\t"
            "movq $0x2222222222222222, %%rdx\n\t"
            "movq $0x3333333333333333, %%rsi\n\t"
            "movq $0x4444444444444444, %%rdi\n\t"
            "movq $0x5555555555555555, %%r8\n\t"
            "movq $0x6666666666666666, %%r9\n\t"
            "movq $0x7777777777777777, %%r10\n\t"
            "movq $0x8888888888888888, %%r11\n\t"
            "movq $0x9999999999999999, %%r12\n\t"
            "movq $0xAAAAAAAAAAAAAAAA, %%r13\n\t"
            "movq $0xBBBBBBBBBBBBBBBB, %%r14\n\t"
            "movq $0xCCCCCCCCCCCCCCCC, %%r15\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
#elif defined(__aarch64__)
        __asm__ volatile (
            "mov x0, #0x1111\n\t"
            "mov x1, #0x2222\n\t"
            "mov x2, #0x3333\n\t"
            "mov x3, #0x4444\n\t"
            "mov x4, #0x5555\n\t"
            "mov x5, #0x6666\n\t"
            "mov x6, #0x7777\n\t"
            "mov x7, #0x8888\n\t"
            "mov x8, #0x9999\n\t"
            "mov x9, #0xAAAA\n\t"
            "mov x10, #0xBBBB\n\t"
            "mov x11, #0xCCCC\n\t"
            "mov x12, #0xDDDD\n\t"
            "mov x13, #0xEEEE\n\t"
            "mov x14, #0xFFFF\n\t"
            "mov x15, #0x1234\n\t"
            "mov x16, #0x5678\n\t"
            "mov x17, #0x9ABC\n\t"
            "mov x18, #0xDEF0\n\t"
            "mov x19, #0x2468\n\t"
            "mov x20, #0x1357\n\t"
            "mov x21, #0x9BDF\n\t"
            "mov x22, #0xACE0\n\t"
            "mov x23, #0xFEDC\n\t"
            "mov x24, #0xBA98\n\t"
            "mov x25, #0x7654\n\t"
            "mov x26, #0x3210\n\t"
            "mov x27, #0xABCD\n\t"
            "mov x28, #0xEF01\n\t"
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
        );
#endif
        
        // Continue computation after assembly clobber
        i6 = i5 + (int)f3 * (int)d1;
        i7 = i6 ^ (int)complex_result;
        i8 = i7 * i3 - i4;
        i9 = i8 / (i2 | 1) + i1;
        i10 = i9 << 1 + i5 >> 2;
        
        l6 = l5 + (long)f4 * (long)d2;
        l7 = l6 ^ (long)complex_result;
        l8 = l7 * l3 - l4;
        l9 = l8 / (l2 | 1) + l1;
        l10 = l9 << 1 + l5 >> 2;
        
        f8 = f7 * (float)i6 + (float)l6;
        f9 = f8 / (f5 + 1.0f) * (float)d3;
        f10 = f9 + (float)complex_result - f1;
        
        d8 = d7 * (double)i7 + (double)l7;
        d9 = d8 / (d5 + 1.0) * (double)f6;
        d10 = d9 + complex_result - d1;
        
        // Call function with many arguments to stress calling convention
        double func_result = many_args_function(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            &volatile_global_int, &volatile_global_double
        );
        
        // Final computation mixing everything
        double final_result = 
            complex_result * 0.5 + 
            func_result * 0.3 +
            (double)i10 * 0.1 +
            (double)l10 * 0.05 +
            (double)f10 * 0.03 +
            d10 * 0.02;
        
        // Store with complex addressing
        output[write_idx] = final_result;
        
        // Update volatile variables to force memory writes
        vi1 = i10;
        vd1 = final_result;
        
        // Update indices for next iteration
        idx1 = (idx1 + i1) % 8;
        idx2 = (idx2 + i2) % 8;
        idx3 = (idx3 + i3) % 8;
        idx4 = (idx4 + i4) % 8;
    }
    
    // Force use of all local variables to prevent optimization
    volatile_global_int = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    volatile_global_double = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    volatile_global_float = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
}

int main(void)
{
    // Allocate and initialize arrays
    double* input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = sin((double)i * 0.1) * 100.0 + (double)(i % 37);
    }
    
    // Perform heavy computation
    compute_heavy(input, output, ARRAY_SIZE, 17);
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        // Mix in some volatile accesses
        checksum += volatile_global_double * 0.001;
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Volatile globals: int=%d, double=%f, float=%f\n",
           volatile_global_int, volatile_global_double, volatile_global_float);
    
    free(input);
    free(output);
    
    return 0;
}
