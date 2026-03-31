/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for addressing modes */
struct DataBlock {
    int indices[16];
    double values[16];
    float floats[16];
    long longs[8];
};

/* Argument-heavy helper function - forces register/stack reloads */
__attribute__((noinline))
static double heavy_args_func(int a1, int a2, int a3, int a4,
                              float f1, float f2, float f3, float f4,
                              double d1, double d2, double d3, double d4,
                              long l1, long l2, void *p1, void *p2) {
    /* Mix all argument types in computation */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += *(double*)p1 + *(double*)p2;
    
    /* Force memory access */
    result += global_volatile_double;
    
    return result * (double)a3 / (double)a4 + d3 - d4 + f3 - f4;
}

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(struct DataBlock *input, struct DataBlock *output, 
                         int *int_array, double *double_array, int n) {
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
    
    /* Pointer variables */
    int *p1, *p2, *p3;
    double *dp1, *dp2, *dp3;
    float *fp1, *fp2, *fp3;
    
    /* Volatile locals to force memory ops */
    volatile int vi1, vi2, vi3;
    volatile double vd1, vd2, vd3;
    volatile float vf1, vf2, vf3;
    
    /* Initialize pointers */
    p1 = int_array;
    p2 = int_array + ARRAY_SIZE/2;
    dp1 = double_array;
    dp2 = double_array + ARRAY_SIZE/2;
    fp1 = (float*)double_array; /* Aliasing for extra complexity */
    
    /* Main computation loop with complex data dependencies */
    for (int outer = 0; outer < n; outer++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (outer * 7 + 3) % ARRAY_SIZE;
        int idx2 = (outer * 13 + 5) % ARRAY_SIZE;
        int idx3 = (outer * 17 + 11) % ARRAY_SIZE;
        int stride = 8;
        
        /* Load from arrays with complex addressing */
        i1 = int_array[idx1 * stride + idx2 + idx3];
        i2 = int_array[idx2 * stride + idx1 + idx3];
        i3 = int_array[idx3 * stride + idx1 + idx2];
        
        /* Structure field accesses */
        l1 = input->longs[outer % 8];
        l2 = input->longs[(outer + 1) % 8];
        d1 = input->values[outer % 16];
        d2 = input->values[(outer + 1) % 16];
        f1 = input->floats[outer % 16];
        f2 = input->floats[(outer + 1) % 16];
        
        /* Start long chain of mixed-type computations */
        /* Each computation depends on previous result */
        f3 = (float)i1 * f1 + f2;
        d3 = (double)i2 * d1 + d2;
        i4 = (int)f3 + i3;
        l3 = (long)d3 * l1 + l2;
        f4 = (float)l3 / f3 + f1;
        d4 = (double)i4 * d3 + d2;
        i5 = (int)f4 * i4 + i3;
        l4 = (long)d4 + l3 * l2;
        f5 = (float)l4 * f4 - f3;
        d5 = (double)i5 / d4 * d3;
        i6 = (int)f5 + i5 - i4;
        l5 = (long)d5 * l4 / l3;
        f6 = (float)l5 + f5 * f4;
        d6 = (double)i6 - d5 + d4;
        i7 = (int)f6 * i6 + global_volatile_int;
        l6 = (long)d6 * l5 + global_volatile_int;
        f7 = (float)l6 / f6 + global_volatile_float;
        d7 = (double)i7 * d6 + global_volatile_double;
        
        /* More computations to use all declared variables */
        i8 = i7 * 2 - i6;
        i9 = i8 + i5 * 3;
        i10 = i9 / 2 + i4;
        i11 = i10 * i7 - i6;
        i12 = i11 + global_volatile_int;
        i13 = i12 * 3 / 2;
        i14 = i13 - i10 + i9;
        i15 = i14 * i8;
        
        l7 = l6 * 2 - l5;
        l8 = l7 + l4 * 3;
        l9 = l8 / 2 + l3;
        l10 = l9 * l7 - l6;
        
        f8 = f7 * 2.0f - f6;
        f9 = f8 + f5 * 3.0f;
        f10 = f9 / 2.0f + f4;
        f11 = f10 * f8 - f7;
        f12 = f11 + global_volatile_float;
        
        d8 = d7 * 2.0 - d6;
        d9 = d8 + d5 * 3.0;
        d10 = d9 / 2.0 + d4;
        d11 = d10 * d8 - d7;
        d12 = d11 + global_volatile_double;
        
        /* Inline assembly that clobbers MANY registers */
        /* This forces spills and reloads around the asm block */
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
            "xorpd %%xmm0, %%xmm0\n"
            "xorpd %%xmm1, %%xmm1\n"
            "xorpd %%xmm2, %%xmm2\n"
            "xorpd %%xmm3, %%xmm3\n"
            "xorpd %%xmm4, %%xmm4\n"
            "xorpd %%xmm5, %%xmm5\n"
            "xorpd %%xmm6, %%xmm6\n"
            "xorpd %%xmm7, %%xmm7\n"
            "xorpd %%xmm8, %%xmm8\n"
            "xorpd %%xmm9, %%xmm9\n"
            "xorpd %%xmm10, %%xmm10\n"
            "xorpd %%xmm11, %%xmm11\n"
            "xorpd %%xmm12, %%xmm12\n"
            "xorpd %%xmm13, %%xmm13\n"
            "xorpd %%xmm14, %%xmm14\n"
            "xorpd %%xmm15, %%xmm15\n"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
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
            :
            :
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
        __asm__ volatile ("" ::: "memory");
#endif
        
        /* Continue computation after asm clobber */
        /* All values need to be reloaded from memory or recomputed */
        f13 = f12 * 1.5f + f11;
        f14 = f13 - f10 * 0.5f;
        f15 = f14 + global_volatile_float;
        f16 = f15 * f13 / f12;
        
        d13 = d12 * 1.5 + d11;
        d14 = d13 - d10 * 0.5;
        d15 = d14 + global_volatile_double;
        d16 = d15 * d13 / d12;
        
        i16 = i15 + 1000;
        i17 = i16 * 2 - i15;
        i18 = i17 + global_volatile_int;
        i19 = i18 * 3 / 2;
        i20 = i19 - i17 + i16;
        
        /* Call argument-heavy function - forces more reloads */
        double func_result = heavy_args_func(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&d5, (void*)&d6
        );
        
        /* Use function result in further computation */
        d17 = d16 + func_result;
        f17 = f16 + (float)func_result;
        i20 += (int)func_result;
        
        /* Complex store with addressing */
        output->values[outer % 16] = d17;
        output->floats[outer % 16] = f17;
        output->longs[outer % 8] = i20;
        output->indices[outer % 16] = i20;
        
        /* Volatile stores to force memory operations */
        vi1 = i20;
        vd1 = d17;
        vf1 = f17;
        
        /* More complex array indexing */
        int store_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % ARRAY_SIZE;
        int_array[store_idx] = i20;
        double_array[store_idx] = d17;
        
        /* Dependency chain for next iteration */
        i1 = i20;
        f1 = f17;
        d1 = d17;
        l1 = i20;
    }
    
    /* Final volatile sink to prevent optimization */
    volatile double final_sink = d20 + f20 + i20 + l10;
    (void)final_sink;
}

int main(void) {
    /* Allocate and initialize data */
    struct DataBlock *input = malloc(sizeof(struct DataBlock));
    struct DataBlock *output = malloc(sizeof(struct DataBlock));
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input || !output || !int_array || !double_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    for (int i = 0; i < 16; i++) {
        input->indices[i] = rand() % 1000;
        input->values[i] = (double)rand() / RAND_MAX * 100.0;
        input->floats[i] = (float)rand() / RAND_MAX * 100.0f;
        if (i < 8) {
            input->longs[i] = rand() % 10000;
        }
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        double_array[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, int_array, double_array, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += output->indices[i];
        checksum += (long)output->values[i];
        checksum += (long)output->floats[i];
        if (i < 8) {
            checksum += output->longs[i];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(int_array);
    free(double_array);
    
    return 0;
}
