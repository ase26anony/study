/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, long *checksum);
void test_secondary_reloads(int iterations, double *in, double *out, long *checksum);
void test_optional_reloads(int iterations, float *in, float *out, long *checksum);
void test_control_flow_reloads(int iterations, int mode, int *in, int *out, long *checksum);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vec128;
volatile __m256d global_vec256;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    if (argc >= 2) iterations = atoi(argv[1]);
    if (argc >= 3) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5 + 0.25;
        float_array[i] = i * 0.75f + 0.125f;
    }
    
    long total_checksum = 0;
    
    /* Execute test functions with heavy register pressure */
    test_primary_reloads(iterations, int_array, int_out, &total_checksum);
    test_secondary_reloads(iterations, double_array, double_out, &total_checksum);
    test_optional_reloads(iterations, float_array, float_out, &total_checksum);
    test_control_flow_reloads(iterations, mode, int_array, int_out, &total_checksum);
    
    printf("Final checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    free(int_array);
    free(int_out);
    free(double_array);
    free(double_out);
    free(float_array);
    free(float_out);
    
    return 0;
}

/* Primary reloads with many operands and mixed constraints */
void test_primary_reloads(int iterations, int *in, int *out, long *checksum) {
    /* Many live variables to exhaust registers */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    long sum = 0;
    
    /* Initialize many variables to create register pressure */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    i = 9; j = 10; k = 11; l = 12; m = 13; n = 14; o = 15; p = 16;
    
    /* Unrolled loop with complex inline assembly */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex asm with 8+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (tmp1),     /* General register */
            "=&r" (tmp2),    /* Early clobber */
            "=q" (tmp3),     /* Byte register (a,b,c,d) */
            "=a" (tmp4),     /* Accumulator */
            "=d" (tmp5),     /* Data register */
            "=r" (tmp6),
            "=t" (tmp7),     /* Top of FP stack */
            "=m" (tmp8)      /* Memory */
            :
            /* Inputs with mixed constraints */
            "r" (a),         /* Register */
            "i" (123),       /* Immediate */
            "m" (*in),       /* Memory */
            "r" (b),
            "0" (c),         /* Matching constraint with output 0 */
            "g" (d),         /* General (register or memory) */
            "r" (e),
            "rm" (f)         /* Register or memory */
            :
            /* Clobbers - many registers to force spills */
            "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
            "cc", "memory"
        );
        
        /* Update many live variables */
        a += tmp1; b += tmp2; c += tmp3; d += tmp4;
        e += tmp5; f += tmp6; g += tmp7; h += tmp8;
        i = a * b; j = c * d; k = e * f; l = g * h;
        m = i + j; n = k + l; o = m * n; p = o >> 2;
        
        /* Another asm with vector operands to increase pressure */
        __m128i v1 = _mm_set_epi32(a, b, c, d);
        __m128i v2 = _mm_set_epi32(e, f, g, h);
        __m128i v3;
        
        __asm__ volatile (
            "movdqa %1, %0\n\t"
            "paddd %2, %0\n\t"
            : "=x" (v3)      /* XMM register constraint */
            : "xm" (v1),     /* XMM register or memory */
              "xm" (v2)
            : "xmm0", "xmm1"
        );
        
        /* Use results to prevent optimization */
        int idx = (iter * 17) % ARRAY_SIZE;
        out[idx] = p + _mm_extract_epi32(v3, 0);
        sum += out[idx];
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
    }
    
    *checksum += sum;
}

/* Secondary reload patterns with mismatched constraints */
void test_secondary_reloads(int iterations, double *in, double *out, long *checksum) {
    double d1, d2, d3, d4, d5, d6, d7, d8;
    double result1, result2, result3, result4;
    long sum = 0;
    
    /* Initialize with array values */
    d1 = in[0]; d2 = in[1]; d3 = in[2]; d4 = in[3];
    d5 = in[4]; d6 = in[5]; d7 = in[6]; d8 = in[7];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Force secondary reload by using "a" constraint then "b" constraint */
        int tmp_a, tmp_b;
        
        /* First asm uses accumulator */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "addl $100, %%eax\n\t"
            : "=a" (tmp_a)
            : "r" (iter)
            : "eax"
        );
        
        /* Second asm requires base register, forcing move from eax to ebx */
        __asm__ volatile (
            "movl %%ebx, %0\n\t"
            : "=r" (tmp_b)
            : "b" (tmp_a)    /* Constrained to ebx */
            : "ebx"
        );
        
        /* Complex FP asm with memory constraints that may need secondary reloads */
        __asm__ volatile (
            /* Outputs with FP constraints */
            "=t" (result1),   /* Top of FP stack */
            "=u" (result2),   /* Second FP stack */
            "=f" (result3),   /* Any FP register */
            "=m" (result4)    /* Memory */
            :
            /* Inputs that may require secondary reloads */
            "f" (d1),         /* FP register */
            "m" (d2),         /* Memory - may need reload if in register */
            "g" (d3),         /* General - could be memory needing reload */
            "t" (d4),         /* Top of stack */
            "u" (d5)          /* Second stack */
            :
            /* Clobber FP stack to force reg moves */
            "st(6)", "st(7)"
        );
        
        /* Use legacy register constraints that may need secondary moves on x86-64 */
        long legacy_val;
        __asm__ volatile (
            "mov %1, %%rax\n\t"
            "shl $2, %%rax\n\t"
            : "=R" (legacy_val)  /* Legacy register (eax, ebx, ecx, edx) */
            : "r" (tmp_b)
            : "rax"
        );
        
        /* Update values and compute output */
        d1 = result1 * 0.9;
        d2 = result2 * 1.1;
        d3 = result3 * 0.8;
        d4 = result4 * 1.2;
        
        int idx = (iter * 13) % ARRAY_SIZE;
        out[idx] = d1 + d2 + d3 + d4 + legacy_val;
        sum += (long)out[idx];
        
        /* Rotate values to change allocation patterns */
        double temp = d1;
        d1 = d2; d2 = d3; d3 = d4; d4 = d5;
        d5 = d6; d6 = d7; d7 = d8; d8 = temp;
    }
    
    *checksum += sum;
}

/* Optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, float *in, float *out, long *checksum) {
    float f1, f2, f3, f4, f5, f6, f7, f8;
    float results[8];
    long sum = 0;
    
    f1 = in[0]; f2 = in[1]; f3 = in[2]; f4 = in[3];
    f5 = in[4]; f6 = in[5]; f7 = in[6]; f8 = in[7];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Use optional constraints with '?' modifier */
        float opt1, opt2, opt3;
        
        __asm__ volatile (
            "movss %2, %0\n\t"
            "addss %3, %0\n\t"
            "movss %0, %1\n\t"
            : "=?r" (opt1),   /* Optional output */
              "=?m" (opt2)    /* Optional memory output */
            : "r" (f1),
              "rm" (f2)
            : "xmm0"
        );
        
        /* Another asm that could be combined but has different clobbers */
        __asm__ volatile (
            "mulss %1, %0\n\t"
            : "+?r" (opt1)    /* Optional read-write */
            : "rm" (f3)
            : "xmm1"          /* Different clobber prevents combination */
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm with volatile to prevent combination */
        __asm__ volatile (
            "subss %1, %0\n\t"
            : "+r" (opt1)
            : "rm" (f4)
            : "xmm0"          /* Same clobber but volatile prevents combine */
        );
        
        /* Complex asm with multiple optional outputs */
        __asm__ volatile (
            "movaps %4, %0\n\t"
            "movaps %5, %1\n\t"
            "addps %0, %1\n\t"
            "movhlps %1, %2\n\t"
            "movaps %1, %3\n\t"
            : "=?x" (results[0]),  /* Optional XMM */
              "=?x" (results[1]),
              "=?x" (results[2]),
              "=?m" (results[3])   /* Optional memory */
            : "x" (_mm_set_ps(f1, f2, f3, f4)),
              "x" (_mm_set_ps(f5, f6, f7, f8))
            : "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Use results */
        int idx = (iter * 11) % ARRAY_SIZE;
        out[idx] = results[0] + results[1] + results[2] + results[3] + opt1 + opt2;
        sum += (long)out[idx];
        
        /* Update variables to change allocation */
        f1 += 0.1f; f2 += 0.2f; f3 += 0.3f; f4 += 0.4f;
        f5 += 0.5f; f6 += 0.6f; f7 += 0.7f; f8 += 0.8f;
        
        /* Force spills by using many variables */
        global_counter += iter;
        global_double *= 1.001;
    }
    
    *checksum += sum;
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int iterations, int mode, int *in, int *out, long *checksum) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x = 0, y = 0, z = 0;
    long sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Different asm blocks in different control flow paths */
        if (iter % 3 == 0) {
            /* Path 1: Register-heavy asm */
            __asm__ volatile (
                "imull %1, %0\n\t"
                "addl %2, %0\n\t"
                : "+r" (a)
                : "r" (b), "i" (100)
                : "cc"
            );
            x = a;
        } else if (iter % 3 == 1) {
            /* Path 2: Memory-heavy asm */
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "subl %2, %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=m" (out[iter % ARRAY_SIZE])
                : "r" (c), "m" (in[iter % ARRAY_SIZE])
                : "eax", "cc", "memory"
            );
            x = out[iter % ARRAY_SIZE];
        } else {
            /* Path 3: Mixed constraints */
            __asm__ volatile (
                "leal (%1, %2, 2), %0\n\t"
                : "=r" (x)
                : "r" (d), "r" (e)
                : "cc"
            );
        }
        
        /* Nested loop with asm to increase pressure */
        for (int j = 0; j < (mode % 5); j++) {
            int temp;
            __asm__ volatile (
                "movl %1, %0\n\t"
                "rorl $4, %0\n\t"
                : "=r" (temp)
                : "r" (x + j)
                : "cc"
            );
            y += temp;
        }
        
        /* Conditional asm based on runtime value */
        if (x > y) {
            __asm__ volatile (
                "cmpl %1, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0\n\t"
                : "+r" (z)
                : "r" (y)
                : "eax", "cc"
            );
        } else {
            __asm__ volatile (
                "xorl %0, %0\n\t"
                : "=r" (z)
                :
                : "cc"
            );
        }
        
        /* Complex asm in loop with many live values */
        int r1, r2, r3;
        __asm__ volatile (
            "movl %4, %0\n\t"
            "movl %5, %1\n\t"
            "addl %0, %1\n\t"
            "imull %6, %1\n\t"
            "movl %1, %2\n\t"
            : "=&r" (r1),  /* Early clobber */
              "=r" (r2),
              "=m" (r3)
            : "0" (a),     /* Matching constraint */
              "r" (b),
              "r" (c),
              "i" (mode)   /* Immediate from runtime */
            : "cc"
        );
        
        /* Update checksum */
        sum += x + y + z + r1 + r2 + r3;
        
        /* Rotate values for next iteration */
        int temp = a;
        a = b; b = c; c = d; d = e; e = temp + iter;
        
        /* Use global to prevent optimization */
        global_counter += z;
    }
    
    *checksum += sum;
}
