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

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += (double)((intptr_t)p1 % 1000);
    result += (double)((intptr_t)p2 % 1000);
    
    /* Force memory access */
    result += global_volatile_double;
    result += (double)global_volatile_float;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(const int* input, double* output, int size) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    int i16 = 16, i17 = 17, i18 = 18, i19 = 19, i20 = 20;
    
    /* Long variables */
    long l1 = 1000L, l2 = 2000L, l3 = 3000L, l4 = 4000L, l5 = 5000L;
    long l6 = 6000L, l7 = 7000L, l8 = 8000L, l9 = 9000L, l10 = 10000L;
    
    /* Float variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    float f11 = 11.11f, f12 = 12.12f, f13 = 13.13f, f14 = 14.14f, f15 = 15.15f;
    
    /* Double variables */
    double d1 = 1.01, d2 = 2.02, d3 = 3.03, d4 = 4.04, d5 = 5.05;
    double d6 = 6.06, d7 = 7.07, d8 = 8.08, d9 = 9.09, d10 = 10.10;
    double d11 = 11.11, d12 = 12.12, d13 = 13.13, d14 = 14.14, d15 = 15.15;
    
    /* Pointer variables */
    const int* p1 = input;
    double* p2 = output;
    volatile int* volatile_p = &global_volatile_int;
    
    /* Index variables for complex array access */
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3, idx5 = 4;
    int stride1 = 8, stride2 = 16, stride3 = 32;
    
    /* Temporary variables for intermediate results */
    double temp1, temp2, temp3, temp4, temp5;
    float ftemp1, ftemp2, ftemp3;
    int itemp1, itemp2, itemp3;
    
    /* Main computation loop with unbroken dependency chain */
    for (int iter = 0; iter < ITERATIONS && iter < size; iter++) {
        /* Complex array indexing with multiple terms */
        int array_idx = (iter * stride1 + idx1 * stride2 + idx2 * stride3) % size;
        
        /* Load from input array with volatile access */
        int input_val = input[array_idx] + *volatile_p;
        
        /* Long chain of mixed-type computations */
        /* Each computation depends on previous results */
        f1 = (float)input_val * f2 + f3;
        d1 = (double)f1 * d2 + d3;
        i1 = (int)d1 + i2 * i3;
        l1 = (long)i1 * l2 + l3;
        f4 = (float)l1 * f5 / f6;
        d4 = (double)f4 + d5 * d6;
        i4 = i1 + i5 * (int)d4;
        f7 = f1 + f8 * (float)i4;
        d7 = d1 + d8 * (double)f7;
        l4 = l1 + l5 * (long)d7;
        f9 = (float)l4 * f10 - f11;
        d9 = d4 + d10 / d11;
        i6 = i4 + i7 * (int)f9;
        f12 = f7 + f13 * (float)i6;
        d12 = d7 + d13 * (double)f12;
        l6 = l4 + l7 * (long)d12;
        
        /* More computations to increase pressure */
        temp1 = (double)i1 * d1 + (double)i2 * d2;
        temp2 = (double)f1 * (double)f2 + (double)f3 * (double)f4;
        temp3 = (double)l1 / 1000.0 + (double)l2 / 2000.0;
        temp4 = temp1 + temp2 * temp3;
        temp5 = sin(temp4) + cos(d3) * tan(d4);
        
        ftemp1 = (float)temp5 * f5 + f6;
        ftemp2 = ftemp1 * f7 - f8;
        ftemp3 = ftemp2 / f9 + f10;
        
        itemp1 = (int)ftemp3 * i8 + i9;
        itemp2 = itemp1 * i10 - i11;
        itemp3 = itemp2 + i12 * (int)ftemp2;
        
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
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
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
        
        /* Continue computation after assembly clobber */
        /* All variables need to be reloaded */
        f13 = ftemp3 * f14 + f15;
        d13 = (double)f13 * d14 - d15;
        i7 = itemp3 + i13 * (int)d13;
        l7 = (long)i7 * l8 + l9;
        f14 = (float)l7 * f15 / f1;
        d14 = d13 + d15 * (double)f14;
        
        /* Call function with many arguments */
        /* This requires moving values to argument registers/stack */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)p1, (void*)p2
        );
        
        /* Use function result in further computation */
        d15 = d14 + func_result * 0.5;
        i8 = i7 + (int)(func_result * 100.0);
        
        /* Complex store with volatile */
        int store_idx = (iter * 3 + idx3 * 5 + idx4 * 7) % size;
        output[store_idx] = d15 + (double)i8 + (double)l7;
        
        /* Update indices for next iteration */
        idx1 = (idx1 + 1) % 5;
        idx2 = (idx2 + 2) % 7;
        idx3 = (idx3 + 3) % 11;
        idx4 = (idx4 + 5) % 13;
        idx5 = (idx5 + 7) % 17;
        
        /* Modify volatile global to force memory writes */
        global_volatile_int = iter;
        global_volatile_double = d15;
        global_volatile_float = f14;
    }
    
    /* Final volatile sink to prevent optimization */
    volatile double final_sink = d15 + (double)i8 + (double)l7;
    (void)final_sink;
}

int main() {
    /* Initialize arrays with pseudo-random data */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        output_array[i] = 0.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(input_array, output_array, ARRAY_SIZE);
    
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
