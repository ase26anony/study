#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
static double many_args_function(
    int a1, double b1, float c1, long d1,
    int a2, double b2, float c2, long d2,
    int a3, double b3, float c3, long d3,
    int a4, double b4, float c4, long d4,
    int a5, double b5, float c5, long d5,
    int a6, double b6, float c6, long d6
) {
    /* Complex mixing of types to require multiple register classes */
    double sum = (double)a1 * b1 + (double)c1 * d1;
    sum += (double)a2 * b2 + (double)c2 * d2;
    sum += (double)a3 * b3 + (double)c3 * d3;
    sum += (double)a4 * b4 + (double)c4 * d4;
    sum += (double)a5 * b5 + (double)c5 * d5;
    sum += (double)a6 * b6 + (double)c6 * d6;
    
    /* Use volatile globals to force memory reloads */
    sum += global_volatile_int + global_volatile_double + global_volatile_float;
    
    return sum;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static double compute_heavy(const int* input_int, const double* input_double, 
                           const float* input_float, long* output, int size) {
    /* Declare MANY local variables to create register pressure */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Additional non-volatile variables for computation chains */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long j1, j2, j3, j4, j5, j6, j7, j8, j9, j10;
    float g1, g2, g3, g4, g5, g6, g7, g8, g9, g10;
    double h1, h2, h3, h4, h5, h6, h7, h8, h9, h10;
    
    double total_sum = 0.0;
    
    /* Complex loop with data dependencies and mixed types */
    for (int idx = 0; idx < size; idx++) {
        /* Complex array indexing with multiple terms */
        int idx1 = (idx * 7 + 3) % size;
        int idx2 = (idx * 13 + 5) % size;
        int idx3 = (idx * 17 + 11) % size;
        int idx4 = (idx * 19 + 7) % size;
        
        /* Load data with complex addressing */
        i1 = input_int[idx1] + input_int[idx2];
        i2 = input_int[idx3] * input_int[idx4];
        i3 = input_int[idx] + global_volatile_int;
        
        d1 = input_double[idx1] * 1.5;
        d2 = input_double[idx2] + 2.5;
        d3 = input_double[idx3] / 3.0;
        
        f1 = input_float[idx1] * 0.5f;
        f2 = input_float[idx2] + 1.5f;
        f3 = input_float[idx3] - 0.25f;
        
        /* Long chain of mixed-type computations */
        h1 = (double)i1 * d1 + (double)f1;
        h2 = (double)i2 * d2 + (double)f2;
        h3 = (double)i3 * d3 + (double)f3;
        
        g1 = (float)h1 * 0.3f + (float)h2 * 0.7f;
        g2 = (float)h2 * 0.4f + (float)h3 * 0.6f;
        g3 = (float)h3 * 0.5f + (float)h1 * 0.5f;
        
        j1 = (long)(h1 * 1000.0);
        j2 = (long)(h2 * 1000.0);
        j3 = (long)(h3 * 1000.0);
        
        i4 = (int)g1 + (int)g2 + (int)g3;
        i5 = i4 * i1 - i2 + i3;
        
        h4 = (double)i5 / 17.0 + h1 - h2 + h3;
        h5 = h4 * 2.0 - h1 * 0.5 + h2 * 1.5;
        h6 = h5 / 3.0 + h4 * 0.25 - h3 * 0.75;
        
        g4 = (float)h4 + (float)h5 * 0.3f;
        g5 = (float)h5 + (float)h6 * 0.4f;
        g6 = (float)h6 + (float)h4 * 0.5f;
        
        i6 = (int)(g4 * 10) + (int)(g5 * 20) + (int)(g6 * 30);
        i7 = i6 * i4 - i5 + global_volatile_int;
        
        h7 = (double)i7 * 0.01 + h4 * 0.99;
        h8 = h7 * 1.1 + h5 * 0.9;
        h9 = h8 * 1.2 + h6 * 0.8;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__)
        __asm__ volatile (
            "# Clobber many registers\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15"
        );
        /* For AArch64 */
#elif defined(__aarch64__)
        __asm__ volatile (
            "# Clobber many registers\n\t"
            : 
            : 
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"
        );
#endif
        
        /* Continue computation after clobber */
        h10 = h7 + h8 + h9;
        g7 = (float)h10 * 0.333f;
        i8 = (int)(g7 * 100);
        
        /* Call function with many arguments */
        double func_result = many_args_function(
            i1, h1, g1, j1,
            i2, h2, g2, j2,
            i3, h3, g3, j3,
            i4, h4, g4, j1 + j2,
            i5, h5, g5, j2 + j3,
            i6, h6, g6, j1 + j3
        );
        
        h10 += func_result * 0.001;
        
        /* More mixed-type operations */
        i9 = i8 + (int)h10 + (int)g7;
        i10 = i9 * 2 - i8 / 2;
        
        g8 = (float)i10 * 0.01f + g7;
        g9 = g8 * 1.5f - g4 * 0.5f + g5 * 0.3f;
        g10 = g9 / 2.0f + g6 * 0.7f;
        
        h1 = (double)g10 + h10 * 0.5;
        h2 = h1 * 2.0 - h10;
        h3 = h2 / 3.0 + h1;
        
        /* Store result with complex indexing */
        output[idx] = (long)(h1 + h2 + h3 + i10 + g10);
        
        /* Update volatile variables to force memory stores */
        v1 = i1; v2 = i2; v3 = i3; v4 = i4; v5 = i5;
        v6 = i6; v7 = i7; v8 = i8; v9 = i9; v10 = i10;
        
        l1 = j1; l2 = j2; l3 = j3;
        
        f1 = g1; f2 = g2; f3 = g3; f4 = g4; f5 = g5;
        f6 = g6; f7 = g7; f8 = g8; f9 = g9; f10 = g10;
        
        d1 = h1; d2 = h2; d3 = h3; d4 = h4; d5 = h5;
        d6 = h6; d7 = h7; d8 = h8; d9 = h9; d10 = h10;
        
        total_sum += h1 + h2 + h3 + h4 + h5 + h6 + h7 + h8 + h9 + h10;
    }
    
    return total_sum;
}

int main() {
    /* Initialize arrays with pseudo-random data */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* output_array = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = rand() % 1000;
        double_array[i] = (double)(rand() % 10000) / 100.0;
        float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    /* Perform heavy computation */
    double total = 0.0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += compute_heavy(int_array, double_array, float_array, 
                              output_array, ARRAY_SIZE);
    }
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Total: %f\n", total);
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(output_array);
    
    return 0;
}
