/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test function 1: Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *input, int *output) {
    volatile int a = input[0];
    volatile int b = input[1];
    volatile int c = input[2];
    volatile int d = input[3];
    volatile int e = input[4];
    volatile int f = input[5];
    volatile int g = input[6];
    volatile int h = input[7];
    
    /* Create many live variables to exhaust registers */
    int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e, v6 = f, v7 = g, v8 = h;
    int v9 = a + b, v10 = c + d, v11 = e + f, v12 = g + h;
    int v13 = a * b, v14 = c * d, v15 = e * f, v16 = g * h;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with multiple constraints to force reloads */
        __asm__ volatile (
            /* Output operands with different constraints */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movb %%al, %[out2]\n\t"
            "movw %%ax, %[out3]\n\t"
            : [out1] "=rm" (v1),        /* register or memory */
              [out2] "=q" (v2),         /* byte register constraint */
              [out3] "=r" (v3)          /* general register */
            : [in1] "irm" (input[i]),   /* immediate, register, or memory */
              [in2] "r" (v4),           /* register only */
              [in3] "i" (0x1234)        /* immediate only */
            : "eax", "cc", "memory"
        );
        
        /* Another asm with earlyclobber and matching constraints */
        int temp1 = v1, temp2 = v2;
        __asm__ volatile (
            "leal (%[a], %[b], 2), %[res]\n\t"
            "addl %[c], %[res]\n\t"
            : [res] "=&r" (temp1)       /* earlyclobber */
            : [a] "0" (temp1),          /* matching constraint */
              [b] "r" (temp2),
              [c] "rm" (v5)
            : "cc"
        );
        
        /* Mix with floating point to increase pressure */
        double d1 = v1, d2 = v2;
        __asm__ volatile (
            "addsd %[x], %[y]\n\t"
            "movsd %[y], %[out]\n\t"
            : [out] "=t" (d1)           /* top of FP stack */
            : [x] "t" (d1),
              [y] "rm" (d2)
            : "st(1)"
        );
        
        /* Unrolled section with many live values */
        if (i % 2 == 0) {
            __asm__ volatile (
                "movl %[in], %%ecx\n\t"
                "roll $3, %%ecx\n\t"
                "movl %%ecx, %[out]\n\t"
                : [out] "=r" (v4)
                : [in] "m" (v3)         /* memory constraint */
                : "ecx", "cc"
            );
        }
        
        /* Store results to prevent optimization */
        output[i] = v1 + v2 + v3 + v4;
    }
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations, long *input, long *output) {
    /* Use x86-specific constraints that may require secondary reloads */
    for (int i = 0; i < iterations; i++) {
        long a = input[i];
        long b = input[i + 1];
        long result;
        
        /* Force accumulator constraint then use result elsewhere */
        __asm__ volatile (
            "movq %[val], %%rax\n\t"
            "addq $1, %%rax\n\t"
            : "=a" (result)             /* accumulator constraint */
            : [val] "rm" (a)            /* may need secondary reload */
            : "cc"
        );
        
        /* Now use result with base register constraint */
        long final;
        __asm__ volatile (
            "movq %[src], %%rbx\n\t"
            "addq %%rax, %%rbx\n\t"
            "movq %%rbx, %[dst]\n\t"
            : [dst] "=rm" (final)
            : [src] "b" (b),            /* base register constraint */
              "a" (result)              /* from previous asm */
            : "rbx", "cc"
        );
        
        /* Mix with legacy register constraints */
        if (i % 3 == 0) {
            __asm__ volatile (
                "xchgq %%rbx, %%rcx\n\t"
                "addq %%rcx, %[out]\n\t"
                : [out] "+R" (final)    /* legacy register constraint */
                : 
                : "rbx", "rcx", "cc"
            );
        }
        
        output[i] = final;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int iterations, float *input, float *output) {
    /* Use optional constraints */
    for (int i = 0; i < iterations; i++) {
        float a = input[i];
        float b = input[i + 1];
        float c = input[i + 2];
        
        float res1, res2;
        
        /* Optional output constraint */
        __asm__ volatile (
            "addss %[x], %[y]\n\t"
            : [y] "=?r" (res1)          /* optional constraint */
            : [x] "r" (a),
              [y] "0" (b)
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't be combined due to barrier */
        __asm__ volatile (
            "mulss %[x], %[y]\n\t"
            : [y] "=r" (res2)
            : [x] "r" (c),
              [y] "0" (res1)
            : "cc"
        );
        
        /* Volatile asm with different clobbers */
        __asm__ volatile (
            "movss %[in], %%xmm0\n\t"
            "sqrtss %%xmm0, %%xmm0\n\t"
            "movss %%xmm0, %[out]\n\t"
            : [out] "=rm" (output[i])
            : [in] "rm" (res2)
            : "xmm0", "cc"
        );
    }
}

/* Test function 4: AVX intrinsics + inline asm for maximum pressure */
void test_avx_reloads(int iterations, double *input, double *output) {
    /* Use AVX registers to increase register pressure */
    __m256d avx_vec[UNROLL_FACTOR];
    double scalar_acc[UNROLL_FACTOR];
    
    for (int i = 0; i < UNROLL_FACTOR; i++) {
        avx_vec[i] = _mm256_set_pd(input[i*4], input[i*4+1], 
                                  input[i*4+2], input[i*4+3]);
        scalar_acc[i] = 0.0;
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix AVX operations with scalar asm */
        for (int i = 0; i < UNROLL_FACTOR; i++) {
            /* AVX operation */
            avx_vec[i] = _mm256_mul_pd(avx_vec[i], 
                                      _mm256_set1_pd(1.0001));
            
            /* Scalar asm that uses same values - creates pressure */
            double temp;
            __asm__ volatile (
                "vmovsd %[vec], %[temp]\n\t"
                "addsd %[acc], %[temp]\n\t"
                "vmovsd %[temp], %[acc]\n\t"
                : [acc] "+x" (scalar_acc[i]),  /* xmm constraint */
                  [temp] "=x" (temp)
                : [vec] "x" (avx_vec[i][0])
                : "cc"
            );
            
            /* Complex asm with many operands */
            if (iter % 2 == 0) {
                double a = scalar_acc[i];
                double b = avx_vec[i][1];
                double c, d;
                
                __asm__ volatile (
                    "vaddsd %[a], %[b], %[c]\n\t"
                    "vmulsd %[c], %[b], %[d]\n\t"
                    : [c] "=x" (c),
                      [d] "=x" (d)
                    : [a] "x" (a),
                      [b] "x" (b)
                    : "cc"
                );
                
                scalar_acc[i] = c + d;
            }
        }
        
        /* Conditional asm with runtime-dependent path */
        if (iter % 3 == 0) {
            double sum = 0.0;
            for (int i = 0; i < UNROLL_FACTOR; i++) {
                sum += scalar_acc[i];
            }
            
            __asm__ volatile (
                "movq %[sum], %%rax\n\t"
                "bsrq %%rax, %%rcx\n\t"
                "movq %%rcx, %[out]\n\t"
                : [out] "=r" (output[iter])
                : [sum] "rm" ((long)sum)
                : "rax", "rcx", "cc"
            );
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    /* Initialize test data */
    int int_data[ARRAY_SIZE];
    long long_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    double double_data[ARRAY_SIZE];
    int int_output[ARRAY_SIZE] = {0};
    long long_output[ARRAY_SIZE] = {0};
    float float_output[ARRAY_SIZE] = {0};
    double double_output[ARRAY_SIZE] = {0};
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        long_data[i] = i * 5L + 2;
        float_data[i] = i * 1.5f;
        double_data[i] = i * 2.7;
    }
    
    /* Execute tests based on mode */
    long long checksum = 0;
    
    if (mode & 1) {
        test_primary_reloads(iterations % 100, int_data, int_output);
        for (int i = 0; i < iterations % 100; i++) {
            checksum += int_output[i];
        }
    }
    
    if (mode & 2) {
        test_secondary_reloads(iterations % 100, long_data, long_output);
        for (int i = 0; i < iterations % 100; i++) {
            checksum += long_output[i];
        }
    }
    
    if (mode & 4) {
        test_optional_reloads(iterations % 100, float_data, float_output);
        for (int i = 0; i < iterations % 100; i++) {
            checksum += (long)float_output[i];
        }
    }
    
    if (mode & 8) {
        test_avx_reloads(iterations % 50, double_data, double_output);
        for (int i = 0; i < iterations % 50; i++) {
            checksum += (long)double_output[i];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
