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

/* Complex structure for varied memory access */
struct ComplexData {
    int i[16];
    float f[16];
    double d[16];
    long l[16];
};

/* Helper function with many arguments to stress calling convention */
__attribute__((noinline))
double many_args_function(
    int a1, int a2, int a3, int a4,
    float f1, float f2, float f3, float f4,
    double d1, double d2, double d3, double d4,
    long l1, long l2, void* p1, void* p2
) {
    /* Complex computation mixing all types */
    double result = (double)a1 * d1 + (double)f1;
    result += (double)a2 * d2 + (double)f2;
    result += (double)a3 * d3 + (double)f3;
    result += (double)a4 * d4 + (double)f4;
    result += (double)l1 / 1000.0;
    result += (double)l2 / 2000.0;
    
    /* Force memory access through pointers */
    if (p1) result += *(double*)p1;
    if (p2) result += *(float*)p2;
    
    return result;
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
void compute_heavy(struct ComplexData* input, struct ComplexData* output, int iterations) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    /* Pointer variables */
    int* p1;
    float* p2;
    double* p3;
    
    /* Index variables for complex array access */
    int idx1, idx2, idx3, idx4, idx5;
    
    /* Volatile locals to inhibit optimizations */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3;
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Initialize index variables with complex expressions */
        idx1 = (iter * 13) % 16;
        idx2 = (iter * 17) % 16;
        idx3 = (iter * 19) % 16;
        idx4 = (iter * 23) % 16;
        idx5 = (iter * 29) % 16;
        
        /* Complex array indexing with multiple terms */
        i1 = input->i[idx1 * 2 + idx2];
        i2 = input->i[idx3 * 3 + idx4];
        i3 = input->i[idx5 * 5 + idx1];
        i4 = input->i[idx2 * 7 + idx3];
        i5 = input->i[idx4 * 11 + idx5];
        
        /* Load floating point data with complex indexing */
        f1 = input->f[idx1 * 2 + idx2];
        f2 = input->f[idx3 * 3 + idx4];
        f3 = input->f[idx5 * 5 + idx1];
        f4 = input->f[idx2 * 7 + idx3];
        f5 = input->f[idx4 * 11 + idx5];
        
        d1 = input->d[idx1 * 2 + idx2];
        d2 = input->d[idx3 * 3 + idx4];
        d3 = input->d[idx5 * 5 + idx1];
        d4 = input->d[idx2 * 7 + idx3];
        d5 = input->d[idx4 * 11 + idx5];
        
        l1 = input->l[idx1 * 2 + idx2];
        l2 = input->l[idx3 * 3 + idx4];
        l3 = input->l[idx5 * 5 + idx1];
        l4 = input->l[idx2 * 7 + idx3];
        l5 = input->l[idx4 * 11 + idx5];
        
        /* Start long chain of dependent computations */
        /* Mix integer and floating point operations */
        f6 = (float)i1 * f1 + (float)i2;
        d6 = (double)f2 * d1 + (double)i3;
        i6 = (int)(f3 * 100.0f) + i4;
        l6 = (long)(d2 * 1000.0) + l1;
        
        f7 = f4 * 2.0f + (float)l2;
        d7 = d3 * 3.0 + (double)i5;
        i7 = i1 * 2 + (int)d4;
        l7 = l3 * 3 + (long)f5;
        
        /* More mixed computations */
        f8 = (float)(i6 + i7) / f6;
        d8 = (double)(l6 + l7) / d6;
        i8 = (int)(f7 * d7);
        l8 = (long)(d8 * f8);
        
        f9 = f8 * global_volatile_float + vf1;
        d9 = d8 * global_volatile_double + vd1;
        i9 = i8 * global_volatile_int + vi1;
        l9 = l8 * (long)global_volatile_int + (long)vi2;
        
        /* Continue the dependency chain */
        f10 = f9 * 1.1f - f6;
        d10 = d9 * 1.11 - d6;
        i10 = i9 * 3 - i6;
        l10 = l9 * 5 - l6;
        
        f11 = sinf(f10) + cosf(f7);
        d11 = sin(d10) + cos(d7);
        i11 = i10 ^ i7;
        l11 = l10 | l7;
        
        f12 = f11 * 2.0f + (float)i11;
        d12 = d11 * 3.0 + (double)l11;
        i12 = i11 + (int)f12;
        l12 = l11 + (long)d12;
        
        /* Insert inline assembly that clobbers many registers */
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
        
        /* Continue computations after assembly clobber */
        f13 = f12 * 1.5f + (float)i12;
        d13 = d12 * 1.6 + (double)l12;
        i13 = i12 * 2 + (int)f13;
        l13 = l12 * 3 + (long)d13;
        
        f14 = sqrtf(f13) + f11;
        d14 = sqrt(d13) + d11;
        i14 = i13 << 2;
        l14 = l13 << 3;
        
        /* Call function with many arguments - stresses calling convention */
        double func_result = many_args_function(
            i1, i2, i3, i4,
            f1, f2, f3, f4,
            d1, d2, d3, d4,
            l1, l2, (void*)&i5, (void*)&f5
        );
        
        /* Use function result in further computations */
        f15 = f14 + (float)func_result;
        d15 = d14 + func_result;
        i15 = i14 + (int)func_result;
        l15 = l14 + (long)func_result;
        
        /* Final computations */
        f16 = f15 * 0.5f;
        d16 = d15 * 0.6;
        i16 = i15 / 2;
        l16 = l15 / 3;
        
        /* Store results with complex indexing */
        output->f[idx1 * 2 + idx2] = f16;
        output->d[idx3 * 3 + idx4] = d16;
        output->i[idx5 * 5 + idx1] = i16;
        output->l[idx2 * 7 + idx3] = l16;
        
        /* Use volatile variables to force memory writes */
        vi1 = i16;
        vf1 = f16;
        vd1 = d16;
        
        /* Global volatile access */
        global_volatile_int = i16;
        global_volatile_float = f16;
        global_volatile_double = d16;
    }
}

int main() {
    /* Allocate and initialize data */
    struct ComplexData* input = (struct ComplexData*)malloc(sizeof(struct ComplexData));
    struct ComplexData* output = (struct ComplexData*)malloc(sizeof(struct ComplexData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < 16; i++) {
        input->i[i] = rand() % 1000;
        input->f[i] = (float)rand() / RAND_MAX * 100.0f;
        input->d[i] = (double)rand() / RAND_MAX * 1000.0;
        input->l[i] = (long)rand() * 1000L;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ITERATIONS);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += output->i[i];
        checksum += (long)output->f[i];
        checksum += (long)output->d[i];
        checksum += output->l[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
