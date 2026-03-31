/* reload_stress.c - Program to stress GCC's reload pass */
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

/* Complex structure for varied access patterns */
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    int arr[8];
};

/* Argument-heavy helper function - forces register/stack pressure */
__attribute__((noinline))
static double heavy_callee(int a1, long a2, float a3, double a4,
                          int a5, long a6, float a7, double a8,
                          int a9, long a10, float a11, double a12,
                          int a13, long a14, float a15, double a16) {
    /* Complex mixing of arguments */
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    return sum * 0.5;
}

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(const int* input, double* output, 
                         const struct MixedData* data, int size) {
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int idx1, idx2, idx3, idx4;
    volatile int sink_volatile; /* Force memory reloads */
    
    /* Initialize with complex expressions */
    i1 = input[0] + global_volatile_int;
    l1 = (long)i1 * 37;
    f1 = (float)l1 * 0.123f + global_volatile_float;
    d1 = (double)f1 * 2.345 + global_volatile_double;
    
    /* Create long dependency chains */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % size;
        idx2 = (iter * 13) % size;
        idx3 = (iter * 23) % size;
        idx4 = (iter * 47) % size;
        
        /* Load with complex addressing modes */
        i2 = input[idx1 * 2 + idx2] + data[idx3].arr[idx4 % 8];
        i3 = data[idx2].i * 3 - input[idx4];
        i4 = i1 + i2 * i3;
        i5 = i4 / (i2 + 1) + data[idx1].arr[iter % 8];
        
        /* Mixed integer/long operations */
        l2 = (long)i2 * l1 + (long)i3;
        l3 = l2 / (l1 + 1) * 17;
        l4 = l3 - (long)i4 * 29;
        l5 = l4 + (long)data[idx3].l;
        
        /* Mixed float/double operations with conversions */
        f2 = (float)i2 * 0.456f + (float)l2 * 0.001f;
        f3 = f1 * f2 - (float)i3;
        f4 = f3 / (f2 + 0.1f) + data[idx4].f;
        f5 = f4 * 2.0f - global_volatile_float;
        
        d2 = (double)f2 * 1.234 + (double)l3;
        d3 = d1 * d2 - (double)i4;
        d4 = d3 / (d2 + 0.01) + data[idx1].d;
        d5 = d4 * 3.14159 + global_volatile_double;
        
        /* More dependency chains mixing all types */
        i6 = (int)f3 + (int)d3 * i5;
        i7 = i6 - (int)((float)l4 * 0.5f);
        i8 = i7 * 2 + data[idx2].arr[i6 % 8];
        
        l6 = (long)d4 * 11 + l5;
        l7 = l6 / (l5 + 1) + (long)i7;
        l8 = l7 - (long)((double)i8 * 0.25);
        
        f6 = (float)d4 * 0.333f + f5;
        f7 = f6 * (float)i6 - (float)l6 * 0.001f;
        f8 = f7 / (f6 + 0.5f) + (float)data[idx3].i;
        
        d6 = (double)f6 * 1.5 + d5;
        d7 = d6 * (double)i7 - (double)l7;
        d8 = d7 / (d6 + 0.001) + data[idx4].d;
        
        /* Inline assembly that clobbers MANY registers */
        /* For x86_64 */
        #if defined(__x86_64__)
        asm volatile(
            "# Clobber many registers\n"
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
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        /* For AArch64 */
        #elif defined(__aarch64__)
        asm volatile(
            "# Clobber many registers\n"
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
        
        /* Continue dependency chain after clobber */
        i9 = i8 + (int)f7 * 2;
        i10 = i9 - (int)d7 + global_volatile_int;
        
        l7 = l8 + (long)f8 * 3;
        l8 = l7 - (long)d8 * 2;
        
        f9 = f8 * 1.1f + (float)i9 * 0.01f;
        f10 = f9 - (float)l8 * 0.001f + global_volatile_float;
        
        d9 = d8 * 1.01 + (double)i10 * 0.001;
        d10 = d9 - (double)l7 * 0.0001 + global_volatile_double;
        
        /* Call argument-heavy function with mixed types */
        double call_result = heavy_callee(
            i1, l1, f1, d1,
            i2, l2, f2, d2,
            i3, l3, f3, d3,
            i4, l4, f4, d4
        );
        
        /* Use result in further computations */
        d10 += call_result * 0.1;
        f10 += (float)call_result;
        i10 += (int)call_result;
        
        /* Complex store with addressing */
        int store_idx = (idx1 + idx2 + idx3 + idx4) % size;
        output[store_idx] = d10 + (double)f10 + (double)i10 + (double)l8;
        
        /* Volatile sink to force stores */
        sink_volatile = i10;
        
        /* Rotate values for next iteration */
        i1 = i10;
        l1 = l8;
        f1 = f10;
        d1 = d10;
    }
}

int main(void) {
    /* Allocate and initialize data */
    int* input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct MixedData* data = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!input || !output || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = (i * 123456789) % 1000;
        output[i] = 0.0;
        data[i].i = (i * 987654321) % 500;
        data[i].l = (long)i * 12345;
        data[i].f = (float)i * 0.123f;
        data[i].d = (double)i * 0.456;
        for (int j = 0; j < 8; j++) {
            data[i].arr[j] = (i + j * 111) % 100;
        }
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, data, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        if (i % 7 == 0) {
            checksum += input[i];
            checksum += data[i].d;
        }
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(data);
    
    return 0;
}
