/* reload_stress.c - Designed to trigger GCC reload pass uncovered lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Volatile globals to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for varied memory access */
struct ComplexData {
    int ints[16];
    double doubles[8];
    float floats[12];
    long longs[4];
    char padding[64]; /* Force different alignments */
};

/* Argument-heavy helper function - NOINLINE to prevent inlining */
__attribute__((noinline))
double heavy_args_func(int a1, int a2, int a3, int a4,
                       double b1, double b2, double b3, double b4,
                       float c1, float c2, float c3, float c4,
                       long d1, long d2, void* ptr1, void* ptr2) {
    /* Complex computation mixing all types */
    double result = (double)a1 * b1 + (double)a2 * b2;
    result += (double)c1 * (double)c2;
    result += (double)d1 / (double)(d2 + 1);
    result += *(double*)ptr1 + *(float*)ptr2;
    
    /* Force side effects */
    global_volatile_int += a3 + a4;
    global_volatile_double *= b3 + b4;
    global_volatile_float += c3 + c4;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
void compute_heavy(struct ComplexData* input, struct ComplexData* output, 
                   int stride, int iterations) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Pointer variables */
    int* ip1, *ip2, *ip3;
    double* dp1, *dp2;
    float* fp1, *fp2;
    
    /* Index variables with complex relationships */
    int i, j, k, m, n, p, q;
    
    /* Initialize locals with volatile reads */
    v1 = global_volatile_int;
    v2 = v1 * 2;
    v3 = v2 + global_volatile_int;
    v4 = v3 - 17;
    v5 = v4 / 3;
    v6 = v5 * v2;
    v7 = v6 + v3;
    v8 = v7 - v4;
    v9 = v8 * v5;
    v10 = v9 / (v1 + 1);
    
    l1 = (long)v1 * 1000;
    l2 = l1 + (long)v2;
    l3 = l2 * (long)v3;
    l4 = l3 - (long)v4;
    l5 = l4 / (long)(v5 + 1);
    l6 = l5 + l2;
    l7 = l6 * l3;
    l8 = l7 - l4;
    
    f1 = global_volatile_float;
    f2 = f1 * 2.0f;
    f3 = f2 + global_volatile_float;
    f4 = f3 - 17.5f;
    f5 = f4 / 3.0f;
    f6 = f5 * f2;
    f7 = f6 + f3;
    f8 = f7 - f4;
    f9 = f8 * f5;
    f10 = f9 / (f1 + 1.0f);
    
    d1 = global_volatile_double;
    d2 = d1 * 2.0;
    d3 = d2 + global_volatile_double;
    d4 = d3 - 17.5;
    d5 = d4 / 3.0;
    d6 = d5 * d2;
    d7 = d6 + d3;
    d8 = d7 - d4;
    d9 = d8 * d5;
    d10 = d9 / (d1 + 1.0);
    
    /* Main computation loop with extreme register pressure */
    for (i = 0; i < iterations; i++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (i * stride) % ARRAY_SIZE;
        int idx2 = (i * stride + i/2) % ARRAY_SIZE;
        int idx3 = (i * stride + i/3 + i/4) % ARRAY_SIZE;
        int idx4 = (i * stride * 2 - i/5) % ARRAY_SIZE;
        
        /* Load data with complex addressing */
        v1 = input[idx1].ints[0] + v10;
        v2 = input[idx2].ints[1] * v9;
        v3 = input[idx3].ints[2] - v8;
        v4 = input[idx4].ints[3] / (v7 + 1);
        
        l1 = (long)input[idx1].ints[4] + l8;
        l2 = (long)input[idx2].ints[5] * l7;
        l3 = (long)input[idx3].ints[6] - l6;
        l4 = (long)input[idx4].ints[7] / (l5 + 1);
        
        f1 = input[idx1].floats[0] + f10;
        f2 = input[idx2].floats[1] * f9;
        f3 = input[idx3].floats[2] - f8;
        f4 = input[idx4].floats[3] / (f7 + 1.0f);
        
        d1 = input[idx1].doubles[0] + d10;
        d2 = input[idx2].doubles[1] * d9;
        d3 = input[idx3].doubles[2] - d8;
        d4 = input[idx4].doubles[3] / (d7 + 1.0);
        
        /* Long chain of mixed-type computations */
        f5 = (float)v1 * (float)d1 + (float)l1;
        d5 = (double)v2 * (double)f2 + (double)l2;
        v5 = (int)f3 * (int)d3 + (int)l3;
        l5 = (long)v4 + (long)f4 + (long)d4;
        
        f6 = f5 * 2.0f - (float)v5 / 3.0f;
        d6 = d5 * 1.5 + (double)l5 / 4.0;
        v6 = v5 * 3 - (int)f6 + (int)d6;
        l6 = l5 * 2 + (long)v6 - (long)f6;
        
        f7 = sqrtf(fabsf(f6)) + (float)v6 * 0.01f;
        d7 = sqrt(fabs(d6)) + (double)l6 * 0.01;
        v7 = abs(v6) + (int)(f7 * 100.0f);
        l7 = labs(l6) + (long)(d7 * 100.0);
        
        /* More mixed computations */
        f8 = (float)v7 * 0.5f + (float)l7 * 0.25f;
        d8 = (double)v7 * 0.5 + (double)l7 * 0.25;
        v8 = (int)(f8 * 2.0f) + (int)(d8 * 3.0);
        l8 = (long)(f8 * 4.0f) + (long)(d8 * 5.0);
        
        f9 = f8 + f7 + f6 + f5 + f4 + f3 + f2 + f1;
        d9 = d8 + d7 + d6 + d5 + d4 + d3 + d2 + d1;
        v9 = v8 + v7 + v6 + v5 + v4 + v3 + v2 + v1;
        l9 = l8 + l7 + l6 + l5 + l4 + l3 + l2 + l1;
        
        f10 = (float)v9 / (float)(l9 + 1) + f9;
        d10 = (double)v9 / (double)(l9 + 1) + d9;
        
        /* Inline assembly that clobbers MANY registers */
        /* For x86_64 */
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
              "memory", "cc"
        );
        /* For AArch64 */
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
              "memory", "cc"
        );
#endif
        
        /* Call argument-heavy function with mixed types */
        double arg_result = heavy_args_func(
            v1, v2, v3, v4,
            d1, d2, d3, d4,
            f1, f2, f3, f4,
            l1, l2,
            (void*)&input[idx1].doubles[0],
            (void*)&input[idx2].floats[0]
        );
        
        /* Use result in further computation */
        f10 += (float)arg_result;
        d10 += arg_result;
        v10 += (int)arg_result;
        l10 += (long)arg_result;
        
        /* Complex store with indexing */
        output[idx1].ints[0] = v10;
        output[idx2].ints[1] = v9;
        output[idx3].ints[2] = v8;
        output[idx4].ints[3] = v7;
        
        output[idx1].doubles[0] = d10;
        output[idx2].doubles[1] = d9;
        output[idx3].doubles[2] = d8;
        output[idx4].doubles[3] = d7;
        
        output[idx1].floats[0] = f10;
        output[idx2].floats[1] = f9;
        output[idx3].floats[2] = f8;
        output[idx4].floats[3] = f7;
        
        output[idx1].longs[0] = l10;
        output[idx2].longs[1] = l9;
        output[idx3].longs[2] = l8;
        output[idx4].longs[3] = l7;
        
        /* Update volatile globals */
        global_volatile_int += v10;
        global_volatile_double += d10;
        global_volatile_float += f10;
    }
}

int main() {
    struct ComplexData* input = (struct ComplexData*)malloc(
        ARRAY_SIZE * sizeof(struct ComplexData));
    struct ComplexData* output = (struct ComplexData*)malloc(
        ARRAY_SIZE * sizeof(struct ComplexData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 16; j++) {
            input[i].ints[j] = rand() % 1000;
        }
        for (int j = 0; j < 8; j++) {
            input[i].doubles[j] = (double)rand() / RAND_MAX * 100.0;
        }
        for (int j = 0; j < 12; j++) {
            input[i].floats[j] = (float)rand() / RAND_MAX * 100.0f;
        }
        for (int j = 0; j < 4; j++) {
            input[i].longs[j] = (long)rand() * 1000L;
        }
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, 7, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += output[i].ints[j];
        }
        for (int j = 0; j < 8; j++) {
            checksum += (long)output[i].doubles[j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    printf("Volatile globals: int=%d, double=%f, float=%f\n",
           global_volatile_int, global_volatile_double, global_volatile_float);
    
    free(input);
    free(output);
    
    return 0;
}
