#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Global volatile variables to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    double b1, double b2, double b3, double b4,
    float c1, float c2, float c3, float c4,
    long d1, long d2, void* ptr1, void* ptr2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * b1 + (double)a2 * b2;
    result += (double)c1 * (double)c2;
    result += (double)d1 / (double)d2;
    result += *(double*)ptr1 + *(double*)ptr2;
    
    /* Force memory access */
    result += global_volatile_double;
    
    return result * (double)a3 / (double)a4 + b3 - b4 + c3 - c4;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-optimize-sibling-calls")))
static void compute_heavy(double* input, double* output, int size, int stride)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15;
    
    /* Pointer/array index variables */
    int idx1, idx2, idx3, idx4;
    volatile int sink_volatile;  /* Volatile sink to prevent optimization */
    
    /* Initialize with pseudo-random values */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    i6 = 6; i7 = 7; i8 = 8; i9 = 9; i10 = 10;
    i11 = 11; i12 = 12; i13 = 13; i14 = 14; i15 = 15;
    i16 = 16; i17 = 17; i18 = 18; i19 = 19; i20 = 20;
    
    l1 = 100; l2 = 200; l3 = 300; l4 = 400; l5 = 500;
    l6 = 600; l7 = 700; l8 = 800; l9 = 900; l10 = 1000;
    
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    f6 = 6.6f; f7 = 7.7f; f8 = 8.8f; f9 = 9.9f; f10 = 10.10f;
    f11 = 11.11f; f12 = 12.12f; f13 = 13.13f; f14 = 14.14f; f15 = 15.15f;
    
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    d6 = 6.06; d7 = 7.07; d8 = 8.08; d9 = 9.09; d10 = 10.10;
    d11 = 11.11; d12 = 12.12; d13 = 13.13; d14 = 14.14; d15 = 15.15;
    
    /* Main computation loop with complex data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % size;
        idx2 = (iter * 13) % size;
        idx3 = (iter * 23) % size;
        idx4 = (iter * 37) % size;
        
        /* Load from input array with complex addressing */
        d1 = input[idx1 * stride + idx2];
        d2 = input[idx2 * stride + idx3];
        d3 = input[idx3 * stride + idx4];
        d4 = input[idx4 * stride + idx1];
        
        /* LONG chain of mixed-type computations */
        /* This creates dependencies preventing register reuse */
        
        /* Integer to float/double conversions */
        f1 = (float)i1 * (float)i2 + f1;
        d5 = (double)i3 * (double)i4 + d1;
        
        /* Mixed integer/float arithmetic */
        f2 = f1 * (float)d2 + (float)i5;
        d6 = d3 * (double)f3 + (double)i6;
        
        /* More conversions and arithmetic */
        i7 = (int)f4 + (int)d4;
        i8 = i7 * i1 - i2;
        
        f5 = (float)i8 / (float)i9 + f2;
        d7 = (double)i10 / (double)i11 + d5;
        
        /* Use volatile globals in computation */
        f6 = f5 * global_volatile_float;
        d8 = d6 * global_volatile_double;
        
        i12 = i8 + global_volatile_int;
        
        /* More mixed computations */
        f7 = (float)l1 * f6 + (float)l2;
        d9 = (double)l3 * d7 + (double)l4;
        
        f8 = f7 * (float)d8;
        d10 = d9 * (double)f8;
        
        i13 = (int)f8 + (int)d10;
        i14 = i13 * i12 - i3;
        
        f9 = (float)i14 * f3;
        d11 = (double)i15 * d4;
        
        /* Even more computations to increase pressure */
        f10 = f9 + f4 - f5 * f6;
        d12 = d11 + d5 - d6 * d7;
        
        i16 = (int)f10 + (int)d12;
        i17 = i16 * i4 / i5;
        
        f11 = (float)i17 * (float)i18;
        d13 = (double)i19 * (double)i20;
        
        f12 = f11 * (float)d13;
        d14 = d12 * (double)f12;
        
        /* Use long variables */
        l5 = (long)f12 * l1;
        l6 = (long)d14 * l2;
        
        f13 = (float)l5 + (float)l6;
        d15 = (double)l7 + (double)l8;
        
        /* Final conversions */
        i18 = (int)f13 + (int)d15 + i17;
        f14 = (float)i18 * f13;
        
        /* Inline assembly that clobbers MANY registers */
        /* This forces spills and reloads */
        __asm__ volatile (
#if defined(__aarch64__)
            /* Clobber all general purpose registers */
            "mov x0, x0\n"  /* NOP to prevent empty asm */
            : /* no outputs */
            : /* no inputs */
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              /* Clobber floating point registers */
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
#elif defined(__x86_64__)
            /* Clobber all x86_64 registers */
            "mov %%rax, %%rax\n"  /* NOP */
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              /* Clobber xmm/ymm registers */
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
              /* Clobber mmx registers */
              "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
#endif
        );
        
        /* Call function with many arguments - forces argument passing reloads */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            d1, d2, d3, d4,
            f1, f2, f3, f4,
            l1, l2, (void*)&i5, (void*)&i6
        );
        
        /* Use function result in computation */
        f15 = f14 * (float)func_result;
        
        /* Complex store with addressing */
        output[idx1 * stride + idx2] = d15 + (double)f15;
        output[idx2 * stride + idx3] = (double)i18 + d14;
        
        /* Volatile store to prevent optimization */
        sink_volatile = i18 + (int)f15;
        
        /* Create circular dependencies for next iteration */
        i1 = i18;
        i2 = sink_volatile;
        f1 = f15;
        d1 = func_result;
    }
    
    /* Final volatile store */
    global_volatile_int = sink_volatile;
}

int main(void)
{
    /* Allocate and initialize arrays */
    double* input = (double*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(double));
    double* output = (double*)malloc(ARRAY_SIZE * ARRAY_SIZE * sizeof(double));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i++) {
        input[i] = (double)rand() / RAND_MAX * 100.0;
        output[i] = 0.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ARRAY_SIZE, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE * ARRAY_SIZE; i += 97) {
        checksum += output[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    
    return 0;
}
