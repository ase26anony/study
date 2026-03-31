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

/* Complex structure for memory addressing */
struct DataBlock {
    int ints[64];
    double doubles[64];
    float floats[64];
    long longs[64];
};

/* Argument-heavy helper function - forces register/stack pressure */
__attribute__((noinline))
double heavy_args_func(int a1, double b1, float c1, long d1,
                       int a2, double b2, float c2, long d2,
                       int a3, double b3, float c3, long d3,
                       int a4, double b4, float c4, long d4,
                       int a5, double b5, float c5, long d5,
                       int a6, double b6, float c6, long d6) {
    /* Complex mixing of types */
    double sum = (double)a1 * b1 + (double)c1 * (double)d1;
    sum += (double)a2 / b2 + (double)c2 * (double)d2;
    sum += (double)a3 * b3 - (double)c3 * (double)d3;
    sum += (double)a4 + b4 + (double)c4 * (double)d4;
    sum += (double)a5 - b5 + (double)c5 * (double)d5;
    sum += (double)a6 * b6 * (double)c6 * (double)d6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum += (double)global_volatile_int;
    sum += (double)global_volatile_float;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
void compute_heavy(struct DataBlock* input, struct DataBlock* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    volatile int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Long variables */
    volatile long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    volatile long l6 = 600, l7 = 700, l8 = 800, l9 = 900, l10 = 1000;
    
    /* Float variables */
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    
    /* Double variables */
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    volatile double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    
    /* Pointer variables */
    int* p1 = input->ints;
    double* p2 = input->doubles;
    float* p3 = input->floats;
    long* p4 = input->longs;
    
    /* Index variables for complex addressing */
    int i, j, k, m, n;
    
    for (i = 0; i < iterations; i++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (i * 3 + v1) % 64;
        int idx2 = (i * 5 + v2) % 64;
        int idx3 = (i * 7 + v3) % 64;
        int idx4 = (i * 11 + v4) % 64;
        int idx5 = (i * 13 + v5) % 64;
        
        /* Load data with complex addressing */
        int load1 = input->ints[idx1 * 2 + v6];
        double load2 = input->doubles[idx2 * 3 + v7];
        float load3 = input->floats[idx3 * 4 + v8];
        long load4 = input->longs[idx4 * 5 + v9];
        
        /* Long chain of mixed-type computations with data dependencies */
        /* This prevents register reuse */
        f1 = (float)load1 * f2 + (float)d1;
        d1 = (double)f1 * d2 + (double)load2;
        v1 = (int)d1 + v2 * load1;
        l1 = (long)v1 * l2 + load4;
        f2 = (float)l1 / f3 + (float)load3;
        d2 = d1 * d3 + (double)f2;
        v2 = v1 + v3 * (int)f2;
        l2 = l1 + l3 * (long)d2;
        f3 = f1 + f4 * (float)v2;
        d3 = d2 - d4 * (double)l2;
        v3 = v2 - v4 * (int)f3;
        l3 = l2 - l4 * (long)d3;
        f4 = f3 * f5 + (float)v3;
        d4 = d3 / d5 + (double)l3;
        v4 = v3 / v5 + (int)f4;
        l4 = l3 / l5 + (long)d4;
        f5 = f4 - f6 * (float)v4;
        d5 = d4 - d6 * (double)l4;
        v5 = v4 - v6 * (int)f5;
        l5 = l4 - l6 * (long)d5;
        f6 = f5 * f7 + (float)v5;
        d6 = d5 * d7 + (double)l5;
        v6 = v5 * v7 + (int)f6;
        l6 = l5 * l7 + (long)d6;
        f7 = f6 / f8 + (float)v6;
        d7 = d6 / d8 + (double)l6;
        v7 = v6 / v8 + (int)f7;
        l7 = l6 / l8 + (long)d7;
        
        /* Insert inline assembly that clobbers many registers */
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
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15",
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
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
              "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
              "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9",
              "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19",
              "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29",
              "d30", "d31", "memory"
        );
#endif
        
        /* Continue computation after assembly clobber */
        f8 = f7 * f9 + (float)v7;
        d8 = d7 * d9 + (double)l7;
        v8 = v7 * v9 + (int)f8;
        l8 = l7 * l9 + (long)d8;
        f9 = f8 / f10 + (float)v8;
        d9 = d8 / d10 + (double)l8;
        v9 = v8 / v10 + (int)f9;
        l9 = l8 / l10 + (long)d9;
        
        /* Call argument-heavy function - forces argument passing reloads */
        double func_result = heavy_args_func(
            v1, d1, f1, l1,
            v2, d2, f2, l2,
            v3, d3, f3, l3,
            v4, d4, f4, l4,
            v5, d5, f5, l5,
            v6, d6, f6, l6
        );
        
        /* More mixed-type operations with function result */
        f10 = (float)func_result * f1 + f2;
        d10 = func_result * d1 + d2;
        v10 = (int)func_result + v1 * v2;
        l10 = (long)func_result + l1 * l2;
        
        /* Complex store with addressing */
        int store_idx = (idx1 + idx2 + idx3 + idx4) % 64;
        output->ints[store_idx * 3 + v10 % 8] = v10;
        output->doubles[store_idx * 2 + (v10 / 2) % 8] = d10;
        output->floats[store_idx * 4 + (v10 / 4) % 8] = f10;
        output->longs[store_idx * 5 + (v10 / 5) % 8] = l10;
        
        /* Use volatile globals in computation */
        v1 += global_volatile_int;
        d1 += global_volatile_double;
        f1 += global_volatile_float;
    }
    
    /* Volatile sink to prevent elimination */
    volatile double sink = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    (void)sink;
}

int main() {
    /* Initialize data structures */
    struct DataBlock input, output;
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < 64; i++) {
        input.ints[i] = rand() % 1000;
        input.doubles[i] = (double)rand() / RAND_MAX * 100.0;
        input.floats[i] = (float)rand() / RAND_MAX * 100.0f;
        input.longs[i] = (long)rand() * 1000L;
        
        /* Initialize output */
        output.ints[i] = 0;
        output.doubles[i] = 0.0;
        output.floats[i] = 0.0f;
        output.longs[i] = 0L;
    }
    
    /* Perform heavy computation */
    compute_heavy(&input, &output, ITERATIONS);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += output.ints[i];
        checksum += (long)output.doubles[i];
        checksum += (long)output.floats[i];
        checksum += output.longs[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
