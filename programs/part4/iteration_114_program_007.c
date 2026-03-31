/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test functions */
void test_primary_reloads(int *in, int *out, int iterations);
void test_secondary_reloads(double *in, double *out, int mode);
void test_optional_reloads(long *in, long *out, int optional_flag);
void test_control_flow_reloads(float *data, int size, int threshold);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;

/* Main test function with heavy register pressure */
void test_primary_reloads(int *in, int *out, int iterations) {
    /* Many live variables to create register pressure */
    register int r0 asm("eax") = in[0];
    register int r1 asm("ebx") = in[1];
    register int r2 asm("ecx") = in[2];
    register int r3 asm("edx") = in[3];
    register int r4 asm("esi") = in[4];
    register int r5 asm("edi") = in[5];
    int t0, t1, t2, t3, t4, t5, t6, t7;
    
    /* Unrolled loop with complex asm patterns */
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 7 operands, mixed constraints */
        __asm__ volatile (
            "movl %[imm], %%eax\n\t"
            "addl %%eax, %[out1]\n\t"
            "imull %[in1], %[out2]\n\t"
            "orl %[in2], %[out3]\n\t"
            "shrl $3, %[out4]\n\t"
            "leal (%[in3],%[in4],2), %[out5]"
            : [out1] "+&r" (r0),        /* earlyclobber */
              [out2] "=r" (t0),
              [out3] "=q" (t1),         /* byte register constraint */
              [out4] "=r" (t2),
              [out5] "=r" (t3)
            : [imm] "i" (0x1234),       /* immediate */
              [in1] "rm" (r1),          /* register or memory */
              [in2] "r" (r2),
              [in3] "r" (r3),
              [in4] "r" (r4)
            : "eax", "memory", "cc"
        );
        
        /* Another asm with matching constraints */
        __asm__ volatile (
            "mov %[in], %%rax\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=r" (t4)
            : [in] "0" (t0),            /* matching constraint */
              "b" (r5)
            : "rax", "rbx"
        );
        
        /* Use vector intrinsics alongside scalar to increase pressure */
        __m128i v0 = _mm_set_epi32(r0, r1, r2, r3);
        __m128i v1 = _mm_set_epi32(r4, r5, t0, t1);
        __m128i v2 = _mm_add_epi32(v0, v1);
        
        /* Extract results back to scalar */
        t5 = _mm_extract_epi32(v2, 0);
        t6 = _mm_extract_epi32(v2, 1);
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* More asm with different constraints */
        __asm__ volatile (
            "testl %[val], %[val]\n\t"
            "setnz %[out]"
            : [out] "=q" (t7)           /* byte register output */
            : [val] "r" (t5)
            : "cc"
        );
        
        /* Rotate registers to keep them live */
        int tmp = r0;
        r0 = r1; r1 = r2; r2 = r3; r3 = r4; r4 = r5; r5 = tmp;
        
        /* Store results with complex addressing */
        out[i % ARRAY_SIZE] = t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
    }
    
    /* Final asm with accumulator constraint */
    __asm__ volatile (
        "addl %%eax, %[acc]"
        : [acc] "+m" (global_accumulator)
        : "a" (r0)
        : "cc"
    );
}

/* Force secondary reloads through constraint mismatches */
void test_secondary_reloads(double *in, double *out, int mode) {
    double d0, d1, d2, d3, d4, d5;
    
    /* asm requiring specific register classes */
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Force use of x87 stack with 't' constraint */
        __asm__ volatile (
            "fldl %[in1]\n\t"
            "fldl %[in2]\n\t"
            "faddp\n\t"
            "fstpl %[out]"
            : [out] "=m" (d0)
            : [in1] "m" (in[i*2]),
              [in2] "m" (in[i*2 + 1])
            : "st", "st(1)"
        );
        
        /* Mix x87 and SSE constraints */
        __asm__ volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %[in2], %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=x" (d1)           /* SSE register constraint */
            : [in] "x" (d0),
              [in2] "m" (in[i])
            : "xmm0"
        );
        
        /* Legacy register constraint that may need secondary reload */
        __asm__ volatile (
            "movq %[in], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[out]"
            : [out] "=R" (d2)           /* Legacy register constraint */
            : [in] "m" (d1)
            : "rax"
        );
        
        /* Complex pattern with multiple constraints */
        __asm__ volatile (
            "mov %[in1], %%rax\n\t"
            "mov %[in2], %%rbx\n\t"
            "add %%rbx, %%rax\n\t"
            "mov %%rax, %[out1]\n\t"
            "imul %%rbx, %%rax\n\t"
            "mov %%rax, %[out2]"
            : [out1] "=&a" (d3),        /* earlyclobber + accumulator */
              [out2] "=r" (d4)
            : [in1] "r" ((long)d1),
              [in2] "r" ((long)d2)
            : "rbx", "cc"
        );
        
        out[i] = d0 + d1 + d2 + d3 + d4;
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(long *in, long *out, int optional_flag) {
    long l0, l1, l2, l3;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Optional output constraint */
        __asm__ volatile (
            "movq %[in], %%rax\n\t"
            "testq %%rax, %%rax\n\t"
            "jz 1f\n\t"
            "addq $100, %%rax\n\t"
            "1:\n\t"
            "movq %%rax, %[out]"
            : [out] "=?r" (l0)          /* optional constraint */
            : [in] "m" (in[i])
            : "rax", "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't combine due to different clobbers */
        __asm__ volatile (
            "movq %[in], %%rax\n\t"
            "subq $50, %%rax\n\t"
            "movq %%rax, %[out]"
            : [out] "=r" (l1)
            : [in] "m" (in[i])
            : "rax"                     /* Different clobber than above */
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Third similar asm with volatile */
        __asm__ volatile (
            "movq %[in], %%rcx\n\t"     /* Different register */
            "xorq $0xFF, %%rcx\n\t"
            "movq %%rcx, %[out]"
            : [out] "=r" (l2)
            : [in] "m" (in[i])
            : "rcx", "cc"
        );
        
        /* Complex asm that uses optional_flag to affect constraints */
        if (optional_flag) {
            __asm__ volatile (
                "movq %[in1], %%rax\n\t"
                "cmovz %[in2], %%rax\n\t"
                "movq %%rax, %[out]"
                : [out] "=r" (l3)
                : [in1] "r" (l0),
                  [in2] "rm" (l1),      /* May need secondary reload */
                  "z" (global_counter)
                : "rax", "cc"
            );
        } else {
            __asm__ volatile (
                "movq %[in], %%rax\n\t"
                "movq %%rax, %[out]"
                : [out] "=r" (l3)
                : [in] "rm" (l2)
                : "rax"
            );
        }
        
        out[i] = l0 + l1 + l2 + l3;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(float *data, int size, int threshold) {
    float f0, f1, f2, f3;
    int i = 0;
    
    while (i < size) {
        /* Live variables across control flow */
        register float r0 asm("xmm0") = data[i];
        register float r1 asm("xmm1") = data[i+1];
        register float r2 asm("xmm2") = data[i+2];
        
        if (global_counter > threshold) {
            /* Branch with specific asm constraints */
            __asm__ volatile (
                "addss %[in2], %[in1]\n\t"
                "mulss %[in3], %[in1]\n\t"
                "movss %[in1], %[out]"
                : [out] "=x" (f0)
                : [in1] "0" (r0),       /* matching constraint */
                  [in2] "x" (r1),
                  [in3] "xm" (r2)       /* may need secondary reload */
                : "xmm0"
            );
        } else {
            /* Different branch, different constraints */
            __asm__ volatile (
                "subss %[in2], %[in1]\n\t"
                "divss %[in3], %[in1]\n\t"
                "movss %[in1], %[out]"
                : [out] "=t" (f0)       /* x87 top of stack */
                : [in1] "0" (r0),
                  [in2] "t" (r1),
                  [in3] "m" (r2)
                : "st(1)"
            );
        }
        
        /* Loop with varying asm patterns */
        for (int j = 0; j < 4; j++) {
            float temp = f0 + j;
            
            __asm__ volatile (
                "movss %[in], %%xmm0\n\t"
                "cvtss2sd %%xmm0, %%xmm0\n\t"
                "mulsd %[scale], %%xmm0\n\t"
                "cvtsd2ss %%xmm0, %%xmm0\n\t"
                "movss %%xmm0, %[out]"
                : [out] "=x" (f1)
                : [in] "x" (temp),
                  [scale] "m" (2.5)
                : "xmm0"
            );
            
            /* Nested conditional */
            if (j % 2 == 0) {
                __asm__ volatile (
                    "sqrtss %[in], %[out]"
                    : [out] "=x" (f2)
                    : [in] "x" (f1)
                );
            } else {
                __asm__ volatile (
                    "rcpss %[in], %[out]"
                    : [out] "=x" (f2)
                    : [in] "x" (f1)
                );
            }
            
            f0 = f2;
            data[i] = f0;
        }
        
        i += 4;
        global_counter++;
    }
}

int main(int argc, char **argv) {
    /* Parse command line */
    int iterations = 100;
    int mode = 1;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    printf("Running reload tests: iterations=%d, mode=%d\n", iterations, mode);
    
    /* Initialize test data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long *long_data = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5;
        long_data[i] = i * 7L;
        float_data[i] = i * 2.0f;
    }
    
    /* Output arrays */
    int *int_out = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_out = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long *long_out = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    /* Run tests */
    test_primary_reloads(int_data, int_out, iterations);
    test_secondary_reloads(double_data, double_out, mode);
    test_optional_reloads(long_data, long_out, mode & 1);
    test_control_flow_reloads(float_data, ARRAY_SIZE, iterations / 2);
    
    /* Compute checksum */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += (long)double_out[i];
        checksum += long_out[i];
        checksum += (long)float_data[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %ld\n", global_accumulator);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(long_data);
    free(float_data);
    free(int_out);
    free(double_out);
    free(long_out);
    
    return 0;
}
