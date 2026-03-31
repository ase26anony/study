/* reload_test.c - Complex inline assembly to trigger reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_control_flow_reloads(int iterations, long *input, long *output, int mode);

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
    int int_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    long long_array[ARRAY_SIZE];
    
    /* Fill arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 0.25;
        float_array[i] = i * 0.75f + 0.125f;
        long_array[i] = i * 5L + 2L;
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_array, int_array + ARRAY_SIZE/2);
    test_secondary_reloads(iterations, double_array, double_array + ARRAY_SIZE/2);
    test_optional_reloads(iterations, float_array, float_array + ARRAY_SIZE/2);
    test_control_flow_reloads(iterations, long_array, long_array + ARRAY_SIZE/2, mode);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
        checksum += (unsigned long long)(double_array[i] * 1000);
        checksum += (unsigned long long)(float_array[i] * 1000);
        checksum += long_array[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}

/* Force register pressure with many live variables and complex asm */
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Many scalar variables to exhaust registers */
    register int r0 asm("eax") = input[0];
    register int r1 asm("ebx") = input[1];
    register int r2 asm("ecx") = input[2];
    register int r3 asm("edx") = input[3];
    register int r4 asm("esi") = input[4];
    register int r5 asm("edi") = input[5];
    int r6 = input[6], r7 = input[7], r8 = input[8], r9 = input[9];
    int r10 = input[10], r11 = input[11], r12 = input[12], r13 = input[13];
    int r14 = input[14], r15 = input[15];
    
    /* Vector variables to increase pressure */
    __m128i v0 = _mm_set_epi32(r0, r1, r2, r3);
    __m128i v1 = _mm_set_epi32(r4, r5, r6, r7);
    __m256d vd0 = _mm256_set_pd(r8 * 0.1, r9 * 0.2, r10 * 0.3, r11 * 0.4);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex inline assembly with many operands and constraints */
        __asm__ volatile (
            /* Multiple output operands with different constraints */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            
            /* Byte operation with q constraint */
            "movb %b[in4], %%al\n\t"
            "addb %b[in5], %%al\n\t"
            "movb %%al, %b[out2]\n\t"
            
            /* Memory operand with offset */
            "addl $1, %[mem1]\n\t"
            
            /* Early clobber to prevent reuse */
            "movl %[in6], %[out3]\n\t"
            "addl %%eax, %[out3]\n\t"
            
            /* Matching constraint */
            "movl %[in7], %[out4]\n\t"
            : [out1] "=&r" (r0),           /* early clobber reg */
              [out2] "=q" (r1),            /* byte register */
              [out3] "=&r" (r2),           /* early clobber */
              [out4] "=0" (r3),            /* matching constraint */
              [mem1] "+m" (input[i])       /* memory read-write */
            : [in1] "r" (r4),
              [in2] "r" (r5),
              [in3] "rm" (r6),             /* register or memory */
              [in4] "q" ((char)r7),
              [in5] "i" (5),               /* immediate */
              [in6] "r" (r8),
              [in7] "r" (r9),
              "a" (r0)                     /* fixed register input */
            : "cc", "memory"
        );
        
        /* Another asm with different constraints */
        __asm__ volatile (
            "cpuid\n\t"  /* Serializing instruction */
            : "=a" (r4), "=b" (r5), "=c" (r6), "=d" (r7)
            : "a" (0), "c" (0)
            : "memory"
        );
        
        /* Rotate registers to keep them all live */
        int tmp = r0; r0 = r1; r1 = r2; r2 = r3; r3 = r4;
        r4 = r5; r5 = r6; r6 = r7; r7 = r8; r8 = r9; r9 = r10;
        r10 = r11; r11 = r12; r12 = r13; r13 = r14; r14 = r15;
        r15 = tmp;
        
        /* Use vector variables to keep them live */
        v0 = _mm_add_epi32(v0, v1);
        vd0 = _mm256_add_pd(vd0, _mm256_set1_pd(0.01));
    }
    
    /* Store results */
    output[0] = r0 + r1 + r2 + r3;
    global_counter += r0;
}

/* Force secondary reloads with mismatched constraints */
void test_secondary_reloads(int iterations, double *input, double *output) {
    double d0 = input[0], d1 = input[1], d2 = input[2], d3 = input[3];
    double d4 = input[4], d5 = input[5], d6 = input[6], d7 = input[7];
    double d8, d9, d10, d11;
    
    /* x87 FPU stack operations - requires specific register classes */
    for (int i = 0; i < iterations; i++) {
        /* Mix x87 and SSE constraints */
        __asm__ volatile (
            /* x87 operation with t constraint (top of stack) */
            "fldl %[in1]\n\t"
            "faddl %[in2]\n\t"
            "fstpl %[out1]\n\t"
            
            /* SSE2 operation */
            "movsd %[in3], %%xmm0\n\t"
            "addsd %[in4], %%xmm0\n\t"
            "movsd %%xmm0, %[out2]\n\t"
            
            /* Convert between x87 and SSE */
            "fldl %[in5]\n\t"
            "fstpl %[temp]\n\t"
            "movsd %[temp], %%xmm1\n\t"
            "addsd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, %[out3]\n\t"
            : [out1] "=t" (d8),      /* x87 top of stack */
              [out2] "=x" (d9),      /* SSE register */
              [out3] "=x" (d10),
              [temp] "=m" (d11)      /* temp memory */
            : [in1] "m" (d0),
              [in2] "m" (d1),
              [in3] "x" (d2),        /* SSE input */
              [in4] "x" (d3),
              [in5] "m" (d4)
            : "xmm0", "xmm1", "st", "memory"
        );
        
        /* Legacy register constraint that might need secondary reload */
        __asm__ volatile (
            "mov %[in6], %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm2\n\t"
            "addsd %[in7], %%xmm2\n\t"
            "movsd %%xmm2, %[out4]\n\t"
            : [out4] "=m" (output[i])
            : [in6] "R" ((int)d5),   /* Legacy register constraint */
              [in7] "x" (d6)
            : "eax", "xmm2", "memory"
        );
        
        /* Rotate values */
        double tmp = d0; d0 = d1; d1 = d2; d2 = d3; d3 = d4;
        d4 = d5; d5 = d6; d6 = d7; d7 = d8; d8 = d9; d9 = d10;
        d10 = tmp;
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, float *input, float *output) {
    float f0 = input[0], f1 = input[1], f2 = input[2], f3 = input[3];
    float f4, f5, f6, f7;
    
    for (int i = 0; i < iterations; i++) {
        /* Optional output constraint */
        __asm__ volatile (
            "movss %[in1], %%xmm0\n\t"
            "addss %[in2], %%xmm0\n\t"
            "movss %%xmm0, %[out1]\n\t"
            "mulss %[in3], %%xmm0\n\t"
            "movss %%xmm0, %[out2]\n\t"
            : [out1] "=?r" (f4),     /* Optional output */
              [out2] "=x" (f5)       /* Required output */
            : [in1] "x" (f0),
              [in2] "x" (f1),
              [in3] "rm" (f2)        /* Register or memory */
            : "xmm0"
        );
        
        /* Another asm that could be combined but won't due to memory clobber */
        __asm__ volatile (
            "movss %[in4], %%xmm1\n\t"
            "subss %[in5], %%xmm1\n\t"
            "movss %%xmm1, %[out3]\n\t"
            : [out3] "=x" (f6)
            : [in4] "x" (f3),
              [in5] "x" (f4)
            : "xmm1", "memory"       /* Memory clobber prevents combine */
        );
        
        /* Volatile asm with different clobbers */
        __asm__ volatile (
            "movss %[in6], %%xmm2\n\t"
            "divss %[in7], %%xmm2\n\t"
            : 
            : [in6] "x" (f5),
              [in7] "x" (f6)
            : "xmm2"
        );
        
        /* Rotate and update */
        float tmp = f0; f0 = f1; f1 = f2; f2 = f3; f3 = f4;
        f4 = f5; f5 = f6; f6 = tmp;
        
        output[i] = f0 + f1 + f2 + f3 + f4 + f5 + f6;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int iterations, long *input, long *output, int mode) {
    long l0 = input[0], l1 = input[1], l2 = input[2], l3 = input[3];
    long l4 = input[4], l5 = input[5], l6 = input[6], l7 = input[7];
    long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Different asm blocks based on control flow */
        if (mode & 1) {
            __asm__ volatile (
                "mov %[in1], %%rax\n\t"
                "add %[in2], %%rax\n\t"
                "mov %%rax, %[out1]\n\t"
                : [out1] "=r" (l0)
                : [in1] "r" (l1),
                  [in2] "rm" (l2)
                : "rax"
            );
        } else {
            __asm__ volatile (
                "mov %[in3], %%rbx\n\t"
                "sub %[in4], %%rbx\n\t"
                "mov %%rbx, %[out2]\n\t"
                : [out2] "=r" (l0)
                : [in3] "r" (l3),
                  [in4] "rm" (l4)
                : "rbx"
            );
        }
        
        /* Switch between different constraint types */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "imul %[in5], %[out3]\n\t"
                    : [out3] "+r" (l1)
                    : [in5] "rm" (l5)
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "xor %[in6], %[out4]\n\t"
                    : [out4] "+r" (l2)
                    : [in6] "rm" (l6)
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "or %[in7], %[out5]\n\t"
                    : [out5] "+r" (l3)
                    : [in7] "rm" (l7)
                    : "cc"
                );
                break;
            case 3:
                __asm__ volatile (
                    "and %[in8], %[out6]\n\t"
                    : [out6] "+r" (l4)
                    : [in8] "rm" (l0)
                    : "cc"
                );
                break;
        }
        
        /* Loop-carried dependency */
        result += l0 + l1 + l2 + l3 + l4;
        
        /* Update mode based on result */
        mode ^= (result & 1);
        
        /* Rotate values */
        long tmp = l0; l0 = l1; l1 = l2; l2 = l3; l3 = l4;
        l4 = l5; l5 = l6; l6 = l7; l7 = tmp;
        
        if (i % 8 == 0) {
            /* Sporadic memory operation to force spills */
            __asm__ volatile (
                "mov %[val], %%rax\n\t"
                "lock xadd %%rax, %[global]\n\t"
                : [global] "+m" (global_counter)
                : [val] "r" (1)
                : "rax", "cc", "memory"
            );
        }
    }
    
    output[0] = result;
}
