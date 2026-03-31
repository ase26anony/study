#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for varied memory accesses */
struct DataBlock {
    int ints[16];
    float floats[16];
    double doubles[8];
    long longs[8];
};

/* Argument-heavy helper function - noinline to prevent inlining */
__attribute__((noinline))
static double heavy_args_func(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* ptr1, void* ptr2
) {
    /* Complex computation mixing all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((uintptr_t)ptr1 % 1000);
    result += (double)((uintptr_t)ptr2 % 1000);
    
    /* Force memory access */
    result += global_volatile_double;
    
    return result * (d3 + d4) / (double)(a3 + a4);
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(struct DataBlock* input, struct DataBlock* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Pointer/index variables */
    int idx1, idx2, idx3, idx4, idx5;
    volatile double sink = 0.0; /* Volatile sink to prevent optimization */
    
    /* Initialize from input structure */
    i1 = input->ints[0]; i2 = input->ints[1]; i3 = input->ints[2]; i4 = input->ints[3];
    f1 = input->floats[0]; f2 = input->floats[1]; f3 = input->floats[2]; f4 = input->floats[3];
    d1 = input->doubles[0]; d2 = input->doubles[1]; d3 = input->doubles[2]; d4 = input->doubles[3];
    l1 = input->longs[0]; l2 = input->longs[1];
    
    /* Main computation loop with unbroken dependency chain */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % 16;
        idx2 = (iter * 3 + 5) % 16;
        idx3 = (iter * 11 + 7) % 16;
        idx4 = (iter * 13 + 11) % 8;
        idx5 = (iter * 17 + 13) % 8;
        
        /* Load more values with complex addressing */
        i5 = input->ints[idx1];
        i6 = input->ints[idx2];
        f5 = input->floats[idx3];
        f6 = input->floats[(idx1 + idx2) % 16];
        d5 = input->doubles[idx4];
        d6 = input->doubles[idx5];
        l3 = input->longs[idx4];
        l4 = input->longs[idx5];
        
        /* EXTREME register pressure computation with mixed types */
        /* Chain 1: Integer to float to double conversions */
        f7 = (float)i1 * f1 + (float)i2 * f2;
        d7 = (double)f7 * d1 + (double)f3 * d2;
        i7 = (int)d7 + i3 * i4;
        
        /* Chain 2: More mixed operations */
        f8 = (float)i5 * 1.5f + f4;
        d8 = d3 * (double)f8 + d4 * (double)f5;
        i8 = (int)(d8 * 100.0) + i6;
        
        /* Chain 3: Long and double interactions */
        d9 = (double)l1 * d5 + (double)l2 * d6;
        l5 = (long)(d9 * 1000.0) + l3 * l4;
        f9 = (float)l5 * 0.001f + f6;
        
        /* Chain 4: Complex floating point */
        d10 = sin(d7) * cos(d8) + tan(d9 * 0.1);
        f10 = (float)d10 * expf(f9);
        i9 = (int)(f10 * 100.0f) + i7 + i8;
        
        /* Chain 5: More dependencies */
        f11 = (float)i9 * 0.01f + f7;
        d11 = (double)f11 * d10 + d7 * d8;
        l6 = (long)(d11 * 10000.0) + l5;
        
        /* Chain 6: Integer-heavy */
        i10 = i9 * 3 + i7 * 2 + i8;
        i11 = i10 % 1000 + i5 * i6;
        f12 = (float)i11 * 0.001f + f8;
        
        /* Chain 7: Double-heavy */
        d12 = d9 * d10 * d11;
        f13 = (float)d12 + f9 * f10;
        i12 = (int)(f13 * 1000.0f) + i10;
        
        /* Chain 8: Long conversions */
        l7 = l6 * 2 + (long)i12 * 3;
        d13 = (double)l7 * 0.0001 + d12;
        f14 = (float)d13 * 2.0f + f11;
        
        /* Chain 9: More mixed */
        i13 = (int)f14 + i11 * 2;
        f15 = (float)i13 * 0.5f + f12;
        d14 = (double)f15 * 3.0 + d13;
        l8 = (long)d14 + l7;
        
        /* Chain 10: Final complex chain */
        i14 = i12 + i13 * 3;
        f16 = (float)i14 + f13 * 2.0f;
        d15 = d14 * (double)f16 + global_volatile_double;
        l9 = (long)(d15 * 100.0) + l8;
        f17 = (float)l9 * 0.01f;
        
        /* Inline assembly that clobbers MANY registers */
        /* This forces spills and reloads */
        __asm__ volatile (
#if defined(__aarch64__)
            /* Clobber ARM64 registers */
            "mov x0, %0\n"
            "mov x1, %1\n"
            "add x0, x0, x1\n"
            : 
            : "r" (i14), "r" (l9)
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
#elif defined(__x86_64__)
            /* Clobber x86_64 registers */
            "mov %0, %%rax\n"
            "add %1, %%rax\n"
            : 
            : "r" (i14), "r" (l9)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
#else
            /* Generic clobber */
            : : : "memory"
#endif
        );
        
        /* Call argument-heavy function with mixed types */
        /* This creates pressure on argument-passing registers */
        d16 = heavy_args_func(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2,
            (void*)&input->ints[0],
            (void*)&input->floats[0]
        );
        
        /* More computation after call */
        f18 = (float)d16 + f17;
        i15 = (int)f18 + i14;
        d17 = (double)i15 * 0.01 + d15;
        
        /* Complex store with multiple index calculations */
        output->ints[(idx1 + idx2 + iter) % 16] = i15;
        output->floats[(idx3 * 2 + idx4) % 16] = f18;
        output->doubles[(idx4 + idx5 + iter) % 8] = d17;
        output->longs[(iter + idx1 + idx3) % 8] = l9;
        
        /* Volatile store to force memory operation */
        sink += d17 + (double)f18 + (double)i15 + (double)l9;
        
        /* Rotate values for next iteration (creates dependencies) */
        i1 = i15; i2 = i14; i3 = i13; i4 = i12;
        f1 = f18; f2 = f17; f3 = f16; f4 = f15;
        d1 = d17; d2 = d16; d3 = d15; d4 = d14;
        l1 = l9; l2 = l8;
    }
    
    return sink;
}

int main(void) {
    struct DataBlock input, output;
    double checksum = 0.0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < 16; i++) {
        input.ints[i] = rand() % 1000;
        input.floats[i] = (float)rand() / (float)RAND_MAX * 100.0f;
    }
    for (int i = 0; i < 8; i++) {
        input.doubles[i] = (double)rand() / (double)RAND_MAX * 100.0;
        input.longs[i] = (long)rand() * 1000L;
    }
    
    /* Perform heavy computation */
    checksum = compute_heavy(&input, &output, ITERATIONS);
    
    /* Additional volatile operations to prevent optimization */
    global_volatile_int = (int)checksum;
    global_volatile_float = (float)checksum;
    global_volatile_double = checksum;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %.15f\n", checksum);
    
    /* Use output to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += output.ints[i];
    }
    printf("Output int sum: %d\n", sum);
    
    return 0;
}
