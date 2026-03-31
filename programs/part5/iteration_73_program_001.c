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

// Complex structure for varied memory accesses
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    int arr[8];
    struct MixedData* next;
};

// Argument-heavy helper function - force register pressure for arguments
__attribute__((noinline))
double heavy_args_func(int a1, long a2, float a3, double a4,
                       int a5, long a6, float a7, double a8,
                       int a9, long a10, float a11, double a12,
                       int a13, long a14, float a15, double a16) {
    // Mix all arguments in complex ways
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    
    // Force memory barrier
    asm volatile("" ::: "memory");
    
    return sum * 0.5;
}

// Main computation function with extreme register pressure
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(struct MixedData* input, struct MixedData* output, int size) {
    // Declare MANY local variables to create register pressure
    // Integer variables
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long l1 = 10, l2 = 20, l3 = 30, l4 = 40, l5 = 50;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile long l6 = 60, l7 = 70, l8 = 80, l9 = 90, l10 = 100;
    
    // Floating point variables
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d6 = 6.66, d7 = 7.77, d8 = 8.88, d9 = 9.99, d10 = 10.1010;
    
    // More variables for additional pressure
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    long l11 = 110, l12 = 120, l13 = 130, l14 = 140, l15 = 150;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    double d11 = 11.111, d12 = 12.122, d13 = 13.133, d14 = 14.144, d15 = 15.155;
    
    // Pointer variables
    int* p1 = &v1;
    long* p2 = &l1;
    float* p3 = &f1;
    double* p4 = &d1;
    
    // Loop with complex addressing and mixed operations
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Complex array indexing with multiple terms
        int idx1 = (iter * 7 + v1) % size;
        int idx2 = (iter * 13 + v2) % size;
        int idx3 = (iter * 17 + v3) % size;
        int idx4 = (iter * 23 + v4) % size;
        
        // Load from input with complex addressing
        struct MixedData* in1 = &input[idx1 * 2 + idx2 % 2];
        struct MixedData* in2 = &input[idx3 * 3 + idx4 % 3];
        
        // Long chain of mixed-type computations with data dependencies
        // This prevents register reuse
        f1 = (float)in1->i * 0.5f + f2;
        d1 = (double)in1->l * 0.25 + d2;
        v1 = (int)(f1 * 100.0f) + in1->arr[0];
        l1 = (long)(d1 * 1000.0) + in1->arr[1];
        
        f2 = f1 * 1.1f + (float)in2->i * 0.3f;
        d2 = d1 * 1.01 + (double)in2->l * 0.7;
        v2 = v1 * 2 + (int)(f2 * 50.0f);
        l2 = l1 / 3 + (long)(d2 * 200.0);
        
        // More mixed operations
        f3 = (float)v1 * 0.25f + (float)l1 * 0.0001f;
        d3 = (double)v2 * 0.125 + (double)l2 * 0.00001;
        v3 = (int)f3 * 3 + (int)d3;
        l3 = (long)(f3 * 1000.0f) + (long)(d3 * 10000.0);
        
        // Type conversions that require different register classes
        f4 = (float)((double)f3 + d3);
        d4 = (double)((float)d2 + f2);
        v4 = (int)f4 + (int)d4;
        l4 = (long)f4 * (long)d4;
        
        // Complex addressing for array access
        int arr_idx = (v1 * v2 + v3 * v4 - l1 + l2) % 8;
        f5 = in1->arr[arr_idx] * 0.01f + in2->arr[(arr_idx + 1) % 8] * 0.02f;
        d5 = (double)in1->arr[(arr_idx + 2) % 8] * 0.001 + 
             (double)in2->arr[(arr_idx + 3) % 8] * 0.002;
        
        // Inline assembly that clobbers MANY registers
        // This forces spills and reloads
#if defined(__x86_64__)
        asm volatile(
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
        asm volatile(
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
            "mov x29, #0\n"
            "mov x30, #0\n"
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
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
              "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
              "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9",
              "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19",
              "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory"
        );
#endif
        
        // Call argument-heavy function with mixed types
        // This forces register allocation for argument passing
        double result = heavy_args_func(
            v1, l1, f1, d1,
            v2, l2, f2, d2,
            v3, l3, f3, d3,
            v4, l4, f4, d4
        );
        
        // Use the result in further computations
        f6 = (float)result * 0.5f + f5;
        d6 = result * 0.25 + d5;
        v5 = (int)(f6 * 100.0f) + (int)(d6 * 50.0);
        
        // Store to output with complex addressing
        int out_idx = (iter * 11 + v5) % size;
        struct MixedData* out = &output[out_idx];
        
        out->i = v1 + v2 + v3 + v4 + v5;
        out->l = l1 + l2 + l3 + l4;
        out->f = f1 + f2 + f3 + f4 + f5 + f6;
        out->d = d1 + d2 + d3 + d4 + d5 + d6;
        
        // Complex array store with computed index
        int store_idx = (v1 * 3 + v2 * 5 + v3 * 7) % 8;
        out->arr[store_idx] = v5;
        out->arr[(store_idx + 1) % 8] = (int)f6;
        out->arr[(store_idx + 2) % 8] = (int)d6;
        
        // Use volatile globals to force memory operations
        v6 = global_volatile_int + v1;
        f7 = global_volatile_float * f1;
        d7 = global_volatile_double * d1;
        
        // More mixed operations to keep values live
        f8 = (float)v6 * 0.1f + f7;
        d8 = (double)l1 * 0.01 + d7;
        v7 = (int)(f8 * 10.0f) + (int)(d8 * 5.0);
        
        // Chain dependencies to next iteration
        v1 = v7 % 100;
        l1 = (l1 + v7) % 1000;
        f1 = f8 * 0.9f;
        d1 = d8 * 0.99;
    }
    
    // Volatile sink to prevent elimination
    volatile int sink __attribute__((unused)) = v1 + v2 + v3 + v4 + v5;
}

int main() {
    // Allocate and initialize data
    struct MixedData* input = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    struct MixedData* output = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].i = rand() % 1000;
        input[i].l = rand() % 10000;
        input[i].f = (float)(rand() % 1000) * 0.1f;
        input[i].d = (double)(rand() % 1000) * 0.01;
        
        for (int j = 0; j < 8; j++) {
            input[i].arr[j] = rand() % 100;
        }
        
        input[i].next = (i < ARRAY_SIZE - 1) ? &input[i + 1] : NULL;
    }
    
    // Perform heavy computation
    compute_heavy(input, output, ARRAY_SIZE);
    
    // Compute checksum to prevent dead code elimination
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].i;
        checksum += output[i].l;
        checksum += (long)(output[i].f * 100.0f);
        checksum += (long)(output[i].d * 1000.0);
        
        for (int j = 0; j < 8; j++) {
            checksum += output[i].arr[j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    // Cleanup
    free(input);
    free(output);
    
    return 0;
}
