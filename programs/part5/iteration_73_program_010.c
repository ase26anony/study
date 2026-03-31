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

// Complex structure for varied memory access
struct ComplexData {
    int i;
    long l;
    float f;
    double d;
    int arr[8];
};

// Argument-heavy function - forces register/stack pressure
__attribute__((noinline))
double heavy_args_func(int a1, int a2, int a3, int a4, int a5, int a6,
                       float f1, float f2, float f3, float f4,
                       double d1, double d2, double d3, double d4,
                       long l1, long l2, void* ptr1, void* ptr2) {
    // Complex computation mixing all types
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += *(double*)ptr1 + *(float*)ptr2;
    
    // Force memory access
    result += global_volatile_double;
    result += (double)global_volatile_float;
    
    return result * (double)a3 - (double)a4 + d3 - d4;
}

// Main computation function with extreme register pressure
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(struct ComplexData* input, struct ComplexData* output, int size) {
    // Declare MANY local variables to create register pressure
    // Integer variables
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    // Long variables
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    // Float variables
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    // Double variables
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    // Pointer variables
    int* p1; float* p2; double* p3; long* p4;
    
    // Volatile locals to prevent optimization
    volatile int vi1 = 1, vi2 = 2, vi3 = 3;
    volatile float vf1 = 1.0f, vf2 = 2.0f;
    volatile double vd1 = 1.0, vd2 = 2.0;
    
    // Initialize with some values
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    l1 = 1000L; l2 = 2000L; l3 = 3000L;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f;
    d1 = 1.11; d2 = 2.22; d3 = 3.33; d4 = 4.44;
    
    // Complex loop with data dependencies
    for (int idx = 0; idx < size; idx++) {
        // Complex array indexing with multiple terms
        int stride = 16;
        int offset1 = (idx * stride + idx / 2 + idx % 3) % size;
        int offset2 = (idx * 7 + idx / 3 + idx % 5) % size;
        int offset3 = (idx * 11 + idx / 4 + idx % 7) % size;
        
        // Load from input with complex addressing
        i1 = input[offset1].i + input[offset2].arr[idx % 8];
        i2 = input[offset2].i * 2 - input[offset3].arr[(idx + 1) % 8];
        i3 = i1 + i2 + idx;
        i4 = i2 * 3 - i1;
        i5 = i3 + i4 + input[offset1].arr[idx % 8];
        
        // Mix integer and floating point operations
        f1 = (float)i1 * 1.5f + (float)i2 * 0.5f;
        f2 = (float)i3 * 2.0f - (float)i4 * 1.0f;
        f3 = f1 + f2 + (float)input[offset1].f;
        f4 = f2 * 3.0f - f1 + (float)input[offset2].f;
        
        // More floating point operations
        d1 = (double)f1 * 1.25 + (double)f2 * 0.75;
        d2 = (double)f3 * 2.5 - (double)f4 * 1.5;
        d3 = d1 + d2 + input[offset1].d;
        d4 = d2 * 3.0 - d1 + input[offset2].d;
        
        // Long integer operations
        l1 = (long)i1 * 1000L + (long)i2 * 500L;
        l2 = (long)i3 * 2000L - (long)i4 * 1000L;
        l3 = l1 + l2 + input[offset1].l;
        l4 = l2 * 3L - l1 + input[offset2].l;
        
        // Chain of dependent operations - prevents register reuse
        i6 = i5 + (int)f1 + (int)d1;
        i7 = i6 * 2 - (int)f2 + (int)d2;
        i8 = i7 + i3 + (int)(f3 * 2.0f);
        i9 = i8 - i4 + (int)(d3 * 0.5);
        i10 = i9 + (int)l1 % 100 + (int)l2 % 50;
        
        f5 = (float)i6 * 0.1f + (float)i7 * 0.2f;
        f6 = f5 + (float)i8 * 0.3f - (float)i9 * 0.4f;
        f7 = f6 * 2.0f + (float)i10;
        f8 = f7 - f3 + f4 * 0.5f;
        
        d5 = (double)i6 * 0.01 + (double)i7 * 0.02;
        d6 = d5 + (double)i8 * 0.03 - (double)i9 * 0.04;
        d7 = d6 * 2.0 + (double)i10;
        d8 = d7 - d3 + d4 * 0.5;
        
        l5 = (long)i6 * 10L + (long)i7 * 20L;
        l6 = l5 + (long)i8 * 30L - (long)i9 * 40L;
        l7 = l6 * 2L + (long)i10;
        l8 = l7 - l3 + l4 * 5L;
        
        // More chained operations
        for (int j = 0; j < 4; j++) {
            i11 = i10 + j;
            f9 = f8 + (float)j * 0.1f;
            d9 = d8 + (double)j * 0.01;
            l9 = l8 + (long)j * 100L;
            
            i12 = i11 * (j + 1) + (int)f9;
            f10 = f9 * (float)(j + 1) + (float)i12;
            d10 = d9 * (double)(j + 1) + (double)i12;
            l10 = l9 * (long)(j + 1) + (long)i12;
            
            // Use volatile variables in computation
            i12 += vi1 + vi2 + vi3;
            f10 += vf1 + vf2;
            d10 += vd1 + vd2;
            
            // Complex addressing within nested loop
            int complex_idx = (idx * 13 + j * 7 + i12 % 5) % size;
            i13 = input[complex_idx].arr[j] + i12;
            f11 = input[complex_idx].f + f10;
            d11 = input[complex_idx].d + d10;
            l11 = input[complex_idx].l + l10;
            
            // Store intermediate results
            output[complex_idx].arr[j] = i13;
            output[complex_idx].f = f11;
            output[complex_idx].d = d11;
            output[complex_idx].l = l11;
        }
        
        // Inline assembly that clobbers MANY registers
        // This forces spills and reloads
        __asm__ volatile (
#if defined(__aarch64__)
            // Clobber all general purpose registers
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
            // Clobber floating point registers
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
              "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9",
              "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19",
              "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29",
              "d30", "d31", "memory"
#elif defined(__x86_64__)
            // Clobber all x86_64 registers
            "xor %%rax, %%rax\n"
            "xor %%rbx, %%rbx\n"
            "xor %%rcx, %%rcx\n"
            "xor %%rdx, %%rdx\n"
            "xor %%rsi, %%rsi\n"
            "xor %%rdi, %%rdi\n"
            "xor %%r8, %%r8\n"
            "xor %%r9, %%r9\n"
            "xor %%r10, %%r10\n"
            "xor %%r11, %%r11\n"
            "xor %%r12, %%r12\n"
            "xor %%r13, %%r13\n"
            "xor %%r14, %%r14\n"
            "xor %%r15, %%r15\n"
            // Clobber floating point registers
            "fldz\n"
            "fldz\n"
            "fldz\n"
            "fldz\n"
            "fldz\n"
            "fldz\n"
            "fldz\n"
            "fldz\n"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15", "st", "st(1)", "st(2)", "st(3)",
              "st(4)", "st(5)", "st(6)", "st(7)", "memory"
#endif
        );
        
        // Call argument-heavy function with mixed types
        // This creates pressure on calling convention handling
        double func_result = heavy_args_func(
            i1, i2, i3, i4, i5, i6,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2,
            (void*)&input[offset1], (void*)&input[offset2]
        );
        
        // Use function result in further computation
        d9 = d8 + func_result * 0.5;
        f9 = f8 + (float)func_result;
        i14 = i13 + (int)func_result;
        l12 = l11 + (long)func_result;
        
        // Final store with complex addressing
        int final_idx = (idx * 17 + i14 % 13 + (int)f9 % 7) % size;
        output[final_idx].i = i14;
        output[final_idx].f = f9;
        output[final_idx].d = d9;
        output[final_idx].l = l12;
        
        // Store to all array elements
        for (int k = 0; k < 8; k++) {
            output[final_idx].arr[k] = i14 + k * 2 + (int)(f9 * k) + (int)(d9 * k);
        }
        
        // Update volatile globals
        global_volatile_int = i14;
        global_volatile_float = f9;
        global_volatile_double = d9;
    }
}

int main() {
    // Allocate and initialize arrays
    struct ComplexData* input = (struct ComplexData*)malloc(ARRAY_SIZE * sizeof(struct ComplexData));
    struct ComplexData* output = (struct ComplexData*)malloc(ARRAY_SIZE * sizeof(struct ComplexData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pseudo-random data
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].i = rand() % 1000;
        input[i].l = rand() % 10000;
        input[i].f = (float)rand() / (float)RAND_MAX * 100.0f;
        input[i].d = (double)rand() / (double)RAND_MAX * 1000.0;
        for (int j = 0; j < 8; j++) {
            input[i].arr[j] = rand() % 100;
        }
    }
    
    // Perform heavy computation multiple times
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input, output, ARRAY_SIZE);
        
        // Swap buffers to create data dependencies between iterations
        struct ComplexData* temp = input;
        input = output;
        output = temp;
    }
    
    // Compute checksum to prevent dead code elimination
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].i;
        checksum += output[i].l;
        checksum += (long)output[i].f;
        checksum += (long)output[i].d;
        for (int j = 0; j < 8; j++) {
            checksum += output[i].arr[j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
