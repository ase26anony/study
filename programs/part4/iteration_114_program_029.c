/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, int *in, int *out, float *fin, float *fout);
void test_optional_reloads(int iterations, int *in, int *out, long long *lin, long long *lout);
void test_control_flow_reloads(int iterations, int mode, int *in, int *out);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.141592653589793;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data types */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long long *ll_array = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 0.5;
        float_array[i] = i * 0.75f + 0.25f;
        ll_array[i] = (long long)i * 7 + 3;
    }
    
    printf("Starting reload tests with iterations=%d, mode=%d\n", iterations, mode);
    
    /* Execute test functions to trigger various reload patterns */
    test_primary_reloads(iterations, int_array, int_out, double_array, double_out);
    test_secondary_reloads(iterations, int_array, int_out, float_array, float_out);
    test_optional_reloads(iterations, int_array, int_out, ll_array, ll_out);
    test_control_flow_reloads(iterations, mode, int_array, int_out);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += (unsigned long long)(double_out[i] * 1000);
        checksum += (unsigned long long)(float_out[i] * 1000);
        checksum += ll_out[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_array);
    free(int_out);
    free(double_array);
    free(double_out);
    free(float_array);
    free(float_out);
    free(ll_array);
    free(ll_out);
    
    return 0;
}

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    /* Create many live variables to increase register pressure */
    int v0 = in[0], v1 = in[1], v2 = in[2], v3 = in[3];
    int v4 = in[4], v5 = in[5], v6 = in[6], v7 = in[7];
    double d0 = din[0], d1 = din[1], d2 = din[2], d3 = din[3];
    
    /* Unrolled loop with complex asm statements */
    for (int i = 0; i < iterations; i++) {
        /* Extended asm with 5+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (v0),     /* General register */
            "=&r" (v1),    /* Early clobber */
            "=q" (v2),     /* Byte-addressable register (a,b,c,d) */
            "=a" (v3),     /* Accumulator */
            "=d" (v4),     /* Data register */
            /* Inputs with various constraints */
            : "0" (v0),    /* Matching constraint */
              "1" (v1),
              "r" (v2),    /* General register */
              "m" (in[i % ARRAY_SIZE]),  /* Memory */
              "i" (123),   /* Immediate */
              "r" (v5),
              "r" (v6),
              "r" (v7),
              "t" (d0),    /* Top of FP stack */
              "u" (d1)     /* Second FP stack */
            /* Clobber list */
            : "cc", "memory", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2"
        );
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "=r" (v5),
            "=r" (v6),
            "=t" (d2),     /* Top of FP stack for double */
            "=u" (d3),     /* Second FP stack */
            : "r" (v0),
              "r" (v1),
              "m" (in[(i + 1) % ARRAY_SIZE]),
              "r" (global_counter),
              "f" (d0),    /* Floating point register */
              "f" (d1)
            : "cc", "memory", "xmm3", "xmm4"
        );
        
        /* Use vector intrinsics to increase register pressure */
        __m128i vec1 = _mm_set_epi32(v0, v1, v2, v3);
        __m128i vec2 = _mm_set_epi32(v4, v5, v6, v7);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        
        /* Store results */
        int idx = i % (ARRAY_SIZE / UNROLL_FACTOR);
        out[idx * UNROLL_FACTOR + 0] = v0;
        out[idx * UNROLL_FACTOR + 1] = v1;
        out[idx * UNROLL_FACTOR + 2] = v2;
        out[idx * UNROLL_FACTOR + 3] = v3;
        dout[idx * 2] = d2;
        dout[idx * 2 + 1] = d3;
        
        /* Update global to prevent dead code elimination */
        global_counter += vec3[0];
    }
}

/* Force secondary reload patterns */
void test_secondary_reloads(int iterations, int *in, int *out, float *fin, float *fout) {
    /* Variables with specific register requirements */
    register int r0 asm("eax");
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    r0 = in[0]; r1 = in[1]; r2 = in[2]; r3 = in[3];
    
    for (int i = 0; i < iterations; i++) {
        /* Asm with constraints requiring specific registers */
        __asm__ volatile (
            /* Output must be in ebx */
            "=b" (r1),
            /* Inputs with mixed constraints */
            : "a" (r0),      /* Must be in eax */
              "c" (r2),      /* Must be in ecx */
              "d" (r3),      /* Must be in edx */
              "m" (in[i % ARRAY_SIZE]),  /* Memory operand */
              "R" (r0)       /* Legacy register constraint */
            : "cc", "memory", "r8", "r9", "r10", "r11"
        );
        
        /* Force secondary reload by using result in different constraint */
        __asm__ volatile (
            "=a" (r0),      /* Result in eax */
            "=r" (r2),      /* General register */
            : "b" (r1),     /* Input must be in ebx */
              "r" (r3),
              "m" (fin[i % ARRAY_SIZE])  /* Float in memory */
            : "cc", "memory", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Use AVX registers to increase pressure */
        __m256d avx_vec = _mm256_set_pd(fin[0], fin[1], fin[2], fin[3]);
        __m256d avx_vec2 = _mm256_set_pd(fin[4], fin[5], fin[6], fin[7]);
        __m256d avx_result = _mm256_add_pd(avx_vec, avx_vec2);
        
        /* Store results */
        fout[i % ARRAY_SIZE] = (float)avx_result[0] + r0 + r1;
        out[i % ARRAY_SIZE] = r0 + r1 + r2 + r3;
        
        global_counter += r0;
    }
}

/* Test optional and non-combine reloads */
void test_optional_reloads(int iterations, int *in, int *out, long long *lin, long long *lout) {
    long long l0 = lin[0], l1 = lin[1], l2 = lin[2];
    int v0 = in[0], v1 = in[1], v2 = in[2];
    
    for (int i = 0; i < iterations; i++) {
        /* Asm with optional constraints */
        __asm__ volatile (
            "=?r" (v0),     /* Optional output */
            "=r" (v1),
            "=&r" (v2),     /* Early clobber */
            : "0" (v0),     /* Matching constraint */
              "r" (v1),
              "m" (lin[i % ARRAY_SIZE]),  /* 64-bit memory */
              "i" (456)     /* Immediate */
            : "cc", "memory"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "=?r" (l0),     /* Optional 64-bit output */
            "=r" (l1),
            : "r" (v0),
              "r" (v1),
              "m" (in[i % ARRAY_SIZE])
            : "cc", "memory", "rax", "rbx", "rcx"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Complex asm with many clobbers to prevent combining */
        __asm__ volatile (
            "=r" (l2),
            "=r" (v2),
            : "r" (l0),
              "r" (l1),
              "m" (global_double)
            : "cc", "memory", 
              "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* Store results */
        lout[i % ARRAY_SIZE] = l0 + l1 + l2;
        out[i % ARRAY_SIZE] = v0 + v1 + v2;
        
        global_counter += v2;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int mode, int *in, int *out) {
    int a = in[0], b = in[1], c = in[2], d = in[3];
    int e = in[4], f = in[5], g = in[6], h = in[7];
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional asm execution */
        if (mode & 1) {
            __asm__ volatile (
                "=r" (a),
                "=r" (b),
                "=&r" (c),
                : "0" (a),
                  "1" (b),
                  "r" (c),
                  "m" (in[(i + 8) % ARRAY_SIZE]),
                  "i" (i)
                : "cc", "memory", "r8", "r9"
            );
        }
        
        if (mode & 2) {
            __asm__ volatile (
                "=r" (d),
                "=r" (e),
                "=q" (f),  /* Byte register */
                : "r" (d),
                  "r" (e),
                  "r" (f),
                  "m" (in[(i + 16) % ARRAY_SIZE])
                : "cc", "memory", "r10", "r11"
            );
        }
        
        /* Loop with asm inside */
        for (int j = 0; j < 4; j++) {
            __asm__ volatile (
                "=r" (g),
                "=r" (h),
                : "r" (g),
                  "r" (h),
                  "m" (in[(i + j) % ARRAY_SIZE]),
                  "r" (j)
                : "cc", "memory"
            );
            
            /* Nested condition */
            if (j % 2 == 0) {
                __asm__ volatile (
                    "=a" (a),  /* Must be in eax */
                    : "b" (b), /* Must be in ebx */
                      "c" (c), /* Must be in ecx */
                      "d" (d)  /* Must be in edx */
                    : "cc"
                );
            }
        }
        
        /* Store results with complex indexing */
        int idx = (i * 7) % ARRAY_SIZE;
        out[idx] = a + b + c + d + e + f + g + h;
        
        /* Update mode based on result to affect control flow */
        mode = (mode + out[idx]) % 4;
        
        global_counter += a;
    }
}
