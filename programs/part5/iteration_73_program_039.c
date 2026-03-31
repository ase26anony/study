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
    double b1, double b2, double b3, double b4,
    float c1, float c2, float c3, float c4,
    long d1, long d2, int* e1, double* e2)
{
    /* Complex computation mixing all argument types */
    volatile double result = 0.0;
    
    result += (double)a1 * b1 + (double)a2 * b2;
    result += (double)c1 * c2 * (double)a3;
    result += (double)d1 / (b3 + 1.0);
    result += (*e1) * (*e2) + (double)a4;
    
    /* Force memory access */
    *e1 += (int)result;
    *e2 += result;
    
    return result + c3 + c4 + (double)d2;
}

/* Complex structure for indirect access */
struct MixedData {
    int ints[8];
    double doubles[4];
    float floats[8];
    long longs[2];
    volatile int volatile_int;
};

/* Main computation function with extreme register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
static void compute_heavy(struct MixedData* data, int size, 
                         volatile int* output_int, volatile double* output_double)
{
    /* Declare MANY local variables to create register pressure */
    /* Integer variables */
    register int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    register int i11, i12, i13, i14, i15, i16, i17, i18, i19, i20;
    
    /* Long variables */
    register long l1, l2, l3, l4, l5, l6, l7, l8;
    
    /* Float variables */
    register float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    register float f11, f12, f13, f14, f15;
    
    /* Double variables */
    register double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    register double d11, d12, d13, d14, d15;
    
    /* Pointer variables */
    int* p1, *p2, *p3;
    double* dp1, *dp2;
    float* fp1;
    
    /* Volatile variables to prevent optimization */
    volatile int vi1, vi2, vi3;
    volatile double vd1, vd2;
    volatile float vf1, vf2;
    
    /* Initialize pointers */
    p1 = &data->ints[0];
    p2 = &data->ints[4];
    p3 = output_int;
    dp1 = &data->doubles[0];
    dp2 = output_double;
    fp1 = &data->floats[0];
    
    /* Complex loop with data dependencies */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Load values with complex array indexing */
        i1 = data->ints[(outer * 3) % 8];
        i2 = data->ints[(outer * 5 + 1) % 8];
        i3 = data->ints[(outer * 7 + 2) % 8];
        i4 = data->ints[(outer * 11 + 3) % 8];
        
        d1 = data->doubles[(outer * 2) % 4];
        d2 = data->doubles[(outer * 3 + 1) % 4];
        d3 = data->doubles[(outer * 5 + 2) % 4];
        
        f1 = data->floats[(outer * 3) % 8];
        f2 = data->floats[(outer * 5 + 1) % 8];
        f3 = data->floats[(outer * 7 + 2) % 8];
        f4 = data->floats[(outer * 11 + 3) % 8];
        
        l1 = data->longs[outer % 2];
        l2 = data->longs[(outer + 1) % 2];
        
        /* Long chain of mixed-type computations */
        /* Each computation depends on previous results */
        d4 = (double)i1 * d1 + (double)i2 * d2;
        f5 = (float)d4 * f1 + f2 * f3;
        i5 = (int)f5 + i3 * i4;
        d5 = (double)i5 / (d3 + 1.0);
        f6 = (float)d5 + f4 * 2.0f;
        i6 = (int)f6 + (int)(d4 * 100.0);
        d6 = (double)i6 * M_PI + d2;
        f7 = sqrtf(fabsf(f6)) + f5;
        i7 = i5 ^ i6 + (int)l1;
        d7 = sin(d6) + cos(d5) * d4;
        f8 = (float)d7 * f7 + f3;
        i8 = i7 * 31 - (int)(f8 * 10.0f);
        d8 = (double)i8 / 17.0 + d6;
        f9 = (float)d8 * 0.5f + f4;
        i9 = (int)(f9 * 100.0f) + i2 * i3;
        d9 = exp(d8 * 0.01) + d7;
        f10 = logf(fabsf(f9) + 1.0f) + f8;
        i10 = i9 & 0xFF + i4 << 2;
        d10 = (double)i10 * 0.01 + d9;
        
        /* More computations to increase pressure */
        f11 = f10 * 1.1f + f7 * 0.9f;
        i11 = i10 + (int)(f11 * 2.0f);
        d11 = d10 * 1.01 + (double)i11 * 0.001;
        f12 = (float)d11 + f10;
        i12 = i11 * 3 - (int)f12;
        d12 = (double)i12 / 3.14159;
        f13 = (float)d12 * f11;
        i13 = (int)f13 + i8 + i9;
        d13 = d12 * d11 + d10;
        f14 = f13 * 2.0f - f12;
        i14 = i13 ^ i12 + (int)l2;
        d14 = sqrt(d13 * d13 + d12 * d12);
        f15 = (float)d14 * 0.5f;
        i15 = (int)f15 * i14;
        d15 = (double)i15 / 256.0;
        
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
            "xorpd %%xmm0, %%xmm0\n"
            "xorpd %%xmm1, %%xmm1\n"
            "xorpd %%xmm2, %%xmm2\n"
            "xorpd %%xmm3, %%xmm3\n"
            "xorpd %%xmm4, %%xmm4\n"
            "xorpd %%xmm5, %%xmm5\n"
            "xorpd %%xmm6, %%xmm6\n"
            "xorpd %%xmm7, %%xmm7\n"
            "xorpd %%xmm8, %%xmm8\n"
            "xorpd %%xmm9, %%xmm9\n"
            "xorpd %%xmm10, %%xmm10\n"
            "xorpd %%xmm11, %%xmm11\n"
            "xorpd %%xmm12, %%xmm12\n"
            "xorpd %%xmm13, %%xmm13\n"
            "xorpd %%xmm14, %%xmm14\n"
            "xorpd %%xmm15, %%xmm15\n"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15",
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
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
              "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
              "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d8", "d9",
              "d10", "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19",
              "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29",
              "d30", "d31", "memory"
        );
#endif
        
        /* Continue computation after clobbering */
        i16 = i15 + (int)(d15 * 1000.0);
        f11 = f15 * 0.8f + (float)i16 * 0.01f;
        d11 = d15 * 1.1 + (double)f11;
        i17 = i16 * 2 - (int)d11;
        
        /* Complex array indexing with multiple terms */
        int idx1 = (i17 + outer * 7) % ARRAY_SIZE;
        int idx2 = (i16 * 3 + outer * 11) % ARRAY_SIZE;
        int idx3 = (i15 * 5 + outer * 13) % ARRAY_SIZE;
        
        /* Access volatile global arrays */
        vi1 = global_int_array[idx1];
        vd1 = global_double_array[idx2];
        vf1 = global_float_array[idx3];
        
        /* More mixed computations with volatile accesses */
        i18 = i17 + vi1;
        d12 = d11 + vd1 * 0.5;
        f12 = f11 + vf1;
        
        /* Call function with many arguments - stresses argument passing */
        double func_result = many_args_function(
            i18, i17, i16, i15,
            d12, d11, d10, d9,
            f12, f11, f10, f9,
            l1, l2,
            p1, dp1
        );
        
        /* Use function result in further computation */
        i19 = (int)func_result + i18;
        d13 = func_result * 0.5 + d12;
        f13 = (float)func_result + f12;
        
        /* Store results with complex addressing */
        int store_idx = (outer * 17 + i19) % (size / 2);
        data->ints[store_idx % 8] = i19;
        data->doubles[store_idx % 4] = d13;
        data->floats[store_idx % 8] = f13;
        
        /* Store to volatile outputs */
        *p3 = i19;
        *dp2 = d13;
        
        /* Update pointers with complex arithmetic */
        p3 = output_int + ((outer * 19) % (ARRAY_SIZE / 4));
        dp2 = output_double + ((outer * 23) % (ARRAY_SIZE / 4));
        
        /* Final chain of dependent computations */
        i20 = i19 ^ (int)l1 + (int)(d13 * 100.0);
        f14 = (float)i20 * 0.01f + f13;
        d14 = (double)f14 + d13;
        f15 = sqrtf(fabsf(f14)) + f13;
        d15 = sqrt(fabs(d14)) + d13;
        
        /* Volatile store to force memory operation */
        vi2 = i20;
        vd2 = d15;
        vf2 = f15;
        
        /* Use volatiles in computation */
        data->volatile_int = vi2 + (int)vd2 + (int)vf2;
    }
}

int main(void) {
    struct MixedData data;
    volatile int output_int[ARRAY_SIZE];
    volatile double output_double[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    for (int i = 0; i < 8; i++) {
        data.ints[i] = rand() % 1000;
        data.floats[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 4; i++) {
        data.doubles[i] = (double)(rand() % 1000) / 10.0;
    }
    
    data.longs[0] = rand() % 10000;
    data.longs[1] = rand() % 10000;
    data.volatile_int = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_double_array[i] = (double)(rand() % 1000) / 10.0;
        global_float_array[i] = (float)(rand() % 1000) / 10.0f;
        output_int[i] = 0;
        output_double[i] = 0.0;
    }
    
    /* Perform heavy computation */
    compute_heavy(&data, ARRAY_SIZE, output_int, output_double);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output_int[i];
        checksum += (long)output_double[i];
    }
    
    checksum += data.volatile_int;
    
    printf("Checksum: %ld\n", checksum);
    printf("Result: %d, %f\n", output_int[0], output_double[0]);
    
    return 0;
}
