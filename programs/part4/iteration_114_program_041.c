#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
static void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
static void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout);
static void test_optional_reloads(int iterations, unsigned *in, unsigned *out, __m128i *vin, __m128i *vout);
static void test_control_flow_reloads(int mode, int *data, int *result);

/* Helper to create register pressure */
static inline void create_register_pressure(int a, int b, int c, int d, int e, 
                                           int f, int g, int h, int i, int j) {
    /* Force these to stay in registers */
    __asm__ volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e),
                        "+r"(f), "+r"(g), "+r"(h), "+r"(i), "+r"(j));
}

void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    volatile int barrier = 0;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        int r0 = in[i * UNROLL_FACTOR + 0];
        int r1 = in[i * UNROLL_FACTOR + 1];
        int r2 = in[i * UNROLL_FACTOR + 2];
        int r3 = in[i * UNROLL_FACTOR + 3];
        int r4 = in[i * UNROLL_FACTOR + 4];
        int r5 = in[i * UNROLL_FACTOR + 5];
        int r6 = in[i * UNROLL_FACTOR + 6];
        int r7 = in[i * UNROLL_FACTOR + 7];
        int r8 = in[i * UNROLL_FACTOR + 8];
        int r9 = in[i * UNROLL_FACTOR + 9];
        int r10 = in[i * UNROLL_FACTOR + 10];
        int r11 = in[i * UNROLL_FACTOR + 11];
        int r12 = in[i * UNROLL_FACTOR + 12];
        int r13 = in[i * UNROLL_FACTOR + 13];
        int r14 = in[i * UNROLL_FACTOR + 14];
        int r15 = in[i * UNROLL_FACTOR + 15];
        
        double d0 = din[i * 2];
        double d1 = din[i * 2 + 1];
        
        /* Complex inline asm with multiple constraints */
        __asm__ volatile (
            /* Multiple outputs with different constraints */
            "movl %[in0], %%eax\n\t"
            "addl %[in1], %%eax\n\t"
            "imull %[in2], %%eax\n\t"
            "movl %%eax, %[out0]\n\t"
            
            /* Byte register constraint */
            "movb %b[in3], %%cl\n\t"
            "addb %b[in4], %%cl\n\t"
            "movb %%cl, %b[out1]\n\t"
            
            /* Memory operand with immediate */
            "addl $0x1234, %[out2]\n\t"
            
            /* Early clobber with matching constraint */
            "movl %[in5], %[out3]\n\t"
            "addl $1, %[out3]\n\t"
            
            /* Top of stack constraint */
            "fldl %[din0]\n\t"
            "fldl %[din1]\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpl %[dout0]\n\t"
            
            : [out0] "=r" (out[i * UNROLL_FACTOR + 0]),
              [out1] "=q" (out[i * UNROLL_FACTOR + 1]),  /* byte register */
              [out2] "+m" (out[i * UNROLL_FACTOR + 2]),
              [out3] "=&r" (out[i * UNROLL_FACTOR + 3]), /* early clobber */
              [dout0] "=m" (dout[i])
            : [in0] "r" (r0),
              [in1] "r" (r1),
              [in2] "r" (r2),
              [in3] "r" (r3),
              [in4] "r" (r4),
              [in5] "0" (r5),  /* matching constraint */
              [din0] "m" (d0),
              [din1] "m" (d1)
            : "eax", "ecx", "edx", "st", "st(1)", "memory", "cc"
        );
        
        /* More asm with different mode constraints */
        __asm__ volatile (
            "mov %[in6], %%r8d\n\t"
            "lea (%%r8d, %%r8d, 2), %%r9d\n\t"
            "mov %%r9d, %[out4]\n\t"
            "movzwl %w[in7], %%r10d\n\t"  /* word mode */
            "mov %%r10d, %[out5]\n\t"
            : [out4] "=r" (out[i * UNROLL_FACTOR + 4]),
              [out5] "=r" (out[i * UNROLL_FACTOR + 5])
            : [in6] "r" (r6),
              [in7] "r" (r7)
            : "r8", "r9", "r10", "cc"
        );
        
        /* Create register pressure to force spills */
        create_register_pressure(r8, r9, r10, r11, r12, r13, r14, r15, 
                                out[i * UNROLL_FACTOR + 0], out[i * UNROLL_FACTOR + 1]);
        
        /* Memory barrier to prevent combination */
        if (barrier) {
            __asm__ volatile("" ::: "memory");
        }
    }
}

void test_secondary_reloads(int iterations, long long *in, long long *out, float *fin, float *fout) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        long long ll_in = in[i];
        float f_in = fin[i];
        
        /* Force secondary reload by using 'a' constraint with memory operand */
        __asm__ volatile (
            /* 'a' constraint forces accumulator, may need secondary reload if in memory */
            "movl %[fin], %%eax\n\t"
            "cvtsi2ss %%eax, %%xmm0\n\t"
            "addss %[fconst], %%xmm0\n\t"
            "movss %%xmm0, %[fout]\n\t"
            
            /* Mix register classes - may need secondary move */
            "mov %[llin], %%rax\n\t"
            "mov %%rax, %%rbx\n\t"  /* 'b' constraint would force base register */
            "add $0x1000, %%rbx\n\t"
            "mov %%rbx, %[llout]\n\t"
            : [fout] "=m" (fout[i]),
              [llout] "=r" (out[i])
            : [fin] "a" ((int)f_in),  /* 'a' constraint - may need secondary reload */
              [llin] "rm" (ll_in),    /* 'rm' - register or memory */
              [fconst] "X" (1.0f)
            : "rax", "rbx", "xmm0", "memory", "cc"
        );
        
        /* Another asm with legacy register constraint */
        __asm__ volatile (
            "mov %[val], %%edi\n\t"
            "rorl $8, %%edi\n\t"
            "mov %%edi, %[result]\n\t"
            : [result] "=R" (out[i + iterations])  /* Legacy register constraint */
            : [val] "r" ((int)in[i])
            : "edi", "cc"
        );
    }
}

void test_optional_reloads(int iterations, unsigned *in, unsigned *out, __m128i *vin, __m128i *vout) {
    int i;
    volatile int prevent_combine = 1;
    
    for (i = 0; i < iterations; i++) {
        unsigned u1 = in[i * 2];
        unsigned u2 = in[i * 2 + 1];
        __m128i v = vin[i];
        
        /* Optional output constraint */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "testl %%eax, %%eax\n\t"
            "jz 1f\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out1]\n\t"
            "2:\n\t"
            : [out1] "=?r" (out[i * 2])  /* Optional output */
            : [in1] "r" (u1)
            : "eax", "cc"
        );
        
        /* Memory barrier between similar asm statements */
        if (prevent_combine) {
            __asm__ volatile("" ::: "memory");
        }
        
        /* Similar asm but with different clobbers to prevent combination */
        __asm__ volatile (
            "movl %[in2], %%ebx\n\t"
            "imull $3, %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out2] "=r" (out[i * 2 + 1])
            : [in2] "r" (u2)
            : "ebx", "cc"
        );
        
        /* Vector operations to increase register pressure */
        __m128i v2 = _mm_add_epi32(v, _mm_set1_epi32(1));
        __m128i v3 = _mm_mullo_epi32(v2, _mm_set1_epi32(3));
        vout[i] = _mm_xor_si128(v3, _mm_set1_epi32(0xFF));
    }
}

void test_control_flow_reloads(int mode, int *data, int *result) {
    int i, temp;
    
    /* Complex control flow with asm inside conditionals */
    for (i = 0; i < ARRAY_SIZE / 4; i++) {
        if (mode & 0x1) {
            /* Path 1: Use accumulator constraint */
            __asm__ volatile (
                "movl %[data], %%eax\n\t"
                "andl $0xF0F0F0F0, %%eax\n\t"
                "movl %%eax, %[res]\n\t"
                : [res] "=r" (temp)
                : [data] "a" (data[i])  /* 'a' constraint */
                : "cc"
            );
            result[i] = temp;
        } else if (mode & 0x2) {
            /* Path 2: Use different register constraints */
            __asm__ volatile (
                "movl %[data], %%ecx\n\t"
                "orl $0x0F0F0F0F, %%ecx\n\t"
                "movl %%ecx, %[res]\n\t"
                : [res] "=r" (temp)
                : [data] "c" (data[i])  /* 'c' constraint */
                : "cc"
            );
            result[i] = temp;
        } else {
            /* Path 3: Memory operand with immediate */
            __asm__ volatile (
                "xorl %%eax, %%eax\n\t"
                "addl $42, %[res]\n\t"
                : [res] "+m" (data[i])
                :
                : "eax", "cc"
            );
            result[i] = data[i];
        }
        
        /* Loop-dependent branching */
        if (data[i] & 0x80000000) {
            __asm__ volatile (
                "movl %[val], %%edx\n\t"
                "shrl $16, %%edx\n\t"
                "movl %%edx, %[out]\n\t"
                : [out] "=r" (result[i + ARRAY_SIZE/4])
                : [val] "r" (data[i])
                : "edx", "cc"
            );
        }
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > ARRAY_SIZE / UNROLL_FACTOR) 
        iterations = ARRAY_SIZE / UNROLL_FACTOR;
    
    /* Allocate and initialize arrays */
    int *int_in = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long long *ll_in = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    long long *ll_out = (long long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long long));
    unsigned *uint_in = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    unsigned *uint_out = (unsigned*)aligned_alloc(64, ARRAY_SIZE * sizeof(unsigned));
    float *float_in = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_in = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    __m128i *vec_in = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    int *control_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *control_result = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3 + 1;
        ll_in[i] = i * 5LL + 2;
        uint_in[i] = i * 7 + 3;
        float_in[i] = i * 1.1f;
        double_in[i] = i * 2.2;
        control_data[i] = i * 11 + (i % 3);
        vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations, ll_in, ll_out, float_in, float_out);
    test_optional_reloads(iterations, uint_in, uint_out, vec_in, vec_out);
    test_control_flow_reloads(mode, control_data, control_result);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += ll_out[i] & 0xFFFFFFFF;
        checksum += uint_out[i];
        checksum += (unsigned)float_out[i];
        checksum += (unsigned)double_out[i];
        checksum += control_result[i];
        
        /* Sum vector elements */
        __m128i v = vec_out[i];
        int vsum = _mm_extract_epi32(v, 0) + _mm_extract_epi32(v, 1) +
                   _mm_extract_epi32(v, 2) + _mm_extract_epi32(v, 3);
        checksum += vsum;
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_in);
    free(int_out);
    free(ll_in);
    free(ll_out);
    free(uint_in);
    free(uint_out);
    free(float_in);
    free(float_out);
    free(double_in);
    free(double_out);
    free(vec_in);
    free(vec_out);
    free(control_data);
    free(control_result);
    
    return 0;
}
