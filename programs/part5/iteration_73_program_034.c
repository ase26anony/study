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

/* Structure with mixed types for complex addressing */
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    int arr[4];
};

/* Argument-heavy helper function - marked noinline */
__attribute__((noinline))
static double heavy_args_func(
    int a1, long a2, float a3, double a4,
    int a5, long a6, float a7, double a8,
    int a9, long a10, float a11, double a12,
    int a13, long a14, float a15, double a16,
    volatile int* a17, volatile float* a18
) {
    /* Complex computation mixing all arguments */
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    
    /* Force memory accesses */
    sum += *a17 + *a18;
    
    return sum * 0.5;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(
    const int* input_int,
    const float* input_float,
    const double* input_double,
    int* output_int,
    float* output_float,
    double* output_double,
    struct MixedData* mixed_data,
    int size
) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int* p1, *p2, *p3;
    volatile float* fp1, *fp2, *fp3;
    volatile double* dp1, *dp2, *dp3;
    
    /* Additional variables for complex addressing */
    int idx1, idx2, idx3, idx4, idx5;
    long stride1, stride2;
    float f_idx1, f_idx2;
    double d_idx1, d_idx2;
    
    /* Initialize pointers */
    p1 = (volatile int*)input_int;
    fp1 = (volatile float*)input_float;
    dp1 = (volatile double*)input_double;
    
    /* Main computation loop with extreme register pressure */
    for (int i = 0; i < size; i++) {
        /* Complex array indexing with multiple terms */
        idx1 = i * 3;
        idx2 = i * 5;
        idx3 = i * 7;
        idx4 = i * 11;
        idx5 = i * 13;
        
        stride1 = i * 17;
        stride2 = i * 19;
        
        f_idx1 = (float)i * 1.234f;
        f_idx2 = (float)i * 5.678f;
        d_idx1 = (double)i * 2.345;
        d_idx2 = (double)i * 6.789;
        
        /* Load data with complex addressing */
        v1 = input_int[idx1];
        v2 = input_int[idx2 + 1];
        v3 = input_int[idx3 * 2];
        v4 = input_int[idx4 + idx5];
        v5 = input_int[stride1 % size];
        
        f1 = input_float[idx1];
        f2 = input_float[idx2 + 2];
        f3 = input_float[idx3 * 3];
        f4 = input_float[(idx4 + idx5) % size];
        f5 = input_float[stride2 % size];
        
        d1 = input_double[idx1];
        d2 = input_double[idx2 + 3];
        d3 = input_double[idx3 * 4];
        d4 = input_double[(idx4 * 2 + idx5) % size];
        d5 = input_double[(stride1 + stride2) % size];
        
        /* Structure field access with complex indexing */
        l1 = mixed_data[idx1 % (size/4)].l;
        l2 = mixed_data[idx2 % (size/4)].l;
        f6 = mixed_data[idx3 % (size/4)].f;
        d6 = mixed_data[idx4 % (size/4)].d;
        
        /* Long chain of mixed-type computations */
        /* This creates dependencies preventing register reuse */
        f7 = (float)v1 * f1 + (float)v2 * f2;
        d7 = (double)v3 * d1 + (double)v4 * d2;
        l3 = (long)(f3 * 1000.0f) + (long)(d3 * 1000.0);
        f8 = f4 * global_volatile_float + (float)d4;
        d8 = d5 * global_volatile_double + (double)f5;
        
        v6 = (int)(f7 * 10.0f) + (int)(d7 * 10.0);
        v7 = v5 + global_volatile_int + (int)l1;
        l4 = l2 + (long)v6 * 3L;
        f9 = f6 * 2.0f + (float)l3 * 0.001f;
        d9 = d6 * 3.0 + (double)v7 * 0.01;
        
        /* More mixed computations */
        v8 = (int)((f8 + f9) * 100.0f);
        v9 = (int)((d8 + d9) * 50.0);
        l5 = (long)v8 * (long)v9;
        f10 = (float)l4 * 0.0001f + (float)l5 * 0.00001f;
        d10 = (double)v8 * 0.001 + (double)v9 * 0.0001;
        
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
            "pxor %%xmm15, %%xmm15"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
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
            "fmov d31, #0.0"
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
        __asm__ volatile ("# Dummy assembly" ::: "memory");
#endif
        
        /* Call argument-heavy function with mixed types */
        double result = heavy_args_func(
            v1, l1, f1, d1,
            v2, l2, f2, d2,
            v3, l3, f3, d3,
            v4, l4, f4, d4,
            &global_volatile_int, &global_volatile_float
        );
        
        /* Use result in further computation */
        v10 = (int)(result * 1000.0);
        l6 = (long)(result * 1000000.0);
        f10 += (float)result;
        d10 += result;
        
        /* Store results with complex addressing */
        output_int[idx1 % size] = v6 + v7 + v8 + v9 + v10;
        output_float[idx2 % size] = f7 + f8 + f9 + f10;
        output_double[idx3 % size] = d7 + d8 + d9 + d10;
        
        /* More structure accesses */
        mixed_data[i % (size/4)].arr[0] = v6;
        mixed_data[i % (size/4)].arr[1] = v7;
        mixed_data[i % (size/4)].arr[2] = v8;
        mixed_data[i % (size/4)].arr[3] = v9;
        
        /* Force memory barrier */
        __asm__ volatile ("" ::: "memory");
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int* input_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* input_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* input_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    int* output_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* output_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* output_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    struct MixedData* mixed_data = (struct MixedData*)malloc(
        (ARRAY_SIZE/4) * sizeof(struct MixedData));
    
    if (!input_int || !input_float || !input_double ||
        !output_int || !output_float || !output_double ||
        !mixed_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_int[i] = rand() % 1000;
        input_float[i] = (float)rand() / (float)RAND_MAX * 100.0f;
        input_double[i] = (double)rand() / (double)RAND_MAX * 100.0;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        mixed_data[i].i = rand() % 1000;
        mixed_data[i].l = rand() * 1000L;
        mixed_data[i].f = (float)rand() / (float)RAND_MAX * 50.0f;
        mixed_data[i].d = (double)rand() / (double)RAND_MAX * 50.0;
        for (int j = 0; j < 4; j++) {
            mixed_data[i].arr[j] = rand() % 100;
        }
    }
    
    /* Perform heavy computation multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_heavy(
            input_int, input_float, input_double,
            output_int, output_float, output_double,
            mixed_data, ARRAY_SIZE
        );
        
        /* Swap buffers to create different access patterns */
        int* temp_int = input_int;
        input_int = output_int;
        output_int = temp_int;
        
        float* temp_float = input_float;
        input_float = output_float;
        output_float = temp_float;
        
        double* temp_double = input_double;
        input_double = output_double;
        output_double = temp_double;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    volatile long sink = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_int[i];
        checksum += (long)output_float[i];
        checksum += (long)output_double[i];
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        checksum += mixed_data[i].i;
        checksum += mixed_data[i].l;
        checksum += (long)mixed_data[i].f;
        checksum += (long)mixed_data[i].d;
        for (int j = 0; j < 4; j++) {
            checksum += mixed_data[i].arr[j];
        }
    }
    
    sink = checksum;
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(input_int);
    free(input_float);
    free(input_double);
    free(output_int);
    free(output_float);
    free(output_double);
    free(mixed_data);
    
    return 0;
}
