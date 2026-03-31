/* reload_test.c - Complex inline assembly to trigger reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_control_flow_reloads(int iterations, int mode, int *input, int *output);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Complex inline assembly with many operands */
static inline uint64_t complex_asm_5op(uint64_t a, uint64_t b, uint64_t c, 
                                       uint64_t d, uint64_t e) {
    uint64_t result1, result2, result3;
    
    /* 5+ operands with mixed constraints */
    __asm__ volatile (
        "mov %[a_cpy], %[a]\n\t"
        "add %[b], %[a_cpy]\n\t"
        "imul %[c], %[a_cpy]\n\t"
        "sub %[d], %[a_cpy]\n\t"
        "xor %[e], %[a_cpy]\n\t"
        "mov %[res1], %[a_cpy]\n\t"
        "shr $8, %[res1]\n\t"
        "mov %[res2], %[a_cpy]\n\t"
        "and $0xFF, %[res2]\n\t"
        "lea (%[res1], %[res2], 2), %[res3]"
        : [res1] "=&r" (result1),  /* Early clobber */
          [res2] "=&q" (result2),  /* Byte register constraint */
          [res3] "=r" (result3),
          [a_cpy] "=&r" (a)        /* Early clobber output */
        : [a] "0" (a),             /* Matching constraint */
          [b] "rm" (b),            /* Register or memory */
          [c] "r" (c),
          [d] "im" (d),            /* Immediate */
          [e] "r" (e)
        : "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* Assembly with accumulator constraint */
static inline uint32_t accumulator_asm(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t result;
    
    __asm__ volatile (
        "movl %[a], %%eax\n\t"
        "mull %[b]\n\t"
        "addl %[c], %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (result)
        : [a] "a" (a),     /* Must be in eax */
          [b] "r" (b),
          [c] "rm" (c)     /* Register or memory */
        : "eax", "edx", "cc"
    );
    
    return result;
}

/* Assembly requiring secondary reloads */
static inline double secondary_reload_asm(double a, double b, int c) {
    double result;
    int temp;
    
    /* First asm with specific register constraint */
    __asm__ volatile (
        "mov %[c], %%ebx\n\t"
        "cvtsi2sd %%ebx, %%xmm0\n\t"
        "addsd %[a], %%xmm0"
        : "=t" (result)        /* Top of FP stack */
        : [a] "x" (a),         /* XMM register */
          [c] "r" (c)
        : "xmm0", "ebx", "cc"
    );
    
    /* Second asm using different register class */
    __asm__ volatile (
        "mov %%eax, %[temp]\n\t"
        "cvtsi2sd %[temp], %%xmm1\n\t"
        "mulsd %[b], %%xmm1\n\t"
        "addsd %%xmm1, %[result]"
        : [result] "+t" (result),
          [temp] "=m" (temp)
        : [b] "x" (b)
        : "xmm1", "cc", "memory"
    );
    
    return result;
}

/* Optional constraint asm */
static inline int optional_reload_asm(int a, int b, int *ptr) {
    int result;
    int optional_out;
    
    __asm__ volatile (
        "test %[a], %[a]\n\t"
        "cmovz %[b], %[a]\n\t"
        "add (%[ptr]), %[a]\n\t"
        "mov %[a], %[result]\n\t"
        "mov $0, %[opt]"
        : [result] "=r" (result),
          [opt] "=?r" (optional_out),  /* Optional output */
          [a] "+r" (a)
        : [b] "r" (b),
          [ptr] "r" (ptr)
        : "cc", "memory"
    );
    
    global_counter += optional_out;  /* Use optional result */
    return result;
}

void test_primary_reloads(int iterations, int *input, int *output) {
    /* Create many live variables to pressure registers */
    int v1 = input[0];
    int v2 = input[1];
    int v3 = input[2];
    int v4 = input[3];
    int v5 = input[4];
    int v6 = input[5];
    int v7 = input[6];
    int v8 = input[7];
    int v9 = input[8];
    int v10 = input[9];
    int v11 = input[10];
    int v12 = input[11];
    int v13 = input[12];
    int v14 = input[13];
    int v15 = input[14];
    int v16 = input[15];
    
    /* Unrolled loop with many asm statements */
    for (int i = 0; i < iterations; i++) {
        /* Mix scalar and vector operations */
        __m128i vec1 = _mm_set_epi32(v1, v2, v3, v4);
        __m128i vec2 = _mm_set_epi32(v5, v6, v7, v8);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        
        /* Complex asm with many operands */
        v1 = complex_asm_5op(v1, v2, v3, v4, i);
        v2 = complex_asm_5op(v2, v3, v4, v5, i+1);
        v3 = complex_asm_5op(v3, v4, v5, v6, i+2);
        v4 = complex_asm_5op(v4, v5, v6, v7, i+3);
        
        /* More live variables */
        int t1 = v9 + v10;
        int t2 = v11 * v12;
        int t3 = v13 ^ v14;
        int t4 = v15 | v16;
        
        /* Another asm block */
        __asm__ volatile (
            "imul %[t1], %[v9]\n\t"
            "add %[t2], %[v10]\n\t"
            "xor %[t3], %[v11]\n\t"
            "or %[t4], %[v12]"
            : [v9] "+r" (v9),
              [v10] "+r" (v10),
              [v11] "+r" (v11),
              [v12] "+r" (v12)
            : [t1] "rm" (t1),
              [t2] "rm" (t2),
              [t3] "rm" (t3),
              [t4] "rm" (t4)
            : "cc"
        );
        
        /* Use vector result */
        int *vec_data = (int*)&vec3;
        v13 += vec_data[0];
        v14 += vec_data[1];
        v15 += vec_data[2];
        v16 += vec_data[3];
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
    }
    
    /* Store results */
    output[0] = v1; output[1] = v2; output[2] = v3; output[3] = v4;
    output[4] = v5; output[5] = v6; output[6] = v7; output[7] = v8;
    output[8] = v9; output[9] = v10; output[10] = v11; output[11] = v12;
    output[12] = v13; output[13] = v14; output[14] = v15; output[15] = v16;
}

void test_secondary_reloads(int iterations, double *input, double *output) {
    double d1 = input[0];
    double d2 = input[1];
    double d3 = input[2];
    double d4 = input[3];
    double d5 = input[4];
    double d6 = input[5];
    
    /* Use AVX vectors alongside scalar doubles */
    __m256d avx_vec1 = _mm256_set_pd(d1, d2, d3, d4);
    __m256d avx_vec2 = _mm256_set_pd(d5, d6, d1, d2);
    
    for (int i = 0; i < iterations; i++) {
        /* Mix AVX and scalar operations */
        __m256d avx_result = _mm256_add_pd(avx_vec1, avx_vec2);
        
        /* Scalar asm requiring secondary reloads */
        d1 = secondary_reload_asm(d1, d2, i);
        d2 = secondary_reload_asm(d2, d3, i+1);
        
        /* More register pressure */
        double t1 = d3 * d4;
        double t2 = d5 / d6;
        double t3 = d1 + d2;
        double t4 = d3 - d4;
        
        /* Assembly with legacy register constraints */
        long long temp;
        __asm__ volatile (
            "movq %[t1], %%rax\n\t"
            "addq %[t2], %%rax\n\t"
            "movq %%rax, %[temp]"
            : [temp] "=R" (temp)  /* Legacy register constraint */
            : [t1] "rm" ((long long)t1),
              [t2] "rm" ((long long)t2)
            : "rax", "cc"
        );
        
        d5 = (double)temp + t3;
        
        /* Update vectors */
        double *avx_data = (double*)&avx_result;
        d6 += avx_data[0] + avx_data[1];
        
        /* Memory operation that might need reload */
        __asm__ volatile (
            "movsd %[d4], (%[ptr])\n\t"
            "movsd (%[ptr]), %[d4]"
            : [d4] "+x" (d4)
            : [ptr] "r" (&global_double)
            : "memory"
        );
    }
    
    output[0] = d1; output[1] = d2; output[2] = d3;
    output[3] = d4; output[4] = d5; output[5] = d6;
}

void test_optional_reloads(int iterations, float *input, float *output) {
    float f1 = input[0];
    float f2 = input[1];
    float f3 = input[2];
    float f4 = input[3];
    
    int int1 = (int)f1;
    int int2 = (int)f2;
    int int3 = (int)f3;
    int int4 = (int)f4;
    
    for (int i = 0; i < iterations; i++) {
        /* Optional reload asm */
        int1 = optional_reload_asm(int1, int2, &global_counter);
        int2 = optional_reload_asm(int2, int3, &global_counter);
        
        /* Volatile barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        int3 = optional_reload_asm(int3, int4, &global_counter);
        
        /* Different clobber list to prevent combination */
        __asm__ volatile (
            "addl $1, %[int4]"
            : [int4] "+r" (int4)
            :: "cc"  /* Different from previous asm */
        );
        
        /* Convert back to float with SSE */
        f1 = (float)int1;
        f2 = (float)int2;
        f3 = (float)int3;
        f4 = (float)int4;
        
        /* SSE operations */
        __m128 sse_vec = _mm_set_ps(f1, f2, f3, f4);
        sse_vec = _mm_add_ps(sse_vec, _mm_set1_ps(1.0f));
        
        float *sse_data = (float*)&sse_vec;
        f1 = sse_data[0];
        f2 = sse_data[1];
        f3 = sse_data[2];
        f4 = sse_data[3];
        
        /* Another volatile barrier */
        __asm__ volatile ("" ::: "memory");
    }
    
    output[0] = f1; output[1] = f2; output[2] = f3; output[3] = f4;
}

void test_control_flow_reloads(int iterations, int mode, int *input, int *output) {
    int a = input[0];
    int b = input[1];
    int c = input[2];
    int d = input[3];
    
    for (int i = 0; i < iterations; i++) {
        /* Control flow dependent asm */
        if (mode & 1) {
            __asm__ volatile (
                "mov %[a], %%eax\n\t"
                "add %[b], %%eax\n\t"
                "mov %%eax, %[a]"
                : [a] "+r" (a)
                : [b] "rm" (b)
                : "eax", "cc"
            );
        } else {
            __asm__ volatile (
                "mov %[a], %%ebx\n\t"
                "sub %[b], %%ebx\n\t"
                "mov %%ebx, %[a]"
                : [a] "+r" (a)
                : [b] "rm" (b)
                : "ebx", "cc"
            );
        }
        
        /* Loop-dependent asm */
        for (int j = 0; j < (i % 4); j++) {
            __asm__ volatile (
                "imul %[c], %[d]\n\t"
                "add $1, %[c]"
                : [c] "+r" (c),
                  [d] "+r" (d)
                :: "cc"
            );
        }
        
        /* Switch with asm in cases */
        switch (i % 3) {
            case 0:
                __asm__ volatile (
                    "xor %[a], %[b]"
                    : [b] "+r" (b)
                    : [a] "r" (a)
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "or %[a], %[c]"
                    : [c] "+r" (c)
                    : [a] "r" (a)
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "and %[a], %[d]"
                    : [d] "+r" (d)
                    : [a] "r" (a)
                    : "cc"
                );
                break;
        }
        
        /* Accumulator asm that depends on control flow */
        a = accumulator_asm(a, b, c);
        
        /* Update mode based on computation */
        mode ^= (a & 1);
    }
    
    output[0] = a; output[1] = b; output[2] = c; output[3] = d;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        printf("  iterations: Number of loop iterations (e.g., 100)\n");
        printf("  mode: Test mode bitmask (0-7)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (mode < 0 || mode > 7) mode = 0;
    
    /* Initialize arrays with mixed data */
    int int_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    int output_int[ARRAY_SIZE];
    double output_double[ARRAY_SIZE];
    float output_float[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 0.5;
        float_array[i] = i * 0.75f + 0.25f;
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_array, output_int);
    test_secondary_reloads(iterations, double_array, output_double);
    test_optional_reloads(iterations, float_array, output_float);
    test_control_flow_reloads(iterations, mode, int_array + 16, output_int + 16);
    
    /* Compute checksum to ensure all asm executed */
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += output_int[i];
        checksum += (uint64_t)output_double[i];
        checksum += (uint32_t)output_float[i];
    }
    
    checksum += global_counter;
    
    printf("Checksum: %lu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
