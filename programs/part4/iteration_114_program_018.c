/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function 1: Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    volatile int i, j, k; /* volatile to prevent optimization */
    int a, b, c, d, e, f, g, h;
    double da, db, dc, dd, de, df, dg, dh;
    __m128i v1, v2, v3, v4;
    __m256d vd1, vd2, vd3, vd4;
    
    /* Initialize many live variables to create register pressure */
    a = input[0]; b = input[1]; c = input[2]; d = input[3];
    e = input[4]; f = input[5]; g = input[6]; h = input[7];
    da = dinput[0]; db = dinput[1]; dc = dinput[2]; dd = dinput[3];
    de = dinput[4]; df = dinput[5]; dg = dinput[6]; dh = dinput[7];
    
    v1 = _mm_set_epi32(a, b, c, d);
    v2 = _mm_set_epi32(e, f, g, h);
    vd1 = _mm256_set_pd(da, db, dc, dd);
    vd2 = _mm256_set_pd(de, df, dg, dh);
    
    for (i = 0; i < iterations; i++) {
        /* Complex asm with multiple operands and mixed constraints */
        __asm__ volatile (
            /* Output operands with different constraints */
            "=r" (a),     /* general register */
            "=&r" (b),    /* earlyclobber general register */
            "=q" (c),     /* byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (d),     /* accumulator */
            "=d" (e),     /* data register */
            "=t" (da),    /* top of FPU stack */
            "=r" (db),    /* general register for double */
            
            /* Input operands with mixed constraints */
            : "0" (a),    /* matching constraint - same as output 0 */
              "r" (b), 
              "m" (input[i % ARRAY_SIZE]),  /* memory operand */
              "i" (12345),                   /* immediate */
              "r" (c),
              "g" (d),     /* general (register or memory) */
              "rm" (da),   /* register or memory - may need secondary reload */
              "r" (db),
              
            /* Clobber list */
            : "memory", "cc", 
              "xmm0", "xmm1", "xmm2", "xmm3",  /* SSE registers */
              "ymm0", "ymm1", "ymm2", "ymm3",  /* AVX registers */
              "st", "st(1)", "st(2)", "st(3)"  /* FPU stack */
        );
        
        /* Second asm block to prevent combining */
        __asm__ volatile ("" ::: "memory");
        
        /* Another complex asm with different constraints */
        __asm__ volatile (
            "movl %[imm], %%eax\n\t"
            "addl %%eax, %[out1]\n\t"
            "imull %[in1], %[out2]\n\t"
            "movq %[din], %%xmm0\n\t"
            "addsd %%xmm0, %[dout]"
            
            : [out1] "=r" (f), 
              [out2] "=r" (g),
              [dout] "=t" (dc)
            
            : [imm] "i" (54321),
              [in1] "r" (h),
              [din] "m" (dinput[i % ARRAY_SIZE])
            
            : "eax", "xmm0", "cc"
        );
        
        /* Unrolled computation to increase register pressure */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            k = i + j;
            /* Force many live values */
            __asm__ volatile (
                "addl %[val1], %[res1]\n\t"
                "subl %[val2], %[res2]\n\t"
                "xorl %[val3], %[res3]"
                
                : [res1] "+r" (a),
                  [res2] "+r" (b),
                  [res3] "+r" (c)
                
                : [val1] "r" (d),
                  [val2] "r" (e),
                  [val3] "r" (f)
                
                : "cc"
            );
            
            /* Use vector intrinsics alongside scalar operations */
            v3 = _mm_add_epi32(v1, v2);
            v4 = _mm_sub_epi32(v1, v2);
            vd3 = _mm256_add_pd(vd1, vd2);
            vd4 = _mm256_sub_pd(vd1, vd2);
            
            /* Mix vector and scalar results */
            int vtemp[4];
            _mm_storeu_si128((__m128i*)vtemp, v3);
            g += vtemp[0] + vtemp[1] + vtemp[2] + vtemp[3];
        }
        
        /* Store results to prevent dead code elimination */
        output[i % ARRAY_SIZE] = a + b + c + d + e + f + g + h;
        doutput[i % ARRAY_SIZE] = da + db + dc + dd + de + df + dg + dh;
    }
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations, long long *linput, long long *loutput) {
    int i;
    long long la, lb, lc, ld;
    int ia, ib, ic, id;
    
    for (i = 0; i < iterations; i++) {
        la = linput[i % ARRAY_SIZE];
        lb = linput[(i + 1) % ARRAY_SIZE];
        lc = linput[(i + 2) % ARRAY_SIZE];
        ld = linput[(i + 3) % ARRAY_SIZE];
        
        /* Force secondary reloads with mismatched constraints */
        __asm__ volatile (
            /* "R" constraint for legacy register - may need secondary reload 
               if allocated to R8-R15 */
            "movq %[in1], %%rax\n\t"
            "addq %%rax, %[out1]\n\t"
            "movq %[in2], %%rbx\n\t"
            "subq %%rbx, %[out2]"
            
            : [out1] "=R" (la),  /* Legacy register constraint */
              [out2] "=R" (lb)
            
            : [in1] "r" (lc),
              [in2] "r" (ld),
              "0" (la)  /* Matching constraint */
            
            : "rax", "rbx", "cc"
        );
        
        /* Mix operand types requiring register moves */
        ia = (int)la;
        ib = (int)lb;
        
        __asm__ volatile (
            /* "a" constraint for input, "b" for output - forces move */
            "movl %%eax, %%ebx\n\t"
            "addl %[val], %%ebx"
            
            : "=b" (ic)
            
            : "a" (ia),
              [val] "r" (ib)
            
            : "cc"
        );
        
        /* Memory operand with "rm" constraint that may need secondary reload */
        __asm__ volatile (
            "imull %[mem], %%eax"
            
            : "+a" (id)
            
            : [mem] "rm" (loutput[i % ARRAY_SIZE])  /* May be in memory */
            
            : "cc"
        );
        
        loutput[i % ARRAY_SIZE] = la + lb + ic + id;
    }
}

/* Test function 3: Optional reloads and non-combine patterns */
void test_optional_reloads(int iterations, float *finput, float *foutput) {
    int i;
    float fa, fb, fc, fd;
    int opt1, opt2;
    
    for (i = 0; i < iterations; i++) {
        fa = finput[i % ARRAY_SIZE];
        fb = finput[(i + 1) % ARRAY_SIZE];
        fc = finput[(i + 2) % ARRAY_SIZE];
        fd = finput[(i + 3) % ARRAY_SIZE];
        
        /* Optional constraint with '?' modifier */
        __asm__ volatile (
            "addss %[in1], %[out1]\n\t"
            "mulss %[in2], %[out2]"
            
            : [out1] "=r" (fa),
              [out2] "=?r" (fb)  /* Optional output */
            
            : [in1] "r" (fc),
              [in2] "r" (fd),
              "0" (fa)
            
            : "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to prevent combining */
        __asm__ volatile (
            "subss %[in1], %[out1]\n\t"
            "divss %[in2], %[out2]"
            
            : [out1] "=r" (fc),
              [out2] "=r" (fd)
            
            : [in1] "r" (fa),
              [in2] "r" (fb)
            
            : "cc", "xmm0", "xmm1"  /* Different clobber list */
        );
        
        /* Force optional reload to be used */
        opt1 = (int)fa;
        opt2 = (int)fb;
        
        __asm__ volatile (
            "cmpl %[val1], %[val2]\n\t"
            "setg %[out]"
            
            : [out] "=r" (opt1)
            
            : [val1] "r" (opt1),
              [val2] "r" (opt2)
            
            : "cc"
        );
        
        foutput[i % ARRAY_SIZE] = fa + fb + fc + fd + opt1;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int iterations, int *input, int *output) {
    int i, a, b, c, d;
    
    for (i = 0; i < iterations; i++) {
        a = input[i % ARRAY_SIZE];
        b = input[(i + 1) % ARRAY_SIZE];
        c = input[(i + 2) % ARRAY_SIZE];
        d = input[(i + 3) % ARRAY_SIZE];
        
        /* Different asm blocks on different control flow paths */
        if (mode & 1) {
            __asm__ volatile (
                "xorl %%ecx, %%ecx\n\t"
                "addl %[in1], %[out1]\n\t"
                "adcl %[in2], %[out2]"
                
                : [out1] "=r" (a),
                  [out2] "=r" (b)
                
                : [in1] "r" (c),
                  [in2] "r" (d),
                  "0" (a)
                
                : "ecx", "cc"
            );
        } else {
            __asm__ volatile (
                "movl $1, %%ecx\n\t"
                "subl %[in1], %[out1]\n\t"
                "sbbl %[in2], %[out2]"
                
                : [out1] "=r" (a),
                  [out2] "=r" (b)
                
                : [in1] "r" (c),
                  [in2] "r" (d),
                  "0" (a)
                
                : "ecx", "cc"
            );
        }
        
        /* Loop-dependent asm */
        for (int j = 0; j < (mode & 3); j++) {
            __asm__ volatile (
                "roll $1, %[val]"
                
                : [val] "+r" (c)
                
                :: "cc"
            );
        }
        
        output[i % ARRAY_SIZE] = a + b + c + d;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > 1000000) iterations = 1000000;
    
    /* Allocate and initialize arrays */
    int *input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *dinput = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *doutput = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long long *linput = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    long long *loutput = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    float *finput = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *foutput = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!input || !output || !dinput || !doutput || 
        !linput || !loutput || !finput || !foutput) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mixed data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = i * 3 + 1;
        output[i] = 0;
        dinput[i] = i * 0.5 + 1.0;
        doutput[i] = 0.0;
        linput[i] = (long long)i * 1000000LL + 123456LL;
        loutput[i] = 0LL;
        finput[i] = i * 0.25f + 0.5f;
        foutput[i] = 0.0f;
    }
    
    printf("Starting reload tests with %d iterations, mode %d\n", iterations, mode);
    
    /* Execute all test functions to trigger various reload patterns */
    test_primary_reloads(iterations, input, output, dinput, doutput);
    test_secondary_reloads(iterations / 2, linput, loutput);
    test_optional_reloads(iterations / 2, finput, foutput);
    test_control_flow_reloads(mode, iterations / 4, input, output);
    
    /* Compute checksum to ensure all asm blocks executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (unsigned long long)doutput[i];
        checksum += loutput[i];
        checksum += (unsigned long long)foutput[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(input);
    free(output);
    free(dinput);
    free(doutput);
    free(linput);
    free(loutput);
    free(finput);
    free(foutput);
    
    return 0;
}
