#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions prototypes */
static void test_primary_reloads(int iterations, int *input, int *output);
static void test_secondary_reloads(int iterations, double *input, double *output);
static void test_optional_reloads(int iterations, float *input, float *output);
static void test_control_flow_reloads(int mode, long *input, long *output);

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_mode = 0;

/* Complex inline assembly with multiple operands and constraints */
static inline void complex_asm_operation(
    int in1, int in2, int in3, int in4, int in5,
    int *out1, int *out2, int *out3, int *out4, int *out5)
{
    /* Force register pressure with many live variables */
    register int r1 asm("eax") = in1;
    register int r2 asm("ebx") = in2;
    register int r3 asm("ecx") = in3;
    register int r4 asm("edx") = in4;
    register int r5 asm("esi") = in5;
    
    /* Extended asm with 5+ operands, mixed constraints */
    __asm__ volatile (
        /* Outputs with different constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "imull %[in3], %[out2]\n\t"
        "orl %[in4], %[out3]\n\t"
        "xorl %[in5], %[out4]\n\t"
        "leal (%[in1],%[in2],4), %[out5]\n\t"
        
        /* Additional operations to create dependencies */
        "movl %[out1], %%eax\n\t"
        "movl %[out2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        : /* outputs */
          [out1] "=&r" (*out1),  /* earlyclobber */
          [out2] "=r" (*out2),
          [out3] "=q" (*out3),   /* byte register constraint */
          [out4] "=r" (*out4),
          [out5] "=r" (*out5)
        
        : /* inputs */
          [in1] "r" (r1),
          [in2] "r" (r2),
          [in3] "r" (r3),
          [in4] "r" (r4),
          [in5] "r" (r5),
          "m" (*out1),           /* memory constraint */
          "i" (12345)            /* immediate constraint */
        
        : /* clobbers */
          "eax", "ebx", "ecx", "edx", "esi", "edi",
          "cc", "memory"
    );
}

/* Secondary reload test with mismatched constraints */
static inline void secondary_reload_asm(
    double d1, double d2, double *mem_out,
    int reg_class_constraint)
{
    double temp;
    
    /* Force secondary reload by using register class constraints
       that may not be satisfied directly */
    __asm__ volatile (
        /* Using 'a' constraint (accumulator) */
        "movq %[d1], %%rax\n\t"
        "movq %[d2], %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, %[temp]\n\t"
        
        /* Now use result with 'b' constraint (base register) */
        "movq %[temp], %%rbx\n\t"
        "movq %%rbx, %[out]\n\t"
        
        : [temp] "=&a" (temp),   /* earlyclobber + accumulator */
          [out] "=m" (*mem_out)
        
        : [d1] "rm" (d1),        /* register or memory - may force secondary reload */
          [d2] "rm" (d2),
          [regc] "R" (reg_class_constraint)  /* legacy register constraint */
        
        : "rax", "rbx", "rcx", "memory"
    );
}

/* Optional reload test with '?' modifier */
static inline int optional_reload_asm(float f1, float f2, float *opt_out)
{
    int result;
    float optional_temp;
    
    __asm__ volatile (
        "movss %[f1], %%xmm0\n\t"
        "addss %[f2], %%xmm0\n\t"
        "movss %%xmm0, %[temp]\n\t"
        "cvttss2si %[temp], %[result]\n\t"
        
        : [result] "=r" (result),
          [temp] "=?m" (optional_temp),  /* optional output */
          [opt] "=?r" (*opt_out)         /* optional output */
        
        : [f1] "x" (f1),                 /* SSE register constraint */
          [f2] "x" (f2)
        
        : "xmm0", "xmm1", "memory"
    );
    
    return result;
}

/* AVX operations to increase register pressure */
static void avx_register_pressure(__m256d *data, int count)
{
    __m256d accum = _mm256_setzero_pd();
    __m256d coeff = _mm256_set1_pd(1.01);
    
    for (int i = 0; i < count; i += 4) {
        /* Multiple AVX operations keeping many values live */
        __m256d v1 = _mm256_load_pd(&data[i]);
        __m256d v2 = _mm256_load_pd(&data[i + 4]);
        __m256d v3 = _mm256_load_pd(&data[i + 8]);
        __m256d v4 = _mm256_load_pd(&data[i + 12]);
        
        v1 = _mm256_mul_pd(v1, coeff);
        v2 = _mm256_mul_pd(v2, coeff);
        v3 = _mm256_mul_pd(v3, coeff);
        v4 = _mm256_mul_pd(v4, coeff);
        
        accum = _mm256_add_pd(accum, v1);
        accum = _mm256_add_pd(accum, v2);
        accum = _mm256_add_pd(accum, v3);
        accum = _mm256_add_pd(accum, v4);
        
        _mm256_store_pd(&data[i], v1);
        _mm256_store_pd(&data[i + 4], v2);
        _mm256_store_pd(&data[i + 8], v3);
        _mm256_store_pd(&data[i + 12], v4);
    }
    
    /* Use accum to prevent optimization */
    data[0] = _mm256_add_pd(data[0], accum);
}

/* Primary reload test function */
static void test_primary_reloads(int iterations, int *input, int *output)
{
    int temp[UNROLL_FACTOR];
    
    /* Unrolled loop with many live variables */
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many live scalar variables */
        int v0 = input[iter * UNROLL_FACTOR + 0] + global_seed;
        int v1 = input[iter * UNROLL_FACTOR + 1] * 2;
        int v2 = input[iter * UNROLL_FACTOR + 2] | 0xFF;
        int v3 = input[iter * UNROLL_FACTOR + 3] ^ v0;
        int v4 = input[iter * UNROLL_FACTOR + 4] + v1;
        int v5 = input[iter * UNROLL_FACTOR + 5] - v2;
        int v6 = input[iter * UNROLL_FACTOR + 6] & v3;
        int v7 = input[iter * UNROLL_FACTOR + 7] | v4;
        int v8 = input[iter * UNROLL_FACTOR + 8] ^ v5;
        int v9 = input[iter * UNROLL_FACTOR + 9] + v6;
        int v10 = input[iter * UNROLL_FACTOR + 10] * v7;
        int v11 = input[iter * UNROLL_FACTOR + 11] - v8;
        int v12 = input[iter * UNROLL_FACTOR + 12] & v9;
        int v13 = input[iter * UNROLL_FACTOR + 13] | v10;
        int v14 = input[iter * UNROLL_FACTOR + 14] ^ v11;
        int v15 = input[iter * UNROLL_FACTOR + 15] + v12;
        
        /* Complex asm with all live variables */
        complex_asm_operation(v0, v1, v2, v3, v4,
                             &temp[0], &temp[1], &temp[2], &temp[3], &temp[4]);
        
        /* Another asm block with different constraints */
        __asm__ volatile (
            "movl %[v5], %%eax\n\t"
            "addl %[v6], %%eax\n\t"
            "movl %%eax, %[t5]\n\t"
            "imull %[v7], %[t6]\n\t"
            "orl %[v8], %[t7]\n\t"
            : [t5] "=r" (temp[5]),
              [t6] "=r" (temp[6]),
              [t7] "=q" (temp[7])  /* byte register */
            : [v5] "r" (v5),
              [v6] "r" (v6),
              [v7] "r" (v7),
              [v8] "r" (v8)
            : "eax", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* More asm with matching constraints */
        __asm__ volatile (
            "movl %[v9], %[t8]\n\t"
            "addl %[v10], %[t8]\n\t"
            "movl %[t8], %[t9]\n\t"
            : [t8] "=&r" (temp[8]),  /* earlyclobber */
              [t9] "=r" (temp[9])
            : [v9] "0" (v9),         /* matching constraint */
              [v10] "r" (v10)
            : "cc"
        );
        
        /* Store results */
        for (int i = 0; i < 10; i++) {
            output[iter * UNROLL_FACTOR + i] = temp[i] + v13 + v14 + v15;
        }
    }
}

/* Secondary reload test function */
static void test_secondary_reloads(int iterations, double *input, double *output)
{
    for (int i = 0; i < iterations; i++) {
        double d1 = input[i * 2];
        double d2 = input[i * 2 + 1];
        double result;
        
        /* This may require secondary reloads due to constraint mismatches */
        secondary_reload_asm(d1, d2, &result, i);
        
        /* Use different register classes in sequence */
        __asm__ volatile (
            /* Force use of specific registers */
            "movq %[src], %%rax\n\t"
            "movq %%rax, %%rbx\n\t"
            "addq $1, %%rbx\n\t"
            "movq %%rbx, %[dst]\n\t"
            : [dst] "=b" (output[i])  /* must be in rbx */
            : [src] "a" ((long)result) /* must be in rax */
            : "rax", "rbx", "cc"
        );
        
        /* Another asm with 't' constraint (top of stack for x87) */
        if (i % 3 == 0) {
            double temp;
            __asm__ volatile (
                "fldl %[in]\n\t"
                "fstpl %[out]\n\t"
                : [out] "=t" (temp)  /* x87 top of stack */
                : [in] "m" (result)
                : "st", "st(1)", "st(2)", "st(3)"
            );
            output[i] += temp;
        }
    }
}

/* Optional reload test function */
static void test_optional_reloads(int iterations, float *input, float *output)
{
    for (int i = 0; i < iterations; i++) {
        float f1 = input[i];
        float f2 = input[iterations - i - 1];
        float opt_result;
        
        /* Optional output that may not be used */
        int int_result = optional_reload_asm(f1, f2, &opt_result);
        
        /* Use the result conditionally */
        if (int_result > 0) {
            output[i] = opt_result;
        } else {
            /* Different asm that won't combine with previous */
            __asm__ volatile (
                "movss %[in], %%xmm0\n\t"
                "mulss %[scale], %%xmm0\n\t"
                "movss %%xmm0, %[out]\n\t"
                : [out] "=m" (output[i])
                : [in] "x" (f1),
                  [scale] "x" (f2)
                : "xmm0", "xmm1"
            );
        }
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers */
        __asm__ volatile (
            "movss %[in], %%xmm2\n\t"
            "addss %[in2], %%xmm2\n\t"
            "movss %%xmm2, %[out]\n\t"
            : [out] "=m" (output[i + iterations])
            : [in] "x" (f1),
              [in2] "x" (f2)
            : "xmm2", "xmm3"  /* Different clobbers prevent combination */
        );
    }
}

/* Control flow dependent reload test */
static void test_control_flow_reloads(int mode, long *input, long *output)
{
    long accumulator = 0;
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Different asm based on control flow */
        if (mode & 1) {
            __asm__ volatile (
                "movq %[in], %%rax\n\t"
                "addq $0x1234, %%rax\n\t"
                "movq %%rax, %[out]\n\t"
                : [out] "=r" (output[i])
                : [in] "r" (input[i])
                : "rax", "cc"
            );
        } else {
            __asm__ volatile (
                "movq %[in], %%rbx\n\t"
                "subq $0x5678, %%rbx\n\t"
                "movq %%rbx, %[out]\n\t"
                : [out] "=r" (output[i])
                : [in] "r" (input[i])
                : "rbx", "cc"
            );
        }
        
        /* Nested conditionals with asm */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                __asm__ volatile (
                    "movq %[a], %%rcx\n\t"
                    "addq %[b], %%rcx\n\t"
                    "movq %%rcx, %[a]\n\t"
                    : [a] "+r" (accumulator)
                    : [b] "r" ((long)j)
                    : "rcx", "cc"
                );
            }
        }
        
        /* Switch between different constraint types */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "movq %[in], %%r8\n\t"
                    "movq %%r8, %[out]\n\t"
                    : [out] "=r" (output[i + ARRAY_SIZE/2])
                    : [in] "r" (accumulator)
                    : "r8", "r9"
                );
                break;
            case 1:
                __asm__ volatile (
                    "movq %[in], %%r10\n\t"
                    "movq %%r10, %[out]\n\t"
                    : [out] "=R" (output[i + ARRAY_SIZE/2]) /* legacy reg constraint */
                    : [in] "r" (accumulator)
                    : "r10", "r11"
                );
                break;
            default:
                __asm__ volatile (
                    "movq %[in], %%r12\n\t"
                    "movq %%r12, %[out]\n\t"
                    : [out] "=r" (output[i + ARRAY_SIZE/2])
                    : [in] "m" (accumulator)  /* memory constraint */
                    : "r12", "r13"
                );
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        printf("  iterations: number of iterations (1-1000)\n");
        printf("  mode: test mode bitmask (1=primary, 2=secondary, 4=optional, 8=control)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    if (iterations <= 0 || iterations > 1000) {
        iterations = 100;
    }
    
    /* Initialize arrays with mixed data */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    float *float_data = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    long *long_data = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_output = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    
    __m256d *avx_data = (__m256d*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + global_seed;
        double_data[i] = i * 1.5;
        float_data[i] = i * 0.75f;
        long_data[i] = i * 7L;
        ((double*)avx_data)[i] = i * 2.0;
    }
    
    /* Run tests based on mode */
    if (mode & 1) {
        printf("Running primary reload tests...\n");
        test_primary_reloads(iterations, int_data, int_output);
    }
    
    if (mode & 2) {
        printf("Running secondary reload tests...\n");
        test_secondary_reloads(iterations, double_data, double_output);
    }
    
    if (mode & 4) {
        printf("Running optional reload tests...\n");
        test_optional_reloads(iterations, float_data, float_output);
    }
    
    if (mode & 8) {
        printf("Running control flow reload tests...\n");
        test_control_flow_reloads(mode, long_data, long_output);
    }
    
    /* Increase register pressure with AVX operations */
    avx_register_pressure(avx_data, ARRAY_SIZE / 8);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (unsigned long long)double_output[i];
        checksum += (unsigned long long)float_output[i];
        checksum += long_output[i];
        checksum += (unsigned long long)((double*)avx_data)[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(int_output);
    free(double_data);
    free(double_output);
    free(float_data);
    free(float_output);
    free(long_data);
    free(long_output);
    free(avx_data);
    
    return 0;
}
