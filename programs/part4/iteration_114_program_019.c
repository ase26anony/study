#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function 1: Primary reloads with register pressure */
void test_primary_reloads(int iterations, int* in_ints, double* in_doubles, 
                         int* out_ints, double* out_doubles) {
    volatile int a = in_ints[0];
    volatile int b = in_ints[1];
    volatile int c = in_ints[2];
    volatile int d = in_ints[3];
    volatile int e = in_ints[4];
    volatile int f = in_ints[5];
    volatile int g = in_ints[6];
    volatile int h = in_ints[7];
    
    volatile double da = in_doubles[0];
    volatile double db = in_doubles[1];
    volatile double dc = in_doubles[2];
    volatile double dd = in_doubles[3];
    
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, f, g, h);
    __m256d vd1 = _mm256_set_pd(da, db, dc, dd);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with many operands and mixed constraints */
        int r1, r2, r3, r4, r5;
        double d1, d2;
        
        /* Force register allocation with earlyclobber and matching constraints */
        __asm__ volatile (
            "movl %[imm], %%eax\n\t"
            "addl %%eax, %[r1]\n\t"
            "imull %[r2], %[r1]\n\t"
            "movq %[mem1], %%mm0\n\t"
            "paddd %%mm0, %%mm0\n\t"
            "movq %%mm0, %[r3]\n\t"
            : [r1] "=&r" (r1), [r3] "=r" (r3), "=a" (r4), "=d" (r5), "=t" (d1)
            : [r2] "0" (a + i), [imm] "i" (12345), [mem1] "m" (in_ints[i % ARRAY_SIZE]),
              "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h),
              "x" (vd1)
            : "mm0", "cc", "memory"
        );
        
        /* Another asm with different register classes */
        __asm__ volatile (
            "mov %[in_a], %%al\n\t"
            "mov %[in_b], %%bl\n\t"
            "add %%bl, %%al\n\t"
            "mov %%al, %[out_q]\n\t"
            "fldl %[in_dbl]\n\t"
            "fstpl %[out_dbl]\n\t"
            : [out_q] "=q" (r2), [out_dbl] "=m" (d2)
            : [in_a] "a" (r1 & 0xFF), [in_b] "b" ((r3 >> 8) & 0xFF),
              [in_dbl] "m" (in_doubles[i % ARRAY_SIZE])
            : "cc"
        );
        
        /* Unrolled section to increase register pressure */
        #pragma unroll(UNROLL_FACTOR)
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            int temp1 = a + b + c + d + e + f + g + h + j;
            double temp2 = da + db + dc + dd + j;
            
            /* Memory barrier to prevent combining */
            __asm__ volatile ("" ::: "memory");
            
            __asm__ volatile (
                "lea (%[idx], %[base], 4), %[res]\n\t"
                : [res] "=r" (temp1)
                : [base] "r" (temp1), [idx] "r" (j)
                : "cc"
            );
            
            out_ints[(i * UNROLL_FACTOR + j) % ARRAY_SIZE] = temp1;
            out_doubles[(i * UNROLL_FACTOR + j) % ARRAY_SIZE] = temp2;
        }
        
        /* Update live variables to keep them in use */
        a = r1 ^ r2;
        b = r3 + i;
        da = d1 + d2;
    }
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles) {
    for (int i = 0; i < iterations; i++) {
        int r1, r2, r3;
        double d1;
        
        /* Force secondary reload by using 'R' constraint with potential R8-R15 allocation */
        __asm__ volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            : [out1] "=R" (r1)  /* Legacy register constraint */
            : [in1] "r" (in_ints[i]), [in2] "r" (in_ints[i + 1])
            : "eax", "cc"
        );
        
        /* Mismatched constraints requiring secondary reload */
        __asm__ volatile (
            "movd %[vec_in], %%eax\n\t"
            "cvtsi2sd %%eax, %%xmm0\n\t"
            "movsd %%xmm0, %[dbl_out]\n\t"
            : [dbl_out] "=rm" (d1)  /* Can be reg or mem, but instruction needs reg */
            : [vec_in] "x" (_mm_set1_epi32(in_ints[i]))
            : "xmm0", "eax", "cc"
        );
        
        /* Accumulator constraint followed by different register class */
        int acc_val;
        __asm__ volatile (
            "mov %[in], %%eax\n\t"
            "imul %[mul], %%eax\n\t"
            : "=a" (acc_val)
            : [in] "a" (r1), [mul] "r" (i)
            : "cc"
        );
        
        /* Now force move from accumulator to base register */
        __asm__ volatile (
            "xchg %%eax, %%ebx\n\t"
            "add $1, %%ebx\n\t"
            "xchg %%ebx, %%eax\n\t"
            : "=b" (r2)
            : "a" (acc_val)
            : "cc"
        );
        
        /* Optional constraint with '?' modifier */
        __asm__ volatile (
            "test %[opt], %[opt]\n\t"
            "jz 1f\n\t"
            "mov %[opt], %[out_opt]\n\t"
            "1:\n\t"
            : [out_opt] "=?r" (r3)  /* Optional output */
            : [opt] "r" (i & 1)
            : "cc"
        );
        
        out_ints[i % ARRAY_SIZE] = r1 + r2 + r3;
        out_doubles[i % ARRAY_SIZE] = d1;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles) {
    volatile int barrier = 0;
    
    for (int i = 0; i < iterations; i++) {
        int r1, r2, r3;
        double d1, d2;
        
        /* First asm with specific clobbers */
        __asm__ volatile (
            "mov %[in1], %%ecx\n\t"
            "rol $8, %%ecx\n\t"
            "mov %%ecx, %[out1]\n\t"
            : [out1] "=r" (r1)
            : [in1] "rm" (in_ints[i])
            : "ecx", "cc"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobber list - won't combine */
        __asm__ volatile (
            "mov %[in1], %%edx\n\t"
            "ror $8, %%edx\n\t"
            "mov %%edx, %[out1]\n\t"
            : [out1] "=r" (r2)
            : [in1] "rm" (in_ints[i + 1])
            : "edx", "cc"
        );
        
        /* Volatile asm with optional constraints */
        __asm__ volatile (
            "cmpl $0, %[cond]\n\t"
            "setne %%al\n\t"
            "movzbl %%al, %[out]\n\t"
            : [out] "=?r" (r3)  /* Optional output */
            : [cond] "r" (i % 3)
            : "eax", "cc"
        );
        
        /* Multiple outputs with earlyclobber to prevent sharing */
        __asm__ volatile (
            "movq %[in_dbl], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out1]\n\t"
            "movsd %%xmm0, %[out2]\n\t"
            : [out1] "=&r" (d1), [out2] "=&r" (d2)
            : [in_dbl] "m" (in_doubles[i % ARRAY_SIZE])
            : "xmm0"
        );
        
        out_ints[i % ARRAY_SIZE] = r1 ^ r2 ^ r3;
        out_doubles[i % ARRAY_SIZE] = d1 + d2;
        
        /* Control flow dependent asm */
        if (i % 5 == 0) {
            __asm__ volatile (
                "bsrl %[in], %[out]\n\t"
                : [out] "=r" (barrier)
                : [in] "r" (i)
                : "cc"
            );
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        printf("  iterations: Number of loop iterations (e.g., 100)\n");
        printf("  mode: 1=primary, 2=secondary, 3=optional, 4=all\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (mode < 1 || mode > 4) mode = 4;
    
    /* Initialize data arrays */
    int* in_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* in_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* out_ints1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* out_ints2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* out_ints3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* out_doubles1 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* out_doubles2 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* out_doubles3 = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    /* Fill with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 7;
        in_doubles[i] = i * 0.5 + 1.25;
        out_ints1[i] = out_ints2[i] = out_ints3[i] = 0;
        out_doubles1[i] = out_doubles2[i] = out_doubles3[i] = 0.0;
    }
    
    /* Execute based on mode */
    if (mode == 1 || mode == 4) {
        test_primary_reloads(iterations, in_ints, in_doubles, out_ints1, out_doubles1);
    }
    if (mode == 2 || mode == 4) {
        test_secondary_reloads(iterations, in_ints, in_doubles, out_ints2, out_doubles2);
    }
    if (mode == 3 || mode == 4) {
        test_optional_reloads(iterations, in_ints, in_doubles, out_ints3, out_doubles3);
    }
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += out_ints1[i] + out_ints2[i] + out_ints3[i];
        checksum += (unsigned long long)(out_doubles1[i] * 1000);
        checksum += (unsigned long long)(out_doubles2[i] * 1000);
        checksum += (unsigned long long)(out_doubles3[i] * 1000);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(in_ints);
    free(in_doubles);
    free(out_ints1);
    free(out_ints2);
    free(out_ints3);
    free(out_doubles1);
    free(out_doubles2);
    free(out_doubles3);
    
    return 0;
}
