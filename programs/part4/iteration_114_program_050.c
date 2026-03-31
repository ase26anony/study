/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout);
void test_optional_reloads(int iterations, unsigned char *cin, unsigned char *cout, 
                          __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int count, int *data, int *result);

/* Main test function with heavy register pressure */
void heavy_computation(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int mode = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Allocate various data types to create register pressure */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_result = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long *long_data = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_result = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    float *float_data = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_result = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_result = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    unsigned char *char_data = (unsigned char*)aligned_alloc(64, ARRAY_SIZE);
    unsigned char *char_result = (unsigned char*)aligned_alloc(64, ARRAY_SIZE);
    __m128i *vec_data = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_result = (__m128i*)aligned_alloc(64, ARRAY_SIZE * sizeof(__m128i));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        long_data[i] = i * 5L + 2;
        float_data[i] = i * 1.5f;
        double_data[i] = i * 2.7;
        char_data[i] = (i * 7) & 0xFF;
        vec_data[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute tests to trigger reloads */
    test_primary_reloads(iterations, int_data, int_result, double_data, double_result);
    test_secondary_reloads(iterations, long_data, long_result, float_data, float_result);
    test_optional_reloads(iterations, char_data, char_result, vec_data, vec_result);
    test_control_flow_reloads(mode, iterations, int_data, int_result);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_result[i];
        checksum += long_result[i];
        checksum += *(unsigned*)&float_result[i];
        checksum += *(unsigned long long*)&double_result[i];
        checksum += char_result[i];
        checksum += _mm_extract_epi32(vec_result[i], 0);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data); free(int_result);
    free(long_data); free(long_result);
    free(float_data); free(float_result);
    free(double_data); free(double_result);
    free(char_data); free(char_result);
    free(vec_data); free(vec_result);
}

/* Primary reload test with complex constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    volatile int temp1, temp2, temp3, temp4, temp5;
    volatile double dtemp1, dtemp2;
    
    /* Unrolled loop with many live variables to create register pressure */
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - UNROLL_FACTOR);
        
        /* Multiple asm statements with diverse constraints */
        __asm__ volatile (
            /* Complex constraints: mixed input/output, earlyclobber, matching */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[din1], %%xmm0\n\t"
            "cvtsi2sd %%eax, %%xmm1\n\t"
            "addsd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, %[dout1]\n\t"
            : [out1] "=r" (out[idx]), 
              [dout1] "=m" (dout[idx]),
              "=&a" (temp1),     /* earlyclobber accumulator */
              "=&d" (temp2)      /* earlyclobber data register */
            : [in1] "rm" (in[idx]), 
              [in2] "i" (0x1234), /* immediate */
              [in3] "r" (idx),
              [din1] "xm" (din[idx]), /* memory with possible immediate */
              "0" (temp3)        /* matching constraint */
            : "xmm0", "xmm1", "memory", "cc"
        );
        
        /* Another asm with different register classes */
        __asm__ volatile (
            "mov %[in], %%bx\n\t"
            "addw $1, %%bx\n\t"
            "mov %%bx, %[out]\n\t"
            : [out] "=q" (temp4)  /* byte register constraint */
            : [in] "r" (in[idx + 1])
            : "bx", "cc"
        );
        
        /* Top-of-stack constraint */
        __asm__ volatile (
            "fldl %[din]\n\t"
            "fstpl %[dout]\n\t"
            : [dout] "=t" (dtemp1)
            : [din] "m" (din[idx + 2])
            : "st", "st(1)"
        );
        
        /* Prevent combination with memory barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* More complex constraints in nested scope */
        if (idx % 3 == 0) {
            __asm__ volatile (
                "movl %[in1], %%ecx\n\t"
                "leal (%%ecx, %%ecx, 2), %%ecx\n\t"
                "movl %%ecx, %[out1]\n\t"
                "movl %[in2], %%edx\n\t"
                "xorl %%ecx, %%edx\n\t"
                : [out1] "=c" (out[idx + 3]), /* output in ecx */
                  "=&d" (temp5)               /* earlyclobber edx */
                : [in1] "0" (in[idx + 3]),    /* matching constraint */
                  [in2] "r" (0xABCD)
                : "cc"
            );
        }
    }
}

/* Secondary reload test with mismatched constraints */
void test_secondary_reloads(int iterations, long *lin, long *lout, float *fin, float *fout) {
    volatile long ltemp1, ltemp2;
    volatile float ftemp1, ftemp2;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        
        /* Force secondary reload: "a" constraint with memory operand */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=a" (ltemp1)  /* output must be in eax */
            : [in1] "m" (lin[idx]), /* memory operand may need secondary reload */
              [in2] "r" (lin[idx + 1])
            : "cc"
        );
        
        /* Different register class in subsequent asm */
        __asm__ volatile (
            "mov %[in], %%ebx\n\t"
            "addl $1, %%ebx\n\t"
            : "=b" (ltemp2)        /* output in ebx */
            : [in] "a" (ltemp1)    /* input from previous eax output */
            : "cc"
        );
        
        /* Legacy register constraint that may need secondary reload */
        __asm__ volatile (
            "movd %[fin], %%xmm0\n\t"
            "cvtps2pd %%xmm0, %%xmm1\n\t"
            "movd %%xmm1, %[fout]\n\t"
            : [fout] "=R" (fout[idx])  /* legacy register constraint */
            : [fin] "m" (fin[idx])
            : "xmm0", "xmm1"
        );
        
        /* Mixed size constraints */
        __asm__ volatile (
            "movzbl %[cin], %%eax\n\t"
            "addb $32, %%al\n\t"
            "movb %%al, %[cout]\n\t"
            : [cout] "=q" (((unsigned char*)lout)[idx]) /* byte register */
            : [cin] "m" (((unsigned char*)lin)[idx])
            : "eax", "cc"
        );
    }
}

/* Optional reload test with '?' constraints */
void test_optional_reloads(int iterations, unsigned char *cin, unsigned char *cout, 
                          __m128i *vin, __m128i *vout) {
    volatile __m128i vtemp1, vtemp2;
    volatile int itemp;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 4);
        
        /* Optional output constraint */
        __asm__ volatile (
            "pmovzxbw %[vin], %[vout]\n\t"
            : [vout] "=?x" (vtemp1)  /* optional SSE register */
            : [vin] "m" (vin[idx])
            : /* no clobbers - different from next asm to prevent combine */
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers */
        __asm__ volatile (
            "pmovzxbw %[vin], %[vout]\n\t"
            : [vout] "=?x" (vtemp2)
            : [vin] "m" (vin[idx + 1])
            : "xmm0"  /* different clobber prevents combine */
        );
        
        /* Optional input with complex addressing */
        __asm__ volatile (
            "paddb %[v1], %[v2]\n\t"
            "movdqa %[v2], %[vout]\n\t"
            : [vout] "=x" (vout[idx])
            : [v1] "?x" (vtemp1),    /* optional input */
              [v2] "0" (vtemp2)      /* matching constraint */
            : "cc"
        );
        
        /* Byte operations with optional constraints */
        __asm__ volatile (
            "movb %[cin], %%al\n\t"
            "rorb $4, %%al\n\t"
            "movb %%al, %[cout]\n\t"
            : [cout] "=?q" (cout[idx])  /* optional byte register */
            : [cin] "m" (cin[idx])
            : "al", "cc"
        );
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int count, int *data, int *result) {
    volatile int a, b, c, d, e, f, g, h;
    
    /* Many live variables to increase register pressure */
    a = data[0];
    b = data[1];
    c = data[2];
    d = data[3];
    e = data[4];
    f = data[5];
    g = data[6];
    h = data[7];
    
    for (int i = 0; i < count; i++) {
        /* Complex control flow with asm statements */
        if (mode == 1) {
            __asm__ volatile (
                "addl %%ebx, %%eax\n\t"
                "imull %%ecx, %%eax\n\t"
                : "=a" (a), "+b" (b), "+c" (c)
                : "0" (a), "1" (b), "2" (c)
                : "cc"
            );
        } else if (mode == 2) {
            __asm__ volatile (
                "xorl %%edx, %%eax\n\t"
                "roll $3, %%eax\n\t"
                : "=a" (d), "=d" (e)
                : "0" (d), "1" (e)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "leal (%%eax, %%eax, 4), %%eax\n\t"
                "addl %%esi, %%eax\n\t"
                : "=a" (f), "=S" (g)
                : "0" (f), "1" (g)
                : "cc"
            );
        }
        
        /* Nested loop with more asm */
        for (int j = 0; j < 3; j++) {
            __asm__ volatile (
                "movl %[in], %%eax\n\t"
                "addl $1, %%eax\n\t"
                "movl %%eax, %[out]\n\t"
                : [out] "=rm" (result[i * 3 + j])
                : [in] "irm" ((i * 17 + j * 5) & 0xFF) /* mixed immediate/register/memory */
                : "eax", "cc"
            );
        }
        
        /* Switch between different constraint types */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "movl %%eax, %%ebx\n\t"
                    "addl $1, %%ebx\n\t"
                    : "=b" (h)
                    : "a" (a)
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "movl %%ecx, %%edx\n\t"
                    "subl $1, %%edx\n\t"
                    : "=d" (h)
                    : "c" (b)
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "xchgl %%eax, %%ebx\n\t"
                    : "+a" (a), "+b" (b)
                    :
                    : "cc"
                );
                break;
            case 3:
                __asm__ volatile (
                    "cmpl %%eax, %%ebx\n\t"
                    "setg %%al\n\t"
                    : "=a" (c)
                    : "a" (a), "b" (b)
                    : "cc"
                );
                break;
        }
    }
    
    /* Store results to prevent optimization */
    result[0] = a + b + c + d + e + f + g + h;
}

int main(int argc, char **argv) {
    heavy_computation(argc, argv);
    return 0;
}
