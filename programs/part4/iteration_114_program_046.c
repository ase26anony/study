/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Global arrays to create register pressure */
static int int_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static __m128i vec128_array[ARRAY_SIZE/4];
static __m256d vec256_array[ARRAY_SIZE/8];

/* Test function 1: Primary reloads with diverse constraints */
void test_primary_reloads(int iterations, int mode) {
    volatile int a, b, c, d, e, f, g, h;
    volatile double da, db, dc, dd;
    volatile float fa, fb, fc, fd;
    volatile long la, lb, lc, ld;
    
    /* Create many live variables to exhaust registers */
    int v1 = int_array[0], v2 = int_array[1], v3 = int_array[2];
    int v4 = int_array[3], v5 = int_array[4], v6 = int_array[5];
    int v7 = int_array[6], v8 = int_array[7], v9 = int_array[8];
    int v10 = int_array[9], v11 = int_array[10], v12 = int_array[11];
    int v13 = int_array[12], v14 = int_array[13], v15 = int_array[14];
    int v16 = int_array[15];
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 8 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),     /* general register */
            "=&r" (b),    /* earlyclobber */
            "=q" (c),     /* byte register (a,b,c,d) */
            "=a" (d),     /* accumulator */
            "=d" (e),     /* data register */
            "=t" (fa),    /* top of FP stack */
            "=m" (int_array[i % 16]), /* memory */
            "=r" (f)      /* general register */
            
            /* Inputs with mixed constraints */
            : "r" (v1),   /* register */
              "i" (123),  /* immediate */
              "m" (int_array[(i+1) % 16]), /* memory */
              "r" (v2),
              "a" (v3),   /* accumulator */
              "d" (v4),   /* data register */
              "g" (v5),   /* general or memory */
              "rm" (v6)   /* register or memory */
            
            /* Clobber many registers */
            : "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "cc", "memory"
        );
        
        /* Another asm with matching constraints to force reloads */
        __asm__ volatile (
            "addl %2, %0\n\t"
            "imull %3, %1\n\t"
            "movl %0, %4\n\t"
            : "=&r" (g), "=r" (h), "+r" (v7), "+r" (v8)
            : "0" (v9), "1" (v10), "m" (int_array[i % 8]), "i" (456)
            : "cc"
        );
        
        /* Use all variables to keep them live */
        v11 = a + b + c;
        v12 = d * e;
        v13 = (int)fa + f;
        v14 = g ^ h;
        
        /* Unrolled computation to increase register pressure */
        v15 = v1 * v2 + v3 - v4;
        v16 = v5 | v6 & v7;
        v1 = v8 << 2;
        v2 = v9 >> 1;
        v3 = v10 + v11;
        v4 = v12 - v13;
        v5 = v14 * v15;
        v6 = v16 / (v1 + 1);
    }
    
    /* Store results back */
    int_array[0] = v1; int_array[1] = v2;
    int_array[2] = v3; int_array[3] = v4;
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations) {
    volatile int result1, result2, result3;
    volatile long long ll_result;
    volatile __m128i vec_result;
    
    for (int i = 0; i < iterations; i++) {
        /* Force secondary reload by using 'a' constraint with memory operand */
        int input_val = int_array[i % 32];
        
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result1)
            : "m" (input_val)  /* Memory operand with 'a' constraint usage */
            : "eax", "cc"
        );
        
        /* Use legacy register constraint 'R' that might need secondary reload */
        __asm__ volatile (
            "movq %1, %%rax\n\t"
            "shlq $2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=R" (ll_result)  /* Legacy register constraint */
            : "r" ((long long)int_array[i % 16])
            : "rax", "cc"
        );
        
        /* Mix x87 and SSE constraints */
        double dval = double_array[i % 16];
        float fval;
        
        __asm__ volatile (
            "fldl %1\n\t"
            "fstps %0\n\t"
            : "=m" (fval)
            : "m" (dval)
            : "st", "st(1)", "st(2)"
        );
        
        float_array[i % 16] = fval;
        
        /* Vector intrinsic with register pressure */
        __m128i v1 = vec128_array[i % 8];
        __m128i v2 = vec128_array[(i+1) % 8];
        
        __asm__ volatile (
            "paddd %1, %0\n\t"
            : "+x" (v1)
            : "x" (v2)
            : "xmm0", "xmm1"
        );
        
        vec128_array[i % 8] = v1;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int iterations) {
    volatile int opt1, opt2, opt3;
    volatile int out1, out2;
    
    for (int i = 0; i < iterations; i++) {
        /* Use optional constraints with '?' modifier */
        __asm__ volatile (
            "movl %2, %0\n\t"
            "testl %3, %3\n\t"
            "cmovnel %4, %0\n\t"
            : "=?r" (opt1), "=?r" (opt2)  /* Optional outputs */
            : "r" (int_array[i % 16]),
              "r" (int_array[(i+1) % 16]),
              "rm" (int_array[(i+2) % 16])
            : "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "addl $1, %0\n\t"
            "subl $2, %1\n\t"
            : "+r" (opt1), "+r" (opt2)
            :
            : "cc"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Use volatile to prevent optimization */
        __asm__ volatile (
            "imull %2, %0\n\t"
            "xorl %3, %1\n\t"
            : "=&r" (out1), "=r" (out2)
            : "r" (opt1), "r" (opt2), "m" (int_array[i % 8])
            : "cc"
        );
        
        int_array[i % 8] = out1 + out2;
    }
}

/* Test function 4: Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int threshold) {
    volatile int cf1, cf2, cf3;
    volatile double cf_d1, cf_d2;
    
    for (int i = 0; i < iterations; i++) {
        /* Reloads inside conditionals */
        if (int_array[i % 32] > threshold) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "leal (%%eax,%%eax,2), %0\n\t"
                : "=r" (cf1)
                : "r" (i)
                : "eax", "cc"
            );
        } else {
            __asm__ volatile (
                "movl %1, %%ebx\n\t"
                "negl %%ebx\n\t"
                "movl %%ebx, %0\n\t"
                : "=r" (cf1)
                : "r" (i)
                : "ebx", "cc"
            );
        }
        
        /* Loop-dependent asm with many live variables */
        int live1 = cf1, live2 = i * 2, live3 = i + 100;
        int live4 = live1 ^ live2, live5 = live3 & live1;
        int live6 = live4 | live5, live7 = live6 << 1;
        
        for (int j = 0; j < 4; j++) {
            __asm__ volatile (
                "addl %2, %0\n\t"
                "subl %3, %1\n\t"
                : "+r" (live4), "+r" (live5)
                : "r" (live6), "r" (live7)
                : "cc"
            );
            
            /* Use result in FP asm */
            cf_d1 = (double)live4;
            cf_d2 = (double)live5;
            
            __asm__ volatile (
                "addsd %1, %0\n\t"
                "mulsd %2, %0\n\t"
                : "+x" (cf_d1)
                : "x" (cf_d2), "m" (double_array[j])
                : "xmm0", "xmm1"
            );
            
            double_array[j] = cf_d1;
        }
        
        int_array[i % 32] = cf1 + live4 + live5;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (mode < 0 || mode > 3) mode = 1;
    
    printf("Running reload tests with iterations=%d, mode=%d\n", 
           iterations, mode);
    
    /* Initialize arrays with mixed data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5;
        float_array[i] = i * 0.75f;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        vec128_array[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
    }
    
    for (int i = 0; i < ARRAY_SIZE/8; i++) {
        vec256_array[i] = _mm256_set_pd(i*8+7, i*8+6, i*8+5, i*8+4,
                                        i*8+3, i*8+2, i*8+1, i*8);
    }
    
    /* Run all test functions to trigger various reload patterns */
    test_primary_reloads(iterations, mode);
    test_secondary_reloads(iterations / 2);
    test_optional_reloads(iterations / 4);
    test_control_flow_reloads(iterations, mode * 100);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
        checksum += (unsigned long long)(double_array[i] * 1000);
        checksum += (unsigned int)(float_array[i] * 1000);
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
