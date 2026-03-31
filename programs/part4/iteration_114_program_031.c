/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(double *dinput, float *foutput, int count);
void test_optional_reloads(long *linput, long *loutput, int mode);
void test_control_flow_reloads(int argc, char **argv, int *results);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vec = _mm_setzero_si128();

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *input, int *output) {
    int i, j;
    int a, b, c, d, e, f, g, h;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    long long ltmp1, ltmp2;
    double dtmp1, dtmp2;
    float ftmp1, ftmp2;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        /* Load many variables to create register pressure */
        a = input[i * 8 + 0];
        b = input[i * 8 + 1];
        c = input[i * 8 + 2];
        d = input[i * 8 + 3];
        e = input[i * 8 + 4];
        f = input[i * 8 + 5];
        g = input[i * 8 + 6];
        h = input[i * 8 + 7];
        
        /* Complex asm with 8 operands mixing constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (tmp1),     /* general register */
            "=&r" (tmp2),    /* earlyclobber general register */
            "=q" (tmp3),     /* byte register (a,b,c,d) */
            "=a" (tmp4),     /* accumulator */
            "=d" (tmp5),     /* data register */
            "=t" (dtmp1),    /* top of FPU stack */
            "=u" (ftmp1),    /* second FPU register */
            "=m" (output[i * 8])  /* memory output */
            
            /* Inputs with various constraints */
            : "r" (a),       /* general register */
              "i" (12345),   /* immediate */
              "m" (input[i * 8 + 1]),  /* memory */
              "r" (b),
              "0" (c),       /* matches first output */
              "g" (d),       /* general or memory */
              "rm" (e),      /* register or memory */
              "a" (f)        /* accumulator */
            
            /* Clobber list */
            : "cc", "memory", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Another asm with vector operations */
        __m128i v1 = _mm_set_epi32(a, b, c, d);
        __m128i v2 = _mm_set_epi32(e, f, g, h);
        __m128i v3;
        
        __asm__ volatile (
            "movdqa %1, %0\n\t"
            "paddd %2, %0\n\t"
            : "=x" (v3)
            : "xm" (v1), "xm" (v2)
            : "xmm4", "xmm5"
        );
        
        /* Use results to prevent optimization */
        output[i * 8 + 1] = tmp1 + tmp2 + tmp3;
        output[i * 8 + 2] = (int)(dtmp1 * 100.0);
        output[i * 8 + 3] = (int)(ftmp1 * 1000.0f);
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* More asm with different constraints */
        __asm__ volatile (
            "imul %2, %1\n\t"
            "add %1, %0\n\t"
            : "=r" (ltmp1), "=&r" (ltmp2)
            : "r" (g), "0" ((long long)h), "1" ((long long)a)
            : "cc"
        );
        
        output[i * 8 + 4] = (int)(ltmp1 >> 32);
    }
}

/* Force secondary reloads with mismatched constraints */
void test_secondary_reloads(double *dinput, float *foutput, int count) {
    int i;
    double d1, d2, d3;
    float f1, f2;
    long double ld1;
    
    for (i = 0; i < count; i += 4) {
        d1 = dinput[i];
        d2 = dinput[i + 1];
        d3 = dinput[i + 2];
        
        /* asm requiring specific register classes */
        __asm__ volatile (
            /* Output requiring FPU stack */
            "=t" (ld1),
            
            /* Inputs with constraints that may need secondary reloads */
            : "f" (d1),      /* FPU register */
              "m" (dinput[i + 3]),  /* Memory - may need secondary reload */
              "r" ((int)d1), /* Integer register */
              "a" (i),       /* Accumulator */
              "b" (global_counter)  /* Base register */
            
            /* Clobber specific registers to force moves */
            : "cc", "st(1)", "st(2)", "st(3)",
              "rax", "rbx", "rcx", "rdx"
        );
        
        /* Another asm with legacy register constraints */
        int r8_val, r9_val;
        __asm__ volatile (
            "mov %2, %0\n\t"
            "mov %3, %1\n\t"
            : "=R" (r8_val), "=R" (r9_val)  /* Legacy register constraint */
            : "r" (i), "r" (i * 2)
            : "r8", "r9"
        );
        
        /* Use 'a' constraint then 'b' constraint forcing move */
        int acc_val, base_val;
        __asm__ volatile (
            "movl %%eax, %0\n\t"
            : "=r" (acc_val)
            : "a" (i * 3)
            : "cc"
        );
        
        __asm__ volatile (
            "movl %1, %%ebx\n\t"
            "addl $1, %%ebx\n\t"
            : "=b" (base_val)
            : "r" (acc_val)
            : "cc"
        );
        
        foutput[i] = (float)ld1 + (float)r8_val + (float)base_val;
        
        /* Complex asm with optional constraints */
        int opt1, opt2;
        __asm__ volatile (
            "mov %2, %0\n\t"
            "test %3, %3\n\t"
            "cmovnz %4, %1\n\t"
            : "=r" (opt1), "=?r" (opt2)  /* opt2 is optional */
            : "r" (i), "r" (global_counter), "i" (0)
            : "cc"
        );
        
        foutput[i + 1] = (float)opt1 + (float)opt2;
    }
}

/* Test optional reloads and nocombine scenarios */
void test_optional_reloads(long *linput, long *loutput, int mode) {
    int i;
    long l1, l2, l3, l4;
    
    for (i = 0; i < ARRAY_SIZE / 2; i++) {
        l1 = linput[i];
        l2 = linput[i + ARRAY_SIZE/4];
        
        /* First asm with specific clobbers */
        __asm__ volatile (
            "add %2, %1\n\t"
            "sub %3, %0\n\t"
            : "=r" (l3), "=r" (l4)
            : "r" (l1), "r" (l2), "0" (l1), "1" (l2)
            : "cc", "r11", "r12"
        );
        
        /* Memory barrier prevents combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers - won't combine */
        __asm__ volatile (
            "add %2, %1\n\t"
            "sub %3, %0\n\t"
            : "=r" (loutput[i]), "=r" (loutput[i + ARRAY_SIZE/4])
            : "r" (l3), "r" (l4), "0" (l3), "1" (l4)
            : "cc", "r13", "r14", "r15"
        );
        
        /* Optional output with '?' constraint */
        long optional_out;
        __asm__ volatile (
            "test %1, %1\n\t"
            "jz 1f\n\t"
            "mov %1, %0\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "mov $0, %0\n\t"
            "2:\n\t"
            : "=?r" (optional_out)  /* Optional output */
            : "r" (mode)
            : "cc"
        );
        
        loutput[i] += optional_out;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int argc, char **argv, int *results) {
    int i, j;
    int tmp_results[8] = {0};
    
    /* Multiple conditionals with asm inside */
    for (i = 0; i < argc; i++) {
        if (argv[i][0] == 'a') {
            int x = i * 2;
            int y = i * 3;
            
            __asm__ volatile (
                "lea (%1, %2, 2), %0\n\t"
                : "=r" (tmp_results[0])
                : "r" (x), "r" (y)
                : "cc"
            );
        } else if (argv[i][0] == 'b') {
            int a = i;
            int b = i + 1;
            
            __asm__ volatile (
                "imull %1, %0\n\t"
                : "+r" (a)
                : "r" (b)
                : "cc"
            );
            
            tmp_results[1] = a;
        }
        
        /* Loop with asm that depends on iteration */
        for (j = 0; j < 4; j++) {
            int val = i * 10 + j;
            int out;
            
            __asm__ volatile (
                "mov %1, %0\n\t"
                "rol $4, %0\n\t"
                : "=r" (out)
                : "r" (val)
                : "cc"
            );
            
            tmp_results[2 + j] += out;
        }
    }
    
    /* Combine results */
    for (i = 0; i < 8; i++) {
        results[i] = tmp_results[i];
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > ARRAY_SIZE/8) iterations = ARRAY_SIZE/8;
    
    /* Allocate and initialize arrays */
    int *input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *dinput = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *foutput = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long *linput = (long*)malloc(ARRAY_SIZE * sizeof(long));
    long *loutput = (long*)malloc(ARRAY_SIZE * sizeof(long));
    int *results = (int*)malloc(8 * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = i * 3 + 1;
        output[i] = 0;
        dinput[i] = i * 0.5;
        foutput[i] = 0.0f;
        linput[i] = i * 7L;
        loutput[i] = 0L;
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, input, output);
    test_secondary_reloads(dinput, foutput, ARRAY_SIZE/4);
    test_optional_reloads(linput, loutput, mode);
    test_control_flow_reloads(argc, argv, results);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (unsigned long long)(foutput[i] * 1000);
        checksum += loutput[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(input);
    free(output);
    free(dinput);
    free(foutput);
    free(linput);
    free(loutput);
    free(results);
    
    return 0;
}
