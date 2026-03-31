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

/* Helper function with many arguments to stress argument passing */
__attribute__((noinline))
static double many_args_function(
    int a1, int a2, int a3, int a4,
    double b1, double b2, double b3, double b4,
    float c1, float c2, float c3, float c4,
    long d1, long d2, int* e1, float* e2)
{
    /* Complex computation mixing all types */
    double result = (double)a1 * b1 + (double)a2 * b2;
    result += (double)c1 * (double)c2;
    result += (double)d1 / (b3 + 1.0);
    result += (*e1) * (*e2);
    
    /* Force memory access */
    *e1 += a3 + a4;
    *e2 += c3 + c4;
    
    return result + b4 + d2;
}

/* Critical function with extreme register pressure */
__attribute__((noinline))
static double compute_heavy(volatile int* input_int, 
                           volatile double* input_double,
                           volatile float* input_float,
                           int size)
{
    /* Declare many local variables to create register pressure */
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
    
    /* Index variables for complex array access */
    register int idx1 = 0, idx2 = 0, idx3 = 0, idx4 = 0;
    register int stride1 = 16, stride2 = 8, stride3 = 4;
    
    /* Volatile sink to prevent optimization */
    volatile double sink = 0.0;
    
    /* Complex loop with data dependencies */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Reset some variables each iteration */
        idx1 = outer % 64;
        idx2 = (outer * 3) % 64;
        idx3 = (outer * 5) % 64;
        idx4 = (outer * 7) % 64;
        
        /* Complex array indexing with multiple terms */
        i1 = input_int[idx1 * stride1 + idx2];
        i2 = input_int[idx2 * stride2 + idx3 + idx4];
        i3 = input_int[idx3 * stride3 + idx1 * 2 + outer];
        
        /* Load floating point values with complex addressing */
        f1 = input_float[idx1 * 8 + idx2];
        f2 = input_float[idx2 * 4 + idx3];
        d1 = input_double[idx3 * 2 + idx4];
        d2 = input_double[idx4 * 4 + idx1];
        
        /* Long chain of mixed-type computations */
        /* This creates dependencies preventing register reuse */
        l1 = (long)i1 * i2 + i3;
        f3 = (float)l1 * f1 + f2;
        d3 = (double)f3 * d1 - d2;
        i4 = (int)d3 + i1 * i2;
        f4 = (float)i4 / (f1 + 1.0f);
        d4 = (double)f4 * d3 / (d1 + 1.0);
        l2 = (long)d4 + l1 * 3;
        f5 = (float)l2 + f3 * 2.0f;
        d5 = d4 * 0.5 + sin(d3);
        i5 = (int)(d5 * 100.0) + i4;
        
        /* More computations creating web of dependencies */
        for (int inner = 0; inner < 4; inner++) {
            i6 = i5 + inner * i4;
            f6 = f5 * (float)inner + f4;
            d6 = d5 * (double)inner + cos(d4);
            l3 = l2 + inner * 7L;
            
            i7 = i6 * 3 - i5;
            f7 = f6 / (float)(inner + 2) + f3;
            d7 = d6 * 1.1 + tan(d5 * 0.01);
            l4 = l3 >> 2 + inner;
            
            i8 = i7 ^ i6;
            f8 = sqrtf(f7 * f7 + f6 * f6);
            d8 = sqrt(d7 * d7 + d6 * d6);
            l5 = l4 | l3;
        }
        
        /* Inline assembly that clobbers many registers */
        /* This forces spills and reloads */
#if defined(__x86_64__)
        __asm__ volatile (
            "movq $0, %%rax\n"
            "movq $0, %%rbx\n"
            "movq $0, %%rcx\n"
            "movq $0, %%rdx\n"
            "movq $0, %%rsi\n"
            "movq $0, %%rdi\n"
            "movq $0, %%r8\n"
            "movq $0, %%r9\n"
            "movq $0, %%r10\n"
            "movq $0, %%r11\n"
            "movq $0, %%r12\n"
            "movq $0, %%r13\n"
            "movq $0, %%r14\n"
            "movq $0, %%r15\n"
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
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory"
        );
#elif defined(__aarch64__)
        __asm__ volatile (
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
        
        /* Continue computation after clobbering */
        i9 = i8 * 2 + i7;
        f9 = f8 * 3.14f + f7;
        d9 = d8 * 2.71828 + d7;
        l6 = l5 * 5 + l4;
        
        /* Call function with many arguments */
        /* This stresses argument passing registers */
        int temp_int = i9;
        float temp_float = f9;
        double result = many_args_function(
            i1, i2, i3, i4,
            d1, d2, d3, d4,
            f1, f2, f3, f4,
            l1, l2, &temp_int, &temp_float
        );
        
        /* Use result in further computation */
        i10 = (int)result + i9;
        f10 = (float)result + f9;
        d10 = result * 0.5 + d9;
        l7 = (long)result + l6;
        
        /* Complex store with addressing */
        int store_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7 + outer) % ARRAY_SIZE;
        global_int_array[store_idx] = i10;
        global_float_array[store_idx] = f10;
        global_double_array[store_idx] = d10;
        
        /* Update sink to prevent dead code elimination */
        sink += (double)i10 + f10 + d10 + l7;
        
        /* More mixed-type operations */
        i11 = i10 ^ 0x55AA55AA;
        f1 = f10 * 0.9f + 0.1f;
        d1 = d10 * 0.9 + 0.1;
        l8 = l7 << 3;
        
        i12 = i11 * 3 / 2;
        f2 = sqrtf(f1);
        d2 = sqrt(d1);
        l9 = l8 >> 1;
        
        /* Type conversions that may require different register classes */
        i13 = (int)((double)i12 * d2);
        f3 = (float)((long)i13 * l9);
        d3 = (double)((int)f3 * i13);
        l10 = (long)(d3 * 1000.0);
    }
    
    return sink;
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Call the register-pressure function */
    double result = compute_heavy(global_int_array, 
                                 global_double_array,
                                 global_float_array,
                                 ARRAY_SIZE);
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %f\n", result);
    
    /* Additional computation to ensure all code is used */
    double final_sum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += global_int_array[i] + 
                    global_float_array[i] + 
                    global_double_array[i];
    }
    printf("Final sum: %f\n", final_sum);
    
    return 0;
}
