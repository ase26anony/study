/* reload_stress.c - Program to stress GCC's reload pass */
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
static double heavy_calculation(
    int a1, long a2, float a3, double a4,
    int a5, long a6, float a7, double a8,
    int a9, long a10, float a11, double a12,
    int a13, long a14, float a15, double a16,
    volatile int* a17, volatile double* a18
) {
    /* Mix all arguments in complex ways */
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    
    /* Force memory accesses */
    sum += *a17 + *a18;
    
    /* Use volatile globals */
    sum += global_volatile_int + global_volatile_double;
    
    return sum * 0.5;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(struct MixedData* input, struct MixedData* output, int size) {
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Additional pointer/index variables */
    int idx1, idx2, idx3, idx4;
    volatile int sink_volatile = 0;
    volatile double sink_double = 0.0;
    
    /* Initialize some values */
    i1 = global_volatile_int;
    l1 = (long)i1 * 100;
    f1 = global_volatile_float;
    d1 = global_volatile_double;
    
    /* Complex loop with data dependencies */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        idx1 = outer % size;
        idx2 = (outer * 7) % size;
        idx3 = (outer * 13) % size;
        idx4 = (outer * 23) % size;
        
        /* Load from input with complex addressing */
        i2 = input[idx1].i + input[idx2].arr[outer % 8];
        i3 = input[idx3].i * 2 - input[idx4].arr[(outer + 1) % 8];
        
        l2 = input[idx1].l + (long)input[idx2].arr[outer % 8] * 3;
        l3 = input[idx3].l - (long)input[idx4].arr[(outer + 2) % 8] * 5;
        
        f2 = input[idx1].f + input[idx2].f * 1.5f;
        f3 = input[idx3].f - input[idx4].f * 2.5f;
        
        d2 = input[idx1].d + input[idx2].d * 1.7;
        d3 = input[idx3].d - input[idx4].d * 3.1;
        
        /* Long chain of mixed-type computations */
        i4 = (int)((float)i2 * f2 + (double)i3 * d2);
        i5 = (int)((double)l2 * d3 - (float)l3 * f3);
        
        f4 = (float)i4 * 0.25f + (float)l2 * 0.125f;
        f5 = (float)i5 * 0.333f - (float)l3 * 0.166f;
        
        d4 = (double)f4 * 1.234 + (double)i4 * 5.678;
        d5 = (double)f5 * 9.012 - (double)i5 * 3.456;
        
        l4 = (long)(d4 * 1000.0) + (long)(f4 * 100.0f);
        l5 = (long)(d5 * 2000.0) - (long)(f5 * 200.0f);
        
        i6 = i4 * i5 + (int)(l4 % 256);
        i7 = i2 * i3 - (int)(l5 % 128);
        
        f6 = f4 * f5 + (float)(i6 % 64);
        f7 = f2 * f3 - (float)(i7 % 32);
        
        d6 = d4 * d5 + (double)(i6 % 16);
        d7 = d2 * d3 - (double)(i7 % 8);
        
        l6 = l4 * l5 + (long)(d6 * 10.0);
        l7 = l2 * l3 - (long)(d7 * 20.0);
        
        i8 = (int)(sqrt(fabs((double)i6)) + sqrt(fabs((double)i7)));
        i9 = (int)(log(fabs(d6) + 1.0) * 100.0);
        
        f8 = sinf(f6) + cosf(f7);
        f9 = tanf(f4) * atanf(f5);
        
        d8 = sin(d6) * cos(d7);
        d9 = tan(d4) + atan(d5);
        
        l8 = (long)(d8 * 1000000.0) ^ (long)(d9 * 100000.0);
        l9 = (long)(f8 * 10000.0f) | (long)(f9 * 1000.0f);
        
        i10 = i8 ^ i9 ^ (int)l8 ^ (int)l9;
        
        /* Inline assembly that clobbers MANY registers */
        __asm__ volatile (
#if defined(__aarch64__)
            /* Clobber ARM64 registers */
            "mov x0, %0\n"
            "mov x1, %1\n"
            "mov x2, %2\n"
            "mov x3, %3\n"
            "mov x4, %4\n"
            "mov x5, %5\n"
            "mov x6, %6\n"
            "mov x7, %7\n"
            "mov x8, %8\n"
            "mov x9, %9\n"
            "mov x10, %10\n"
            "mov x11, %11\n"
            "mov x12, %12\n"
            "mov x13, %13\n"
            "mov x14, %14\n"
            "mov x15, %15\n"
            "mov x16, %16\n"
            "mov x17, %17\n"
            "mov x18, %18\n"
            "mov x19, %19\n"
            "mov x20, %20\n"
            "mov x21, %21\n"
            "mov x22, %22\n"
            "mov x23, %23\n"
            "mov x24, %24\n"
            "mov x25, %25\n"
            "mov x26, %26\n"
            "mov x27, %27\n"
            "mov x28, %28\n"
            "mov x29, %29\n"
            "mov x30, %30\n"
            :
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
              "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10),
              "r"(l1), "r"(l2), "r"(l3), "r"(l4), "r"(l5),
              "r"(l6), "r"(l7), "r"(l8), "r"(l9), "r"(idx1),
              "r"(idx2), "r"(idx3), "r"(idx4), "r"(outer),
              "r"(&sink_volatile), "r"(&sink_double),
              "r"(&global_volatile_int), "r"(&global_volatile_double),
              "r"(&global_volatile_float), "r"(input), "r"(output)
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
              "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
              "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",
              "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19",
              "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
#elif defined(__x86_64__)
            /* Clobber x86_64 registers */
            "mov %0, %%rax\n"
            "mov %1, %%rbx\n"
            "mov %2, %%rcx\n"
            "mov %3, %%rdx\n"
            "mov %4, %%rsi\n"
            "mov %5, %%rdi\n"
            "mov %6, %%r8\n"
            "mov %7, %%r9\n"
            "mov %8, %%r10\n"
            "mov %9, %%r11\n"
            "mov %10, %%r12\n"
            "mov %11, %%r13\n"
            "mov %12, %%r14\n"
            "mov %13, %%r15\n"
            :
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
              "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10),
              "r"(l1), "r"(l2), "r"(l3), "r"(l4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#endif
        );
        
        /* Call argument-heavy function - forces more register pressure */
        d10 = heavy_calculation(
            i1, l1, f1, d1,
            i2, l2, f2, d2,
            i3, l3, f3, d3,
            i4, l4, f4, d4,
            &sink_volatile, &sink_double
        );
        
        /* More mixed computations after call */
        f10 = (float)d10 * 0.5f + f5 * 0.25f;
        l10 = (long)(d10 * 100.0) + l5;
        
        /* Complex array store with multiple index terms */
        int store_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7 + idx4 * 11) % size;
        output[store_idx].i = i10;
        output[store_idx].l = l10;
        output[store_idx].f = f10;
        output[store_idx].d = d10;
        
        /* Volatile store to force memory operation */
        sink_volatile = i10;
        sink_double = d10;
        
        /* Update variables for next iteration (create dependencies) */
        i1 = i10 / 2;
        l1 = l10 >> 1;
        f1 = f10 * 0.9f;
        d1 = d10 * 0.8;
    }
}

int main() {
    /* Initialize with pseudo-random data */
    struct MixedData* input = malloc(ARRAY_SIZE * sizeof(struct MixedData));
    struct MixedData* output = malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with varied data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].i = (i * 123456789) % 1000;
        input[i].l = (long)i * 987654321;
        input[i].f = (float)i * 1.2345f;
        input[i].d = (double)i * 3.14159;
        
        for (int j = 0; j < 8; j++) {
            input[i].arr[j] = (i * j * 13579) % 256;
        }
        
        /* Initialize output */
        output[i].i = 0;
        output[i].l = 0L;
        output[i].f = 0.0f;
        output[i].d = 0.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ARRAY_SIZE);
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].i;
        checksum += output[i].l;
        checksum += (long)(output[i].f * 1000.0f);
        checksum += (long)(output[i].d * 1000.0);
        
        for (int j = 0; j < 8; j++) {
            checksum += output[i].arr[j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
