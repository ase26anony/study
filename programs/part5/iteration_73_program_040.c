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

/* Helper function with many arguments to force argument passing complexity */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)f1 * d2;
    result += (double)a2 * (double)l1 / (d3 + 1.0);
    result += (double)((int)f2 * a3) + (double)((long)f3 * a4);
    result *= (p1 != p2) ? 1.1 : 0.9;
    return result + global_volatile_double;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
void compute_heavy(const int* input, double* output, int size) {
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int vi1, vi2, vi3;  /* Volatile locals for memory ops */
    volatile double vd1, vd2;
    
    /* Additional pointer/index variables for complex addressing */
    int idx1, idx2, idx3, stride1, stride2;
    const int* ptr1, *ptr2;
    double* out_ptr;
    
    /* Initialize some values */
    stride1 = size / 8;
    stride2 = size / 16;
    vi1 = global_volatile_int;
    vd1 = global_volatile_double;
    
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Complex loop with data dependencies preventing register reuse */
        for (int i = 0; i < size; i++) {
            /* Complex array indexing with multiple terms */
            idx1 = (i * 3) / 2;
            idx2 = (i + stride1) % size;
            idx3 = (i * 5 + stride2) % size;
            
            /* Load with complex addressing - forces MEM rtx */
            i1 = input[i];
            i2 = input[idx1] + vi1;
            i3 = input[idx2] * 2;
            i4 = input[idx3] - global_volatile_int;
            
            /* Long chain of mixed-type computations */
            f1 = (float)i1 * 1.5f + global_volatile_float;
            f2 = (float)i2 / 3.0f - f1;
            f3 = (float)i3 * f1 + f2;
            f4 = (float)i4 * f3 - f2;
            
            d1 = (double)f1 * 2.71828;
            d2 = (double)f2 * 3.14159 + vd1;
            d3 = (double)f3 * d1 / (d2 + 1.0);
            d4 = (double)f4 * d3 - d2;
            
            l1 = (long)(d1 * 1000.0);
            l2 = (long)(d2 * 1000.0) + l1;
            l3 = (long)(d3 * 1000.0) * l2;
            l4 = (long)(d4 * 1000.0) - l3;
            
            /* More computations creating web of dependencies */
            i5 = (int)((l1 + l2) / 1000);
            i6 = (int)((l3 - l4) / 500);
            i7 = i5 * i6 + i1;
            i8 = i6 / (i2 + 1) + i3;
            i9 = i7 * i8 - i4;
            i10 = i9 + i5 - i6;
            
            f5 = (float)i5 * 0.25f;
            f6 = (float)i6 * 0.5f + f5;
            f7 = (float)i7 * f5 - f6;
            f8 = (float)i8 * f7 / (f6 + 0.1f);
            f9 = (float)i9 + f8 * f7;
            f10 = (float)i10 * f9 - f8;
            
            d5 = (double)f5 * 1.1;
            d6 = (double)f6 * 1.2 + d5;
            d7 = (double)f7 * d6 / 1.3;
            d8 = (double)f8 * 1.4 - d7;
            d9 = (double)f9 * d8 + d6;
            d10 = (double)f10 * 2.0 - d9;
            
            l5 = (long)d5 * 2L;
            l6 = (long)d6 * 3L + l5;
            l7 = (long)d7 * 4L - l6;
            l8 = (long)d8 * 5L;
            l9 = (long)d9 * 6L + l8;
            l10 = (long)d10 * 7L - l9;
            
            /* Inline assembly that clobbers MANY registers */
            /* For x86_64 */
            #ifdef __x86_64__
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
            /* For AArch64 */
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
            #else
            /* Generic clobber for other architectures */
            __asm__ volatile ("" : : : "memory");
            #endif
            
            /* Call function with many arguments - forces register/stack reloads */
            double func_result = many_args_function(
                i1, i2, i3, i4,
                f1, f2, f3, f4,
                d1, d2, d3, d4,
                l1, l2, (void*)&i1, (void*)&i2
            );
            
            /* More computations after function call */
            d10 += func_result * 0.01;
            l10 += (long)(func_result * 1000.0);
            
            /* Complex store with addressing */
            int out_idx = (i * 7 + outer * 3) % size;
            output[out_idx] = d10 + (double)l10 * 0.001;
            
            /* Volatile store to force memory operation */
            vd2 = d10;
            vi2 = (int)l10;
            vi3 = out_idx;
            
            /* Dependency chain continues */
            i1 = vi2 + 1;
            f1 = (float)vd2 * 0.5f;
            d1 = (double)vi3 * 0.25;
        }
    }
    
    /* Final volatile store to prevent optimization */
    global_volatile_int = vi2;
    global_volatile_double = vd2;
}

int main() {
    /* Allocate and initialize arrays */
    int* input_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* output_array = (double*)calloc(ARRAY_SIZE, sizeof(double));
    
    if (!input_array || !output_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Perform heavy computation */
    compute_heavy(input_array, output_array, ARRAY_SIZE);
    
    /* Calculate checksum to prevent dead code elimination */
    double checksum = 0.0;
    volatile double volatile_checksum = 0.0;  /* Volatile to force computation */
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_array[i];
        volatile_checksum += output_array[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Volatile checksum: %f\n", volatile_checksum);
    
    /* Cleanup */
    free(input_array);
    free(output_array);
    
    return 0;
}
