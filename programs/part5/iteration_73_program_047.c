/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile global variables to force memory operations */
volatile int volatile_global_int = 42;
volatile double volatile_global_double = 3.14159;
volatile float volatile_global_float = 2.71828f;

/* Complex structure for diverse memory accesses */
struct ComplexData {
    int ints[16];
    double doubles[8];
    float floats[12];
    long longs[4];
    char padding[64];
};

/* Argument-heavy helper function - noinline to prevent optimization */
__attribute__((noinline))
static double heavy_callee(
    int a1, int a2, int a3, int a4,
    double b1, double b2, double b3, double b4,
    float c1, float c2, float c3, float c4,
    long d1, long d2, void* ptr1, void* ptr2
) {
    /* Mix all argument types in computation */
    double result = (double)a1 * b1 + (double)a2 * b2;
    result += (double)c1 * (double)c2;
    result += (double)d1 / (double)d2;
    result += *(double*)ptr1 + *(float*)ptr2;
    
    /* Force memory access */
    volatile_global_int = a1 + a2;
    volatile_global_double = result;
    
    return result * 0.5;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
static double compute_heavy(struct ComplexData* input, 
                           struct ComplexData* output, 
                           int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int i1 = 10, i2 = 20, i3 = 30, i4 = 40, i5 = 50;
    int i6 = 60, i7 = 70, i8 = 80, i9 = 90, i10 = 100;
    int i11 = 110, i12 = 120, i13 = 130, i14 = 140, i15 = 150;
    
    /* Long variables */
    long l1 = 1000L, l2 = 2000L, l3 = 3000L, l4 = 4000L;
    long l5 = 5000L, l6 = 6000L, l7 = 7000L, l8 = 8000L;
    
    /* Float variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double variables */
    double d1 = 1.111, d2 = 2.222, d3 = 3.333, d4 = 4.444;
    double d5 = 5.555, d6 = 6.666, d7 = 7.777, d8 = 8.888;
    double d9 = 9.999, d10 = 10.1010, d11 = 11.1111, d12 = 12.1212;
    
    /* Pointer variables */
    int* p1 = &i1;
    float* p2 = &f1;
    double* p3 = &d1;
    volatile int* p4 = &volatile_global_int;
    
    /* Index variables for complex array access */
    int idx1 = 0, idx2 = 2, idx3 = 4, idx4 = 8;
    int stride1 = 3, stride2 = 5, stride3 = 7;
    
    double total = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        int array_idx = idx1 * stride1 + idx2 * stride2 + idx3 * stride3 + idx4 + iter;
        array_idx = array_idx % 16;
        
        /* Load from input with complex addressing */
        i1 = input->ints[array_idx];
        i2 = input->ints[(array_idx * 2) % 16];
        i3 = input->ints[(array_idx + 5) % 16];
        
        f1 = input->floats[array_idx % 12];
        f2 = input->floats[(array_idx + 3) % 12];
        
        d1 = input->doubles[array_idx % 8];
        d2 = input->doubles[(array_idx + 2) % 8];
        
        /* Long chain of mixed-type computations with data dependencies */
        /* This prevents register reuse */
        d3 = (double)i1 * d1 + (double)i2 * d2;
        f3 = (float)d3 * f1 + f2 * 2.0f;
        i4 = (int)f3 + i3 * 3;
        d4 = (double)i4 / 17.0 + d3 * 0.5;
        f4 = (float)d4 * 3.14f;
        i5 = i4 + (int)f4;
        d5 = d4 * 2.0 - (double)i5 / 11.0;
        
        l1 = (long)i5 * 1000L;
        d6 = (double)l1 / 1234.0 + d5;
        f5 = (float)d6 * 2.71828f;
        i6 = (int)(f5 * 100.0f);
        
        d7 = (double)i6 * 0.01 + d6;
        f6 = f5 + (float)d7;
        i7 = i6 + (int)f6;
        d8 = d7 * 1.1 - (double)i7 * 0.001;
        
        /* More computations to use all variables */
        f7 = f3 + f4 + f5 + f6;
        d9 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
        i8 = i1 + i2 + i3 + i4 + i5 + i6 + i7;
        l2 = l1 + (long)i8;
        
        f8 = (float)l2 * 0.0001f;
        d10 = (double)f8 * 10000.0;
        i9 = (int)d10;
        
        /* Use volatile variables in computation */
        i10 = i9 + volatile_global_int;
        d11 = d10 + volatile_global_double;
        f9 = f8 + volatile_global_float;
        
        /* Inline assembly that clobbers many registers */
        /* This forces spills and reloads */
#if defined(__aarch64__)
        __asm__ volatile(
            "mov x0, %0\n"
            "mov x1, %1\n"
            "mov x2, %2\n"
            "mov x3, %3\n"
            "fmov d0, %4\n"
            "fmov d1, %5\n"
            "fmov d2, %6\n"
            "fmov d3, %7\n"
            "fmov s4, %8\n"
            "fmov s5, %9\n"
            : 
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
              "r"(d1), "r"(d2), "r"(d3), "r"(d4),
              "r"(f1), "r"(f2)
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
              "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",
              "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23",
              "s24", "s25", "s26", "s27", "s28", "s29", "s30", "s31",
              "memory"
        );
#elif defined(__x86_64__)
        __asm__ volatile(
            "mov %0, %%rax\n"
            "mov %1, %%rbx\n"
            "mov %2, %%rcx\n"
            "mov %3, %%rdx\n"
            "movq %4, %%xmm0\n"
            "movq %5, %%xmm1\n"
            "movq %6, %%xmm2\n"
            "movq %7, %%xmm3\n"
            : 
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4),
              "r"(d1), "r"(d2), "r"(d3), "r"(d4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
#endif
        
        /* Call argument-heavy function with mixed types */
        /* This creates pressure on argument-passing registers */
        d12 = heavy_callee(
            i1, i2, i3, i4,
            d1, d2, d3, d4,
            f1, f2, f3, f4,
            l1, l2, (void*)&i5, (void*)&f5
        );
        
        /* More computations after call */
        f10 = (float)d12 * 0.5f;
        i11 = (int)(f10 * 1000.0f);
        l3 = l2 + (long)i11;
        d11 = d12 + (double)l3 * 0.00001;
        
        /* Store results with complex addressing */
        int store_idx = (array_idx + 7) % 16;
        output->ints[store_idx] = i11;
        output->floats[store_idx % 12] = f10;
        output->doubles[store_idx % 8] = d11;
        output->longs[store_idx % 4] = l3;
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 4;
        idx2 = (idx2 + 2) % 6;
        idx3 = (idx3 + 3) % 8;
        idx4 = (idx4 + 1) % 4;
        
        /* Accumulate total for checksum */
        total += d11 + (double)f10 + (double)i11 + (double)l3;
        
        /* Force all variables to be used to prevent optimization */
        volatile int sink = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11;
        (void)sink;
    }
    
    return total;
}

int main(void) {
    /* Initialize data structures */
    struct ComplexData input_data;
    struct ComplexData output_data;
    
    /* Fill input with pseudo-random data */
    srand(42);
    for (int i = 0; i < 16; i++) {
        input_data.ints[i] = rand() % 1000;
    }
    for (int i = 0; i < 12; i++) {
        input_data.floats[i] = (float)rand() / (float)RAND_MAX * 100.0f;
    }
    for (int i = 0; i < 8; i++) {
        input_data.doubles[i] = (double)rand() / (double)RAND_MAX * 1000.0;
    }
    for (int i = 0; i < 4; i++) {
        input_data.longs[i] = (long)rand() * 1000L;
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(&input_data, &output_data, ITERATIONS);
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = result;
    for (int i = 0; i < 16; i++) {
        checksum += (double)output_data.ints[i];
    }
    for (int i = 0; i < 12; i++) {
        checksum += (double)output_data.floats[i];
    }
    
    printf("Checksum: %.15f\n", checksum);
    
    return 0;
}
