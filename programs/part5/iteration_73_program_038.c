#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile arrays to force memory operations */
volatile int global_int_array[ARRAY_SIZE];
volatile double global_double_array[ARRAY_SIZE];
volatile float global_float_array[ARRAY_SIZE];

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(int a1, int a2, int a3, int a4,
                          float f1, float f2, float f3, float f4,
                          double d1, double d2, double d3, double d4,
                          void* p1, void* p2, long l1, long l2) {
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)((int*)p1)[0] * d3;
    result += (double)l1 * (double)l2 / 1000.0;
    result += sin(d4) * cos((double)f3);
    return result * (1.0 + (double)f4 / 100.0);
}

/* Critical function with extreme register pressure */
__attribute__((noinline, optimize("no-tree-vectorize", "no-unroll-loops")))
void compute_heavy(volatile int* input_int, volatile double* input_double, 
                   volatile float* input_float, volatile int* output) {
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int idx1, idx2, idx3, idx4;
    volatile int sink; /* Volatile sink to prevent optimization */
    
    /* Initialize some variables */
    i1 = 1; i2 = 2; i3 = 3; i4 = 4; i5 = 5;
    l1 = 1000L; l2 = 2000L; l3 = 3000L; l4 = 4000L;
    f1 = 1.1f; f2 = 2.2f; f3 = 3.3f; f4 = 4.4f; f5 = 5.5f;
    d1 = 1.01; d2 = 2.02; d3 = 3.03; d4 = 4.04; d5 = 5.05;
    
    /* Complex loop with data dependencies and mixed types */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * 7) % ARRAY_SIZE;
        idx2 = (iter * 13 + 5) % ARRAY_SIZE;
        idx3 = (iter * 17 + idx1) % ARRAY_SIZE;
        idx4 = (iter * 23 + idx2 * 3) % ARRAY_SIZE;
        
        /* Load from volatile arrays (creates MEM rtxes) */
        i6 = input_int[idx1] + input_int[idx2];
        i7 = input_int[idx3] * 2 - input_int[idx4];
        
        f6 = input_float[idx1] * 1.5f + input_float[idx2];
        f7 = input_float[idx3] / 2.0f - input_float[idx4];
        
        d6 = input_double[idx1] * 1.7 + input_double[idx2];
        d7 = input_double[idx3] / 3.0 - input_double[idx4];
        
        /* Long chain of mixed-type computations */
        /* Each computation depends on previous results */
        f8 = (float)i6 * f6 + (float)i7 * f7;
        d8 = (double)f8 * d6 + (double)i6 * d7;
        
        i8 = (int)d8 + i6 * i7 - (int)f8;
        l5 = (long)i8 * l1 + (long)i7 * l2;
        
        f9 = (float)l5 / 1000.0f + f6 * f7;
        d9 = (double)f9 * d6 / d7 + (double)l5 / 10000.0;
        
        i9 = (int)d9 * i8 + (int)f9 * i7;
        l6 = (long)i9 * l3 - (long)i8 * l4;
        
        f10 = (float)l6 * 0.001f + f8 * 0.5f;
        d10 = (double)f10 + d8 * 0.3 + d9 * 0.7;
        
        i10 = (int)d10 + i9 * 2 - (int)f10 * 3;
        l7 = (long)i10 * 100L + l5 * 2L - l6;
        
        /* More mixed computations */
        f1 = f1 * 1.1f + f10;
        f2 = f2 * 0.9f + f9;
        f3 = f3 * 1.2f + f8;
        f4 = f4 * 0.8f + f7;
        f5 = f5 * 1.05f + f6;
        
        d1 = d1 * 1.01 + d10;
        d2 = d2 * 0.99 + d9;
        d3 = d3 * 1.02 + d8;
        d4 = d4 * 0.98 + d7;
        d5 = d5 * 1.03 + d6;
        
        i1 = i1 + i10;
        i2 = i2 + i9;
        i3 = i3 + i8;
        i4 = i4 + i7;
        i5 = i5 + i6;
        
        l1 = l1 + l7;
        l2 = l2 + l6;
        l3 = l3 + l5;
        l4 = l4 + (long)i10 * 10L;
        
        /* Inline assembly that clobbers many registers */
        /* For x86_64 */
#if defined(__x86_64__)
        __asm__ volatile (
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
            :
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
        /* For AArch64 */
#elif defined(__aarch64__)
        __asm__ volatile (
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
            :
            :
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
              "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
              "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
              "x24", "x25", "x26", "x27", "x28",
              "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
              "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
              "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
              "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces register/stack reloads */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            (void*)&input_int[idx1], (void*)&input_int[idx2],
            l1, l2
        );
        
        /* Use the result in more computations */
        d5 = d5 + func_result;
        i5 = i5 + (int)func_result;
        f5 = f5 + (float)func_result;
        
        /* Complex store with volatile */
        sink = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
        output[iter % ARRAY_SIZE] = sink;
        
        /* More complex array indexing for next iteration */
        idx1 = (idx1 * 3 + 7) % ARRAY_SIZE;
        idx2 = (idx2 * 5 + 11) % ARRAY_SIZE;
    }
    
    /* Final volatile store to prevent dead code elimination */
    sink = i1 + i2 + i3 + (int)f1 + (int)d1 + (int)l1;
    output[0] = sink;
}

int main() {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    volatile int output_array[ARRAY_SIZE];
    
    /* Call the compute-heavy function */
    compute_heavy(global_int_array, global_double_array, 
                  global_float_array, output_array);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
