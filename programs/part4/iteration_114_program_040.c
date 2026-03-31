/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions targeting different reload scenarios */
void test_primary_reloads(int iterations, int* in_ints, double* in_doubles, 
                         int* out_ints, double* out_doubles);
void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles);
void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles);

/* Helper to create register pressure */
static inline void create_register_pressure(int* restrict a, int* restrict b, 
                                           int* restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Many live variables to exhaust registers */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        int t3 = c[i] - 5;
        int t4 = t1 ^ t2;
        int t5 = t3 & t4;
        int t6 = t2 | t1;
        int t7 = t4 + t5;
        int t8 = t6 - t7;
        int t9 = t8 * 2;
        int t10 = t9 / 3;
        int t11 = t10 << 2;
        int t12 = t11 >> 1;
        int t13 = t12 + t3;
        int t14 = t13 ^ t5;
        int t15 = t14 & t7;
        int t16 = t15 | t9;
        int t17 = t16 - t11;
        int t18 = t17 * t13;
        int t19 = t18 / t14;
        int t20 = t19 << t15;
        
        a[i] = t20;
        b[i] = t17;
        c[i] = t19;
    }
}

void test_primary_reloads(int iterations, int* in_ints, double* in_doubles,
                         int* out_ints, double* out_doubles) {
    /* Create many live variables to pressure registers */
    int live1 = in_ints[0];
    int live2 = in_ints[1];
    int live3 = in_ints[2];
    int live4 = in_ints[3];
    int live5 = in_ints[4];
    int live6 = in_ints[5];
    int live7 = in_ints[6];
    int live8 = in_ints[7];
    double dlive1 = in_doubles[0];
    double dlive2 = in_doubles[1];
    double dlive3 = in_doubles[2];
    double dlive4 = in_doubles[3];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex inline asm with multiple operands and constraints */
        __asm__ volatile (
            /* Output operands with different constraints */
            "=r" (out_ints[iter*8 + 0]),   /* General register */
            "=q" (out_ints[iter*8 + 1]),   /* Byte-addressable register */
            "=r" (out_ints[iter*8 + 2]),
            "=a" (out_ints[iter*8 + 3]),   /* Accumulator */
            "=d" (out_ints[iter*8 + 4]),   /* Data register */
            
            /* Input operands with mixed constraints */
            "r"  (live1),                  /* Register */
            "rm" (live2),                  /* Register or memory */
            "i"  (0x7FFFFFFF),             /* Immediate */
            "g"  (live3),                  /* General (register, memory, or immediate) */
            "m"  (in_ints[iter]),          /* Memory */
            
            /* Early clobber output */
            "=&r" (out_ints[iter*8 + 5]),
            
            /* Matching constraint tying input to output */
            "0" (live4),
            
            /* Additional constraints for mode mixing */
            "=t" (out_doubles[iter*2]),    /* Top of FP stack */
            "=u" (out_doubles[iter*2 + 1]), /* Second FP stack */
            
            "r" (live5),
            "m" (in_doubles[iter])
            
            : /* No explicit clobbers here, but will add in nested */
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Another asm with different constraints */
        int temp1, temp2, temp3;
        __asm__ volatile (
            "movl %[in1], %[out1]\n\t"
            "addl %[in2], %[out1]\n\t"
            "imull %[in3], %[out1]\n\t"
            "movl %[out1], %[out2]\n\t"
            "xorl %[in4], %[out2]\n\t"
            "movl %[out2], %[out3]"
            : [out1] "=&r" (temp1),
              [out2] "=r" (temp2),
              [out3] "=r" (temp3)
            : [in1] "rm" (live6),
              [in2] "rm" (live7),
              [in3] "rm" (live8),
              [in4] "rm" (in_ints[iter + 1])
            : "cc"
        );
        
        /* Update live variables to prevent optimization */
        live1 = temp1;
        live2 = temp2;
        live3 = temp3;
        live4 = live1 ^ live2;
        live5 = live3 + live4;
        
        /* Use vector intrinsics to increase register pressure */
        if (iter % 4 == 0) {
            __m128i v1 = _mm_set_epi32(live1, live2, live3, live4);
            __m128i v2 = _mm_set_epi32(live5, live6, live7, live8);
            __m128i v3 = _mm_add_epi32(v1, v2);
            __m128i v4 = _mm_mullo_epi32(v3, v1);
            
            int result[4];
            _mm_storeu_si128((__m128i*)result, v4);
            
            live6 = result[0];
            live7 = result[1];
            live8 = result[2];
        }
    }
}

void test_secondary_reloads(int iterations, int* in_ints, double* in_doubles,
                           int* out_ints, double* out_doubles) {
    /* Force secondary reloads by using mismatched constraints */
    
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % 256;
        
        /* asm requiring specific register classes */
        int result1, result2;
        
        /* Force use of 'a' (accumulator) register */
        __asm__ volatile (
            "movl %%eax, %0\n\t"
            "addl $1, %%eax"
            : "=a" (result1)
            : "a" (in_ints[idx])
            : "cc"
        );
        
        /* Now use result1 in 'b' (base) register constraint */
        __asm__ volatile (
            "xchgl %%ebx, %0\n\t"
            "addl %%ebx, %1"
            : "+b" (result1), "=r" (result2)
            : "1" (in_ints[idx + 1])
            : "cc"
        );
        
        /* Force memory operand that needs register for addressing */
        __asm__ volatile (
            "leal (%[base], %[index], 4), %[out]"
            : [out] "=r" (out_ints[iter])
            : [base] "r" (in_ints),
              [index] "r" (idx)
            : "cc"
        );
        
        /* Complex asm with multiple constraints that may need secondary reloads */
        double dtemp;
        __asm__ volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=xm" (dtemp)  /* Memory with possible offset */
            : [in] "xm" (in_doubles[idx]),
              "[out]" (dtemp)
            : "xmm0", "xmm1"
        );
        
        out_doubles[iter] = dtemp;
        
        /* Create register pressure with many live doubles */
        double dlive1 = in_doubles[(idx + 0) % 256];
        double dlive2 = in_doubles[(idx + 1) % 256];
        double dlive3 = in_doubles[(idx + 2) % 256];
        double dlive4 = in_doubles[(idx + 3) % 256];
        double dlive5 = in_doubles[(idx + 4) % 256];
        double dlive6 = in_doubles[(idx + 5) % 256];
        double dlive7 = in_doubles[(idx + 6) % 256];
        double dlive8 = in_doubles[(idx + 7) % 256];
        
        /* Use them in computation to keep them live */
        dlive1 = dlive1 * dlive2 + dlive3;
        dlive2 = dlive4 - dlive5 * dlive6;
        dlive3 = dlive7 / dlive8 + dlive1;
        dlive4 = dlive2 * dlive3 - dlive5;
        
        /* Store back to prevent optimization */
        out_doubles[iter * 2] = dlive1;
        out_doubles[iter * 2 + 1] = dlive2;
    }
}

void test_optional_reloads(int iterations, int* in_ints, double* in_doubles,
                          int* out_ints, double* out_doubles) {
    /* Test optional reloads and nocombine scenarios */
    
    volatile int barrier = 0;  /* Prevent optimization */
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Use optional constraints */
        int opt1, opt2, opt3;
        
        __asm__ volatile (
            "testl %[val], %[val]\n\t"
            "jz 1f\n\t"
            "movl %[val], %[out1]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out1]\n\t"
            "2:\n\t"
            "addl $1, %[out1]"
            : [out1] "=?r" (opt1)  /* Optional output */
            : [val] "rm" (in_ints[iter])
            : "cc"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to prevent combination */
        __asm__ volatile (
            "testl %[val], %[val]\n\t"
            "jz 1f\n\t"
            "movl %[val], %[out1]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %[out1]\n\t"
            "2:"
            : [out1] "=?r" (opt2)
            : [val] "rm" (in_ints[iter + 1])
            : "cc", "memory"  /* Different clobber list */
        );
        
        /* Another barrier */
        barrier = iter;
        
        /* Third similar asm, volatile to prevent combination */
        __asm__ volatile (
            "movl %[in], %[out]\n\t"
            "incl %[out]"
            : [out] "=r" (opt3)
            : [in] "rm" (in_ints[iter + 2])
        );
        
        /* Complex asm with many optional outputs */
        int opt4, opt5, opt6;
        __asm__ (
            "movl %[a], %[o1]\n\t"
            "addl %[b], %[o1]\n\t"
            "movl %[o1], %[o2]\n\t"
            "subl %[c], %[o2]\n\t"
            "movl %[o2], %[o3]"
            : [o1] "=?r" (opt4),  /* Optional */
              [o2] "=?r" (opt5),  /* Optional */
              [o3] "=r" (opt6)    /* Required */
            : [a] "rm" (in_ints[iter * 3]),
              [b] "rm" (in_ints[iter * 3 + 1]),
              [c] "rm" (in_ints[iter * 3 + 2])
            : "cc"
        );
        
        /* Store results with conditional */
        if (opt1 > 0) {
            out_ints[iter * 4] = opt1;
            out_ints[iter * 4 + 1] = opt2;
        } else {
            out_ints[iter * 4] = opt3;
            out_ints[iter * 4 + 1] = opt4;
        }
        
        out_ints[iter * 4 + 2] = opt5;
        out_ints[iter * 4 + 3] = opt6;
        
        /* Create control flow dependent reloads */
        if (iter % 3 == 0) {
            __asm__ volatile (
                "movl $1, %%eax\n\t"
                "cpuid"
                ::: "eax", "ebx", "ecx", "edx", "memory"
            );
        } else if (iter % 3 == 1) {
            __asm__ volatile (
                "rdtsc"
                : "=a" (out_ints[iter]),
                  "=d" (out_ints[iter + 1])
                :: "memory"
            );
        }
    }
}

int main(int argc, char** argv) {
    /* Parse command line arguments */
    int iterations = 100;
    int test_mode = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        test_mode = atoi(argv[2]);
    }
    
    printf("Running reload test with %d iterations, mode %d\n", 
           iterations, test_mode);
    
    /* Allocate and initialize arrays */
    int* in_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* in_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* out_ints = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double* out_doubles = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    
    if (!in_ints || !in_doubles || !out_ints || !out_doubles) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        in_ints[i] = i * 3 + 1;
        in_doubles[i] = i * 0.5 + 1.0;
        out_ints[i] = 0;
        out_doubles[i] = 0.0;
    }
    
    /* Create initial register pressure */
    int* temp1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* temp2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* temp3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        temp1[i] = in_ints[i] ^ 0xAAAAAAAA;
        temp2[i] = in_ints[i] | 0x55555555;
        temp3[i] = in_ints[i] & 0x33333333;
    }
    
    /* Exhaust registers before tests */
    create_register_pressure(temp1, temp2, temp3, ARRAY_SIZE / 4);
    
    /* Run test functions based on mode */
    switch (test_mode) {
        case 0:
            test_primary_reloads(iterations, in_ints, in_doubles, 
                                out_ints, out_doubles);
            break;
        case 1:
            test_secondary_reloads(iterations, in_ints, in_doubles,
                                  out_ints, out_doubles);
            break;
        case 2:
            test_optional_reloads(iterations, in_ints, in_doubles,
                                 out_ints, out_doubles);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations / 3, in_ints, in_doubles,
                                out_ints, out_doubles);
            test_secondary_reloads(iterations / 3, in_ints, in_doubles,
                                  out_ints, out_doubles);
            test_optional_reloads(iterations / 3, in_ints, in_doubles,
                                 out_ints, out_doubles);
            break;
    }
    
    /* Compute checksum to ensure all asm executed */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)out_ints[i];
        checksum += (uint64_t)(out_doubles[i] * 1000);
    }
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(in_ints);
    free(in_doubles);
    free(out_ints);
    free(out_doubles);
    free(temp1);
    free(temp2);
    free(temp3);
    
    return 0;
}
