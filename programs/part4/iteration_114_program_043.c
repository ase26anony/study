#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
static void test_primary_reloads(int iterations, int *in_ints, double *in_doubles, 
                                 int *out_ints, double *out_doubles);
static void test_secondary_reloads(int iterations, int *in_ints, double *in_doubles,
                                   int *out_ints, double *out_doubles);
static void test_optional_reloads(int iterations, int *in_ints, double *in_doubles,
                                  int *out_ints, double *out_doubles);

/* Helper to create register pressure */
static inline void create_register_pressure(int a, int b, int c, int d, int e,
                                            int f, int g, int h, int i, int j) {
    /* Use all parameters to prevent optimization */
    __asm__ volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e),
                        "+r"(f), "+r"(g), "+r"(h), "+r"(i), "+r"(j));
}

/* Complex inline assembly with multiple operands and constraints */
static void complex_asm_operation(int in1, int in2, double in3, double in4,
                                  int *out1, int *out2, double *out3, double *out4) {
    int tmp1, tmp2;
    double tmp3, tmp4;
    
    /* Extended asm with 8 operands, mixed constraints */
    __asm__ volatile (
        "movl %[i1], %[t1]\n\t"
        "addl %[i2], %[t1]\n\t"
        "movl %[t1], %[o1]\n\t"
        "movsd %[d1], %[t3]\n\t"
        "addsd %[d2], %[t3]\n\t"
        "movsd %[t3], %[o3]\n\t"
        "imull $0x1234, %[t1], %[t2]\n\t"
        "movl %[t2], %[o2]\n\t"
        "mulsd %[t3], %[t3]\n\t"
        "movsd %[t3], %[o4]"
        : [t1] "=&r" (tmp1), [t2] "=&r" (tmp2),  /* Early clobber */
          [t3] "=&t" (tmp3), [t3] "=&x" (tmp4),   /* Different reg classes */
          [o1] "=m" (*out1), [o2] "=m" (*out2),
          [o3] "=m" (*out3), [o4] "=m" (*out4)
        : [i1] "irm" (in1),      /* Mixed: immediate, register, or memory */
          [i2] "r" (in2),
          [d1] "xm" (in3),       /* SSE register or memory */
          [d2] "xm" (in4),
          "0" (tmp1),            /* Matching constraint */
          "2" (tmp3)             /* Another matching constraint */
        : "memory", "cc", "rax", "rdx", "xmm0", "xmm1"
    );
}

/* Force secondary reloads with mismatched constraints */
static void secondary_reload_asm(int *in, double *dbl_in, int *out, double *dbl_out) {
    int tmp_int;
    double tmp_dbl;
    
    /* First asm: result in accumulator */
    __asm__ volatile (
        "movl (%[in]), %%eax\n\t"
        "addl $1, %%eax"
        : "=a" (tmp_int)
        : [in] "r" (in)
        : "cc"
    );
    
    /* Second asm: requires base register, forcing move from accumulator */
    __asm__ volatile (
        "movl %%ebx, (%[out])\n\t"
        "movsd (%[din]), %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0"
        : [out] "=m" (*out)
        : "b" (tmp_int),        /* ebx constraint - different from eax */
          [din] "r" (dbl_in)
        : "memory", "xmm0"
    );
    
    /* Memory operand that might need secondary reload */
    __asm__ volatile (
        "movq %[val], %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=m" (*dbl_out)
        : [val] "Rm" (*dbl_in)   /* Legacy register or memory */
        : "rax"
    );
}

/* Test function for primary reloads */
static void test_primary_reloads(int iterations, int *in_ints, double *in_doubles,
                                 int *out_ints, double *out_doubles) {
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Create many live variables to pressure registers */
        int v1 = in_ints[i * 4];
        int v2 = in_ints[i * 4 + 1];
        int v3 = in_ints[i * 4 + 2];
        int v4 = in_ints[i * 4 + 3];
        double d1 = in_doubles[i * 4];
        double d2 = in_doubles[i * 4 + 1];
        double d3 = in_doubles[i * 4 + 2];
        double d4 = in_doubles[i * 4 + 3];
        
        /* Unrolled loop with many asm statements */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            /* Complex asm with many operands */
            complex_asm_operation(v1 + j, v2 + j, d1 + j, d2 + j,
                                 &out_ints[i * 8 + j * 2],
                                 &out_ints[i * 8 + j * 2 + 1],
                                 &out_doubles[i * 8 + j * 2],
                                 &out_doubles[i * 8 + j * 2 + 1]);
            
            /* Create register pressure between asm blocks */
            create_register_pressure(v1, v2, v3, v4, j,
                                     (int)d1, (int)d2, (int)d3, (int)d4, i);
        }
        
        /* Conditional asm to force control-flow dependent reloads */
        if (v1 > v2) {
            __asm__ volatile (
                "cmpl %[a], %[b]\n\t"
                "setg %[c]"
                : [c] "=q" (out_ints[i])  /* Byte register constraint */
                : [a] "r" (v1), [b] "r" (v2)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "xchgl %[a], %[b]"
                : [a] "+r" (v3), [b] "+r" (v4)
                :
                : "cc"
            );
            out_ints[i] = v3 + v4;
        }
    }
}

/* Test function for secondary reloads */
static void test_secondary_reloads(int iterations, int *in_ints, double *in_doubles,
                                   int *out_ints, double *out_doubles) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Use vector intrinsics alongside scalar operations */
        __m128i vec1 = _mm_set_epi32(in_ints[i*4+3], in_ints[i*4+2],
                                     in_ints[i*4+1], in_ints[i*4]);
        __m128i vec2 = _mm_set1_epi32(i);
        __m128i vec_result = _mm_add_epi32(vec1, vec2);
        
        /* Extract to scalars, creating register pressure */
        int extracted[4];
        _mm_storeu_si128((__m128i*)extracted, vec_result);
        
        /* Force secondary reloads */
        secondary_reload_asm(&extracted[0], &in_doubles[i],
                            &out_ints[i*2], &out_doubles[i*2]);
        
        /* Another asm with specific register constraints */
        __asm__ volatile (
            "mov %[in], %%r8\n\t"    /* Force use of R8-R15 */
            "mov %%r8, %[out]"
            : [out] "=m" (out_ints[i*2+1])
            : [in] "R" (extracted[1])  /* Legacy register constraint */
            : "r8"
        );
        
        /* Mix SSE and x87 floating point */
        double x87_result;
        __asm__ volatile (
            "fldl %[in]\n\t"
            "fadd %%st(0), %%st(0)\n\t"
            "fstpl %[out]"
            : [out] "=m" (x87_result)
            : [in] "m" (in_doubles[i])
            : "st", "st(1)"
        );
        out_doubles[i*2+1] = x87_result;
    }
}

/* Test function for optional reloads */
static void test_optional_reloads(int iterations, int *in_ints, double *in_doubles,
                                  int *out_ints, double *out_doubles) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int opt_result;
        
        /* Asm with optional output */
        __asm__ volatile (
            "testl %[val], %[val]\n\t"
            "jz 1f\n\t"
            "movl $1, %[out]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out]\n\t"
            "2:"
            : [out] "=?r" (opt_result)  /* Optional constraint */
            : [val] "r" (in_ints[i])
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "addl $100, %[out]"
            : [out] "+r" (opt_result)
            :
            : "cc"
        );
        
        out_ints[i] = opt_result;
        
        /* Another optional pattern with different clobbers */
        double dbl_opt;
        __asm__ volatile (
            "movsd %[in], %%xmm0\n\t"
            "sqrtsd %%xmm0, %%xmm0"
            : "=x" (dbl_opt)
            : [in] "xm" (in_doubles[i])
            : "xmm0"
        );
        
        /* Volatile asm prevents combination */
        __asm__ volatile (
            "movsd %[in], %%xmm1\n\t"
            "addsd %%xmm0, %%xmm1"
            : "=x" (out_doubles[i])
            : [in] "x" (dbl_opt), "0" (dbl_opt)
            : "xmm1"
        );
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays */
    int *in_ints = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *in_doubles = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *out_ints1 = (int*)calloc(ARRAY_SIZE * 2, sizeof(int));
    double *out_doubles1 = (double*)calloc(ARRAY_SIZE * 2, sizeof(double));
    int *out_ints2 = (int*)calloc(ARRAY_SIZE * 2, sizeof(int));
    double *out_doubles2 = (double*)calloc(ARRAY_SIZE * 2, sizeof(double));
    int *out_ints3 = (int*)calloc(ARRAY_SIZE, sizeof(int));
    double *out_doubles3 = (double*)calloc(ARRAY_SIZE, sizeof(double));
    
    if (!in_ints || !in_doubles || !out_ints1 || !out_doubles1 ||
        !out_ints2 || !out_doubles2 || !out_ints3 || !out_doubles3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mixed data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 1;
        in_doubles[i] = i * 0.5 + 1.0;
    }
    
    /* Run tests based on mode */
    if (mode & 1) {
        test_primary_reloads(iterations, in_ints, in_doubles, 
                            out_ints1, out_doubles1);
    }
    
    if (mode & 2) {
        test_secondary_reloads(iterations, in_ints, in_doubles,
                              out_ints2, out_doubles2);
    }
    
    if (mode & 4) {
        test_optional_reloads(iterations, in_ints, in_doubles,
                             out_ints3, out_doubles3);
    }
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE * 2; i++) {
        checksum += out_ints1[i];
        checksum += *(unsigned long long*)&out_doubles1[i];
        if (i < ARRAY_SIZE * 2) {
            checksum += out_ints2[i];
            checksum += *(unsigned long long*)&out_doubles2[i];
        }
        if (i < ARRAY_SIZE) {
            checksum += out_ints3[i];
            checksum += *(unsigned long long*)&out_doubles3[i];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(in_ints);
    free(in_doubles);
    free(out_ints1);
    free(out_doubles1);
    free(out_ints2);
    free(out_doubles2);
    free(out_ints3);
    free(out_doubles3);
    
    return 0;
}
