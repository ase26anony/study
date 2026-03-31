/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function for primary reload patterns */
void test_primary_reloads(int iterations, int *input, int *output, 
                          double *dinput, double *doutput) {
    volatile int a = input[0];
    volatile int b = input[1];
    volatile int c = input[2];
    volatile int d = input[3];
    volatile int e = input[4];
    volatile int f = input[5];
    volatile int g = input[6];
    volatile int h = input[7];
    
    /* Force many live variables to create register pressure */
    int r0, r1, r2, r3, r4, r5, r6, r7;
    int s0, s1, s2, s3, s4, s5, s6, s7;
    int t0, t1, t2, t3, t4, t5, t6, t7;
    
    /* Mixed constraints with different machine modes */
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 8 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different register classes */
            "=r" (r0),     /* General register */
            "=&r" (r1),    /* Early clobber */
            "=q" (r2),     /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (r3),     /* Accumulator */
            "=d" (r4),     /* Data register */
            "=c" (r5),     /* Counter */
            "=r" (r6),
            "=r" (r7)
            :
            /* Inputs with mixed constraints */
            "r" (a),       /* Register */
            "m" (input[i % ARRAY_SIZE]),  /* Memory */
            "i" (0xFF),    /* Immediate */
            "r" (b),
            "m" (input[(i + 1) % ARRAY_SIZE]),
            "i" (0xAA),
            "r" (c),
            "0" (d)        /* Matching constraint with r0 */
            :
            /* Clobber list */
            "memory", "cc", "xmm0", "xmm1"
        );
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %%eax, %[out2]"
            : [out1] "=r" (s0), [out2] "=m" (output[i % ARRAY_SIZE])
            : [in1] "rm" (r0), [in2] "rm" (r1), [in3] "rm" (r2)
            : "eax", "cc"
        );
        
        /* Unrolled section to increase register pressure */
        #pragma unroll 4
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            int idx = (i * UNROLL_FACTOR + j) % ARRAY_SIZE;
            
            __asm__ volatile (
                /* Multiple output constraints with different modes */
                "=r" (t0),
                "=r" (t1),
                "=r" (t2),
                "=r" (t3)
                :
                /* Mixed input constraints forcing reloads */
                "r" (input[idx]),
                "m" (output[idx]),
                "i" (j),
                "r" (a + b),
                "m" (input[(idx + 1) % ARRAY_SIZE]),
                "0" (t0)  /* Matching constraint */
                :
                "cc"
            );
            
            /* Use results to prevent optimization */
            output[idx] = t0 + t1 + t2 + t3;
        }
        
        /* Rotate variables to keep them live */
        a = r0; b = r1; c = r2; d = r3;
        e = r4; f = r5; g = r6; h = r7;
    }
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, int *input, int *output,
                           __m128i *vinput, __m128i *voutput) {
    /* Force register pressure with many vector variables */
    __m128i v0, v1, v2, v3, v4, v5, v6, v7;
    __m128i w0, w1, w2, w3;
    
    int r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Initialize vectors */
    v0 = _mm_set_epi32(input[0], input[1], input[2], input[3]);
    v1 = _mm_set_epi32(input[4], input[5], input[6], input[7]);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm requiring secondary reloads due to
           mismatched constraints and register classes */
        
        /* First: Force a value into a specific register class */
        __asm__ volatile (
            "mov %[in], %%eax\n\t"
            "mov %%eax, %[out]"
            : [out] "=a" (r8)    /* Must be in eax */
            : [in] "rm" (input[i % ARRAY_SIZE])
            : "eax"
        );
        
        /* Second: Use that value with a different constraint,
           potentially requiring secondary reload */
        __asm__ volatile (
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[result]"
            : [result] "=r" (r9)
            : "a" (r8),          /* Input in eax */
              "b" (input[(i + 1) % ARRAY_SIZE])  /* Input in ebx */
            : "eax", "ebx", "cc"
        );
        
        /* Third: Mix vector and scalar operations */
        __asm__ volatile (
            /* Vector operation */
            "movdqu %[vec_in], %%xmm0\n\t"
            /* Scalar operation using vector result */
            "movd %%xmm0, %%eax\n\t"
            "addl %[scalar_in], %%eax\n\t"
            "movd %%eax, %%xmm1\n\t"
            "movdqu %%xmm1, %[vec_out]"
            : [vec_out] "=m" (voutput[i % 8])
            : [vec_in] "m" (vinput[i % 8]),
              [scalar_in] "r" (r9)
            : "xmm0", "xmm1", "eax", "memory"
        );
        
        /* Force use of legacy register constraint 'R' 
           which may require secondary reload for R8-R15 */
        __asm__ volatile (
            "movl %[in], %%ebx\n\t"
            "leal (%%ebx, %[in2]), %%ecx\n\t"
            "movl %%ecx, %[out]"
            : [out] "=R" (r10)   /* Legacy register constraint */
            : [in] "r" (r8),
              [in2] "r" (r9)
            : "ebx", "ecx", "cc"
        );
        
        /* Use results */
        output[i % ARRAY_SIZE] = r8 + r9 + r10;
        
        /* Alternate vector operations to keep pressure */
        if (i % 2 == 0) {
            v2 = _mm_add_epi32(v0, v1);
            v3 = _mm_sub_epi32(v0, v1);
        } else {
            v4 = _mm_mullo_epi32(v2, v3);
            v5 = _mm_slli_epi32(v4, 2);
        }
    }
}

/* Test function for optional reloads and non-combine patterns */
void test_optional_reloads(int iterations, int *input, int *output) {
    int opt1, opt2, opt3, opt4;
    int req1, req2;
    
    for (int i = 0; i < iterations; i++) {
        /* First asm with optional output constraint */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "testl %%eax, %%eax\n\t"
            "jz 1f\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out1]\n\t"
            "2:\n\t"
            "movl %%eax, %[out2]"
            : [out1] "=?r" (opt1),  /* Optional output */
              [out2] "=r" (req1)    /* Required output */
            : [in1] "r" (input[i % ARRAY_SIZE])
            : "eax", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Second similar asm that could be combined but won't
           due to memory barrier and different clobbers */
        __asm__ volatile (
            "movl %[in1], %%ebx\n\t"
            "subl $5, %%ebx\n\t"
            "movl %%ebx, %[out1]"
            : [out1] "=r" (opt2)
            : [in1] "r" (req1)
            : "ebx", "cc"
        );
        
        /* Another memory barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Third asm with different clobber list */
        __asm__ volatile (
            "movl %[in1], %%ecx\n\t"
            "imull %%ecx, %%ecx\n\t"
            "movl %%ecx, %[out1]"
            : [out1] "=r" (opt3)
            : [in1] "r" (opt2)
            : "ecx", "cc", "memory"
        );
        
        /* Use volatile to force execution order */
        __asm__ volatile (
            "nop"
            :
            :
            : "memory"
        );
        
        /* Final asm that uses all previous results */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "addl %[in3], %%eax\n\t"
            "movl %%eax, %[out1]"
            : [out1] "=r" (opt4)
            : [in1] "r" (opt1),
              [in2] "r" (opt2),
              [in3] "r" (opt3)
            : "eax", "cc"
        );
        
        output[i % ARRAY_SIZE] = opt4;
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays with mixed data */
    int *input = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *dinput = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *doutput = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    __m128i *vinput = (__m128i*)aligned_alloc(64, 8 * sizeof(__m128i));
    __m128i *voutput = (__m128i*)aligned_alloc(64, 8 * sizeof(__m128i));
    
    if (!input || !output || !dinput || !doutput || !vinput || !voutput) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = i * 3 + 1;
        output[i] = 0;
        dinput[i] = i * 0.5;
        doutput[i] = 0.0;
    }
    
    for (int i = 0; i < 8; i++) {
        vinput[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
        voutput[i] = _mm_setzero_si128();
    }
    
    printf("Starting reload tests with iterations=%d, mode=%d\n", 
           iterations, mode);
    
    /* Execute test functions based on mode */
    switch (mode) {
        case 1:
            test_primary_reloads(iterations, input, output, dinput, doutput);
            break;
        case 2:
            test_secondary_reloads(iterations, input, output, vinput, voutput);
            break;
        case 3:
            test_optional_reloads(iterations, input, output);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations/3, input, output, dinput, doutput);
            test_secondary_reloads(iterations/3, input, output, vinput, voutput);
            test_optional_reloads(iterations/3, input, output);
            break;
    }
    
    /* Compute checksum to ensure all asm blocks executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (unsigned long long)doutput[i];
    }
    
    for (int i = 0; i < 8; i++) {
        int32_t v[4];
        _mm_store_si128((__m128i*)v, voutput[i]);
        checksum += v[0] + v[1] + v[2] + v[3];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(dinput);
    free(doutput);
    free(vinput);
    free(voutput);
    
    return 0;
}
