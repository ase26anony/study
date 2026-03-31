#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Volatile globals to force memory operands */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for varied memory accesses */
struct DataBlock {
    int ints[16];
    float floats[16];
    double doubles[8];
    long longs[8];
};

/* Argument-heavy helper function - forces register/stack pressure */
__attribute__((noinline))
double heavy_args_func(int a1, int a2, int a3, int a4,
                      float f1, float f2, float f3, float f4,
                      double d1, double d2, double d3, double d4,
                      long l1, long l2, void* ptr1, void* ptr2) {
    /* Mix all argument types in computation */
    double result = (double)a1 * d1 + (double)a2 * d2;
    result += (double)f1 * (double)f2;
    result += (double)l1 / (double)l2;
    result += *(double*)ptr1 + *(float*)ptr2;
    
    /* Force memory access */
    volatile double sink = result;
    (void)sink;
    
    return result * (a3 + a4) / (f3 + f4 + d3 + d4);
}

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
void compute_heavy(struct DataBlock* input, struct DataBlock* output, 
                   int iterations, int stride) {
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Float variables */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15;
    
    /* Double variables */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15;
    
    /* Pointer/Index variables */
    int idx1, idx2, idx3, idx4, idx5;
    volatile int* volatile_ptr = &global_volatile_int;
    
    /* Initialize with pseudo-random values from input */
    i1 = input->ints[0]; i2 = input->ints[1]; i3 = input->ints[2];
    i4 = input->ints[3]; i5 = input->ints[4]; i6 = input->ints[5];
    i7 = input->ints[6]; i8 = input->ints[7]; i9 = input->ints[8];
    i10 = input->ints[9];
    
    l1 = input->longs[0]; l2 = input->longs[1]; l3 = input->longs[2];
    l4 = input->longs[3]; l5 = input->longs[4];
    
    f1 = input->floats[0]; f2 = input->floats[1]; f3 = input->floats[2];
    f4 = input->floats[3]; f5 = input->floats[4]; f6 = input->floats[5];
    f7 = input->floats[6]; f8 = input->floats[7]; f9 = input->floats[8];
    f10 = input->floats[9];
    
    d1 = input->doubles[0]; d2 = input->doubles[1]; d3 = input->doubles[2];
    d4 = input->doubles[3]; d5 = input->doubles[4];
    
    /* Main computation loop with complex data dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex array indexing with multiple terms */
        idx1 = (iter * stride) % 16;
        idx2 = (iter * 3 + stride) % 16;
        idx3 = (iter * 5 + stride * 2) % 16;
        idx4 = (iter * 7 + stride * 3) % 8;
        idx5 = (iter * 11 + stride * 4) % 8;
        
        /* Long chain of mixed-type computations - prevents register reuse */
        /* Integer computations */
        i11 = i1 * i2 + i3 - i4;
        i12 = i5 / (i6 + 1) + i7 * i8;
        i13 = i9 ^ i10 ^ i11 ^ i12;
        i14 = (i13 << 3) | (i12 >> 2);
        i15 = i14 * global_volatile_int;  /* Volatile memory access */
        
        /* Floating point computations with type conversions */
        f11 = (float)i15 * f1 + f2 - f3;
        f12 = f4 * f5 / (f6 + 1.0f);
        f13 = f7 + f8 * f9 - f10;
        f14 = f11 * f12 + f13 * global_volatile_float;  /* Volatile */
        
        /* Double computations with mixed sources */
        d6 = (double)f14 * d1 + d2 - d3;
        d7 = d4 * d5 / (d6 + 1.0);
        d8 = (double)i14 * d7 + global_volatile_double;  /* Volatile */
        
        /* Long integer computations */
        l6 = l1 * l2 + l3 - l4;
        l7 = l5 ^ l6 ^ (long)i15;
        l8 = l7 << 2 | l6 >> 3;
        
        /* More mixed-type computations */
        d9 = (double)l8 * d8 + (double)f14;
        f15 = (float)d9 * f13 + (float)l7;
        d10 = (double)f15 * d7 + (double)i13;
        
        /* Complex memory accesses with array indexing */
        i16 = input->ints[idx1] + input->ints[idx2] - input->ints[idx3];
        f11 += input->floats[idx1] * input->floats[idx2];
        d11 = input->doubles[idx4] * input->doubles[idx5];
        l9 = input->longs[idx4] + input->longs[idx5];
        
        /* Inline assembly that clobbers MANY registers */
        /* For x86_64 */
        __asm__ volatile (
            "# Clobber many registers to force spills\n"
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
              "memory", "cc"
        );
        
        /* For ARM/aarch64, uncomment this version instead:
        __asm__ volatile (
            "# Clobber many ARM registers\n"
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
              "memory", "cc"
        );
        */
        
        /* Continue computation after clobbering */
        d12 = d10 * d11 + (double)i16;
        f12 = (float)d12 * f11 + (float)l9;
        i17 = (int)f12 * i16 + (int)d11;
        l10 = (long)i17 * l9 + (long)f12;
        
        /* Call argument-heavy function - forces register/stack pressure */
        d13 = heavy_args_func(i1, i2, i3, i4,
                             f1, f2, f3, f4,
                             d1, d2, d3, d4,
                             l1, l2, (void*)&i5, (void*)&f5);
        
        /* More mixed computations with the result */
        d14 = d13 * d12 + (double)l10;
        f13 = (float)d14 * global_volatile_float;  /* Volatile */
        i18 = (int)f13 * global_volatile_int;      /* Volatile */
        
        /* Store results with complex indexing */
        output->ints[idx1] = i17;
        output->ints[idx2] = i18;
        output->floats[idx1] = f12;
        output->floats[idx2] = f13;
        output->doubles[idx4] = d12;
        output->doubles[idx5] = d14;
        output->longs[idx4] = l9;
        output->longs[idx5] = l10;
        
        /* Rotate values to create data dependencies for next iteration */
        i1 = i17; i2 = i18; i3 = i16;
        f1 = f12; f2 = f13; f3 = f11;
        d1 = d12; d2 = d14; d3 = d11;
        l1 = l9; l2 = l10; l3 = l8;
        
        /* Access volatile memory */
        i19 = *volatile_ptr;
        f14 = global_volatile_float;
        d15 = global_volatile_double;
        
        /* Use volatile values */
        i20 = i19 * i18;
        f15 = f14 * f13;
        d15 = d15 * d14;
        
        /* Final store to volatile to prevent optimization */
        volatile int final_sink = i20;
        volatile float float_sink = f15;
        volatile double double_sink = d15;
        (void)final_sink;
        (void)float_sink;
        (void)double_sink;
    }
}

int main() {
    /* Allocate and initialize data */
    struct DataBlock* input = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    struct DataBlock* output = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < 16; i++) {
        input->ints[i] = rand() % 1000;
        input->floats[i] = (float)rand() / (float)RAND_MAX * 100.0f;
    }
    for (int i = 0; i < 8; i++) {
        input->doubles[i] = (double)rand() / (double)RAND_MAX * 1000.0;
        input->longs[i] = rand() * 1000L;
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ITERATIONS, 3);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += output->ints[i];
        checksum += (long)output->floats[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += (long)output->doubles[i];
        checksum += output->longs[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    free(input);
    free(output);
    
    return 0;
}
