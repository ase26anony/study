/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(double *dinput, float *foutput, int count);
void test_optional_reloads(long *linput, long *loutput, int mode);
void test_control_flow_reloads(int *data, int size, int threshold);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;

/* Main test function with complex inline assembly */
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Create many live variables to increase register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize with values from input array */
    v0 = input[0]; v1 = input[1]; v2 = input[2]; v3 = input[3]; v4 = input[4];
    v5 = input[5]; v6 = input[6]; v7 = input[7]; v8 = input[8]; v9 = input[9];
    v10 = input[10]; v11 = input[11]; v12 = input[12]; v13 = input[13]; v14 = input[14];
    v15 = input[15]; v16 = input[16]; v17 = input[17]; v18 = input[18]; v19 = input[19];
    v20 = input[20]; v21 = input[21]; v22 = input[22]; v23 = input[23]; v24 = input[24];
    v25 = input[25]; v26 = input[26]; v27 = input[27]; v28 = input[28]; v29 = input[29];
    
    /* Unrolled loop with complex asm statements */
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 7 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (v0),     /* general register */
            "=&r" (v1),    /* earlyclobber */
            "=q" (v2),     /* byte register (a,b,c,d) */
            "=a" (v3),     /* accumulator */
            "=d" (v4),     /* data register */
            "=r" (v5),     /* general register */
            "=t" (v6)      /* top of FPU stack */
            :
            /* Inputs with mixed constraints */
            "0" (v0),      /* matching constraint */
            "r" (v1),
            "m" (input[i % ARRAY_SIZE]),  /* memory operand */
            "i" (12345),   /* immediate */
            "r" (v7),
            "a" (v8),
            "d" (v9)
            :
            /* Clobber many registers to force spills */
            "rcx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
            "cc", "memory"
        );
        
        /* Another asm block preventing combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Asm with optional constraint */
        __asm__ volatile (
            "addl %[imm], %[val]\n\t"
            : [val] "=r" (v10)
            : [val] "0" (v10),
              [imm] "i" (42)
            : "cc"
        );
        
        /* Force different register classes */
        __asm__ volatile (
            "movl %%eax, %%ebx\n\t"
            "addl %%ecx, %%ebx\n\t"
            : "=b" (v11), "=a" (v12)
            : "a" (v11), "c" (v12), "m" (input[(i + 1) % ARRAY_SIZE])
            : "cc"
        );
        
        /* Update many variables to keep them live */
        v13 = v0 + v1;
        v14 = v2 * v3;
        v15 = v4 ^ v5;
        v16 = v6 + v7;
        v17 = v8 - v9;
        v18 = v10 & v11;
        v19 = v12 | v13;
        v20 = v14 + v15;
        v21 = v16 - v17;
        v22 = v18 ^ v19;
        v23 = v20 * v21;
        v24 = v22 + v23;
        v25 = v24 % 997;
        v26 = v25 + i;
        v27 = v26 * 3;
        v28 = v27 / 2;
        v29 = v28 ^ 0xABCD;
        
        /* Store results to output array */
        output[i % ARRAY_SIZE] = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    }
    
    /* Store final values to prevent dead code elimination */
    output[0] = v0; output[1] = v1; output[2] = v2; output[3] = v3;
    output[4] = v4; output[5] = v5; output[6] = v6; output[7] = v7;
    output[8] = v8; output[9] = v9; output[10] = v10; output[11] = v11;
}

/* Test secondary reload patterns */
void test_secondary_reloads(double *dinput, float *foutput, int count) {
    __m128i vec0, vec1, vec2, vec3, vec4, vec5;
    __m256d dvec0, dvec1, dvec2;
    double d0, d1, d2, d3, d4, d5;
    float f0, f1, f2, f3, f4, f5;
    
    /* Initialize with vector and scalar values */
    vec0 = _mm_set_epi32(1, 2, 3, 4);
    vec1 = _mm_set_epi32(5, 6, 7, 8);
    dvec0 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    d0 = dinput[0]; d1 = dinput[1]; d2 = dinput[2];
    d3 = dinput[3]; d4 = dinput[4]; d5 = dinput[5];
    
    for (int i = 0; i < count; i++) {
        /* Asm requiring specific register classes that may need secondary reloads */
        __asm__ volatile (
            /* Legacy register constraint that may need secondary reload for R8-R15 */
            "movq %[din], %%rax\n\t"
            "addq %%rbx, %%rax\n\t"
            : "=a" (d0), "=b" (d1)
            : [din] "m" (dinput[i % ARRAY_SIZE]),
              "a" (d0), "b" (d1),
              "R" (d2)  /* Legacy register constraint */
            : "cc"
        );
        
        /* Mix vector and scalar operations to increase register pressure */
        vec2 = _mm_add_epi32(vec0, vec1);
        dvec1 = _mm256_add_pd(dvec0, _mm256_set1_pd(d0));
        
        /* Asm with "rm" constraint - may need secondary reload if operand is in memory */
        __asm__ volatile (
            "addsd %[src], %[dst]\n\t"
            : [dst] "=rm" (d3)
            : [dst] "0" (d3),
              [src] "rm" (d4)
            : "cc"
        );
        
        /* Force register moves between different register classes */
        __asm__ volatile (
            "movd %%eax, %[vec]\n\t"
            : [vec] "=x" (vec3)
            : "a" (i),
              "x" (vec2)
            : "cc"
        );
        
        /* More operations to keep values live */
        f0 = (float)d0;
        f1 = (float)d1;
        f2 = (float)d2;
        f3 = (float)d3;
        f4 = (float)d4;
        f5 = (float)d5;
        
        /* Store results */
        foutput[i % ARRAY_SIZE] = f0 + f1 + f2 + f3 + f4 + f5;
        
        /* Update vectors */
        vec0 = _mm_add_epi32(vec0, vec3);
        dvec0 = _mm256_add_pd(dvec0, dvec1);
    }
}

/* Test optional reloads and nocombine patterns */
void test_optional_reloads(long *linput, long *loutput, int mode) {
    long l0, l1, l2, l3, l4, l5, l6, l7;
    volatile long barrier = 0;
    
    l0 = linput[0]; l1 = linput[1]; l2 = linput[2]; l3 = linput[3];
    l4 = linput[4]; l5 = linput[5]; l6 = linput[6]; l7 = linput[7];
    
    for (int i = 0; i < 100; i++) {
        /* First asm with optional output constraint */
        __asm__ volatile (
            "movq %[in], %%rax\n\t"
            "addq $1, %%rax\n\t"
            : "=?r" (l0)  /* Optional constraint */
            : [in] "r" (l0)
            : "rax", "cc"
        );
        
        /* Memory barrier to prevent combination */
        barrier = l0;
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "movq %[in], %%rbx\n\t"
            "addq $2, %%rbx\n\t"
            : "=r" (l1)
            : [in] "r" (l1)
            : "rbx", "cc"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Asm with different clobbers to prevent combination */
        __asm__ volatile (
            "addq %[a], %[b]\n\t"
            : [b] "=r" (l2)
            : [a] "r" (l3),
              [b] "0" (l2)
            : "cc"
        );
        
        /* Complex asm with many operands */
        __asm__ volatile (
            "imulq %[x], %[y]\n\t"
            "addq %[z], %[y]\n\t"
            : [y] "=&r" (l4), "=d" (l5)
            : [x] "r" (l6),
              [y] "0" (l4),
              [z] "r" (l7)
            : "cc"
        );
        
        /* Control flow dependent asm */
        if (mode & 1) {
            __asm__ volatile (
                "xorq %%rax, %%rax\n\t"
                "orq %[val], %%rax\n\t"
                : "=a" (l6)
                : [val] "r" (l0)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "andq $0xFF, %[val]\n\t"
                : [val] "+r" (l7)
                :
                : "cc"
            );
        }
        
        /* Store results */
        loutput[i % 8] = l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7;
    }
}

/* Test control flow dependent reloads */
void test_control_flow_reloads(int *data, int size, int threshold) {
    int temp1, temp2, temp3, temp4;
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        temp1 = data[i];
        temp2 = data[(i + 1) % size];
        temp3 = data[(i + 2) % size];
        temp4 = data[(i + 3) % size];
        
        /* Conditional asm blocks create different reload paths */
        if (temp1 > threshold) {
            __asm__ volatile (
                "subl %[b], %[a]\n\t"
                : [a] "+r" (temp1)
                : [b] "r" (temp2)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "addl %[b], %[a]\n\t"
                : [a] "+r" (temp1)
                : [b] "r" (temp3)
                : "cc"
            );
        }
        
        /* Loop-dependent asm */
        for (int j = 0; j < (i % 4); j++) {
            __asm__ volatile (
                "xorl %%eax, %%eax\n\t"
                "addl $1, %%eax\n\t"
                : "=a" (temp4)
                : "a" (temp4)
                : "cc"
            );
        }
        
        /* Another conditional asm */
        switch (i % 3) {
            case 0:
                __asm__ volatile (
                    "imull %[b], %[a]\n\t"
                    : [a] "+r" (temp2)
                    : [b] "r" (temp3)
                    : "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "orl %[b], %[a]\n\t"
                    : [a] "+r" (temp2)
                    : [b] "r" (temp4)
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "andl %[b], %[a]\n\t"
                    : [a] "+r" (temp2)
                    : [b] "r" (temp1)
                    : "cc"
                );
                break;
        }
        
        result += temp1 + temp2 + temp3 + temp4;
    }
    
    global_accumulator += result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > 10000) iterations = 10000;
    
    /* Allocate and initialize arrays */
    int *int_input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_output = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long *long_input = (long*)malloc(ARRAY_SIZE * sizeof(long));
    long *long_output = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!int_input || !int_output || !double_input || !float_output || 
        !long_input || !long_output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_input[i] = i * 3 + 1;
        double_input[i] = i * 1.5;
        long_input[i] = i * 7L + 3L;
        int_output[i] = 0;
        float_output[i] = 0.0f;
        long_output[i] = 0L;
    }
    
    printf("Starting reload tests with iterations=%d, mode=%d\n", iterations, mode);
    
    /* Run all test functions */
    test_primary_reloads(iterations, int_input, int_output);
    test_secondary_reloads(double_input, float_output, iterations / 2);
    test_optional_reloads(long_input, long_output, mode);
    test_control_flow_reloads(int_input, ARRAY_SIZE / 4, iterations);
    
    /* Compute checksum */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long)float_output[i];
        checksum += long_output[i];
    }
    checksum += global_accumulator;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_input);
    free(int_output);
    free(double_input);
    free(float_output);
    free(long_input);
    free(long_output);
    
    return 0;
}
