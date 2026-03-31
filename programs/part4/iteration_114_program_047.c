#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int* in_ints, double* in_doubles, 
                         int* out_ints, double* out_doubles);
void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles);
void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles);

/* Helper to create register pressure */
static inline void create_register_pressure(int* restrict a, int* restrict b, 
                                           double* restrict c, double* restrict d) {
    /* Many live variables to exhaust registers */
    register int r0 asm("r12") = a[0];
    register int r1 asm("r13") = a[1];
    register int r2 asm("r14") = a[2];
    register int r3 asm("r15") = a[3];
    register double f0 asm("xmm8") = c[0];
    register double f1 asm("xmm9") = c[1];
    register double f2 asm("xmm10") = c[2];
    register double f3 asm("xmm11") = c[3];
    
    /* Force them to be used */
    asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3),
                       "+x"(f0), "+x"(f1), "+x"(f2), "+x"(f3));
    
    b[0] = r0; b[1] = r1; b[2] = r2; b[3] = r3;
    d[0] = f0; d[1] = f1; d[2] = f2; d[3] = f3;
}

void test_primary_reloads(int iterations, int* in_ints, double* in_doubles,
                         int* out_ints, double* out_doubles) {
    /* Complex inline assembly with multiple operands and constraints */
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - UNROLL_FACTOR);
        
        /* Unrolled loop with many live variables */
        for (int j = 0; j < UNROLL_FACTOR; j += 4) {
            int a, b, c, d;
            double x, y, z, w;
            long long la, lb;
            
            /* Mixed constraints: memory, immediate, register, earlyclobber */
            asm volatile (
                /* Multiple output operands with different constraints */
                "movl %[in1], %[out1]\n\t"
                "addl %[imm1], %[out1]\n\t"
                "movq %[in2], %[out2]\n\t"
                "imulq $0x%[imm2], %[out2], %[out2]\n\t"
                "movsd %[in3], %[out3]\n\t"
                "addsd %[out3], %[out3]\n\t"
                "movl %[in4], %[out4]\n\t"
                "andl $0xFF, %[out4]"
                : [out1] "=&r" (a),        /* earlyclobber reg */
                  [out2] "=r" (la),        /* general reg */
                  [out3] "=t" (x),         /* top of FP stack */
                  [out4] "=q" (b)          /* byte-addressable reg */
                : [in1] "rm" (in_ints[idx + j]),      /* reg or memory */
                  [in2] "r" ((long long)in_ints[idx + j + 1]),
                  [in3] "xm" (in_doubles[idx + j]),   /* SSE reg or memory */
                  [in4] "i" (0x12345678),             /* immediate */
                  [imm1] "i" (42),
                  [imm2] "i" (0xABCD)
                : "cc", "memory"
            );
            
            /* Another asm with matching constraints to force reloads */
            asm volatile (
                "mov %[in], %[out]\n\t"
                "add $1, %[out]"
                : [out] "=r" (c)
                : [in] "0" (a),    /* matching constraint */
                  "m" (in_ints[idx + j + 2])  /* memory constraint */
                : "cc"
            );
            
            /* Mix vector and scalar operations */
            __m128i v1 = _mm_set_epi32(in_ints[idx + j], in_ints[idx + j + 1],
                                      in_ints[idx + j + 2], in_ints[idx + j + 3]);
            __m128d v2 = _mm_set_pd(in_doubles[idx + j], in_doubles[idx + j + 1]);
            
            /* Vector operation creates more register pressure */
            v1 = _mm_add_epi32(v1, _mm_set1_epi32(1));
            v2 = _mm_add_pd(v2, _mm_set1_pd(1.0));
            
            /* Store results with complex addressing */
            out_ints[idx + j] = a + b + c;
            out_doubles[idx + j] = x + _mm_cvtsd_f64(v2);
            
            /* Memory barrier to prevent combination */
            asm volatile("" ::: "memory");
        }
        
        /* Call helper to create additional register pressure */
        create_register_pressure(&in_ints[idx], &out_ints[idx],
                                &in_doubles[idx], &out_doubles[idx]);
    }
}

void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles) {
    /* Force secondary reloads with mismatched constraints */
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        
        /* Use x86-specific constraints that may require secondary reloads */
        int a, b;
        double x, y;
        
        /* 'a' constraint (accumulator) followed by 'b' constraint (base) */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %%eax, %%eax\n\t"
            "movl %%eax, %[out1]"
            : [out1] "=r" (a)
            : [in1] "rm" (in_ints[idx])
            : "%eax", "cc"
        );
        
        /* Now use result in 'b' register constraint */
        asm volatile (
            "movl %[in1], %%ebx\n\t"
            "addl %[in2], %%ebx\n\t"
            "movl %%ebx, %[out1]"
            : [out1] "=r" (b)
            : [in1] "b" (a),      /* 'b' constraint */
              [in2] "rm" (in_ints[idx + 1])
            : "cc"
        );
        
        /* Legacy register constraint 'R' that might need secondary reload */
        asm volatile (
            "mov %[in], %%rax\n\t"
            "add $1, %%rax"
            : "=R" (out_ints[idx])  /* Legacy register constraint */
            : [in] "r" (b)
            : "%rax", "cc"
        );
        
        /* SSE to x87 transfer that might need secondary reload */
        asm volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=m" (out_doubles[idx])
            : [in] "x" (in_doubles[idx])  /* SSE register constraint */
            : "%xmm0"
        );
        
        /* Mixed size constraints */
        short s1, s2;
        asm volatile (
            "movw %[in1], %[out1]\n\t"
            "movw %[in2], %[out2]"
            : [out1] "=r" (s1),
              [out2] "=r" (s2)
            : [in1] "m" (*(short*)&in_ints[idx]),
              [in2] "m" (*(short*)&in_ints[idx + 1])
        );
        
        out_ints[idx + 2] = s1 + s2;
    }
}

void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles) {
    /* Test optional reloads and nocombine scenarios */
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 4);
        
        /* Optional constraint with '?' modifier */
        int opt1, opt2;
        asm volatile (
            "test %[in], %[in]\n\t"
            "jz 1f\n\t"
            "movl %[in], %[out]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out]\n\t"
            "2:"
            : [out] "=?r" (opt1)    /* optional output */
            : [in] "r" (in_ints[idx])
            : "cc"
        );
        
        /* Another asm that could be combined but won't due to memory clobber */
        asm volatile (
            "movl %[in], %[out]"
            : [out] "=r" (opt2)
            : [in] "r" (in_ints[idx + 1])
        );
        
        /* Memory barrier to prevent combination */
        asm volatile("" ::: "memory");
        
        /* Similar asm with different clobbers to force nocombine */
        asm volatile (
            "movl %[in], %[out]\n\t"
            "addl $1, %[out]"
            : [out] "=r" (out_ints[idx])
            : [in] "r" (opt1)
            : "cc"
        );
        
        asm volatile (
            "movl %[in], %[out]\n\t"
            "subl $1, %[out]"
            : [out] "=r" (out_ints[idx + 1])
            : [in] "r" (opt2)
            : "cc", "memory"  /* Different clobber prevents combination */
        );
        
        /* Control flow dependent asm */
        if (in_ints[idx] > 0) {
            asm volatile (
                "movl %[in], %%eax\n\t"
                "imull %%eax, %%eax"
                : "=a" (out_ints[idx + 2])
                : [in] "r" (in_ints[idx])
                : "cc"
            );
        } else {
            asm volatile (
                "movl %[in], %%ebx\n\t"
                "negl %%ebx"
                : "=b" (out_ints[idx + 2])
                : [in] "r" (in_ints[idx])
                : "cc"
            );
        }
        
        /* Complex floating point with mixed constraints */
        double d1, d2;
        asm volatile (
            "movsd %[in1], %%xmm0\n\t"
            "movsd %[in2], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[out1]"
            : [out1] "=m" (d1)
            : [in1] "xm" (in_doubles[idx]),
              [in2] "xm" (in_doubles[idx + 1])
            : "%xmm0", "%xmm1"
        );
        
        out_doubles[idx] = d1;
    }
}

int main(int argc, char** argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
        if (mode < 1 || mode > 3) mode = 1;
    }
    
    /* Initialize arrays with mixed data */
    int* in_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* out_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* in_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* out_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!in_ints || !out_ints || !in_doubles || !out_doubles) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 1;
        out_ints[i] = 0;
        in_doubles[i] = i * 0.5;
        out_doubles[i] = 0.0;
    }
    
    /* Run tests based on mode */
    switch (mode) {
        case 1:
            test_primary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            break;
        case 2:
            test_secondary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            break;
        case 3:
            test_optional_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            break;
        default:
            test_primary_reloads(iterations, in_ints, in_doubles, out_ints, out_doubles);
            test_secondary_reloads(iterations / 2, in_ints, in_doubles, out_ints, out_doubles);
            test_optional_reloads(iterations / 2, in_ints, in_doubles, out_ints, out_doubles);
            break;
    }
    
    /* Compute checksum to ensure all asm blocks executed */
    long long int_checksum = 0;
    double double_checksum = 0.0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_checksum += out_ints[i];
        double_checksum += out_doubles[i];
    }
    
    printf("Checksum - ints: %lld, doubles: %f\n", int_checksum, double_checksum);
    
    /* Cleanup */
    free(in_ints);
    free(out_ints);
    free(in_doubles);
    free(out_doubles);
    
    return 0;
}
