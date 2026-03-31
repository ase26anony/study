#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);

/* Helper to create register pressure */
static inline void create_register_pressure(int *live_vars) {
    /* Force many live variables */
    int v0 = live_vars[0], v1 = live_vars[1], v2 = live_vars[2], v3 = live_vars[3];
    int v4 = live_vars[4], v5 = live_vars[5], v6 = live_vars[6], v7 = live_vars[7];
    int v8 = live_vars[8], v9 = live_vars[9], v10 = live_vars[10], v11 = live_vars[11];
    int v12 = live_vars[12], v13 = live_vars[13], v14 = live_vars[14], v15 = live_vars[15];
    
    /* Use all variables to prevent optimization */
    __asm__ volatile("" 
        : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3), 
          "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7),
          "+r"(v8), "+r"(v9), "+r"(v10), "+r"(v11),
          "+r"(v12), "+r"(v13), "+r"(v14), "+r"(v15)
        : 
        : "memory");
    
    live_vars[0] = v0; live_vars[1] = v1; live_vars[2] = v2; live_vars[3] = v3;
    live_vars[4] = v4; live_vars[5] = v5; live_vars[6] = v6; live_vars[7] = v7;
    live_vars[8] = v8; live_vars[9] = v9; live_vars[10] = v10; live_vars[11] = v11;
    live_vars[12] = v12; live_vars[13] = v13; live_vars[14] = v14; live_vars[15] = v15;
}

/* Primary reloads with mixed constraints and modes */
void test_primary_reloads(int iterations, int *input, int *output) {
    int i, j;
    int live_vars[UNROLL_FACTOR];
    
    /* Initialize live variables */
    for (j = 0; j < UNROLL_FACTOR; j++) {
        live_vars[j] = input[j] + j;
    }
    
    for (i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - UNROLL_FACTOR);
        
        /* Complex asm with 8 operands, mixed constraints and modes */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (output[idx]),      /* General register */
            "=&r" (output[idx+1]),   /* Early clobber */
            "=q" (output[idx+2]),    /* Byte register (a,b,c,d) */
            "=a" (output[idx+3]),    /* Accumulator */
            "=d" (output[idx+4]),    /* Data register */
            
            /* Inputs with mixed constraints */
            : "r" (input[idx]),      /* Register */
              "m" (input[idx+1]),    /* Memory */
              "i" (12345),           /* Immediate */
              "r" (input[idx+2] & 0xFF), /* Byte value */
              "a" (input[idx+3]),    /* Must be in accumulator */
              "d" (input[idx+4]),    /* Must be in EDX */
              "0" (output[idx]),     /* Matching constraint */
              "rm" (input[idx+5])    /* Register or memory */
            
            /* Clobber many registers */
            : "rcx", "r8", "r9", "r10", "r11", "cc", "memory"
        );
        
        /* Another asm with different mode requirements */
        long long temp;
        __asm__ volatile (
            "movq %%mm0, %0\n\t"     /* MMX register */
            "movq %1, %%mm1\n\t"
            "paddb %%mm1, %%mm0\n\t"
            "movq %%mm0, %0"
            : "=y" (temp)            /* MMX register constraint */
            : "y" ((long long)input[idx+6])  /* MMX input */
            : "mm0", "mm1"
        );
        output[idx+6] = (int)temp;
        
        /* Create register pressure between asm blocks */
        create_register_pressure(live_vars);
        
        /* XMM register usage to force different register class */
        __m128i vec1, vec2;
        vec1 = _mm_set_epi32(input[idx+7], input[idx+8], 
                            input[idx+9], input[idx+10]);
        vec2 = _mm_set1_epi32(live_vars[0]);
        
        __asm__ volatile (
            "paddd %1, %0\n\t"
            "movdqa %0, %2"
            : "+x" (vec1), "=x" (vec2)
            : "m" (output[idx+7])
            : "memory"
        );
        
        _mm_storeu_si128((__m128i*)&output[idx+7], vec1);
        
        /* Unrolled computation to increase register pressure */
        for (j = 0; j < UNROLL_FACTOR; j += 4) {
            int t1 = live_vars[j] + input[idx + j];
            int t2 = live_vars[j+1] * input[idx + j + 1];
            int t3 = live_vars[j+2] ^ input[idx + j + 2];
            int t4 = live_vars[j+3] | input[idx + j + 3];
            
            /* Another complex asm with control flow dependency */
            if (t1 > t2) {
                __asm__ volatile (
                    "cmpl %1, %0\n\t"
                    "cmovg %2, %0\n\t"
                    "cmovg %3, %4"
                    : "+r" (t1), "+r" (t2)
                    : "r" (t3), "r" (t4), "r" (output[idx + j])
                    : "cc"
                );
            }
            
            output[idx + j] = t1;
            output[idx + j + 1] = t2;
            live_vars[j] = t3;
            live_vars[j + 1] = t4;
        }
    }
}

/* Secondary reload patterns with mismatched constraints */
void test_secondary_reloads(int iterations, double *input, double *output) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 8);
        double a, b, c, d;
        
        /* Force 'a' constraint (accumulator) then use in 'b' constraint */
        __asm__ volatile (
            "mov %1, %%rax\n\t"
            "add %2, %%rax\n\t"
            "mov %%rax, %0"
            : "=a" (a)                /* Output must be in RAX */
            : "b" (input[idx]),       /* Input must be in RBX */
              "r" (input[idx+1])      /* General register */
            : "rbx"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Now use 'a' result with 'b' constraint - may need secondary reload */
        __asm__ volatile (
            "xchg %%rbx, %%rax\n\t"   /* Force register move */
            "movq %%rbx, %0"
            : "=b" (b)                /* Output must be in RBX */
            : "a" (a),                /* Input from previous asm */
              "m" (input[idx+2])      /* Memory operand */
            : "rax"
        );
        
        /* Legacy register constraint that may need secondary reload for R8-R15 */
        long long legacy_reg;
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "shl $2, %%eax\n\t"
            "mov %%eax, %0"
            : "=R" (legacy_reg)       /* Legacy register (eax, ebx, ecx, edx) */
            : "r" (input[idx+3])
            : "eax"
        );
        
        /* XMM to MMX transfer that may need secondary reload */
        __m128d xmm_val = _mm_set_pd(input[idx+4], input[idx+5]);
        __m128i mmx_val;
        
        __asm__ volatile (
            "cvtpd2pi %1, %%mm0\n\t"
            "movq %%mm0, %0"
            : "=y" (mmx_val)          /* MMX output */
            : "x" (xmm_val)           /* XMM input - different register class */
            : "mm0"
        );
        
        /* Use the results */
        output[idx] = a + b;
        output[idx+1] = (double)legacy_reg;
        
        /* AVX-256 to force different register pressure */
        __m256d avx_vec = _mm256_set_pd(input[idx], input[idx+1], 
                                       input[idx+2], input[idx+3]);
        __m256d avx_vec2 = _mm256_set1_pd(input[idx+4]);
        
        __asm__ volatile (
            "vaddpd %1, %0, %0\n\t"
            "vmulpd %2, %0, %0"
            : "+x" (avx_vec)
            : "x" (avx_vec2), "m" (input[idx+5])
            : 
        );
        
        _mm256_storeu_pd(&output[idx+2], avx_vec);
    }
}

/* Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, float *input, float *output) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        int idx = i % (ARRAY_SIZE - 4);
        
        /* Optional output constraint */
        float opt_result;
        __asm__ volatile (
            "test %1, %1\n\t"
            "jz 1f\n\t"
            "cvtsi2ss %1, %0\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "xorps %0, %0\n\t"
            "2:"
            : "=?r" (opt_result)      /* Optional output */
            : "r" ((int)input[idx])
            : "cc"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to force nocombine */
        float result2;
        __asm__ volatile (
            "cvtsi2ss %1, %0"
            : "=r" (result2)
            : "r" ((int)input[idx+1])
            : "st"                    /* Different clobber - x87 stack */
        );
        
        /* Another similar asm with volatile to ensure separate reloads */
        float result3;
        __asm__ __volatile__ (
            "addss %1, %0\n\t"
            "mulss %2, %0"
            : "=r" (result3)
            : "r" (opt_result), "r" (result2)
            : 
        );
        
        output[idx] = result3;
        
        /* Complex asm with multiple optional inputs */
        float final;
        __asm__ volatile (
            "movss %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "mulss %3, %%xmm0\n\t"
            "movss %%xmm0, %0"
            : "=r" (final)
            : "?r" (input[idx+2]),    /* Optional input */
              "?r" (input[idx+3]),    /* Optional input */
              "r" (result3)           /* Required input */
            : "xmm0"
        );
        
        output[idx+1] = final;
        
        /* Control flow dependent asm */
        if (final > 0.5f) {
            __asm__ volatile (
                "rcpss %1, %0"
                : "=x" (final)
                : "x" (final)
                : 
            );
        } else {
            __asm__ volatile (
                "sqrtss %1, %0"
                : "=x" (final)
                : "x" (final)
                : 
            );
        }
        
        output[idx+2] = final;
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Allocate and initialize arrays */
    int *int_input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_input = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_input || !int_output || !double_input || !double_output ||
        !float_input || !float_output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_input[i] = (i * 37) & 0xFFF;
        double_input[i] = (i * 1.61803398875) / 100.0;
        float_input[i] = (i * 2.71828182846f) / 50.0f;
        int_output[i] = 0;
        double_output[i] = 0.0;
        float_output[i] = 0.0f;
    }
    
    /* Run tests based on mode */
    if (mode & 1) {
        test_primary_reloads(iterations, int_input, int_output);
    }
    
    if (mode & 2) {
        test_secondary_reloads(iterations / 2, double_input, double_output);
    }
    
    if (mode & 4) {
        test_optional_reloads(iterations / 2, float_input, float_output);
    }
    
    /* Compute checksum to ensure all asm executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long long)(double_output[i] * 1000.0);
        checksum += (long long)(float_output[i] * 1000.0f);
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(int_input);
    free(int_output);
    free(double_input);
    free(double_output);
    free(float_input);
    free(float_output);
    
    return 0;
}
