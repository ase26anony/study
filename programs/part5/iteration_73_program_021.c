#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile float global_volatile_float = 3.14159f;
volatile double global_volatile_double = 2.71828;

/* Complex structure for memory access patterns */
struct DataBlock {
    int ints[16];
    float floats[16];
    double doubles[16];
    long longs[8];
    char padding[64];
};

/* Helper function with many arguments to force stack passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing types */
    double result = (double)a1 * d1 + (double)f1 * d2;
    result += (double)a2 * sin(d3) + (double)f2 * cos(d4);
    result += (double)l1 * 0.001 + (double)l2 * 0.0001;
    
    /* Force memory access */
    volatile double vol_d = result;
    result += vol_d * 0.5;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer", "no-schedule-insns", "no-schedule-insns2")))
static double compute_heavy(struct DataBlock* input, struct DataBlock* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Pointer/index variables */
    int idx1, idx2, idx3, idx4, idx5;
    volatile int vol_idx; /* Volatile index to prevent optimization */
    
    /* Initialize with some values */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    i11 = 11; i12 = 12; i13 = 13; i14 = 14; i15 = 15;
    i16 = 16; i17 = 17; i18 = 18; i19 = 19; i20 = 20;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    f11 = 11.11f; f12 = 12.12f; f13 = 13.13f; f14 = 14.14f; f15 = 15.15f;
    f16 = 16.16f; f17 = 17.17f; f18 = 18.18f; f19 = 19.19f; f20 = 20.20f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    d16 = 16.16; d17 = 17.17; d18 = 18.18; d19 = 19.19; d20 = 20.20;
    
    idx1 = 0; idx2 = 1; idx3 = 2; idx4 = 3; idx5 = 4;
    vol_idx = 0;
    
    double total_result = 0.0;
    
    /* Main computation loop with extreme register pressure */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        int base_idx = (iter * 7 + idx1 * 3 + idx2 * 5) % 16;
        int offset1 = (base_idx + idx3 * 2 + idx4) % 16;
        int offset2 = (base_idx * 3 + idx5 * 7 + iter) % 16;
        
        /* Load from input arrays with complex addressing */
        i1 = input->ints[base_idx];
        i2 = input->ints[offset1];
        i3 = input->ints[offset2];
        
        f1 = input->floats[base_idx];
        f2 = input->floats[offset1];
        f3 = input->floats[offset2];
        
        d1 = input->doubles[base_idx];
        d2 = input->doubles[offset1];
        d3 = input->doubles[offset2];
        
        l1 = input->longs[base_idx % 8];
        l2 = input->longs[offset1 % 8];
        
        /* LONG chain of mixed-type computations with data dependencies */
        /* Each computation depends on previous results */
        d4 = (double)i1 * d1 + (double)f1 * d2;
        f4 = (float)d4 * f2 + (float)i2 * f3;
        i4 = (int)f4 + i3 + (int)d3;
        d5 = sin(d4) + cos((double)f4) + (double)i4 * 0.01;
        f5 = tanf(f4) + sinf((float)d5) * 2.0f;
        i5 = i4 * 3 + (int)(f5 * 100.0f);
        d6 = d5 * 1.5 + (double)i5 * 0.001;
        f6 = f5 * 1.2f + (float)d6 * 0.5f;
        i6 = i5 / 2 + (int)(f6 * 50.0f);
        d7 = exp(d6 * 0.1) + log(fabs(d5) + 1.0);
        f7 = expf(f6 * 0.2f) + logf(fabsf(f5) + 1.0f);
        i7 = i6 ^ i5 + (int)(d7 * 1000.0);
        d8 = d7 + d6 + d5 + d4 + d3 + d2 + d1;
        f8 = f7 + f6 + f5 + f4 + f3 + f2 + f1;
        i8 = i7 + i6 + i5 + i4 + i3 + i2 + i1;
        
        /* More mixed computations */
        d9 = (double)l1 * d8 + (double)l2 * d7;
        f9 = (float)d9 * 0.3f + (float)i8 * 0.7f;
        i9 = (int)(d9 * f9) + i7 * i6;
        d10 = sqrt(d9 * d9 + d8 * d8);
        f10 = sqrtf(f9 * f9 + f8 * f8);
        i10 = i9 | i8 ^ i7;
        
        /* Use volatile globals in computation */
        d11 = d10 * global_volatile_double;
        f11 = f10 * global_volatile_float;
        i11 = i10 + global_volatile_int;
        
        /* Inline assembly that clobbers many registers */
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
            : /* no outputs */
            : /* no inputs */
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
            : /* no outputs */
            : /* no inputs */
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
              "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
              "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
              "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31",
              "memory"
        );
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("" : : : "memory");
#endif
        
        /* Continue computation after assembly clobber */
        /* This forces reloads of all variables */
        d12 = d11 * 2.0 + sin(d10) * 3.0;
        f12 = f11 * 1.5f + cosf(f10) * 2.5f;
        i12 = i11 * 2 + (int)(d12 * 100.0);
        
        d13 = d12 + d11 + d10 + d9 + d8 + d7 + d6 + d5;
        f13 = f12 + f11 + f10 + f9 + f8 + f7 + f6 + f5;
        i13 = i12 + i11 + i10 + i9 + i8 + i7 + i6 + i5;
        
        /* Call function with many arguments - forces argument passing reloads */
        d14 = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)input, (void*)output
        );
        
        /* More mixed computations */
        f14 = (float)d14 * 0.25f + f13 * 0.75f;
        i14 = (int)(d14 * f14) + i13;
        d15 = pow(d14, 2.0) + sqrt(d13);
        f15 = powf(f14, 2.0f) + sqrtf(f13);
        i15 = i14 * i13 - i12;
        
        /* Final chain */
        d16 = d15 * global_volatile_double + d14 * 0.5;
        f16 = f15 * global_volatile_float + f14 * 0.3f;
        i16 = i15 + global_volatile_int + i14;
        
        d17 = d16 + d15 + d14 + d13 + d12 + d11 + d10;
        f17 = f16 + f15 + f14 + f13 + f12 + f11 + f10;
        i17 = i16 + i15 + i14 + i13 + i12 + i11 + i10;
        
        d18 = (double)i17 * 0.01 + (double)l1 * 0.001 + d17;
        f18 = (float)i16 * 0.02f + (float)l2 * 0.002f + f17;
        i18 = (int)d18 + (int)f18 + i17;
        
        /* Store results with complex addressing */
        vol_idx = (iter + base_idx) % 16;
        output->ints[vol_idx] = i18;
        output->floats[vol_idx] = f18;
        output->doubles[vol_idx] = d18;
        output->longs[vol_idx % 8] = (long)i18 + (long)(d18 * 1000.0);
        
        /* Accumulate to total */
        total_result += d18 + (double)f18 + (double)i18;
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 5;
        idx2 = (idx2 + 2) % 5;
        idx3 = (idx3 + 3) % 5;
        idx4 = (idx4 + 4) % 5;
        idx5 = (idx5 + 5) % 5;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = total_result;
    return sink;
}

int main(void) {
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
        input->floats[i] = (float)rand() / (float)RAND_MAX * 100.0f;
        input->doubles[i] = (double)rand() / (double)RAND_MAX * 100.0;
        if (i < 8) {
            input->longs[i] = (long)rand() * 1000L;
        }
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(input, output, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < 16; i++) {
        checksum += output->ints[i] + output->floats[i] + output->doubles[i];
        if (i < 8) {
            checksum += output->longs[i];
        }
    }
    
    printf("Result: %f\n", result);
    printf("Checksum: %f\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
