/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function 1: Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *in_ints, double *in_doubles, 
                         int *out_ints, double *out_doubles) {
    /* Create many live variables to pressure registers */
    register int r0 asm("eax") = in_ints[0];
    register int r1 asm("ebx") = in_ints[1];
    register int r2 asm("ecx") = in_ints[2];
    register int r3 asm("edx") = in_ints[3];
    register int r4 asm("esi") = in_ints[4];
    register int r5 asm("edi") = in_ints[5];
    
    double d0 = in_doubles[0];
    double d1 = in_doubles[1];
    double d2 = in_doubles[2];
    double d3 = in_doubles[3];
    
    /* Vector variables to consume SSE/AVX registers */
    __m128i v0 = _mm_set_epi32(in_ints[0], in_ints[1], in_ints[2], in_ints[3]);
    __m128i v1 = _mm_set_epi32(in_ints[4], in_ints[5], in_ints[6], in_ints[7]);
    __m256d vd0 = _mm256_set_pd(d0, d1, d2, d3);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with multiple constraints and clobbers */
        /* This should trigger reloads for rclass, inmode, outmode */
        asm volatile (
            /* Multiple output operands with different constraints */
            "movl %[imm], %%eax\n\t"
            "addl %%eax, %[out1]\n\t"
            "imull %[in1], %[out2]\n\t"
            "movq %[din1], %%xmm0\n\t"
            "addsd %%xmm0, %[dout1]\n\t"
            : [out1] "+&r" (r0),        /* earlyclobber */
              [out2] "=r" (r1),
              [dout1] "=t" (d0)         /* top of FP stack */
            : [in1] "rm" (r2),          /* register or memory */
              [din1] "xm" (d1),         /* SSE register or memory */
              [imm] "i" (42)            /* immediate */
            : "eax", "xmm0", "cc", "memory"
        );
        
        /* Another asm with matching constraints to force reload combinations */
        int temp = r3;
        asm volatile (
            "leal (%[base], %[index], 4), %[result]\n\t"
            : [result] "=r" (r4)
            : [base] "0" (r3),          /* matching constraint */
              [index] "r" (temp)
            : "cc"
        );
        
        /* Byte register constraint */
        unsigned char b1, b2;
        asm volatile (
            "movb %[inb], %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %[outb]\n\t"
            : [outb] "=q" (b1)          /* byte register (a,b,c,d) */
            : [inb] "q" ((unsigned char)r5)
            : "al"
        );
        
        /* Unrolled computations to keep many values live */
        if (i % 2 == 0) {
            asm volatile (
                "cmpl %[a], %[b]\n\t"
                "setg %[c]\n\t"
                : [c] "=r" (r5)
                : [a] "r" (r0),
                  [b] "rm" (r1)
                : "cc"
            );
        }
        
        /* Use vector variables to prevent them from being optimized out */
        v0 = _mm_add_epi32(v0, v1);
        vd0 = _mm256_add_pd(vd0, vd0);
    }
    
    /* Store results */
    out_ints[0] = r0 + r1 + r2 + r3 + r4 + r5;
    out_doubles[0] = d0 + d1 + d2 + d3;
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int *in_data, int *out_data) {
    /* Force secondary reloads by using specific register constraints
       that may require intermediate moves */
    
    int a = in_data[0];
    int b = in_data[1];
    int c = in_data[2];
    
    /* 'a' constraint (accumulator) forcing specific register allocation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (out_data[0])
        : "a" (a),          /* input must be in eax */
          "r" (b)
        : "eax"
    );
    
    /* Now use the result with a 'b' constraint (ebx) */
    int d = out_data[0];
    asm volatile (
        "movl %1, %%ebx\n\t"
        "imull %2, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        : "=r" (out_data[1])
        : "b" (d),          /* input must be in ebx */
          "r" (c)
        : "ebx"
    );
    
    /* Legacy register constraint 'R' (ax,bx,cx,dx) that might conflict
       with modern register allocation */
    int e = in_data[3];
    asm volatile (
        "xchgl %%eax, %1\n\t"
        "addl $1, %%eax\n\t"
        "xchgl %%eax, %0\n\t"
        : "=R" (out_data[2])  /* legacy register */
        : "R" (e),            /* legacy register */
          "0" (out_data[2])   /* matching constraint */
        : "eax"
    );
    
    /* Memory operand that might need secondary reload */
    long long large_val = ((long long)in_data[4] << 32) | in_data[5];
    asm volatile (
        "movq %1, %%mm0\n\t"   /* MMX register */
        "movq %%mm0, %0\n\t"
        : "=m" (out_data[3])   /* memory output */
        : "rm" (large_val)     /* register or memory - might need secondary */
        : "mm0"
    );
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int *data, int n, int *results) {
    /* Use optional constraints marked with '?' */
    int opt1 = data[0];
    int opt2 = data[1];
    
    /* Optional output constraint */
    asm volatile (
        "testl %[val], %[val]\n\t"
        "jnz 1f\n\t"
        "movl $0, %[out]\n\t"
        "jmp 2f\n\t"
        "1:\n\t"
        "movl %[val], %[out]\n\t"
        "2:\n\t"
        : [out] "=?r" (results[0])  /* optional output */
        : [val] "r" (opt1)
        : "cc"
    );
    
    /* Memory barrier to prevent reload combination */
    asm volatile ("" ::: "memory");
    
    /* Similar asm that could be combined but won't due to barrier */
    asm volatile (
        "addl $1, %[out]\n\t"
        : [out] "+r" (results[0])
        :
        : "cc"
    );
    
    /* Another memory barrier */
    asm volatile ("" ::: "memory");
    
    /* Different clobber list prevents combination */
    asm volatile (
        "subl $1, %[out]\n\t"
        : [out] "+r" (results[0])
        :
        : "cc", "memory"  /* Different from previous */
    );
    
    /* Conditional execution paths */
    for (int i = 0; i < n; i++) {
        if (data[i] & 1) {
            /* This asm only executed on some paths */
            asm volatile (
                "rorl $1, %[val]\n\t"
                : [val] "+r" (results[i % 4])
                :
                : "cc"
            );
        } else {
            /* Different asm on different path */
            asm volatile (
                "roll $1, %[val]\n\t"
                : [val] "+r" (results[i % 4])
                :
                : "cc"
            );
        }
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Parse command line for iteration counts */
    int iterations = 100;
    int test_mode = 1;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) test_mode = atoi(argv[2]);
    
    /* Initialize test data arrays */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int *results1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *results2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_results = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    /* Fill with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 0.5;
        results1[i] = 0;
        results2[i] = 0;
        double_results[i] = 0.0;
    }
    
    /* Execute test functions based on mode */
    switch (test_mode) {
        case 1:
            test_primary_reloads(iterations, int_data, double_data, 
                                results1, double_results);
            break;
        case 2:
            test_secondary_reloads(int_data, results1);
            break;
        case 3:
            test_optional_reloads(int_data, iterations < ARRAY_SIZE ? 
                                 iterations : ARRAY_SIZE, results1);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations / 3, int_data, double_data, 
                                results1, double_results);
            test_secondary_reloads(int_data, results2);
            test_optional_reloads(int_data, iterations / 10, results1);
            break;
    }
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += results1[i];
        checksum += results2[i];
        checksum += (unsigned long long)(double_results[i] * 1000);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(results1);
    free(results2);
    free(double_results);
    
    return 0;
}
