/* test_reload.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int iterations, int *data, int *result);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mode = 0;

/* Complex inline assembly with many operands and constraints */
static inline void complex_asm_5op(int a, int b, int c, int *out1, int *out2, int *out3) {
    int tmp1, tmp2, tmp3;
    
    /* 5+ operands with mixed constraints */
    __asm__ volatile (
        "movl %[a], %[tmp1]\n\t"
        "addl %[b], %[tmp1]\n\t"
        "imull %[c], %[tmp1]\n\t"
        "movl %[tmp1], %[tmp2]\n\t"
        "shrl $3, %[tmp2]\n\t"
        "leal (%[tmp1], %[tmp2], 2), %[tmp3]\n\t"
        : [tmp1] "=&r" (tmp1),   /* earlyclobber */
          [tmp2] "=&r" (tmp2),   /* earlyclobber */
          [tmp3] "=r" (tmp3)
        : [a] "rmi" (a),         /* register/memory/immediate */
          [b] "rmi" (b),
          [c] "rm" (c)
        : "cc", "memory"
    );
    
    /* Second asm using results with different constraints */
    __asm__ volatile (
        "testl %[tmp1], %[tmp1]\n\t"
        "cmovgl %[tmp2], %[tmp3]\n\t"
        "movl %[tmp3], %0\n\t"
        "movl %[tmp1], %1\n\t"
        "movl %[tmp2], %2\n\t"
        : "=rm" (*out1),         /* register/memory */
          "=rm" (*out2),
          "=rm" (*out3)
        : [tmp1] "r" (tmp1),
          [tmp2] "r" (tmp2),
          [tmp3] "0" (tmp3)      /* matching constraint */
        : "cc"
    );
}

/* Assembly with byte/word register constraints */
static inline void mixed_width_asm(unsigned char *bytes, unsigned short *words, int *dwords) {
    unsigned char b1, b2;
    unsigned short w1, w2;
    int d1, d2;
    
    __asm__ volatile (
        "movb %[b_in], %%al\n\t"
        "movb %%al, %[b1]\n\t"
        "incb %[b1]\n\t"
        "movw %[w_in], %%ax\n\t"
        "movw %%ax, %[w1]\n\t"
        "addw $5, %[w1]\n\t"
        "movl %[d_in], %%eax\n\t"
        "movl %%eax, %[d1]\n\t"
        "shrl $1, %[d1]\n\t"
        : [b1] "=q" (b1),        /* byte register constraint */
          [w1] "=r" (w1),
          [d1] "=r" (d1)
        : [b_in] "m" (*bytes),
          [w_in] "m" (*words),
          [d_in] "m" (*dwords)
        : "eax", "cc"
    );
    
    /* Force different register classes */
    __asm__ volatile (
        "xchgb %%al, %[b2]\n\t"
        "xchgw %%ax, %[w2]\n\t"
        "xchgl %%eax, %[d2]\n\t"
        : [b2] "=q" (b2),
          [w2] "=r" (w2),
          [d2] "=a" (d2)         /* accumulator constraint */
        : "0" (b1),
          "1" (w1),
          "2" (d1)
        : "cc"
    );
    
    *bytes = b2;
    *words = w2;
    *dwords = d2;
}

/* Assembly requiring secondary reloads (register class mismatches) */
static inline void secondary_reload_asm(long long val, long long *out, int mode) {
    long long result;
    
    if (mode & 1) {
        /* Force potential secondary reload with R constraint */
        __asm__ volatile (
            "movq %[val], %%rax\n\t"
            "addq $0x12345678, %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=R" (result)  /* Legacy register constraint */
            : [val] "rm" (val)
            : "rax", "cc"
        );
    } else {
        /* Different constraint to create register pressure */
        __asm__ volatile (
            "movq %[val], %%rdi\n\t"
            "subq $0x87654321, %%rdi\n\t"
            "movq %%rdi, %[result]\n\t"
            : [result] "=r" (result)
            : [val] "rm" (val)
            : "rdi", "cc"
        );
    }
    
    /* Memory barrier to prevent combination */
    __asm__ volatile ("" ::: "memory");
    
    *out = result;
}

/* Assembly with optional constraints */
static inline int optional_reload_asm(int a, int b, int *opt_out) {
    int result, optional_result;
    
    /* Use ? modifier for optional output */
    __asm__ volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "testl %%eax, %%eax\n\t"
        "jz 1f\n\t"
        "movl %%eax, %[opt]\n\t"
        "1:\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=r" (result),
          [opt] "=?r" (optional_result)  /* optional constraint */
        : [a] "rm" (a),
          [b] "rm" (b)
        : "eax", "cc"
    );
    
    if (opt_out && optional_result) {
        *opt_out = optional_result;
    }
    
    return result;
}

/* Vector/SIMD operations mixed with scalar */
static inline __m128i vector_scalar_mix(__m128i vec, int scalar, __m128i *out) {
    __m128i result;
    int temp;
    
    /* Scalar operation creating register pressure */
    __asm__ volatile (
        "movl %[scalar], %%ecx\n\t"
        "shll $3, %%ecx\n\t"
        "movl %%ecx, %[temp]\n\t"
        : [temp] "=r" (temp)
        : [scalar] "rm" (scalar)
        : "ecx", "cc"
    );
    
    /* Vector operation */
    __asm__ volatile (
        "paddd %[vec], %[vec]\n\t"
        "movdqa %[vec], %[result]\n\t"
        : [result] "=x" (result)
        : [vec] "x" (vec)
        : "cc"
    );
    
    /* Another scalar operation */
    __asm__ volatile (
        "addl $1, %[temp]\n\t"
        "movd %[temp], %[result]\n\t"
        "paddd %[result], %[result]\n\t"
        : [result] "+x" (result),
          [temp] "+r" (temp)
        :
        : "cc"
    );
    
    *out = result;
    return result;
}

/* Main test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    int tmp1, tmp2, tmp3;
    double dtmp;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        int a = in[i];
        int b = in[i + 1];
        int c = in[i + 2];
        double d = din[i];
        
        /* Many scalar variables to create register pressure */
        int v1 = a + 1, v2 = b + 2, v3 = c + 3, v4 = a + b, v5 = b + c;
        int v6 = a * 2, v7 = b * 3, v8 = c * 4, v9 = a + c, v10 = v1 + v2;
        int v11 = v3 * 2, v12 = v4 / 2, v13 = v5 - 1, v14 = v6 + v7, v15 = v8 ^ v9;
        
        /* Complex assembly with many operands */
        complex_asm_5op(v1, v2, v3, &tmp1, &tmp2, &tmp3);
        
        /* Mix with double operations */
        __asm__ volatile (
            "movsd %[d], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[dtmp]\n\t"
            : [dtmp] "=x" (dtmp)
            : [d] "m" (d)
            : "xmm0"
        );
        
        /* Use all variables to prevent optimization */
        out[i] = tmp1 + tmp2 + tmp3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15;
        dout[i] = dtmp;
        
        /* Memory barrier to prevent reload combination */
        if (i % 4 == 0) {
            __asm__ volatile ("" ::: "memory");
        }
    }
}

void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        long long val = in[i];
        float fval = fin[i];
        long long result;
        float fresult;
        
        /* Force secondary reload patterns */
        secondary_reload_asm(val, &result, i);
        
        /* Mixed float operations */
        __asm__ volatile (
            "movss %[fval], %%xmm0\n\t"
            "mulss %%xmm0, %%xmm0\n\t"
            "movss %%xmm0, %[fresult]\n\t"
            : [fresult] "=x" (fresult)
            : [fval] "m" (fval)
            : "xmm0"
        );
        
        /* Assembly with mismatched constraints */
        __asm__ volatile (
            "cvtsi2ss %[val], %%xmm1\n\t"
            "addss %[fresult], %%xmm1\n\t"
            "movss %%xmm1, %[fresult]\n\t"
            : [fresult] "+x" (fresult)
            : [val] "rm" ((int)result)
            : "xmm1", "cc"
        );
        
        out[i] = result;
        fout[i] = fresult;
    }
}

void test_optional_reloads(int iterations, char *in, char *out, __m128i *vin, __m128i *vout) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        char c = in[i];
        __m128i vec = vin[i];
        int opt_result;
        __m128i vresult;
        
        /* Optional reload */
        int result = optional_reload_asm(c, i, &opt_result);
        
        /* Vector-scalar mix */
        vector_scalar_mix(vec, result, &vresult);
        
        /* Mixed width operations */
        unsigned char bytes[4] = {c, (char)(c + 1), (char)(c + 2), (char)(c + 3)};
        unsigned short words[2] = {c * 10, c * 20};
        int dwords[1] = {result};
        
        mixed_width_asm(&bytes[0], &words[0], &dwords[0]);
        
        out[i] = bytes[0] + bytes[1] + bytes[2] + bytes[3];
        vout[i] = vresult;
        
        /* Prevent combination with volatile asm */
        __asm__ volatile ("# prevent combine" ::: "memory");
    }
}

void test_control_flow_reloads(int mode, int iterations, int *data, int *result) {
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int x = data[i];
        int y = data[i + 1];
        int z = data[i + 2];
        
        /* Complex control flow */
        if (mode == 0) {
            /* Path with many asm statements */
            for (j = 0; j < 4; j++) {
                int tmp;
                __asm__ volatile (
                    "movl %[x], %%eax\n\t"
                    "addl %[y], %%eax\n\t"
                    "movl %%eax, %[tmp]\n\t"
                    : [tmp] "=r" (tmp)
                    : [x] "rm" (x),
                      [y] "rm" (y)
                    : "eax", "cc"
                );
                x = tmp + j;
            }
        } else if (mode == 1) {
            /* Different path with different constraints */
            for (j = 0; j < 3; j++) {
                int tmp;
                __asm__ volatile (
                    "movl %[z], %%ebx\n\t"
                    "subl %[x], %%ebx\n\t"
                    "movl %%ebx, %[tmp]\n\t"
                    : [tmp] "=r" (tmp)
                    : [z] "rm" (z),
                      [x] "rm" (x)
                    : "ebx", "cc"
                );
                z = tmp - j;
            }
        } else {
            /* Third path mixing everything */
            int tmp1, tmp2;
            __asm__ volatile (
                "imull %[x], %[y]\n\t"
                "movl %[y], %[tmp1]\n\t"
                "addl %[z], %[tmp1]\n\t"
                : [tmp1] "=r" (tmp1),
                  [y] "+r" (y)
                : [x] "rm" (x),
                  [z] "rm" (z)
                : "cc"
            );
            
            __asm__ volatile (
                "leal (%[tmp1], %[tmp1], 2), %[tmp2]\n\t"
                : [tmp2] "=r" (tmp2)
                : [tmp1] "rm" (tmp1)
                : "cc"
            );
            
            x = tmp2;
        }
        
        result[i] = x + y + z;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations > ARRAY_SIZE) iterations = ARRAY_SIZE;
    
    /* Allocate and initialize arrays */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    int *int_out = malloc(ARRAY_SIZE * sizeof(int));
    long long *ll_data = malloc(ARRAY_SIZE * sizeof(long long));
    long long *ll_out = malloc(ARRAY_SIZE * sizeof(long long));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    double *double_out = malloc(ARRAY_SIZE * sizeof(double));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    float *float_out = malloc(ARRAY_SIZE * sizeof(float));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    char *char_out = malloc(ARRAY_SIZE * sizeof(char));
    __m128i *vec_data = malloc(ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = malloc(ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        ll_data[i] = i * 5LL + 2;
        double_data[i] = i * 1.5;
        float_data[i] = i * 2.5f;
        char_data[i] = (i % 256) - 128;
        vec_data[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_data, int_out, double_data, double_out);
    test_secondary_reloads(iterations, ll_data, ll_out, float_data, float_out);
    test_optional_reloads(iterations, char_data, char_out, vec_data, vec_out);
    test_control_flow_reloads(mode, iterations, int_data, int_out);
    
    /* Compute checksum to ensure all code runs */
    unsigned long long checksum = 0;
    for (int i = 0; i < iterations; i++) {
        checksum += int_out[i];
        checksum += ll_out[i];
        checksum += (unsigned long long)double_out[i];
        checksum += (unsigned long long)float_out[i];
        checksum += char_out[i];
        
        /* Extract from vector */
        int vec_vals[4];
        _mm_storeu_si128((__m128i*)vec_vals, vec_out[i]);
        checksum += vec_vals[0] + vec_vals[1] + vec_vals[2] + vec_vals[3];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(int_out);
    free(ll_data);
    free(ll_out);
    free(double_data);
    free(double_out);
    free(float_data);
    free(float_out);
    free(char_data);
    free(char_out);
    free(vec_data);
    free(vec_out);
    
    return 0;
}
