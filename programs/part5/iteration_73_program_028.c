/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operations */
volatile int volatile_global_int = 42;
volatile double volatile_global_double = 3.14159;
volatile float volatile_global_float = 2.71828f;

/* Helper with many arguments to stress argument passing */
__attribute__((noinline))
static double many_args_function(
    int a1, double a2, float a3, long a4,
    int a5, double a6, float a7, long a8,
    int a9, double a10, float a11, long a12,
    int a13, double a14, float a15, long a16,
    volatile int* a17, volatile double* a18)
{
    /* Complex mixing of types */
    double sum = a2 + a6 + a10 + a14;
    sum += (double)a3 + (double)a7 + (double)a11 + (double)a15;
    sum += (double)a1 + (double)a5 + (double)a9 + (double)a13;
    sum += (double)a4 + (double)a8 + (double)a12 + (double)a16;
    
    /* Force memory accesses */
    sum += *a17 + *a18;
    
    return sum * 0.5;
}

/* Complex structure for array indexing */
struct complex_index {
    int stride;
    int offset;
    int scale;
    volatile int modifier;
};

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(
    const double* input,
    double* output,
    int size,
    struct complex_index* idx_info)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1 = volatile_global_int;
    int i1 = v1 * 2;
    int i2 = i1 + 1;
    int i3 = i2 * 3;
    int i4 = i3 - 4;
    int i5 = i4 / 2;
    int i6 = i5 * 7;
    int i7 = i6 + 8;
    int i8 = i7 - 9;
    int i9 = i8 * 10;
    int i10 = i9 / 3;
    
    /* Long variables */
    long l1 = i1 * 100L;
    long l2 = l1 + i2;
    long l3 = l2 * i3;
    long l4 = l3 - i4;
    long l5 = l4 / 5L;
    
    /* Float variables */
    volatile float vf = volatile_global_float;
    float f1 = vf * 1.1f;
    float f2 = f1 + 2.2f;
    float f3 = f2 * 3.3f;
    float f4 = f3 - 4.4f;
    float f5 = f4 / 5.5f;
    float f6 = f5 * 6.6f;
    float f7 = f6 + 7.7f;
    float f8 = f7 - 8.8f;
    
    /* Double variables */
    volatile double vd = volatile_global_double;
    double d1 = vd * 1.111;
    double d2 = d1 + 2.222;
    double d3 = d2 * 3.333;
    double d4 = d3 - 4.444;
    double d5 = d4 / 5.555;
    double d6 = d5 * 6.666;
    double d7 = d6 + 7.777;
    double d8 = d7 - 8.888;
    double d9 = d8 * 9.999;
    double d10 = d9 / 10.101;
    
    /* Pointer variables */
    const double* p1 = input;
    double* p2 = output;
    volatile int* p3 = &volatile_global_int;
    
    /* Index variables for complex addressing */
    int idx1 = 0;
    int idx2 = idx_info->stride;
    int idx3 = idx_info->offset;
    int idx4 = idx_info->scale;
    volatile int idx_mod = idx_info->modifier;
    
    /* Main computation loop with unbroken dependency chain */
    for (int iter = 0; iter < ITERATIONS && iter < size; iter++) {
        /* Complex array indexing with multiple terms */
        int complex_idx = (iter * idx2 + idx3) * idx4 + idx_mod + i1;
        
        /* Load with complex addressing - creates MEM rtx */
        double load_val = input[complex_idx % size];
        
        /* Long chain of mixed-type computations */
        /* Integer to float conversions */
        f1 = (float)i1 * (float)load_val + f2;
        f2 = (float)i2 * f1 - f3;
        
        /* Float to double conversions */
        d1 = (double)f3 * d2 + (double)f4;
        d2 = (double)f5 * d1 - d3;
        
        /* Integer arithmetic */
        i3 = i1 * i2 + i4;
        i4 = i3 - i5 * i6;
        
        /* Double precision */
        d3 = d4 * d5 + d6;
        d4 = d7 / d8 - d9;
        
        /* More mixed operations */
        f3 = (float)d1 * f6 + (float)i3;
        f4 = (float)d2 * f7 - (float)i4;
        
        d5 = (double)f8 * d10 + (double)l1;
        d6 = (double)f1 * d9 - (double)l2;
        
        /* Long integer operations */
        l3 = (long)i5 * l4 + l5;
        l4 = (long)i6 * l3 - l1;
        
        /* More conversions and mixing */
        f5 = (float)l2 * 0.5f + f3;
        f6 = (float)l3 * 1.5f - f4;
        
        d7 = d3 * 2.0 + (double)f5;
        d8 = d4 * 3.0 - (double)f6;
        
        i7 = (int)d5 + i8 * 2;
        i8 = (int)d6 - i9 / 3;
        
        f7 = (float)i7 * 0.7f + f8;
        f8 = (float)i8 * 0.8f - f1;
        
        d9 = (double)i9 * 0.9 + d7;
        d10 = (double)i10 * 1.1 - d8;
        
        /* Inline assembly that clobbers MANY registers */
        /* This forces spills and reloads */
        __asm__ volatile (
#if defined(__aarch64__)
            /* Clobber all general purpose and floating point registers */
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
            /* Also clobber floating point registers */
            "fmov d0, %31\n"
            "fmov d1, %32\n"
            "fmov d2, %33\n"
            "fmov d3, %34\n"
            "fmov d4, %35\n"
            "fmov d5, %36\n"
            "fmov d6, %37\n"
            "fmov d7, %38\n"
            "fmov d8, %39\n"
            "fmov d9, %40\n"
            "fmov d10, %41\n"
            "fmov d11, %42\n"
            "fmov d12, %43\n"
            "fmov d13, %44\n"
            "fmov d14, %45\n"
            "fmov d15, %46\n"
            "fmov d16, %47\n"
            "fmov d17, %48\n"
            "fmov d18, %49\n"
            "fmov d19, %50\n"
            "fmov d20, %51\n"
            "fmov d21, %52\n"
            "fmov d22, %53\n"
            "fmov d23, %54\n"
            "fmov d24, %55\n"
            "fmov d25, %56\n"
            "fmov d26, %57\n"
            "fmov d27, %58\n"
            "fmov d28, %59\n"
            "fmov d29, %60\n"
            "fmov d30, %61\n"
            "fmov d31, %62\n"
            : /* no outputs */
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
              "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10),
              "r"(l1), "r"(l2), "r"(l3), "r"(l4), "r"(l5),
              "r"(p1), "r"(p2), "r"(p3), "r"(idx1), "r"(idx2),
              "r"(idx3), "r"(idx4), "r"(idx_mod), "r"(complex_idx),
              "r"(iter), "r"(size), "r"(idx_info), "r"(v1),
              "r"(volatile_global_int), "r"(volatile_global_double),
              "r"(volatile_global_float),
              "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
              "r"(f6), "r"(f7), "r"(f8), "r"(d1), "r"(d2),
              "r"(d3), "r"(d4), "r"(d5), "r"(d6), "r"(d7),
              "r"(d8), "r"(d9), "r"(d10), "r"(load_val),
              "r"(vf), "r"(vd)
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
#elif defined(__x86_64__)
            /* For x86_64 - clobber many registers */
            "movq %0, %%rax\n"
            "movq %1, %%rbx\n"
            "movq %2, %%rcx\n"
            "movq %3, %%rdx\n"
            "movq %4, %%rsi\n"
            "movq %5, %%rdi\n"
            "movq %6, %%r8\n"
            "movq %7, %%r9\n"
            "movq %8, %%r10\n"
            "movq %9, %%r11\n"
            "movq %10, %%r12\n"
            "movq %11, %%r13\n"
            "movq %12, %%r14\n"
            "movq %13, %%r15\n"
            /* Floating point */
            "movsd %31, %%xmm0\n"
            "movsd %32, %%xmm1\n"
            "movsd %33, %%xmm2\n"
            "movsd %34, %%xmm3\n"
            "movsd %35, %%xmm4\n"
            "movsd %36, %%xmm5\n"
            "movsd %37, %%xmm6\n"
            "movsd %38, %%xmm7\n"
            "movsd %39, %%xmm8\n"
            "movsd %40, %%xmm9\n"
            "movsd %41, %%xmm10\n"
            "movsd %42, %%xmm11\n"
            "movsd %43, %%xmm12\n"
            "movsd %44, %%xmm13\n"
            "movsd %45, %%xmm14\n"
            "movsd %46, %%xmm15\n"
            : /* no outputs */
            : "r"(i1), "r"(i2), "r"(i3), "r"(i4), "r"(i5),
              "r"(i6), "r"(i7), "r"(i8), "r"(i9), "r"(i10),
              "r"(l1), "r"(l2), "r"(l3), "r"(l4), "r"(l5),
              "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5),
              "r"(f6), "r"(f7), "r"(f8), "r"(d1), "r"(d2),
              "r"(d3), "r"(d4), "r"(d5), "r"(d6), "r"(d7),
              "r"(d8), "r"(d9), "r"(d10)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
#endif
        );
        
        /* Call function with many arguments - stresses argument passing reloads */
        double func_result = many_args_function(
            i1, d1, f1, l1,
            i2, d2, f2, l2,
            i3, d3, f3, l3,
            i4, d4, f4, l4,
            p3, &volatile_global_double);
        
        /* Use result in computation */
        d10 = d9 * func_result + d8;
        
        /* Complex store with addressing */
        output[complex_idx % size] = d10 + (double)load_val;
        
        /* Update indices for next iteration */
        idx_mod++;
        idx_info->modifier = idx_mod;
    }
    
    /* Volatile sink to prevent elimination */
    volatile double sink = d10 + f8 + i10 + l5;
    (void)sink;
}

int main(void) {
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    double* input_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (double)rand() / RAND_MAX * 100.0;
        output_array[i] = 0.0;
    }
    
    /* Complex index structure */
    struct complex_index idx_info = {
        .stride = 7,
        .offset = 3,
        .scale = 2,
        .modifier = 1
    };
    
    /* Perform heavy computation */
    compute_heavy(input_array, output_array, ARRAY_SIZE, &idx_info);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(input_array);
    free(output_array);
    
    return 0;
}
