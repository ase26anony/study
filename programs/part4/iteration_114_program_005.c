/* reload_test.c - Complex inline assembly to trigger reload pass logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function for primary reload patterns */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    volatile int a = input[0];
    volatile int b = input[1];
    volatile int c = input[2];
    volatile int d = input[3];
    volatile int e = input[4];
    volatile int f = input[5];
    volatile int g = input[6];
    volatile int h = input[7];
    
    /* Force many live variables to create register pressure */
    int t1 = a, t2 = b, t3 = c, t4 = d, t5 = e, t6 = f, t7 = g, t8 = h;
    int t9 = a + b, t10 = c + d, t11 = e + f, t12 = g + h;
    int t13 = t1 * t2, t14 = t3 * t4, t15 = t5 * t6, t16 = t7 * t8;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with multiple constraints to trigger reload array population */
        __asm__ volatile (
            /* Output operands with different constraints */
            "=r" (t1),     /* General register */
            "=&r" (t2),    /* Earlyclobber general register */
            "=q" (t3),     /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (t4),     /* Accumulator */
            "=d" (t5),     /* Data register */
            "=c" (t6),     /* Counter register */
            "=r" (t7),
            "=r" (t8)
            
            /* Input operands with mixed constraints */
            : "0" (t1),    /* Matching constraint - same as output 0 */
            "r" (t2),
            "rm" (t3),     /* Register or memory */
            "i" (0x1234),  /* Immediate */
            "g" (t5),      /* General (register, memory, or immediate) */
            "r" (t6),
            "m" (input[i % ARRAY_SIZE]),  /* Memory operand */
            "r" (t8)
            
            /* Clobber list */
            : "memory", "cc", "xmm0", "xmm1", "xmm2"
        );
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[din1], %%xmm0\n\t"
            "addsd %[din2], %%xmm0\n\t"
            "movq %%xmm0, %[dout1]"
            : [out1] "=r" (output[i % ARRAY_SIZE]),
              [dout1] "=m" (doutput[i % ARRAY_SIZE])
            : [in1] "rm" (t1),
              [in2] "rm" (t2),
              [in3] "rm" (t3),
              [din1] "xm" (dinput[(i * 2) % ARRAY_SIZE]),
              [din2] "xm" (dinput[(i * 2 + 1) % ARRAY_SIZE])
            : "eax", "xmm0", "memory", "cc"
        );
        
        /* Unrolled section to increase register pressure */
        #pragma unroll(UNROLL_FACTOR)
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            int idx = (i + j) % ARRAY_SIZE;
            __asm__ volatile (
                "addl %%ebx, %%eax\n\t"
                "movl %%eax, %[res]"
                : [res] "=rm" (output[idx])
                : "a" (input[idx]),
                  "b" (j)
                : "cc"
            );
        }
    }
    
    /* Prevent dead code elimination */
    output[0] = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, long long *llinput, long long *lloutput) {
    /* Force use of specific register classes that might require secondary reloads */
    for (int i = 0; i < iterations; i++) {
        long long a = llinput[i % ARRAY_SIZE];
        long long b = llinput[(i + 1) % ARRAY_SIZE];
        long long c = llinput[(i + 2) % ARRAY_SIZE];
        
        /* asm requiring specific registers that might conflict */
        __asm__ volatile (
            /* Force use of 'a' register for input, then move to 'b' register */
            "movq %[in_a], %%rax\n\t"
            "movq %%rax, %%rbx\n\t"
            "addq %[in_b], %%rbx\n\t"
            "movq %%rbx, %[out1]"
            : [out1] "=rm" (lloutput[i % ARRAY_SIZE])
            : [in_a] "a" (a),      /* Must be in rax */
              [in_b] "b" (b)       /* Must be in rbx */
            : "rax", "rbx", "cc"
        );
        
        /* Another asm that might require secondary reload due to constraint mismatch */
        __asm__ volatile (
            "movq %1, %%r8\n\t"
            "addq %2, %%r8\n\t"
            "movq %%r8, %0"
            : "=R" (lloutput[(i + 1) % ARRAY_SIZE])  /* Legacy register constraint */
            : "r" (b),
              "r" (c)
            : "r8", "cc"
        );
    }
}

/* Test function for optional reloads */
void test_optional_reloads(int iterations, float *finput, float *foutput) {
    volatile float f1 = finput[0];
    volatile float f2 = finput[1];
    volatile float f3 = finput[2];
    volatile float f4 = finput[3];
    
    for (int i = 0; i < iterations; i++) {
        /* Use optional constraints */
        float result1, result2;
        
        __asm__ volatile (
            "addss %[in1], %[in2]\n\t"
            "movss %[in2], %[out1]"
            : [out1] "=?r" (result1)  /* Optional output */
            : [in1] "x" (finput[i % ARRAY_SIZE]),
              [in2] "x" (finput[(i + 1) % ARRAY_SIZE])
            : "xmm0"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        __asm__ volatile (
            "mulss %[in1], %[in2]\n\t"
            "movss %[in2], %[out1]"
            : [out1] "=?r" (result2)  /* Optional output */
            : [in1] "x" (finput[(i + 2) % ARRAY_SIZE]),
              [in2] "x" (finput[(i + 3) % ARRAY_SIZE])
            : "xmm0"
        );
        
        foutput[i % ARRAY_SIZE] = result1 + result2;
        
        /* Complex asm with many operands to fill reload array */
        if (i % 3 == 0) {
            __asm__ volatile (
                "=r" (f1),
                "=&r" (f2),
                "=r" (f3),
                "=r" (f4)
                : "0" (f1),
                  "r" (f2),
                  "rm" (f3),
                  "i" (0x3f800000),  /* 1.0f as immediate */
                  "g" (i),
                  "m" (finput[i % ARRAY_SIZE])
                : "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4"
            );
        }
    }
}

/* Mixed scalar and vector operations to increase register pressure */
void test_vector_pressure(int iterations, __m128i *vinput, __m128i *voutput, 
                          __m256d *vdinput, __m256d *vdoutput) {
    /* Mix scalar and vector operations */
    double scalar1 = 1.0, scalar2 = 2.0, scalar3 = 3.0, scalar4 = 4.0;
    double scalar5 = 5.0, scalar6 = 6.0, scalar7 = 7.0, scalar8 = 8.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operations */
        __m128i v1 = vinput[i % (ARRAY_SIZE/4)];
        __m128i v2 = _mm_add_epi32(v1, _mm_set1_epi32(i));
        voutput[i % (ARRAY_SIZE/4)] = v2;
        
        __m256d vd1 = vdinput[i % (ARRAY_SIZE/4)];
        __m256d vd2 = _mm256_add_pd(vd1, _mm256_set1_pd(i * 0.1));
        vdoutput[i % (ARRAY_SIZE/4)] = vd2;
        
        /* Scalar operations in between to force register spilling */
        scalar1 = scalar1 * 1.1 + i;
        scalar2 = scalar2 * 1.2 + i;
        scalar3 = scalar3 * 1.3 + i;
        scalar4 = scalar4 * 1.4 + i;
        scalar5 = scalar5 * 1.5 + i;
        scalar6 = scalar6 * 1.6 + i;
        scalar7 = scalar7 * 1.7 + i;
        scalar8 = scalar8 * 1.8 + i;
        
        /* asm using both scalar and vector constraints */
        if (i % 5 == 0) {
            __asm__ volatile (
                "vmovapd %[vec], %%ymm0\n\t"
                "vaddpd %%ymm0, %%ymm0, %%ymm1\n\t"
                "vmovapd %%ymm1, %[vecout]\n\t"
                "addsd %[sc1], %[sc2]\n\t"
                "movsd %[sc2], %[sout]"
                : [vecout] "=m" (vdoutput[i % (ARRAY_SIZE/4)]),
                  [sout] "=m" (scalar1)
                : [vec] "m" (vdinput[i % (ARRAY_SIZE/4)]),
                  [sc1] "x" (scalar2),
                  [sc2] "x" (scalar3)
                : "ymm0", "ymm1", "xmm0", "xmm1", "memory"
            );
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        printf("  iterations: Number of loop iterations (e.g., 1000)\n");
        printf("  mode: Test mode (1-4)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (mode < 1 || mode > 4) mode = 1;
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long long *ll_array = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    long long *ll_output = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    __m128i *vec_array = (__m128i*)malloc((ARRAY_SIZE/4) * sizeof(__m128i));
    __m128i *vec_output = (__m128i*)malloc((ARRAY_SIZE/4) * sizeof(__m128i));
    __m256d *vecd_array = (__m256d*)malloc((ARRAY_SIZE/4) * sizeof(__m256d));
    __m256d *vecd_output = (__m256d*)malloc((ARRAY_SIZE/4) * sizeof(__m256d));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        int_output[i] = 0;
        double_array[i] = i * 0.5;
        double_output[i] = 0.0;
        float_array[i] = i * 0.25f;
        float_output[i] = 0.0f;
        ll_array[i] = (long long)i * 1000;
        ll_output[i] = 0;
        
        if (i % 4 == 0 && i < ARRAY_SIZE/4) {
            vec_array[i/4] = _mm_set_epi32(i+3, i+2, i+1, i);
            vec_output[i/4] = _mm_setzero_si128();
            vecd_array[i/4] = _mm256_set_pd(i+3.0, i+2.0, i+1.0, i+0.0);
            vecd_output[i/4] = _mm256_setzero_pd();
        }
    }
    
    /* Execute test functions based on mode */
    switch (mode) {
        case 1:
            test_primary_reloads(iterations, int_array, int_output, 
                                double_array, double_output);
            break;
        case 2:
            test_secondary_reloads(iterations, ll_array, ll_output);
            break;
        case 3:
            test_optional_reloads(iterations, float_array, float_output);
            break;
        case 4:
            test_vector_pressure(iterations, vec_array, vec_output,
                                vecd_array, vecd_output);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations/4, int_array, int_output, 
                                double_array, double_output);
            test_secondary_reloads(iterations/4, ll_array, ll_output);
            test_optional_reloads(iterations/4, float_array, float_output);
            test_vector_pressure(iterations/4, vec_array, vec_output,
                                vecd_array, vecd_output);
            break;
    }
    
    /* Compute checksum to ensure all asm blocks executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long long)(double_output[i] * 1000);
        checksum += (long long)(float_output[i] * 1000);
        checksum += ll_output[i];
        
        if (i % 4 == 0 && i < ARRAY_SIZE/4) {
            int v[4];
            _mm_storeu_si128((__m128i*)v, vec_output[i/4]);
            checksum += v[0] + v[1] + v[2] + v[3];
            
            double vd[4];
            _mm256_storeu_pd(vd, vecd_output[i/4]);
            checksum += (long long)(vd[0] + vd[1] + vd[2] + vd[3]);
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(int_output);
    free(double_array);
    free(double_output);
    free(float_array);
    free(float_output);
    free(ll_array);
    free(ll_output);
    free(vec_array);
    free(vec_output);
    free(vecd_array);
    free(vecd_output);
    
    return 0;
}
