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
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    int* p1, float* p2, double* p3, volatile int* p4
) {
    /* Complex computation mixing all argument types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result -= (double)(a3 + a4) / d3;
    result += (*p1) * (*p2) * (*p3) + (*p4);
    
    /* Force memory barrier */
    __asm__ volatile("" ::: "memory");
    
    return result * 0.999;
}

/* Critical computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(volatile int* input_int, volatile double* input_double, 
                   volatile float* input_float, volatile int* output) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Floating point variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Pointer variables */
    volatile int* p1;
    volatile double* p2;
    volatile float* p3;
    
    /* Index variables for complex array access */
    int idx1, idx2, idx3, idx4, idx5;
    int stride1, stride2, stride3;
    
    /* Initialize local variables */
    v1 = *input_int; v2 = v1 + 1; v3 = v2 * 2; v4 = v3 / 3; v5 = v4 - 5;
    v6 = v5 ^ 0xFF; v7 = v6 << 2; v8 = v7 >> 1; v9 = v8 | 0xAA; v10 = v9 & 0x55;
    
    i1 = v1; i2 = v2; i3 = v3; i4 = v4; i5 = v5;
    i6 = v6; i7 = v7; i8 = v8; i9 = v9; i10 = v10;
    
    l1 = (long)i1 * 1000; l2 = l1 + i2; l3 = l2 * i3; l4 = l3 / i4; l5 = l4 - i5;
    l6 = l5 ^ 0xFFFF; l7 = l6 << 4; l8 = l7 >> 2; l9 = l8 | 0xAAAA; l10 = l9 & 0x5555;
    
    f1 = (float)i1 * 0.1f; f2 = f1 + (float)i2; f3 = f2 * (float)i3; 
    f4 = f3 / (float)i4; f5 = f4 - (float)i5;
    f6 = (float)l6 * 0.01f; f7 = f6 + (float)l7; f8 = f7 * (float)l8;
    f9 = f8 / (float)l9; f10 = f9 - (float)l10;
    
    d1 = (double)i1 * 0.01; d2 = d1 + (double)i2; d3 = d2 * (double)i3;
    d4 = d3 / (double)i4; d5 = d4 - (double)i5;
    d6 = (double)l6 * 0.001; d7 = d6 + (double)l7; d8 = d7 * (double)l8;
    d9 = d8 / (double)l9; d10 = d9 - (double)l10;
    
    p1 = input_int; p2 = input_double; p3 = input_float;
    
    stride1 = 16; stride2 = 32; stride3 = 64;
    idx1 = 0; idx2 = 8; idx3 = 16; idx4 = 24; idx5 = 32;
    
    /* Main computation loop with extreme register pressure */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multiple terms */
        int array_idx = idx1 * stride1 + idx2 * stride2 + idx3 + idx4 - idx5;
        array_idx = array_idx & (ARRAY_SIZE - 1); /* Ensure bounds */
        
        /* Load from volatile arrays (creates MEM rtx) */
        int load1 = input_int[array_idx];
        double load2 = input_double[array_idx * 2 % ARRAY_SIZE];
        float load3 = input_float[array_idx * 3 % ARRAY_SIZE];
        
        /* Long chain of mixed-type computations */
        /* This creates dependencies preventing register reuse */
        f1 = (float)load1 * (float)d1 + f2;
        d2 = (double)f1 * d3 + (double)load3;
        i2 = (int)d2 + i3 * load1;
        l3 = (long)i2 * l4 + (long)((int)f3);
        f4 = (float)l3 * f5 + (float)i4;
        d5 = (double)f4 * d6 + d7;
        i5 = (int)d5 ^ i6;
        l6 = l5 * l7 + (long)i5;
        f7 = f6 * (float)l6 + f8;
        d8 = d7 * (double)f7 + d9;
        i7 = i6 + (int)d8 * i8;
        l8 = l7 ^ l9 + (long)i7;
        f9 = f8 / (float)l8 * f10;
        d10 = d9 * 2.0 - d8 + (double)f9;
        
        /* More mixed computations */
        float ftmp1 = f1 * 1.1f + f2 * 0.9f;
        float ftmp2 = f3 * 2.2f - f4 * 1.8f;
        double dtmp1 = d1 * 1.01 + d2 * 0.99;
        double dtmp2 = d3 * 2.02 - d4 * 1.98;
        
        int itmp1 = i1 + i2 * 3 - i3 / 2;
        int itmp2 = i4 ^ i5 | i6 & i7;
        long ltmp1 = l1 * l2 + l3 - l4;
        long ltmp2 = l5 | l6 & l7 ^ l8;
        
        /* Complex expression mixing all types */
        d1 = (double)itmp1 * dtmp1 + (double)ftmp1;
        f2 = (float)ltmp1 * ftmp2 - (float)dtmp2;
        i3 = (int)d1 * (int)f2 + itmp2;
        l4 = (long)i3 * ltmp2 + (long)itmp1;
        
        /* Inline assembly that clobbers MANY registers */
        /* For x86_64 */
#if defined(__x86_64__)
        __asm__ volatile(
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
              "memory"
        );
/* For AArch64 */
#elif defined(__aarch64__)
        __asm__ volatile(
            "# Clobber many registers\n\t"
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
              "memory"
        );
#endif
        
        /* Call function with many arguments - forces register/stack reloads */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            &i5, &f5, &d5, &v1
        );
        
        /* Use function result in further computation */
        d6 = d5 * func_result + d7;
        f6 = f5 * (float)func_result - f7;
        i6 = i5 + (int)(func_result * 1000.0);
        
        /* More complex array access with structure-like pattern */
        int idx_a = (iter * 7) % ARRAY_SIZE;
        int idx_b = (iter * 13) % ARRAY_SIZE;
        int idx_c = (iter * 23) % ARRAY_SIZE;
        
        /* Complex addressing mode simulation */
        int complex_idx = (idx_a * 3 + idx_b * 5 + idx_c * 7) % ARRAY_SIZE;
        float complex_load = input_float[complex_idx] + 
                            input_float[(complex_idx + 1) % ARRAY_SIZE] -
                            input_float[(complex_idx + 2) % ARRAY_SIZE];
        
        /* Update variables to maintain dependencies */
        f7 = f6 * complex_load + f8;
        d7 = d6 / (double)complex_load - d8;
        i7 = i6 ^ (int)complex_load;
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 16;
        idx2 = (idx2 + 3) % 32;
        idx3 = (idx3 + 5) % 64;
        idx4 = (idx4 + 7) % 128;
        idx5 = (idx5 + 11) % 256;
        
        /* Store result to volatile output (creates MEM rtx) */
        output[iter % ARRAY_SIZE] = i7 + (int)f7 + (int)d7;
    }
    
    /* Final sink to volatile variable */
    volatile int final_sink = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    (void)final_sink; /* Prevent unused variable warning */
}

int main() {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    volatile int output_array[ARRAY_SIZE];
    
    /* Perform heavy computation */
    compute_heavy(global_int_array, global_double_array, 
                  global_float_array, output_array);
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= output_array[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
