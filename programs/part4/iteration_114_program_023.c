/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout);
void test_optional_reloads(int iterations, unsigned *uin, unsigned *uout, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int count, int *data, int *result);

/* Main test function with heavy register pressure */
void heavy_computation(int argc, char **argv) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int mode = argc > 2 ? atoi(argv[2]) : 1;
    
    /* Allocate various data types to create different operand requirements */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long *long_data = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_out = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    unsigned *uint_data = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    unsigned *uint_out = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    float *float_data = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    __m128i *vec_data = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        long_data[i] = i * 5L + 2;
        uint_data[i] = i * 7U + 3;
        float_data[i] = i * 1.5f + 4.0f;
        double_data[i] = i * 2.5 + 5.0;
        vec_data[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute tests to trigger different reload patterns */
    test_primary_reloads(iterations, int_data, int_out, double_data, double_out);
    test_secondary_reloads(iterations, long_data, long_out, float_data, float_out);
    test_optional_reloads(iterations, uint_data, uint_out, vec_data, vec_out);
    test_control_flow_reloads(mode, iterations, int_data, int_out);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += long_out[i];
        checksum += uint_out[i];
        checksum += *(unsigned*)&float_out[i];
        checksum += *(unsigned long long*)&double_out[i];
        checksum += _mm_extract_epi32(vec_out[i], 0);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data); free(int_out);
    free(long_data); free(long_out);
    free(uint_data); free(uint_out);
    free(float_data); free(float_out);
    free(double_data); free(double_out);
    free(vec_data); free(vec_out);
}

/* Test 1: Primary reloads with mixed constraints and modes */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    /* Create many live variables to increase register pressure */
    int a0, a1, a2, a3, a4, a5, a6, a7;
    double d0, d1, d2, d3, d4, d5, d6, d7;
    long l0, l1, l2, l3, l4, l5, l6, l7;
    
    /* Unrolled loop with complex asm patterns */
    for (int i = 0; i < iterations; i++) {
        /* Load many values into registers */
        a0 = in[i*8 + 0]; a1 = in[i*8 + 1]; a2 = in[i*8 + 2]; a3 = in[i*8 + 3];
        a4 = in[i*8 + 4]; a5 = in[i*8 + 5]; a6 = in[i*8 + 6]; a7 = in[i*8 + 7];
        d0 = din[i*8 + 0]; d1 = din[i*8 + 1]; d2 = din[i*8 + 2]; d3 = din[i*8 + 3];
        d4 = din[i*8 + 4]; d5 = din[i*8 + 5]; d6 = din[i*8 + 6]; d7 = din[i*8 + 7];
        
        /* Complex asm with 8+ operands, mixed constraints and modes */
        __asm__ volatile (
            /* Multiple outputs with different constraints */
            "mov %[imm], %%eax\n\t"
            "add %[in1], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in2], %[out2]\n\t"
            "lea (%[in3], %[in4], 2), %[out3]\n\t"
            /* Mix register classes: a=accumulator, d=data, r=general, q=byte */
            "mov %[in5], %%al\n\t"
            "add %[in6], %%al\n\t"
            "mov %%al, %[out4]\n\t"
            /* Memory operand with offset */
            "mov %[mem_in], %%ebx\n\t"
            "add $0x1234, %%ebx\n\t"
            "mov %%ebx, %[mem_out]\n\t"
            /* Double precision operations */
            "movsd %[din1], %%xmm0\n\t"
            "addsd %[din2], %%xmm0\n\t"
            "movsd %%xmm0, %[dout1]\n\t"
            /* Early clobber to prevent reuse */
            "mov $0xABCD, %[early1]\n\t"
            "add %[in7], %[early1]\n\t"
            : /* Outputs with diverse constraints */
              [out1] "=r" (a0),           /* general register */
              [out2] "=a" (a1),           /* accumulator */
              [out3] "=d" (a2),           /* data register */
              [out4] "=q" (a3),           /* byte register */
              [mem_out] "=m" (out[i*8]),  /* memory */
              [dout1] "=x" (d0),          /* xmm register */
              [early1] "=&r" (a4)         /* early clobber */
            : /* Inputs with mixed constraints */
              [imm] "i" (0x1000),         /* immediate */
              [in1] "r" (a5),             /* register */
              [in2] "rm" (a6),            /* register or memory */
              [in3] "r" (a7),             /* register */
              [in4] "r" (in[i*8 + 1]),    /* memory reference */
              [in5] "q" ((char)a0),       /* byte register */
              [in6] "q" ((char)a1),       /* byte register */
              [in7] "r" (a2),             /* register */
              [mem_in] "m" (in[i*8 + 2]), /* memory */
              [din1] "x" (d1),            /* xmm register */
              [din2] "xm" (d2)            /* xmm register or memory */
            : /* Clobbers - many registers to force spills */
              "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
        );
        
        /* More asm with matching constraints (forces specific allocation) */
        int tmp1 = a0, tmp2 = a1;
        __asm__ volatile (
            "addl %[inc], %[val]\n\t"
            "movl %[val], %[res]\n\t"
            : [res] "=r" (tmp1), [val] "+0" (tmp2)
            : [inc] "rm" (0x10)
            : "cc"
        );
        
        /* Store results */
        out[i*8 + 0] = a0; out[i*8 + 1] = a1; out[i*8 + 2] = a2;
        out[i*8 + 3] = a3; out[i*8 + 4] = a4;
        dout[i*8 + 0] = d0;
    }
}

/* Test 2: Secondary reload patterns with mismatched constraints */
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout) {
    /* Variables that will need secondary reloads */
    register long r0 asm("rax"), r1 asm("rbx"), r2 asm("rcx");
    register float f0 asm("xmm0"), f1 asm("xmm1"), f2 asm("xmm2");
    
    for (int i = 0; i < iterations; i++) {
        /* Force memory operands for "rm" constraints */
        long mem_long = lin[i];
        float mem_float = fin[i];
        
        /* Asm requiring specific register classes that may need secondary reloads */
        __asm__ volatile (
            /* "a" constraint that may need secondary reload if allocated to R8-R15 */
            "mov %[in_a], %%rax\n\t"
            "add $1, %%rax\n\t"
            "mov %%rax, %[out_a]\n\t"
            /* "b" constraint that conflicts with above */
            "mov %[in_b], %%rbx\n\t"
            "sub %%rax, %%rbx\n\t"
            "mov %%rbx, %[out_b]\n\t"
            /* XMM register with memory operand needing secondary reload */
            "movss %[in_xmm], %%xmm0\n\t"
            "addss %[in_imm], %%xmm0\n\t"
            "movss %%xmm0, %[out_xmm]\n\t"
            : [out_a] "=a" (r0),      /* accumulator output */
              [out_b] "=b" (r1),      /* base register output */
              [out_xmm] "=x" (f0)     /* xmm register output */
            : [in_a] "rm" (mem_long), /* may be in memory -> secondary reload to rax */
              [in_b] "r" (r2),        /* general register */
              [in_xmm] "xm" (mem_float), /* may need secondary reload to xmm0 */
              [in_imm] "x" (1.0f)     /* xmm immediate */
            : "cc"
        );
        
        /* Another asm that uses result in different constraint */
        __asm__ volatile (
            "mov %[prev_a], %%rcx\n\t"
            "imul $3, %%rcx\n\t"
            "mov %%rcx, %[result]\n\t"
            : [result] "=c" (r2)      /* specific constraint */
            : [prev_a] "a" (r0)       /* from accumulator - may need move */
            : "cc"
        );
        
        lout[i] = r0 + r1 + r2;
        fout[i] = f0;
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Test 3: Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, unsigned *uin, unsigned *uout, __m128i *vin, __m128i *vout) {
    for (int i = 0; i < iterations; i++) {
        unsigned opt1, opt2, opt3;
        __m128i vtmp;
        
        /* Asm with optional constraints */
        __asm__ volatile (
            "mov %[in1], %%eax\n\t"
            "test %%eax, %%eax\n\t"
            "jz 1f\n\t"
            "add $100, %%eax\n\t"
            "1:\n\t"
            "mov %%eax, %[out1]\n\t"
            /* Optional output - compiler may omit */
            "mov %[in2], %%ebx\n\t"
            "cmovc %%ebx, %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            : [out1] "=r" (opt1),
              [out2] "=?r" (opt2)  /* optional constraint */
            : [in1] "rm" (uin[i]),
              [in2] "r" (uin[i+1])
            : "rax", "rbx", "cc"
        );
        
        /* Similar asm but with different clobbers to prevent combination */
        __asm__ volatile (
            "movdqu %[vec_in], %%xmm0\n\t"
            "paddd %[vec_const], %%xmm0\n\t"
            "movdqu %%xmm0, %[vec_out]\n\t"
            : [vec_out] "=x" (vtmp)
            : [vec_in] "xm" (vin[i]),
              [vec_const] "x" (_mm_set1_epi32(1))
            : "xmm0"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Another similar asm that won't combine due to barrier */
        __asm__ volatile (
            "movdqu %[vec_in], %%xmm1\n\t"
            "psubd %[vec_const], %%xmm1\n\t"
            "movdqu %%xmm1, %[vec_out]\n\t"
            : [vec_out] "=x" (vout[i])
            : [vec_in] "xm" (vtmp),
              [vec_const] "x" (_mm_set1_epi32(2))
            : "xmm1"
        );
        
        uout[i] = opt1 + opt2;
    }
}

/* Test 4: Control flow dependent reloads */
void test_control_flow_reloads(int mode, int count, int *data, int *result) {
    int a = 0, b = 0, c = 0, d = 0;
    
    /* Complex control flow with asm in different paths */
    for (int i = 0; i < count; i++) {
        if (mode & 1) {
            /* Path 1: Specific register constraints */
            __asm__ volatile (
                "mov %[val], %%eax\n\t"
                "rol $4, %%eax\n\t"
                "mov %%eax, %[res]\n\t"
                : [res] "=a" (a)
                : [val] "rm" (data[i])
                : "cc"
            );
            result[i] = a;
        } else if (mode & 2) {
            /* Path 2: Different constraints */
            __asm__ volatile (
                "mov %[val], %%ebx\n\t"
                "ror $8, %%ebx\n\t"
                "mov %%ebx, %[res]\n\t"
                : [res] "=b" (b)
                : [val] "rm" (data[i+1])
                : "cc"
            );
            result[i] = b;
        } else {
            /* Path 3: Yet another constraint set */
            __asm__ volatile (
                "mov %[val], %%ecx\n\t"
                "shl $16, %%ecx\n\t"
                "mov %%ecx, %[res]\n\t"
                : [res] "=c" (c)
                : [val] "rm" (data[i+2])
                : "cc"
            );
            result[i] = c;
        }
        
        /* Loop with asm that depends on previous results */
        for (int j = 0; j < 4; j++) {
            __asm__ volatile (
                "add %[inc], %[acc]\n\t"
                "mov %[acc], %[out]\n\t"
                : [out] "=r" (d), [acc] "+r" (a)
                : [inc] "i" (j)
                : "cc"
            );
            result[i] += d;
        }
    }
}

int main(int argc, char **argv) {
    printf("Starting reload stress test...\n");
    
    /* Execute the heavy computation */
    heavy_computation(argc, argv);
    
    printf("Test completed.\n");
    return 0;
}
