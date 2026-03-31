/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test function for primary reload patterns */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    volatile int a = input[0];
    volatile int b = input[1];
    volatile int c = input[2];
    volatile int d = input[3];
    volatile int e = input[4];
    volatile int f = input[5];
    volatile int g = input[6];
    volatile int h = input[7];
    
    /* Create many live variables to increase register pressure */
    int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e, v6 = f, v7 = g, v8 = h;
    int v9 = a + b, v10 = c + d, v11 = e + f, v12 = g + h;
    int v13 = a * b, v14 = c * d, v15 = e * f, v16 = g * h;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with multiple constraints to force reloads */
        __asm__ volatile (
            /* Multiple output operands with different constraints */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movb %%al, %[out2]\n\t"
            "movw %%ax, %[out3]\n\t"
            : [out1] "=r" (v1),        /* general register */
              [out2] "=q" (v2),        /* byte register (a,b,c,d) */
              [out3] "=r" (v3)         /* word output */
            : [in1] "rm" (input[i]),   /* register or memory */
              [in2] "i" (0x1234),      /* immediate */
              [in3] "r" (v4)           /* register only */
            : "eax", "cc", "memory"
        );
        
        /* Another asm with earlyclobber and matching constraints */
        int temp1 = v5, temp2 = v6;
        __asm__ volatile (
            "leal (%[src1], %[src2], 2), %[dst1]\n\t"
            "movl %[dst1], %[dst2]\n\t"
            : [dst1] "=&r" (v5),       /* earlyclobber */
              [dst2] "=r" (v6)
            : [src1] "0" (temp1),      /* matching constraint */
              [src2] "r" (temp2)
            : "cc"
        );
        
        /* Mix different register classes */
        __asm__ volatile (
            "movl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            : "=a" (v7), "=b" (v8)
            : "b" (v7), "c" (v8), "d" (v9)
            : "cc"
        );
        
        /* Force different machine modes */
        char byte_out;
        short word_out;
        __asm__ volatile (
            "movb %[in_byte], %%al\n\t"
            "movb %%al, %[out_byte]\n\t"
            "movw %[in_word], %%ax\n\t"
            "movw %%ax, %[out_word]\n\t"
            : [out_byte] "=q" (byte_out),
              [out_word] "=r" (word_out)
            : [in_byte] "q" ((char)v10),
              [in_word] "r" ((short)v11)
            : "ax"
        );
        
        /* Store results to prevent optimization */
        output[i] = v1 + v2 + v3 + byte_out + word_out;
    }
    
    /* Use all variables to prevent dead code elimination */
    output[0] = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, long *input, long *output) {
    /* Create register pressure with many live variables */
    register long r0 asm("rax") = input[0];
    register long r1 asm("rbx") = input[1];
    register long r2 asm("rcx") = input[2];
    register long r3 asm("rdx") = input[3];
    register long r4 asm("rsi") = input[4];
    register long r5 asm("rdi") = input[5];
    
    for (int i = 0; i < iterations; i++) {
        /* Force secondary reload by using specific register constraints
           that may conflict with allocation */
        __asm__ volatile (
            /* Input in 'a' constraint, but we need result in 'b' */
            "movq %[in1], %%rax\n\t"
            "addq %[in2], %%rax\n\t"
            "movq %%rax, %[tmp]\n\t"
            "movq %[tmp], %%rbx\n\t"
            : "=b" (r1), [tmp] "=&r" (r0)
            : [in1] "a" (r0),          /* accumulator constraint */
              [in2] "rm" (input[i])    /* may be in memory -> secondary reload */
            : "cc"
        );
        
        /* Mix legacy and extended registers */
        __asm__ volatile (
            "movq %[src], %%r8\n\t"    /* Extended register */
            "addq %%r8, %[dst]\n\t"
            : [dst] "+R" (r2)          /* Legacy register constraint */
            : [src] "r" (r3)
            : "r8", "cc"
        );
        
        /* Complex addressing mode that might need secondary reload */
        __asm__ volatile (
            "movq (%[base], %[index], 8), %%rax\n\t"
            "addq %%rax, %[sum]\n\t"
            : [sum] "+r" (r4)
            : [base] "r" (&input[0]),
              [index] "r" ((long)i)
            : "rax", "memory", "cc"
        );
        
        output[i] = r1 + r2 + r4;
    }
}

/* Test function for optional reloads */
void test_optional_reloads(int iterations, float *finput, float *foutput) {
    float f1 = finput[0], f2 = finput[1], f3 = finput[2], f4 = finput[3];
    float f5, f6, f7, f8;
    
    for (int i = 0; i < iterations; i++) {
        /* Optional output constraint */
        __asm__ volatile (
            "movss %[in1], %%xmm0\n\t"
            "addss %[in2], %%xmm0\n\t"
            "movss %%xmm0, %[out1]\n\t"
            : [out1] "=?r" (f5)        /* Optional constraint */
            : [in1] "xm" (f1),
              [in2] "xm" (f2)
            : "xmm0"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "movss %[in1], %%xmm0\n\t"
            "mulss %[in2], %%xmm0\n\t"
            "movss %%xmm0, %[out1]\n\t"
            : [out1] "=r" (f6)
            : [in1] "xm" (f3),
              [in2] "xm" (f4)
            : "xmm0"
        );
        
        /* Use volatile to ensure execution */
        __asm__ volatile (
            "movss %[in], %%xmm0\n\t"
            "sqrtss %%xmm0, %%xmm0\n\t"
            : "=x" (f7)
            : [in] "x" (finput[i])
        );
        
        foutput[i] = f5 + f6 + f7;
    }
}

/* Mixed vector and scalar operations */
void test_vector_reloads(int iterations, __m128i *vinput, __m128i *voutput) {
    __m128i v0 = vinput[0];
    __m128i v1 = vinput[1];
    __m128i v2, v3;
    
    int scalar1 = 0, scalar2 = 0, scalar3 = 0, scalar4 = 0;
    int scalar5 = 0, scalar6 = 0, scalar7 = 0, scalar8 = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vector operation */
        v2 = _mm_add_epi32(v0, v1);
        
        /* Interleave with scalar asm to force register shuffling */
        __asm__ volatile (
            "movd %[vec_elem], %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movd %%eax, %[vec_elem]\n\t"
            : [vec_elem] "+x" (v2)
            :
            : "eax", "cc"
        );
        
        /* More scalar variables to increase pressure */
        scalar1 += i;
        scalar2 += scalar1;
        scalar3 += scalar2;
        scalar4 += scalar3;
        
        /* Another asm using specific registers */
        __asm__ volatile (
            "movl %[s1], %%eax\n\t"
            "imull %[s2], %%eax\n\t"
            "movl %%eax, %[s3]\n\t"
            : [s3] "=r" (scalar5)
            : [s1] "rm" (scalar1),
              [s2] "rm" (scalar2)
            : "eax", "cc"
        );
        
        voutput[i] = _mm_add_epi32(v2, _mm_set1_epi32(scalar5));
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *input, int *output, int size) {
    int a = input[0], b = input[1], c = input[2], d = input[3];
    int e, f, g, h;
    
    /* Different asm blocks in different control flow paths */
    if (mode & 1) {
        __asm__ volatile (
            "movl %[in1], %%eax\n\t"
            "xorl %[in2], %%eax\n\t"
            : "=a" (e)
            : [in1] "r" (a),
              [in2] "r" (b)
            : "cc"
        );
    } else {
        __asm__ volatile (
            "movl %[in1], %%ebx\n\t"
            "orl %[in2], %%ebx\n\t"
            : "=b" (e)
            : [in1] "r" (a),
              [in2] "r" (b)
            : "cc"
        );
    }
    
    for (int i = 0; i < size; i++) {
        /* Loop-dependent asm */
        if (i % 3 == 0) {
            __asm__ volatile (
                "addl $1, %[val]\n\t"
                : [val] "+r" (c)
            );
        } else if (i % 3 == 1) {
            __asm__ volatile (
                "subl $1, %[val]\n\t"
                : [val] "+r" (d)
            );
        }
        
        /* Complex asm that depends on loop variable */
        __asm__ volatile (
            "leal (%[a], %[i], 4), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            : [sum] "+r" (f)
            : [a] "r" (a),
              [i] "r" (i)
            : "eax", "cc"
        );
        
        output[i] = c + d + e + f;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Initialize test data */
    int *int_input = malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = malloc(ARRAY_SIZE * sizeof(int));
    long *long_input = malloc(ARRAY_SIZE * sizeof(long));
    long *long_output = malloc(ARRAY_SIZE * sizeof(long));
    float *float_input = malloc(ARRAY_SIZE * sizeof(float));
    float *float_output = malloc(ARRAY_SIZE * sizeof(float));
    __m128i *vec_input = malloc(ARRAY_SIZE * sizeof(__m128i));
    __m128i *vec_output = malloc(ARRAY_SIZE * sizeof(__m128i));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_input[i] = i * 3 + 1;
        long_input[i] = i * 5 + 2;
        float_input[i] = i * 0.5f;
        vec_input[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute test functions */
    test_primary_reloads(iterations, int_input, int_output, 
                         (double*)float_input, (double*)float_output);
    
    test_secondary_reloads(iterations / 2, long_input, long_output);
    
    test_optional_reloads(iterations, float_input, float_output);
    
    test_vector_reloads(iterations / 4, vec_input, vec_output);
    
    test_control_flow_reloads(mode, int_input, int_output, iterations);
    
    /* Compute checksum */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += long_output[i];
        checksum += (unsigned)float_output[i];
        __m128i v = vec_output[i];
        checksum += _mm_extract_epi32(v, 0);
        checksum += _mm_extract_epi32(v, 1);
        checksum += _mm_extract_epi32(v, 2);
        checksum += _mm_extract_epi32(v, 3);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(int_input);
    free(int_output);
    free(long_input);
    free(long_output);
    free(float_input);
    free(float_output);
    free(vec_input);
    free(vec_output);
    
    return 0;
}
