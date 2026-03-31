#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function for primary reloads with many operands */
void test_primary_reloads(int iterations, int *in_ints, double *in_doubles, 
                          float *in_floats, long *out_longs, int *out_ints) {
    volatile int i, j, k;
    volatile double d1, d2, d3, d4, d5;
    volatile float f1, f2, f3, f4;
    volatile long l1, l2, l3, l4;
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Create many live variables to increase register pressure */
    v1 = in_ints[0]; v2 = in_ints[1]; v3 = in_ints[2]; v4 = in_ints[3];
    v5 = in_ints[4]; v6 = in_ints[5]; v7 = in_ints[6]; v8 = in_ints[7];
    v9 = in_ints[8]; v10 = in_ints[9];
    
    d1 = in_doubles[0]; d2 = in_doubles[1]; d3 = in_doubles[2]; d4 = in_doubles[3];
    d5 = in_doubles[4];
    
    f1 = in_floats[0]; f2 = in_floats[1]; f3 = in_floats[2]; f4 = in_floats[3];
    
    for (i = 0; i < iterations; i++) {
        /* Complex inline asm with 7 operands, mixing constraints */
        __asm__ volatile (
            /* Outputs with different constraints and modes */
            "=r" (v1),     /* General register */
            "=&r" (v2),    /* Earlyclobber general register */
            "=q" (v3),     /* Byte-addressable register (a,b,c,d) */
            "=a" (v4),     /* Accumulator */
            "=d" (v5),     /* Data register */
            "=t" (d1),     /* Top of FPU stack */
            "=m" (out_ints[i % ARRAY_SIZE])  /* Memory output */
            
            /* Inputs with mixed constraints */
            : "0" (v1),    /* Matching constraint - same as output 0 */
            "r" (v6),      /* General register input */
            "i" (12345),   /* Immediate */
            "m" (in_ints[i % ARRAY_SIZE]),  /* Memory input */
            "r" (v7),
            "g" (v8),      /* General register or memory */
            "rm" (v9)      /* Register or memory */
            
            /* Clobber many registers to force spills */
            : "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
              "cc", "memory"
        );
        
        /* Another asm with different constraints to prevent combining */
        __asm__ volatile ("" ::: "memory");  /* Memory barrier */
        
        __asm__ volatile (
            "=b" (v6),     /* Base register */
            "=c" (v7),     /* Counter register */
            "=S" (v8),     /* Source index */
            "=D" (v9),     /* Destination index */
            "=?r" (v10)    /* Optional output */
            
            : "a" (v4),    /* Must be in accumulator */
            "d" (v5),      /* Must be in data register */
            "b" (v6),      /* Must be in base register */
            "R" (v1),      /* Legacy register constraint */
            "m" (in_doubles[i % ARRAY_SIZE])
            
            : "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Unrolled section to increase register pressure */
        for (j = 0; j < UNROLL_FACTOR; j++) {
            k = i + j;
            __asm__ volatile (
                "=r" (out_ints[(k) % ARRAY_SIZE]),
                "=r" (out_longs[(k) % ARRAY_SIZE])
                
                : "r" (in_ints[(k) % ARRAY_SIZE]),
                "r" (in_doubles[(k) % ARRAY_SIZE]),
                "r" (in_floats[(k) % ARRAY_SIZE]),
                "i" (k),
                "i" (j)
                
                : "cc"
            );
        }
    }
    
    /* Force use of all variables to prevent optimization */
    out_ints[0] = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    out_longs[0] = (long)(d1 + d2 + d3 + d4 + d5 + f1 + f2 + f3 + f4);
}

/* Test function specifically for secondary reloads */
void test_secondary_reloads(int iterations, __m128i *in_vec, __m256d *in_avx,
                            double *out_doubles, int *out_ints) {
    volatile __m128i vec1, vec2, vec3;
    volatile __m256d avx1, avx2;
    volatile int temp1, temp2, temp3;
    volatile double dtemp;
    
    vec1 = in_vec[0];
    vec2 = in_vec[1];
    vec3 = in_vec[2];
    avx1 = in_avx[0];
    avx2 = in_avx[1];
    
    for (int i = 0; i < iterations; i++) {
        /* Force secondary reload by using 'a' constraint then 'b' constraint */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=b" (temp1)        /* Output in base register */
            : "a" (out_ints[i]),  /* Input must be in accumulator */
              "0" (temp1)         /* Matching constraint */
            : "%eax", "cc"
        );
        
        /* Memory operand that might need secondary reload */
        __asm__ volatile (
            "=r" (temp2),
            "=r" (temp3)
            
            : "rm" (out_ints[i]),      /* Register or memory - may need secondary */
            "rm" (out_doubles[i]),     /* Another register/memory operand */
            "i" (256)                  /* Immediate */
            
            : "cc"
        );
        
        /* Use vector types to increase register pressure */
        vec1 = _mm_add_epi32(vec1, vec2);
        avx1 = _mm256_add_pd(avx1, avx2);
        
        /* Another asm with FPU stack constraints */
        __asm__ volatile (
            "fldl %1\n\t"
            "fstpl %0\n\t"
            : "=m" (out_doubles[i])
            : "m" (in_avx[i % 4])
            : "st", "st(1)"
        );
    }
    
    /* Store results */
    _mm_store_si128((__m128i*)&out_ints[0], vec1);
    _mm256_store_pd(&out_doubles[0], avx1);
}

/* Test function for optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, int *data, int *results) {
    volatile int a, b, c, d, e, f;
    
    a = data[0]; b = data[1]; c = data[2];
    d = data[3]; e = data[4]; f = data[5];
    
    for (int i = 0; i < iterations; i++) {
        /* First asm with optional output */
        __asm__ volatile (
            "=r" (a),
            "=?r" (b),     /* Optional output */
            "=r" (c)
            
            : "0" (a),
            "r" (b),
            "r" (c),
            "i" (i)
            
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to force nocombine */
        __asm__ volatile (
            "=r" (d),
            "=?r" (e),     /* Optional output */
            "=r" (f)
            
            : "0" (d),
            "r" (e),
            "r" (f),
            "i" (i)
            
            : "r8", "r9", "cc"  /* Different clobbers than above */
        );
        
        /* Conditional asm to create control-flow dependent reloads */
        if (i % 2 == 0) {
            __asm__ volatile (
                "=a" (results[i])
                : "b" (a),
                "c" (b),
                "d" (c)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "=d" (results[i])
                : "a" (d),
                "b" (e),
                "c" (f)
                : "cc"
            );
        }
    }
    
    /* Use all variables */
    results[0] = a + b + c + d + e + f;
}

int main(int argc, char *argv[]) {
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
    int *in_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *in_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *in_floats = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long *out_longs = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    int *out_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *results = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    __m128i *in_vec = (__m128i*)aligned_alloc(64, 4 * sizeof(__m128i));
    __m256d *in_avx = (__m256d*)aligned_alloc(64, 4 * sizeof(__m256d));
    double *out_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 1;
        in_doubles[i] = i * 1.5;
        in_floats[i] = i * 0.75f;
        out_ints[i] = 0;
        out_longs[i] = 0;
        out_doubles[i] = 0.0;
        results[i] = 0;
    }
    
    for (int i = 0; i < 4; i++) {
        in_vec[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
        in_avx[i] = _mm256_set_pd(i*4+3, i*4+2, i*4+1, i*4);
    }
    
    /* Run tests based on mode */
    switch (mode) {
        case 1:
            test_primary_reloads(iterations, in_ints, in_doubles, in_floats, 
                                out_longs, out_ints);
            break;
        case 2:
            test_secondary_reloads(iterations, in_vec, in_avx, 
                                  out_doubles, out_ints);
            break;
        case 3:
            test_optional_reloads(iterations, in_ints, results);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations/3, in_ints, in_doubles, in_floats,
                                out_longs, out_ints);
            test_secondary_reloads(iterations/3, in_vec, in_avx,
                                  out_doubles, out_ints);
            test_optional_reloads(iterations/3, in_ints, results);
            break;
    }
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += out_ints[i];
        checksum += (unsigned long long)out_longs[i];
        checksum += (unsigned long long)out_doubles[i];
        checksum += results[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Iterations: %d, Mode: %d\n", iterations, mode);
    
    /* Cleanup */
    free(in_ints);
    free(in_doubles);
    free(in_floats);
    free(out_longs);
    free(out_ints);
    free(results);
    free(in_vec);
    free(in_avx);
    free(out_doubles);
    
    return 0;
}
