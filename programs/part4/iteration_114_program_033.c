#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions prototypes */
static void test_primary_reloads(int iterations, int *input, int *output);
static void test_secondary_reloads(int iterations, double *dinput, double *doutput);
static void test_optional_reloads(int iterations, float *finput, float *foutput);
static void test_vector_reloads(int iterations, __m128i *vinput, __m128i *voutput);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 0.0;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    __m128i *vector_array = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    __m128i *vector_output = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 2.0;
        float_array[i] = i * 0.75f + 1.0f;
        vector_array[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Run tests based on mode */
    if (mode & 1) {
        test_primary_reloads(iterations, int_array, int_output);
    }
    
    if (mode & 2) {
        test_secondary_reloads(iterations, double_array, double_output);
    }
    
    if (mode & 4) {
        test_optional_reloads(iterations, float_array, float_output);
    }
    
    if (mode & 8) {
        test_vector_reloads(iterations, vector_array, vector_output);
    }
    
    /* Compute checksum to ensure all assembly executed */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (uint64_t)(double_output[i] * 1000);
        checksum += (uint64_t)(float_output[i] * 1000);
        int32_t *v = (int32_t*)&vector_output[i];
        checksum += v[0] + v[1] + v[2] + v[3];
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global double: %f\n", global_double);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(vector_array);
    free(int_output);
    free(double_output);
    free(float_output);
    free(vector_output);
    
    return 0;
}

/* Complex inline assembly with many operands to trigger primary reloads */
static void test_primary_reloads(int iterations, int *input, int *output) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Create many live variables to increase register pressure */
    a = input[0];
    b = input[1];
    c = input[2];
    d = input[3];
    e = input[4];
    f = input[5];
    g = input[6];
    h = input[7];
    i = input[8];
    j = input[9];
    k = input[10];
    l = input[11];
    m = input[12];
    n = input[13];
    o = input[14];
    p = input[15];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with 8 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (tmp1),     /* General register */
            "=&r" (tmp2),    /* Early clobber */
            "=q" (tmp3),     /* Byte register (a,b,c,d) */
            "=a" (tmp4),     /* Accumulator */
            
            /* Inputs with mixed constraints */
            : "r" (a),       /* Register */
              "m" (input[iter % ARRAY_SIZE]),  /* Memory */
              "i" (123),     /* Immediate */
              "r" (b),
              "d" (c),       /* DX register */
              "t" (d)        /* Top of FP stack */
            
            /* Clobber many registers */
            : "rcx", "r8", "r9", "r10", "r11", "cc", "memory"
        );
        
        /* Another asm with matching constraint to force reload */
        __asm__ volatile (
            "addl %2, %0\n\t"
            "subl %3, %1\n\t"
            : "=r" (tmp1), "=r" (tmp2)
            : "0" (tmp1),   /* Matching constraint - same as output 0 */
              "r" (tmp3),
              "r" (e),
              "r" (f),
              "r" (g)
            : "cc"
        );
        
        /* Use results to prevent dead code elimination */
        output[iter % ARRAY_SIZE] = tmp1 + tmp2 + tmp3 + tmp4;
        
        /* Rotate live variables to keep them all active */
        int t = a;
        a = b; b = c; c = d; d = e; e = f; f = g; g = h;
        h = i; i = j; j = k; k = l; l = m; m = n; n = o; o = p;
        p = t + iter;
    }
    
    global_counter += a + b + c;
}

/* Force secondary reloads with mismatched constraints */
static void test_secondary_reloads(int iterations, double *dinput, double *doutput) {
    double d1, d2, d3, d4, d5, d6, d7, d8;
    double result1, result2;
    int int_tmp;
    
    /* Initialize with array values */
    d1 = dinput[0];
    d2 = dinput[1];
    d3 = dinput[2];
    d4 = dinput[3];
    d5 = dinput[4];
    d6 = dinput[5];
    d7 = dinput[6];
    d8 = dinput[7];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force secondary reload: "a" constraint with memory operand */
        __asm__ volatile (
            "movq %1, %%rax\n\t"
            "addq %%rax, %0\n\t"
            : "=r" (int_tmp)
            : "a" (dinput[iter % ARRAY_SIZE]),  /* May need secondary reload if not in rax */
              "m" (dinput[(iter + 1) % ARRAY_SIZE])
            : "rax", "cc"
        );
        
        /* Complex FP operation with mixed constraints */
        __asm__ volatile (
            "addsd %2, %0\n\t"
            "mulsd %3, %1\n\t"
            : "=x" (result1),   /* XMM register */
              "=t" (result2)    /* Top of FP stack */
            : "xm" (d1),        /* XMM or memory */
              "x" (d2),
              "r" (iter)        /* Integer in general register */
            : "xmm0", "xmm1", "cc"
        );
        
        /* Another asm that might require secondary reload for R constraint */
        __asm__ volatile (
            "mov %1, %0\n\t"
            : "=R" (int_tmp)    /* Legacy register (eax, ebx, ecx, edx) */
            : "r" (iter * 2)    /* Could be in R8-R15, requiring secondary move */
            : "cc"
        );
        
        doutput[iter % ARRAY_SIZE] = result1 + result2 + int_tmp;
        
        /* Rotate double values */
        double temp = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = d5; d5 = d6; d6 = d7; d7 = d8;
        d8 = temp + iter * 0.1;
    }
    
    global_double += d1 + d2 + d3;
}

/* Test optional reloads and nocombine scenarios */
static void test_optional_reloads(int iterations, float *finput, float *foutput) {
    float f1, f2, f3, f4;
    int opt1, opt2;
    
    f1 = finput[0];
    f2 = finput[1];
    f3 = finput[2];
    f4 = finput[3];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Optional constraint with '?' */
        __asm__ volatile (
            "movl %2, %0\n\t"
            "testl %3, %3\n\t"
            "cmovnzl %3, %1\n\t"
            : "=r" (opt1), "=?r" (opt2)  /* opt2 is optional */
            : "r" (iter), "r" (iter % 2 ? iter : 0)
            : "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "addl $1, %0\n\t"
            : "+r" (opt1)
            : 
            : "cc"
        );
        
        /* Another volatile asm with different clobbers */
        __asm__ volatile (
            "movss %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m" (foutput[iter % ARRAY_SIZE])
            : "m" (finput[iter % ARRAY_SIZE]),
              "x" (f1)
            : "xmm0", "memory"
        );
        
        /* Update live variables */
        f1 = f2 + opt1 * 0.01f;
        f2 = f3 + opt2 * 0.02f;
        f3 = f4 + iter * 0.03f;
        f4 = finput[(iter + 4) % ARRAY_SIZE];
    }
}

/* Vector operations to increase register pressure */
static void test_vector_reloads(int iterations, __m128i *vinput, __m128i *voutput) {
    __m128i v1, v2, v3, v4, v5, v6;
    __m256d avx1, avx2;
    double d1, d2, d3, d4;
    
    /* Initialize vectors */
    v1 = vinput[0];
    v2 = vinput[1];
    v3 = vinput[2];
    v4 = vinput[3];
    v5 = vinput[4];
    v6 = vinput[5];
    
    avx1 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    avx2 = _mm256_set_pd(5.0, 6.0, 7.0, 8.0);
    
    d1 = 1.0; d2 = 2.0; d3 = 3.0; d4 = 4.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mix vector and scalar operations to increase pressure */
        __m128i temp;
        
        /* Vector asm with many operands */
        __asm__ volatile (
            "paddd %1, %0\n\t"
            "pslld $2, %0\n\t"
            : "+x" (v1)
            : "xm" (v2),
              "r" (iter)      /* Integer in general register */
            : "xmm2", "xmm3", "cc"
        );
        
        /* Scalar asm while vectors are live */
        __asm__ volatile (
            "addsd %1, %0\n\t"
            "mulsd %2, %0\n\t"
            : "+x" (d1)
            : "xm" (d2),
              "x" (d3)
            : "cc"
        );
        
        /* Another vector operation */
        __asm__ volatile (
            "vpaddd %1, %0, %0\n\t"
            : "+x" (v3)
            : "xm" (v4)
            : "xmm4", "xmm5"
        );
        
        /* AVX operation */
        __asm__ volatile (
            "vaddpd %1, %0, %0\n\t"
            : "+x" (avx1)
            : "xm" (avx2)
            : "ymm6", "ymm7"
        );
        
        /* Store result */
        voutput[iter % ARRAY_SIZE] = v1;
        
        /* Rotate vectors to keep them live */
        __m128i vtemp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = _mm_add_epi32(vtemp, _mm_set1_epi32(iter));
        
        /* Update AVX registers */
        avx1 = _mm256_add_pd(avx1, avx2);
        avx2 = _mm256_set1_pd(iter * 0.1);
        
        /* Update scalars */
        d1 = d2; d2 = d3; d3 = d4; d4 = iter * 0.01;
    }
}
