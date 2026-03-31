#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operands */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Complex structure for varied memory accesses */
struct DataBlock {
    int ints[16];
    long longs[8];
    float floats[12];
    double doubles[6];
    char padding[64];
};

/* Argument-heavy helper function */
__attribute__((noinline))
static double heavy_args_func(
    int a1, int a2, int a3, int a4,
    long b1, long b2, long b3, long b4,
    float c1, float c2, float c3, float c4,
    double d1, double d2, double d3, double d4,
    void* p1, void* p2, struct DataBlock* block
) {
    /* Complex computation mixing all argument types */
    double sum = (double)a1 + (double)b1 + (double)c1 + d1;
    sum += (double)a2 * (double)b2 * (double)c2 * d2;
    sum += (double)a3 / (double)b3 + (double)c3 - d3;
    sum += (double)a4 * d4 + (double)c4 / (double)b4;
    
    if (p1 != p2) {
        sum *= 1.01;
    }
    
    if (block) {
        sum += block->doubles[0] + block->floats[0];
    }
    
    return sum * 0.999;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static void compute_heavy(struct DataBlock* input, struct DataBlock* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int* p1, *p2, *p3;
    volatile float* fp1, *fp2;
    volatile double* dp1, *dp2;
    
    /* Additional variables for complex addressing */
    int idx1, idx2, idx3, idx4, idx5;
    long stride1, stride2;
    float f_acc1, f_acc2, f_acc3;
    double d_acc1, d_acc2, d_acc3;
    
    /* Initialize some values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    l1 = 100L; l2 = 200L; l3 = 300L; l4 = 400L; l5 = 500L;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    d1 = 10.1; d2 = 20.2; d3 = 30.3; d4 = 40.4; d5 = 50.5;
    
    stride1 = 8;
    stride2 = 16;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 3) % 16;
        idx2 = (iter * 5) % 8;
        idx3 = (iter * 7) % 12;
        idx4 = (iter * 11) % 6;
        idx5 = (iter * 13) % 16;
        
        /* Long chain of data-dependent mixed-type computations */
        /* This prevents register reuse through dependency chains */
        
        /* Integer computations */
        v6 = v1 + v2 * v3 - v4 / (v5 + 1);
        v7 = v6 * v2 + v3 - v4;
        v8 = v7 / (v1 + 1) + v5 * v2;
        v9 = v8 - v3 + v4 * v6;
        v10 = v9 + v7 - v2 * v8;
        
        /* Long integer computations */
        l6 = l1 * l2 + l3 - l4;
        l7 = l6 / (l2 + 1) * l5;
        l8 = l7 + l3 - l4 * l6;
        l9 = l8 * l1 + l5 - l2;
        l10 = l9 / (l3 + 1) + l4 * l7;
        
        /* Floating point computations with type conversions */
        f6 = (float)v6 * f1 + (float)l6 * 0.5f;
        f7 = f2 * (float)v7 - (float)l7 * 0.25f;
        f8 = f3 + (float)v8 / 3.0f - (float)l8;
        f9 = f4 * (float)v9 + (float)l9 * 2.0f;
        f10 = f5 - (float)v10 + (float)l10 / 4.0f;
        
        /* Double precision computations with mixed types */
        d6 = (double)f6 * d1 + (double)v6;
        d7 = d2 * (double)f7 - (double)v7;
        d8 = d3 + (double)f8 / 3.0 - (double)l8;
        d9 = d4 * (double)f9 + (double)l9 * 2.0;
        d10 = d5 - (double)f10 + (double)l10 / 4.0;
        
        /* More complex mixed computations */
        f_acc1 = (float)d6 * f1 + (float)v6 * 0.3f;
        f_acc2 = f2 * (float)d7 - (float)v7 * 0.7f;
        f_acc3 = f3 + (float)d8 / 1.5f - (float)l8;
        
        d_acc1 = (double)f_acc1 * d1 + (double)v6;
        d_acc2 = d2 * (double)f_acc2 - (double)v7;
        d_acc3 = d3 + (double)f_acc3 / 2.5 - (double)l8;
        
        /* Complex memory accesses with multiple index terms */
        /* This stresses addressing modes and may create MEM rtxes */
        v1 = input->ints[idx1] + input->ints[idx5];
        v2 = input->ints[(idx1 + idx2) % 16] - input->ints[(idx3 + idx4) % 16];
        v3 = input->ints[idx1 * 2 % 16] * input->ints[idx2 * 3 % 16];
        
        l1 = input->longs[idx2] + input->longs[(idx2 + 1) % 8];
        l2 = input->longs[idx2 * stride1 % 8] - input->longs[idx4 % 8];
        
        f1 = input->floats[idx3] * input->floats[(idx3 + 2) % 12];
        f2 = input->floats[idx3 * 3 % 12] + input->floats[idx4 * 2 % 12];
        
        d1 = input->doubles[idx4] / input->doubles[(idx4 + 1) % 6];
        d2 = input->doubles[idx4 * 2 % 6] - input->doubles[idx2 % 6];
        
        /* Inline assembly that clobbers many registers */
        /* Forces spills and reloads around the asm block */
#if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile (
            "# Clobber many x86 registers\n"
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
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
#elif defined(__aarch64__) || defined(__arm__)
        __asm__ volatile (
            "# Clobber many ARM registers\n"
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
        
        /* Call argument-heavy function with mixed types */
        /* This forces argument passing in registers and stack */
        double result = heavy_args_func(
            v1, v2, v3, v4,
            l1, l2, l3, l4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            (void*)&v1, (void*)&v2,
            input
        );
        
        /* Use result in further computations */
        d_acc1 += result * 0.5;
        d_acc2 -= result * 0.25;
        d_acc3 *= result + 1.0;
        
        /* Store results with complex addressing */
        output->ints[idx1] = v6 + (int)result;
        output->ints[idx5] = v7 - (int)(result * 2);
        
        output->longs[idx2 % 8] = l6 + (long)(result * 1000);
        output->longs[(idx2 + 1) % 8] = l7 - (long)(result * 500);
        
        output->floats[idx3] = f6 + (float)result;
        output->floats[(idx3 + 3) % 12] = f7 * (float)result;
        
        output->doubles[idx4] = d6 + result;
        output->doubles[(idx4 + 2) % 6] = d7 * result;
        
        /* Update volatile globals */
        global_counter += v6;
        global_double *= (result + 1.0) * 0.999;
        global_float += (float)result * 0.001f;
        
        /* Rotate values to create new dependencies */
        int temp_v = v10;
        v10 = v9; v9 = v8; v8 = v7; v7 = v6; v6 = temp_v;
        
        long temp_l = l10;
        l10 = l9; l9 = l8; l8 = l7; l7 = l6; l6 = temp_l;
        
        float temp_f = f10;
        f10 = f9; f9 = f8; f8 = f7; f7 = f6; f6 = temp_f;
        
        double temp_d = d10;
        d10 = d9; d9 = d8; d8 = d7; d7 = d6; d6 = temp_d;
    }
    
    /* Final volatile store to prevent optimization */
    volatile double final_sink = d_acc1 + d_acc2 + d_acc3;
    (void)final_sink;
}

int main() {
    /* Allocate and initialize data */
    struct DataBlock* input = (struct DataBlock*)aligned_alloc(64, sizeof(struct DataBlock));
    struct DataBlock* output = (struct DataBlock*)aligned_alloc(64, sizeof(struct DataBlock));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < 16; i++) {
        input->ints[i] = rand() % 1000;
    }
    for (int i = 0; i < 8; i++) {
        input->longs[i] = rand() % 10000;
    }
    for (int i = 0; i < 12; i++) {
        input->floats[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    for (int i = 0; i < 6; i++) {
        input->doubles[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ITERATIONS);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += output->ints[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += output->longs[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global double: %f\n", global_double);
    printf("Global float: %f\n", global_float);
    
    free(input);
    free(output);
    
    return 0;
}
