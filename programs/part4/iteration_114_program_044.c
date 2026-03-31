/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 32
#define ARRAY_SIZE 1024

/* Test function for primary reloads with register pressure */
void test_primary_reloads(int iterations, int *input, int *output) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    volatile int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Create register pressure with many live variables */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with multiple operands and mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),     /* General register */
            "=&r" (b),    /* Early clobber */
            "=q" (c),     /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (d),     /* Accumulator */
            "=d" (e),     /* Data register */
            "=t" (f),     /* Top of FPU stack */
            "=m" (output[iter % ARRAY_SIZE]), /* Memory output */
            
            /* Inputs with mixed constraints */
            : "r" (input[iter % ARRAY_SIZE]),  /* Register */
              "i" (0xDEADBEEF),                /* Immediate */
              "m" (input[(iter + 1) % ARRAY_SIZE]), /* Memory */
              "r" (g),                         /* Register */
              "0" (h),                         /* Matching constraint */
              "a" (i),                         /* Accumulator */
              "d" (j),                         /* Data register */
              "rm" (k)                         /* Register or memory */
            
            /* Clobber many registers to force spills */
            : "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "cc", "memory"
        );
        
        /* Unrolled computations to keep many values live */
        a = b + c;
        b = d - e;
        c = f * g;
        d = h / (i + 1);
        e = j | k;
        f = l & m;
        g = n ^ o;
        h = p << 2;
        i = q >> 1;
        j = r + s;
        k = t - u;
        l = v * w;
        m = x % 7;
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "imull %[in2], %%eax\n\t"
            "addl %%eax, %[out1]\n\t"
            "movq %[in3], %%mm0\n\t"
            "paddd %[in4], %%mm0\n\t"
            "movq %%mm0, %[out2]"
            : [out1] "+r" (output[iter % ARRAY_SIZE]),
              [out2] "=m" (output[(iter + 1) % ARRAY_SIZE])
            : [in1] "rm" (a),
              [in2] "rm" (b),
              [in3] "x" (*(__m64*)&input[iter % ARRAY_SIZE]),
              [in4] "x" (*(__m64*)&input[(iter + 2) % ARRAY_SIZE])
            : "eax", "mm0", "mm1", "cc"
        );
    }
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, double *dinput, double *doutput) {
    volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    volatile __m128d v1, v2, v3, v4;
    volatile __m256d y1, y2;
    
    /* Initialize vectors */
    v1 = _mm_set_pd(1.0, 2.0);
    v2 = _mm_set_pd(3.0, 4.0);
    y1 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force secondary reloads with mismatched constraints */
        __asm__ volatile (
            /* "R" constraint for legacy register (eax-edi) */
            "mov %[in1], %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "movsd %%xmm0, %[out1]"
            : [out1] "=m" (doutput[iter % ARRAY_SIZE])
            : [in1] "R" (iter)  /* May need secondary reload if in R8-R15 */
            : "eax", "xmm0", "xmm1"
        );
        
        /* Mix x87 and SSE constraints */
        __asm__ volatile (
            "fldl %[in1]\n\t"
            "faddl %[in2]\n\t"
            "fstpl %[out1]\n\t"
            "movsd %[in3], %%xmm0\n\t"
            "addsd %[in4], %%xmm0\n\t"
            "movsd %%xmm0, %[out2]"
            : [out1] "=m" (doutput[(iter + 1) % ARRAY_SIZE]),
              [out2] "=m" (doutput[(iter + 2) % ARRAY_SIZE])
            : [in1] "m" (dinput[iter % ARRAY_SIZE]),
              [in2] "m" (dinput[(iter + 1) % ARRAY_SIZE]),
              [in3] "x" (v1),
              [in4] "x" (v2)
            : "st", "st(1)", "st(2)", "xmm0"
        );
        
        /* AVX with memory operand that may need secondary reload */
        __asm__ volatile (
            "vmovapd %[in1], %%ymm0\n\t"
            "vmulpd %[in2], %%ymm0, %%ymm1\n\t"
            "vmovapd %%ymm1, %[out1]"
            : [out1] "=m" (doutput[(iter + 3) % ARRAY_SIZE])
            : [in1] "m" (dinput[iter % ARRAY_SIZE * 4]),
              [in2] "x" (y1)
            : "ymm0", "ymm1", "ymm2"
        );
        
        /* Force register move between different register classes */
        __asm__ volatile (
            "mov %[in_a], %%eax\n\t"
            "mov %%eax, %[tmp]\n\t"
            "mov %[tmp], %%ebx\n\t"
            "add %[in_b], %%ebx\n\t"
            "mov %%ebx, %[out]"
            : [out] "=r" (doutput[iter % ARRAY_SIZE]),
              [tmp] "=r" (d1)
            : [in_a] "a" (iter),      /* Must be in eax */
              [in_b] "b" (iter * 2)   /* Must be in ebx */
            : "eax", "ebx", "cc"
        );
    }
}

/* Test function for optional and non-combine reloads */
void test_optional_reloads(int iterations, int *input, int *output) {
    volatile int opt1 = 0, opt2 = 0, opt3 = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Optional constraint with '?' modifier */
        __asm__ volatile (
            "test %[in], %[in]\n\t"
            "jz 1f\n\t"
            "movl $1, %[out]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out]\n\t"
            "2:"
            : [out] "=?r" (opt1)      /* Optional output */
            : [in] "r" (input[iter % ARRAY_SIZE])
            : "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm with different clobbers to prevent combination */
        __asm__ volatile (
            "addl $1, %[out]"
            : [out] "+r" (opt2)
            :: "cc"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Different asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "subl $1, %[out]"
            : [out] "+r" (opt3)
            :: "cc"
        );
        
        /* Complex asm with multiple optional outputs */
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "movl %[in2], %%ebx\n\t"
            "cmpl %%ebx, %%eax\n\t"
            "setg %%al\n\t"
            "setl %%bl"
            : "=?a" (opt1),      /* Optional - conditionally set */
              "=?b" (opt2)       /* Optional - conditionally set */
            : [in1] "r" (input[iter % ARRAY_SIZE]),
              [in2] "r" (input[(iter + 1) % ARRAY_SIZE])
            : "cc"
        );
        
        output[iter % ARRAY_SIZE] = opt1 + opt2 + opt3;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int iterations, int *input, int *output) {
    volatile int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < iterations; i++) {
        if (mode & 1) {
            /* Branch-specific asm with register constraints */
            __asm__ volatile (
                "movl %[in], %%eax\n\t"
                "leal (%%eax,%%eax,2), %%ebx\n\t"
                "movl %%ebx, %[out]"
                : [out] "=r" (x)
                : [in] "r" (input[i % ARRAY_SIZE])
                : "eax", "ebx", "cc"
            );
        } else {
            /* Different asm in else branch */
            __asm__ volatile (
                "movl %[in], %%ecx\n\t"
                "imull $3, %%ecx\n\t"
                "movl %%ecx, %[out]"
                : [out] "=r" (x)
                : [in] "r" (input[i % ARRAY_SIZE])
                : "ecx", "cc"
            );
        }
        
        /* Loop-dependent asm */
        for (int j = 0; j < (i % 8); j++) {
            __asm__ volatile (
                "addl $1, %[val]"
                : [val] "+r" (y)
                :: "cc"
            );
        }
        
        /* Switch with different asm in each case */
        switch (i % 4) {
            case 0:
                __asm__ volatile ("movl $0xAA, %[out]" : [out] "=r" (z));
                break;
            case 1:
                __asm__ volatile ("movl $0xBB, %[out]" : [out] "=r" (z));
                break;
            case 2:
                __asm__ volatile ("movl $0xCC, %[out]" : [out] "=r" (z));
                break;
            case 3:
                __asm__ volatile ("movl $0xDD, %[out]" : [out] "=r" (z));
                break;
        }
        
        output[i % ARRAY_SIZE] = x + y + z;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > 10000) iterations = 10000;
    
    /* Allocate and initialize arrays */
    int *input = malloc(ARRAY_SIZE * sizeof(int));
    int *output = malloc(ARRAY_SIZE * sizeof(int));
    double *dinput = malloc(ARRAY_SIZE * sizeof(double));
    double *doutput = malloc(ARRAY_SIZE * sizeof(double));
    
    if (!input || !output || !dinput || !doutput) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mixed data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = i * 3 + 1;
        output[i] = 0;
        dinput[i] = i * 0.5;
        doutput[i] = 0.0;
    }
    
    printf("Running reload tests with iterations=%d, mode=%d\n", iterations, mode);
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, input, output);
    test_secondary_reloads(iterations / 2, dinput, doutput);
    test_optional_reloads(iterations, input, output);
    test_control_flow_reloads(mode, iterations, input, output);
    
    /* Compute checksum to ensure all asm executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (long long)doutput[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(dinput);
    free(doutput);
    
    return 0;
}
