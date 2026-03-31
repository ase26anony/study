/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile arrays to force memory operations */
volatile int global_int_array[ARRAY_SIZE];
volatile double global_double_array[ARRAY_SIZE];
volatile float global_float_array[ARRAY_SIZE];

/* Helper function with many arguments to stress argument passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((intptr_t)p1 % 1000);
    result -= (double)((intptr_t)p2 % 500);
    result += d3 * d4;
    result += (double)a3 * (double)a4;
    result += (double)f3 * (double)f4;
    
    /* Volatile to prevent optimization */
    volatile double sink = result;
    return sink + 0.1;
}

/* Critical computation function with extreme register pressure */
__attribute__((noinline))
static void compute_heavy(volatile int* input_int, 
                         volatile double* input_double,
                         volatile float* input_float,
                         volatile int* output_int,
                         volatile double* output_double,
                         int size)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    register int i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0;
    register int i6 = 0, i7 = 0, i8 = 0, i9 = 0, i10 = 0;
    register int i11 = 0, i12 = 0, i13 = 0, i14 = 0, i15 = 0;
    
    /* Long variables */
    register long l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0;
    register long l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0;
    
    /* Float variables */
    register float f1 = 0.0f, f2 = 0.0f, f3 = 0.0f, f4 = 0.0f, f5 = 0.0f;
    register float f6 = 0.0f, f7 = 0.0f, f8 = 0.0f, f9 = 0.0f, f10 = 0.0f;
    
    /* Double variables */
    register double d1 = 0.0, d2 = 0.0, d3 = 0.0, d4 = 0.0, d5 = 0.0;
    register double d6 = 0.0, d7 = 0.0, d8 = 0.0, d9 = 0.0, d10 = 0.0;
    
    /* Pointer variables */
    volatile int* p1 = input_int;
    volatile double* p2 = input_double;
    volatile float* p3 = input_float;
    volatile int* p4 = output_int;
    volatile double* p5 = output_double;
    
    /* Index variables for complex array access */
    register int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
    register int stride1 = 16, stride2 = 32, stride3 = 8;
    
    /* Main computation loop with unbroken dependency chain */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Reset indices */
        idx1 = outer % 64;
        idx2 = (outer * 3) % 64;
        idx3 = (outer * 5) % 64;
        idx4 = (outer * 7) % 64;
        
        /* COMPLEX ARRAY ACCESS PATTERNS - stress addressing modes */
        /* Multi-term array indexing */
        i1 = input_int[idx1 * stride1 + idx2 + idx3];
        i2 = input_int[idx2 * stride2 + idx3 * 2 + idx4];
        i3 = input_int[idx3 * stride3 + idx1 * 3 + outer];
        i4 = input_int[idx4 * 16 + idx2 * 4 + idx1];
        
        /* Floating point array accesses with complex indices */
        f1 = input_float[idx1 * 8 + idx2];
        f2 = input_float[idx2 * 12 + idx3];
        d1 = input_double[idx3 * 4 + idx4];
        d2 = input_double[idx4 * 6 + idx1];
        
        /* LONG DEPENDENCY CHAIN - prevents register reuse */
        /* Integer chain */
        i5 = i1 + i2 * 3 - i3 / 2 + i4;
        i6 = i5 * 2 - i2 + i3 * 3;
        i7 = i6 + i4 * 5 - i1 * 2;
        i8 = i7 / 2 + i5 * 3 - i6;
        i9 = i8 * 7 + i3 - i2 * 4;
        i10 = i9 - i7 + i8 * 2;
        i11 = i10 * 3 + i5 - i9;
        i12 = i11 / 4 + i6 * 2 - i10;
        i13 = i12 * 5 + i8 - i11;
        i14 = i13 + i9 * 3 - i12;
        i15 = i14 * 2 - i13 + i10;
        
        /* Long chain with integer mixing */
        l1 = (long)i1 * i2 + (long)i3;
        l2 = l1 * 3 - (long)i4 * 2;
        l3 = l2 + (long)i5 * 5 - l1;
        l4 = l3 * 2 + (long)i6 - l2 * 3;
        l5 = l4 / 2 + (long)i7 * 7;
        l6 = l5 - l3 + (long)i8 * 4;
        l7 = l6 * 3 + (long)i9 - l4;
        l8 = l7 / 5 + (long)i10 * 2;
        l9 = l8 - l5 + (long)i11 * 3;
        l10 = l9 * 2 + (long)i12 - l6;
        
        /* Float/double chain with type mixing */
        f3 = (float)i1 * 0.5f + f1;
        f4 = f2 * 2.0f - (float)i2 * 0.25f;
        f5 = f3 + f4 * 1.5f;
        f6 = (float)l1 * 0.1f + f5;
        f7 = f6 * 3.14f - (float)i3 * 0.33f;
        f8 = f7 + (float)l2 * 0.01f;
        f9 = f8 * 2.71f - f4;
        f10 = f9 + (float)i4 * 0.75f;
        
        d3 = (double)f1 * 1.1 + d1;
        d4 = d2 * 2.2 - (double)f2 * 0.9;
        d5 = d3 + d4 * 1.5 + (double)i5 * 0.01;
        d6 = (double)l3 * 0.001 + d5;
        d7 = d6 * 3.14159 - (double)f3 * 1.1;
        d8 = d7 + (double)l4 * 0.0001;
        d9 = d8 * 2.71828 - d4;
        d10 = d9 + (double)i6 * 0.5 + (double)f4;
        
        /* INLINE ASSEMBLY WITH MANY CLOBBERS - forces spills/reloads */
        /* x86_64 version - clobber most registers */
#if defined(__x86_64__)
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
#elif defined(__aarch64__)
        /* ARM64 version */
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28", "x29", "x30",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
        );
#else
        /* Generic version */
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            :
            :
            : "memory"
        );
#endif

        /* CALL FUNCTION WITH MANY ARGUMENTS - stresses argument passing */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)p1, (void*)p2
        );
        
        /* Use function result in further computation */
        d10 += func_result * 0.5;
        f10 += (float)func_result;
        
        /* More complex array writes with multi-term indices */
        output_int[idx1 * stride1 + idx2] = i15;
        output_int[idx2 * stride2 + idx3] = i14;
        output_double[idx3 * 4 + idx4] = d10;
        output_double[idx4 * 6 + idx1] = d9;
        
        /* Structure-like access pattern using multiple arrays */
        int combined_idx = (idx1 * 17 + idx2 * 13 + idx3 * 11 + idx4 * 7) % ARRAY_SIZE;
        global_int_array[combined_idx] = i13;
        global_double_array[combined_idx] = d8;
        global_float_array[combined_idx] = f10;
        
        /* Volatile sink to prevent dead code elimination */
        volatile double sink = d10 + f10 + i15 + l10;
        (void)sink;
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    srand(time(NULL));
    
    int* input_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* input_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* input_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    int* output_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_int || !input_double || !input_float || 
        !output_int || !output_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with varied data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_int[i] = rand() % 1000;
        input_double[i] = (double)rand() / RAND_MAX * 100.0;
        input_float[i] = (float)rand() / RAND_MAX * 50.0f;
        output_int[i] = 0;
        output_double[i] = 0.0;
        
        global_int_array[i] = 0;
        global_double_array[i] = 0.0;
        global_float_array[i] = 0.0f;
    }
    
    /* Perform heavy computation */
    compute_heavy((volatile int*)input_int,
                  (volatile double*)input_double,
                  (volatile float*)input_float,
                  (volatile int*)output_int,
                  (volatile double*)output_double,
                  ARRAY_SIZE);
    
    /* Compute checksum to prevent optimization */
    long checksum = 0;
    double dchecksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_int[i];
        dchecksum += output_double[i];
        checksum += global_int_array[i];
        dchecksum += global_double_array[i];
    }
    
    printf("Checksum: int=%ld, double=%f\n", checksum, dchecksum);
    
    /* Cleanup */
    free(input_int);
    free(input_double);
    free(input_float);
    free(output_int);
    free(output_double);
    
    return 0;
}
