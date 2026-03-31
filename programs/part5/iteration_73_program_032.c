/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments - forces register/stack pressure */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex computation mixing all types */
    double sum = (double)a1 * b1 + (double)c1 * d1;
    sum += (double)a2 * b2 + (double)c2 * d2;
    sum += (double)a3 * b3 + (double)c3 * d3;
    sum += (double)a4 * b4 + (double)c4 * d4;
    sum += (double)a5 * b5 + (double)c5 * d5;
    sum += (double)a6 * b6 + (double)c6 * d6;
    
    /* Force memory access */
    sum += global_volatile_double;
    sum += global_volatile_float;
    
    return sum * 0.5;
}

/* Structure for complex memory access patterns */
struct DataBlock {
    int ints[16];
    float floats[16];
    double doubles[8];
    long longs[8];
};

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer", "no-schedule-insns", "no-schedule-insns2")))
static double compute_heavy(const int* input_int, const double* input_double,
                           const float* input_float, const long* input_long,
                           int* output_int, double* output_double,
                           float* output_float, long* output_long,
                           struct DataBlock* blocks, int n) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int idx1, idx2, idx3, idx4, idx5;
    volatile long lidx1, lidx2, lidx3;
    volatile float fidx1, fidx2;
    volatile double didx1, didx2;
    
    /* Additional non-volatile variables for computation */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long li1, li2, li3, li4, li5;
    float fi1, fi2, fi3, fi4, fi5, fi6, fi7, fi8;
    double di1, di2, di3, di4, di5, di6, di7, di8;
    
    double total_sum = 0.0;
    
    /* Complex loop with data dependencies and mixed types */
    for (int iter = 0; iter < n; iter++) {
        /* Initialize volatile variables from inputs */
        v1 = input_int[iter % ARRAY_SIZE];
        v2 = input_int[(iter * 3) % ARRAY_SIZE];
        v3 = input_int[(iter * 5) % ARRAY_SIZE];
        v4 = input_int[(iter * 7) % ARRAY_SIZE];
        v5 = input_int[(iter * 11) % ARRAY_SIZE];
        
        d1 = input_double[iter % ARRAY_SIZE];
        d2 = input_double[(iter * 13) % ARRAY_SIZE];
        d3 = input_double[(iter * 17) % ARRAY_SIZE];
        
        f1 = input_float[iter % ARRAY_SIZE];
        f2 = input_float[(iter * 19) % ARRAY_SIZE];
        f3 = input_float[(iter * 23) % ARRAY_SIZE];
        
        l1 = input_long[iter % ARRAY_SIZE];
        l2 = input_long[(iter * 29) % ARRAY_SIZE];
        l3 = input_long[(iter * 31) % ARRAY_SIZE];
        
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 2) % 16;
        idx2 = (iter * 3) % 16;
        idx3 = (iter * 5) % 16;
        idx4 = (iter * 7) % 8;
        idx5 = (iter * 11) % 8;
        
        /* Structure field access with complex indexing */
        v6 = blocks[iter % 4].ints[idx1 * 2 + idx2];
        v7 = blocks[(iter + 1) % 4].ints[idx3 * 3 + idx4];
        
        f4 = blocks[iter % 4].floats[idx1 + idx2 * 2];
        f5 = blocks[(iter + 2) % 4].floats[idx3 + idx4 * 3];
        
        d4 = blocks[iter % 4].doubles[idx4];
        d5 = blocks[(iter + 1) % 4].doubles[idx5];
        
        l4 = blocks[iter % 4].longs[idx4];
        l5 = blocks[(iter + 3) % 4].longs[idx5];
        
        /* Long chain of mixed-type computations with data dependencies */
        i1 = v1 + v2 * 3 - v3 / 2;
        i2 = i1 * v4 + v5 / 7;
        i3 = i2 - v6 * 2 + v7;
        
        fi1 = (float)i1 * f1 + f2 * 2.5f;
        fi2 = fi1 / f3 + f4 * 3.14f;
        fi3 = fi2 * f5 + (float)i2 * 0.5f;
        
        di1 = (double)fi1 * d1 + d2 * 1.618;
        di2 = di1 / d3 + d4 * 2.71828;
        di3 = di2 * d5 + (double)fi2 * 3.14159;
        
        li1 = (long)(di1 * 1000.0) + l1 * 3;
        li2 = li1 - l2 / 5 + l3 * 7;
        li3 = li2 + l4 * 11 - l5 / 13;
        
        /* More computations creating register pressure */
        i4 = i3 * 2 - (int)fi3;
        i5 = i4 + (int)di3 - (int)li3;
        i6 = i5 * 3 + v1 - v7;
        
        fi4 = fi3 * 1.5f + (float)i4 * 0.25f;
        fi5 = fi4 / 2.0f + (float)i5 * 0.125f;
        fi6 = fi5 * 3.0f + (float)i6;
        
        di4 = di3 * 1.1 + (double)fi4 * 0.01;
        di5 = di4 / 1.5 + (double)fi5 * 0.02;
        di6 = di5 * 2.0 + (double)fi6 * 0.03;
        
        li4 = li3 + (long)di4;
        li5 = li4 - (long)di5 * 2;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__) || defined(_M_X64)
        __asm__ volatile (
            "# Clobber many registers\n\t"
            "mov $0, %%rax\n\t"
            "mov $0, %%rbx\n\t"
            "mov $0, %%rcx\n\t"
            "mov $0, %%rdx\n\t"
            "mov $0, %%rsi\n\t"
            "mov $0, %%rdi\n\t"
            "mov $0, %%r8\n\t"
            "mov $0, %%r9\n\t"
            "mov $0, %%r10\n\t"
            "mov $0, %%r11\n\t"
            "mov $0, %%r12\n\t"
            "mov $0, %%r13\n\t"
            "mov $0, %%r14\n\t"
            "mov $0, %%r15\n\t"
            "pxor %%xmm0, %%xmm0\n\t"
            "pxor %%xmm1, %%xmm1\n\t"
            "pxor %%xmm2, %%xmm2\n\t"
            "pxor %%xmm3, %%xmm3\n\t"
            "pxor %%xmm4, %%xmm4\n\t"
            "pxor %%xmm5, %%xmm5\n\t"
            "pxor %%xmm6, %%xmm6\n\t"
            "pxor %%xmm7, %%xmm7\n\t"
            "pxor %%xmm8, %%xmm8\n\t"
            "pxor %%xmm9, %%xmm9\n\t"
            "pxor %%xmm10, %%xmm10\n\t"
            "pxor %%xmm11, %%xmm11\n\t"
            "pxor %%xmm12, %%xmm12\n\t"
            "pxor %%xmm13, %%xmm13\n\t"
            "pxor %%xmm14, %%xmm14\n\t"
            "pxor %%xmm15, %%xmm15\n\t"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory", "cc"
        );
#elif defined(__aarch64__) || defined(__arm64__)
        /* For ARM64 */
        __asm__ volatile (
            "# Clobber many ARM registers\n\t"
            "mov x0, #0\n\t"
            "mov x1, #0\n\t"
            "mov x2, #0\n\t"
            "mov x3, #0\n\t"
            "mov x4, #0\n\t"
            "mov x5, #0\n\t"
            "mov x6, #0\n\t"
            "mov x7, #0\n\t"
            "mov x8, #0\n\t"
            "mov x9, #0\n\t"
            "mov x10, #0\n\t"
            "mov x11, #0\n\t"
            "mov x12, #0\n\t"
            "mov x13, #0\n\t"
            "mov x14, #0\n\t"
            "mov x15, #0\n\t"
            "mov x16, #0\n\t"
            "mov x17, #0\n\t"
            "mov x18, #0\n\t"
            "mov x19, #0\n\t"
            "mov x20, #0\n\t"
            "mov x21, #0\n\t"
            "mov x22, #0\n\t"
            "mov x23, #0\n\t"
            "mov x24, #0\n\t"
            "mov x25, #0\n\t"
            "mov x26, #0\n\t"
            "mov x27, #0\n\t"
            "mov x28, #0\n\t"
            "fmov d0, #0.0\n\t"
            "fmov d1, #0.0\n\t"
            "fmov d2, #0.0\n\t"
            "fmov d3, #0.0\n\t"
            "fmov d4, #0.0\n\t"
            "fmov d5, #0.0\n\t"
            "fmov d6, #0.0\n\t"
            "fmov d7, #0.0\n\t"
            "fmov d8, #0.0\n\t"
            "fmov d9, #0.0\n\t"
            "fmov d10, #0.0\n\t"
            "fmov d11, #0.0\n\t"
            "fmov d12, #0.0\n\t"
            "fmov d13, #0.0\n\t"
            "fmov d14, #0.0\n\t"
            "fmov d15, #0.0\n\t"
            "fmov d16, #0.0\n\t"
            "fmov d17, #0.0\n\t"
            "fmov d18, #0.0\n\t"
            "fmov d19, #0.0\n\t"
            "fmov d20, #0.0\n\t"
            "fmov d21, #0.0\n\t"
            "fmov d22, #0.0\n\t"
            "fmov d23, #0.0\n\t"
            "fmov d24, #0.0\n\t"
            "fmov d25, #0.0\n\t"
            "fmov d26, #0.0\n\t"
            "fmov d27, #0.0\n\t"
            "fmov d28, #0.0\n\t"
            "fmov d29, #0.0\n\t"
            "fmov d30, #0.0\n\t"
            "fmov d31, #0.0\n\t"
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
#else
        /* Generic clobber for other architectures */
        __asm__ volatile ("# Generic clobber" ::: "memory", "cc");
#endif
        
        /* Call function with many arguments - forces argument passing reloads */
        double func_result = many_args_function(
            i1, di1, fi1, li1,
            i2, di2, fi2, li2,
            i3, di3, fi3, li3,
            i4, di4, fi4, li4,
            i5, di5, fi5, li5,
            i6, di6, fi6, (long)di6
        );
        
        /* More computations after function call */
        i7 = i6 + (int)func_result;
        fi7 = fi6 + (float)func_result;
        di7 = di6 + func_result;
        li6 = li5 + (long)func_result;
        
        /* Final store operations with complex addressing */
        output_int[(iter * 2) % ARRAY_SIZE] = i7;
        output_int[(iter * 3) % ARRAY_SIZE] = i6;
        output_int[(iter * 5) % ARRAY_SIZE] = i5;
        
        output_float[(iter * 7) % ARRAY_SIZE] = fi7;
        output_float[(iter * 11) % ARRAY_SIZE] = fi6;
        output_float[(iter * 13) % ARRAY_SIZE] = fi5;
        
        output_double[(iter * 17) % ARRAY_SIZE] = di7;
        output_double[(iter * 19) % ARRAY_SIZE] = di6;
        output_double[(iter * 23) % ARRAY_SIZE] = di5;
        
        output_long[(iter * 29) % ARRAY_SIZE] = li6;
        output_long[(iter * 31) % ARRAY_SIZE] = li5;
        output_long[(iter * 37) % ARRAY_SIZE] = li4;
        
        /* Accumulate to total sum */
        total_sum += (double)i7 + fi7 + di7 + (double)li6;
        
        /* Access volatile globals in the loop */
        total_sum += global_volatile_int;
        total_sum += global_volatile_double;
        total_sum += global_volatile_float;
    }
    
    return total_sum;
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int* input_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* input_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* input_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* input_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    int* output_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* output_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* output_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    struct DataBlock blocks[4];
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_int[i] = rand() % 1000;
        input_double[i] = (double)rand() / RAND_MAX * 100.0;
        input_float[i] = (float)rand() / RAND_MAX * 50.0f;
        input_long[i] = (long)rand() * 1000L;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 16; j++) {
            blocks[i].ints[j] = rand() % 500;
            blocks[i].floats[j] = (float)rand() / RAND_MAX * 25.0f;
        }
        for (int j = 0; j < 8; j++) {
            blocks[i].doubles[j] = (double)rand() / RAND_MAX * 75.0;
            blocks[i].longs[j] = (long)rand() * 500L;
        }
    }
    
    /* Clear output arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        output_int[i] = 0;
        output_double[i] = 0.0;
        output_float[i] = 0.0f;
        output_long[i] = 0L;
    }
    
    /* Perform heavy computation */
    double result = compute_heavy(
        input_int, input_double, input_float, input_long,
        output_int, output_double, output_float, output_long,
        blocks, ITERATIONS
    );
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_int[i] + output_double[i] + output_float[i] + output_long[i];
    }
    checksum += result;
    
    /* Print checksum */
    printf("Checksum: %.15f\n", checksum);
    
    /* Free memory */
    free(input_int);
    free(input_double);
    free(input_float);
    free(input_long);
    free(output_int);
    free(output_double);
    free(output_float);
    free(output_long);
    
    return 0;
}
