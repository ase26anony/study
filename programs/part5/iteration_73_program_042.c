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

/* Complex structure for varied memory accesses */
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    int arr[4];
};

/* Argument-heavy helper function - forces register/stack pressure */
__attribute__((noinline))
double heavy_args_func(int a1, int a2, int a3, int a4,
                       float f1, float f2, float f3, float f4,
                       double d1, double d2, double d3, double d4,
                       long l1, long l2, void* p1, void* p2) {
    /* Complex mixing of all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2 * d3;
    result += (double)l1 / (double)l2;
    result += (double)((uintptr_t)p1 % 1000);
    result += (double)((uintptr_t)p2 % 1000);
    
    /* Force memory access */
    result += global_volatile_double;
    
    return result * (a3 + a4) / (f3 + f4 + d4);
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(struct MixedData* input, struct MixedData* output, int size) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = global_volatile_int;
    int v2, v3, v4, v5, v6, v7, v8, v9, v10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Pointer/index variables */
    int idx1, idx2, idx3, idx4;
    int stride1 = 7, stride2 = 13, stride3 = 17;
    
    /* Initialize with volatile to prevent optimization */
    v2 = v1 * 2;
    v3 = v1 + global_volatile_int;
    v4 = v2 - v3;
    v5 = v3 * v4;
    v6 = v4 / (v1 + 1);
    v7 = v5 % (v2 + 1);
    v8 = v6 ^ v7;
    v9 = v7 | v8;
    v10 = v8 & v9;
    
    /* Initialize long variables with complex dependencies */
    l1 = (long)v1 * v2;
    l2 = (long)v3 * v4 + l1;
    l3 = (long)v5 * v6 + l2;
    l4 = (long)v7 * v8 + l3;
    l5 = (long)v9 * v10 + l4;
    l6 = l1 - l2 + l3 - l4;
    l7 = l2 * l3 / (l4 + 1);
    l8 = l3 % (l5 + 1);
    l9 = l4 ^ l5 ^ l6;
    l10 = l7 | l8 | l9;
    
    /* Initialize float variables with mixed-type operations */
    f1 = (float)v1 + global_volatile_float;
    f2 = (float)v2 * f1;
    f3 = (float)l1 / 1000.0f;
    f4 = f1 + f2 - f3;
    f5 = f2 * f3 / f4;
    f6 = (float)v3 * (float)v4 / f5;
    f7 = f3 + f4 + f5 + f6;
    f8 = f4 * f5 - f6 * f7;
    f9 = (float)l2 / (float)l3 * f8;
    f10 = f5 + f6 + f7 + f8 + f9;
    
    /* Initialize double variables with even more complex dependencies */
    d1 = (double)f1 * global_volatile_double;
    d2 = (double)l1 / (double)l2 + d1;
    d3 = (double)v5 * (double)v6 / d2;
    d4 = d1 + d2 - d3;
    d5 = (double)f2 * (double)f3 * d4;
    d6 = (double)l3 * (double)l4 / d5;
    d7 = d3 * d4 / d5 * d6;
    d8 = (double)f4 + (double)f5 + (double)f6;
    d9 = d4 * d5 * d6 * d7 * d8;
    d10 = d5 / d6 + d7 / d8 - d9;
    
    /* Main computation loop with extreme register pressure */
    for (int i = 0; i < size; i++) {
        /* Complex array indexing with multiple terms */
        idx1 = i * stride1;
        idx2 = i * stride2 + (i % 3);
        idx3 = i * stride3 + (i % 5) * 2;
        idx4 = (i * 11 + i % 7) % size;
        
        /* Load from input with complex addressing */
        int in_i = input[idx1].i + input[idx2].arr[i % 4];
        long in_l = input[idx3].l - input[idx4].l;
        float in_f = input[idx1].f * input[idx2].f;
        double in_d = input[idx3].d / input[idx4].d;
        
        /* Extremely long chain of mixed-type computations */
        /* This creates dependencies that prevent register reuse */
        
        /* Integer chain */
        v2 = in_i + v1;
        v3 = v2 * v1 - in_i;
        v4 = v3 / (v2 + 1) + (in_i % 17);
        v5 = v4 ^ v3 ^ v2;
        v6 = v5 * v4 - v3 * v2;
        v7 = v6 + v5 - v4 + v3;
        v8 = v7 % (v6 + 1) | v5;
        v9 = v8 * v7 / (v6 + 1);
        v10 = v9 - v8 + v7 - v6;
        
        /* Long chain with integer mixing */
        l2 = (long)v2 * in_l + l1;
        l3 = (long)v3 * l2 - in_l;
        l4 = l3 / (l2 + 1) + (in_l % 29);
        l5 = l4 ^ l3 ^ l2;
        l6 = l5 * l4 - l3 * l2;
        l7 = l6 + l5 - l4 + l3;
        l8 = l7 % (l6 + 1) | l5;
        l9 = l8 * l7 / (l6 + 1);
        l10 = l9 - l8 + l7 - l6;
        
        /* Float chain with type conversions */
        f2 = (float)v2 * in_f + f1;
        f3 = (float)l2 * f2 - in_f;
        f4 = f3 / (f2 + 0.1f) + (float)(in_i % 11);
        f5 = f4 * f3 * f2;
        f6 = f5 + f4 - f3 + f2;
        f7 = f6 * (float)v3 / (float)v4;
        f8 = f7 + (float)l3 - (float)l4;
        f9 = f8 * f7 / f6;
        f10 = f9 - f8 + f7 - f6;
        
        /* Double chain with maximum mixing */
        d2 = (double)v3 * in_d + d1;
        d3 = (double)l3 * d2 - in_d;
        d4 = d3 / (d2 + 0.1) + (double)(in_i % 13);
        d5 = d4 * d3 * d2;
        d6 = d5 + d4 - d3 + d2;
        d7 = d6 * (double)v4 / (double)v5;
        d8 = d7 + (double)l4 - (double)l5;
        d9 = d8 * d7 / d6;
        d10 = d9 - d8 + d7 - d6;
        
        /* Insert inline assembly that clobbers MANY registers */
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
        
        /* Call argument-heavy function - forces register/stack reloads */
        double func_result = heavy_args_func(
            v1, v2, v3, v4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&input[i], (void*)&output[i]
        );
        
        /* Use function result in further computation */
        d10 += func_result;
        f10 += (float)func_result;
        v10 += (int)func_result;
        l10 += (long)func_result;
        
        /* Store results with complex addressing */
        int store_idx = (i * 19 + i % 13) % size;
        output[store_idx].i = v10;
        output[store_idx].l = l10;
        output[store_idx].f = f10;
        output[store_idx].d = d10;
        output[store_idx].arr[i % 4] = v9;
        
        /* Update variables for next iteration (create loop-carried dependencies) */
        v1 = v10 % 997;
        l1 = l10 % 99991;
        f1 = fmodf(f10, 100.0f);
        d1 = fmod(d10, 1000.0);
    }
    
    /* Volatile sink to prevent optimization */
    volatile int sink = v10 + (int)l10 + (int)f10 + (int)d10;
    (void)sink;
}

int main() {
    /* Allocate and initialize arrays with pseudo-random data */
    struct MixedData* input = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    struct MixedData* output = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].i = (i * 12345 + 6789) % 10000;
        input[i].l = (long)i * i * 7919;
        input[i].f = (float)(i * 3.14159) / 100.0f;
        input[i].d = (double)(i * 2.71828) / 50.0;
        for (int j = 0; j < 4; j++) {
            input[i].arr[j] = (i * (j + 1) * 1111) % 5555;
        }
        /* Initialize output to zero */
        output[i].i = 0;
        output[i].l = 0;
        output[i].f = 0.0f;
        output[i].d = 0.0;
        for (int j = 0; j < 4; j++) {
            output[i].arr[j] = 0;
        }
    }
    
    /* Perform heavy computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(input, output, ARRAY_SIZE);
        
        /* Swap buffers to create different access patterns */
        struct MixedData* temp = input;
        input = output;
        output = temp;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].i;
        checksum += output[i].l;
        checksum += (long long)output[i].f;
        checksum += (long long)output[i].d;
        for (int j = 0; j < 4; j++) {
            checksum += output[i].arr[j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
