#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

// Volatile globals to force memory operations
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

// Complex structure for varied access patterns
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    char* p;
};

// Argument-heavy helper function - forces register/stack pressure
__attribute__((noinline))
static double heavy_args_func(
    int a1, long a2, float a3, double a4,
    int a5, long a6, float a7, double a8,
    int a9, long a10, float a11, double a12,
    int a13, long a14, float a15, double a16,
    void* ptr1, void* ptr2, struct MixedData* md
) {
    // Complex computation mixing all arguments
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    
    if (ptr1 != ptr2) {
        sum *= 1.01;
    }
    
    if (md) {
        sum += md->d + (double)md->f + (double)md->i + (double)md->l;
    }
    
    return sum;
}

// Main computation function with extreme register pressure
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(double* input, double* output, int size, int stride) {
    // Declare MANY local variables to create register pressure
    // Integer variables
    volatile int v1 = global_volatile_int;
    int i1 = v1 * 2;
    int i2 = i1 + 1;
    int i3 = i2 * 3;
    int i4 = i3 - 7;
    int i5 = i4 / 2;
    int i6 = i5 * 11;
    int i7 = i6 % 13;
    int i8 = i7 << 2;
    int i9 = i8 >> 1;
    int i10 = i9 | 0xFF;
    
    // Long variables
    long l1 = (long)i1 * 1000;
    long l2 = l1 + 500;
    long l3 = l2 * 3;
    long l4 = l3 - 777;
    long l5 = l4 / 5;
    long l6 = l5 * 17;
    long l7 = l6 % 23;
    long l8 = l7 << 3;
    long l9 = l8 >> 2;
    long l10 = l9 & 0xFFFF;
    
    // Float variables
    volatile float vf = global_volatile_float;
    float f1 = vf * 1.1f;
    float f2 = f1 + 2.2f;
    float f3 = f2 * 3.3f;
    float f4 = f3 - 4.4f;
    float f5 = f4 / 5.5f;
    float f6 = f5 * 6.6f;
    float f7 = f6 + 7.7f;
    float f8 = f7 * 8.8f;
    float f9 = f8 - 9.9f;
    float f10 = f9 / 10.1f;
    
    // Double variables
    volatile double vd = global_volatile_double;
    double d1 = vd * 1.111;
    double d2 = d1 + 2.222;
    double d3 = d2 * 3.333;
    double d4 = d3 - 4.444;
    double d5 = d4 / 5.555;
    double d6 = d5 * 6.666;
    double d7 = d6 + 7.777;
    double d8 = d7 * 8.888;
    double d9 = d8 - 9.999;
    double d10 = d9 / 10.111;
    
    // Pointer variables
    double* p1 = input;
    double* p2 = output;
    int* p3 = (int*)input;
    float* p4 = (float*)output;
    
    // Structure for complex access
    struct MixedData md = {i1, l1, f1, d1, (char*)input};
    
    double result = 0.0;
    
    // Complex loop with data dependencies and mixed operations
    for (int idx = 0; idx < size; idx += stride) {
        // Complex array indexing with multiple terms
        int complex_idx = (idx * stride + i1 + i2 - i3) % size;
        complex_idx = (complex_idx < 0) ? -complex_idx : complex_idx;
        
        // Load with complex addressing
        double load_val = input[complex_idx];
        
        // Long chain of mixed-type computations
        // Integer operations
        i1 = i2 + i3;
        i2 = i3 * i4 - i5;
        i3 = i4 / (i6 + 1) + i7;
        i4 = i5 << (i8 % 4);
        i5 = i6 >> (i9 & 3);
        
        // Long operations
        l1 = l2 + l3;
        l2 = l3 * l4 - l5;
        l3 = l4 / (l6 + 1) + l7;
        
        // Float operations with type conversions
        f1 = (float)i1 * f2 + (float)l1;
        f2 = f3 - (float)i2 * 0.5f;
        f3 = f4 / ((float)i3 + 1.0f);
        f4 = (float)l2 * f5;
        
        // Double operations with type conversions
        d1 = (double)i1 * d2 + (double)l1;
        d2 = d3 - (double)i2 * 0.5;
        d3 = d4 / ((double)i3 + 1.0);
        d4 = (double)l2 * d5;
        
        // Mixed float/double operations
        d5 = (double)f1 * d6 + d7;
        d6 = (double)f2 * d7 - d8;
        f5 = (float)d1 * f6 + f7;
        f6 = (float)d2 * f7 - f8;
        
        // More complex chains
        d7 = d8 * d9 + (double)(i4 * i5) - (double)(l3 % 100);
        d8 = d9 / d10 * (double)(f3 * f4);
        d9 = sin(d1) * cos(d2) + tan((double)f5);
        d10 = exp(d3) * log(fabs(d4) + 1.0);
        
        // Inline assembly that clobbers many registers
        // This forces spills and reloads
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
        
        // Call argument-heavy function with mixed types
        // This forces register/stack pressure for argument passing
        double func_result = heavy_args_func(
            i1, l1, f1, d1,
            i2, l2, f2, d2,
            i3, l3, f3, d3,
            i4, l4, f4, d4,
            (void*)p1, (void*)p2, &md
        );
        
        // Use the result in further computation
        d10 += func_result * 0.01;
        
        // Complex store with addressing
        int store_idx = (idx + i1 + i2 * 2 - i3 / 3) % size;
        store_idx = (store_idx < 0) ? -store_idx : store_idx;
        
        // Final computation mixing all types
        double final_val = load_val * d1 
                         + (double)i1 * d2 
                         + (double)l1 * d3 
                         + (double)f1 * d4
                         + (double)f2 * d5
                         + (double)i2 * d6
                         + (double)l2 * d7
                         + (double)f3 * d8
                         + (double)i3 * d9
                         + (double)l3 * d10;
        
        // Store with volatile write to force memory operation
        volatile double* volatile_store = &output[store_idx];
        *volatile_store = final_val;
        
        // Accumulate result
        result += final_val;
        
        // Update structure with new values
        md.i = i1;
        md.l = l1;
        md.f = f1;
        md.d = d1;
    }
    
    return result;
}

int main() {
    // Allocate and initialize arrays
    double* input = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = sin((double)i * 0.1) * 100.0;
        output[i] = 0.0;
    }
    
    double total_result = 0.0;
    
    // Multiple iterations to increase pressure
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Vary stride to create different access patterns
        int stride = (iter % 7) + 1;
        
        double iter_result = compute_heavy(input, output, ARRAY_SIZE, stride);
        total_result += iter_result;
        
        // Swap buffers to create different data flow
        double* temp = input;
        input = output;
        output = temp;
    }
    
    // Compute checksum to prevent dead code elimination
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
    }
    
    printf("Total result: %f\n", total_result);
    printf("Checksum: %f\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
