/* reload_test.c - Complex inline assembly to trigger reload.cc logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_control_flow_reloads(int mode, int *data, int size);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vec = _mm_set_epi32(1, 2, 3, 4);

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *input, int *output) {
    int i, j;
    int temp1, temp2, temp3, temp4, temp5;
    int accum1 = 0, accum2 = 0, accum3 = 0, accum4 = 0;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        /* Force many values to be live simultaneously */
        temp1 = input[i * 4];
        temp2 = input[i * 4 + 1];
        temp3 = input[i * 4 + 2];
        temp4 = input[i * 4 + 3];
        temp5 = global_counter + i;
        
        /* Complex asm with 5+ operands and mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (accum1),      /* General register */
            "=&r" (accum2),     /* Early clobber */
            "=q" (temp1),       /* Byte register (a,b,c,d) */
            "=a" (temp2),       /* Accumulator specific */
            "=d" (temp3),       /* DX register */
            
            /* Inputs with mixed constraints */
            : "0" (accum1),     /* Matching constraint */
              "r" (temp4),      /* General register */
              "rm" (temp5),     /* Register or memory */
              "i" (1234),       /* Immediate */
              "g" (global_counter) /* General (register/memory/immediate) */
            
            /* Clobber list */
            : "cc", "memory",
              "r8", "r9", "r10", "r11"  /* Force spills */
        );
        
        /* Another asm with different mode requirements */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movl %3, %%ebx\n\t"
            "subl %%ebx, %0"
            : "+r" (accum3), "+r" (accum4)
            : "rm" (temp1), "rm" (temp2), "r" (temp3)
            : "eax", "ebx", "cc"
        );
        
        /* Use vector intrinsics to increase register pressure */
        __m128i vec1 = _mm_set_epi32(temp1, temp2, temp3, temp4);
        __m128i vec2 = _mm_set_epi32(accum1, accum2, accum3, accum4);
        __m128i vec3 = _mm_add_epi32(vec1, vec2);
        
        /* Extract results forcing moves between register classes */
        output[i * 4] = _mm_extract_epi32(vec3, 0) + accum1;
        output[i * 4 + 1] = _mm_extract_epi32(vec3, 1) + accum2;
        output[i * 4 + 2] = _mm_extract_epi32(vec3, 2) + accum3;
        output[i * 4 + 3] = _mm_extract_epi32(vec3, 3) + accum4;
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Force secondary reload scenarios */
void test_secondary_reloads(int iterations, double *input, double *output) {
    int i;
    double d1, d2, d3, d4, d5;
    long long ll1, ll2;
    
    for (i = 0; i < iterations; i++) {
        d1 = input[i];
        d2 = input[i + 1];
        d3 = global_double;
        d4 = (double)i * 2.5;
        d5 = d1 + d2;
        
        /* Asm requiring specific register classes that may need secondary reloads */
        __asm__ volatile (
            /* Force use of legacy registers with 'R' constraint */
            "movq %[in1], %%rax\n\t"
            "addq %[in2], %%rax\n\t"
            "movq %%rax, %[out1]\n\t"
            "imulq %[in3], %%rax\n\t"
            "movq %%rax, %[out2]"
            : [out1] "=R" (ll1),  /* Legacy register (ax,bx,cx,dx) */
              [out2] "=r" (ll2)   /* General register */
            : [in1] "rm" ((long long)d1),
              [in2] "rm" ((long long)d2),
              [in3] "r" ((long long)d3)
            : "rax", "cc"
        );
        
        /* Mix x87 and SSE constraints */
        double result;
        __asm__ volatile (
            "fldl %1\n\t"
            "fldl %2\n\t"
            "faddp %%st, %%st(1)\n\t"
            "fstpl %0"
            : "=m" (result)
            : "m" (d4), "m" (d5)
            : "st", "st(1)"
        );
        
        output[i] = result + (double)ll1;
        output[i + 1] = d3 * (double)ll2;
        
        /* Create register pressure with many live doubles */
        __m256d ymm1 = _mm256_set_pd(d1, d2, d3, d4);
        __m256d ymm2 = _mm256_set_pd(d5, result, output[i], output[i + 1]);
        __m256d ymm3 = _mm256_add_pd(ymm1, ymm2);
        
        /* Store forcing spills */
        _mm256_storeu_pd(&output[i * 2], ymm3);
    }
}

/* Test optional reloads and nocombine scenarios */
void test_optional_reloads(int iterations, float *input, float *output) {
    int i;
    float f1, f2, f3, f4;
    
    for (i = 0; i < iterations; i++) {
        f1 = input[i];
        f2 = input[i + 1];
        f3 = (float)i / 10.0f;
        f4 = f1 * f2;
        
        /* Asm with optional constraints */
        float opt_result;
        __asm__ volatile (
            "movss %[in1], %%xmm0\n\t"
            "addss %[in2], %%xmm0\n\t"
            "mulss %[in3], %%xmm0\n\t"
            "movss %%xmm0, %[out]"
            : [out] "=?r" (opt_result)  /* Optional output */
            : [in1] "rm" (f1),
              [in2] "rm" (f2),
              [in3] "rm" (f3)
            : "xmm0"
        );
        
        /* Similar asm that could be combined but won't due to memory barrier */
        __asm__ volatile ("" ::: "memory");
        
        float opt_result2;
        __asm__ volatile (
            "movss %[in1], %%xmm1\n\t"
            "subss %[in2], %%xmm1\n\t"
            "movss %%xmm1, %[out]"
            : [out] "=?r" (opt_result2)  /* Another optional */
            : [in1] "rm" (f4),
              [in2] "rm" (f3)
            : "xmm1"
        );
        
        /* Force nocombine by using different clobber lists */
        float final_result;
        __asm__ volatile (
            "movss %1, %%xmm2\n\t"
            "addss %2, %%xmm2\n\t"
            "movss %%xmm2, %0"
            : "=r" (final_result)
            : "rm" (opt_result), "rm" (opt_result2)
            : "xmm2", "cc"
        );
        
        output[i] = final_result;
        
        /* Another asm with same inputs but different clobbers prevents combining */
        __asm__ volatile (
            "movss %1, %%xmm3\n\t"
            "mulss %2, %%xmm3"
            : /* no outputs */
            : "rm" (opt_result), "rm" (opt_result2)
            : "xmm3", "cc", "memory"
        );
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *data, int size) {
    int i, result = 0;
    int live1 = 1, live2 = 2, live3 = 3, live4 = 4;
    int live5 = 5, live6 = 6, live7 = 7, live8 = 8;
    
    for (i = 0; i < size; i++) {
        /* Complex conditional with many live variables */
        if (mode == 0) {
            __asm__ volatile (
                "addl %1, %0\n\t"
                "subl %2, %0\n\t"
                "imull %3, %0"
                : "+r" (result)
                : "r" (live1), "r" (live2), "rm" (data[i])
                : "cc"
            );
            live1 = result;
        } else if (mode == 1) {
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "xorl %2, %%eax\n\t"
                "movl %%eax, %0"
                : "=r" (result)
                : "r" (live3), "r" (live4), "m" (data[i])
                : "eax", "cc"
            );
            live3 = result;
        } else {
            __asm__ volatile (
                "leal (%1,%2,2), %0"
                : "=r" (result)
                : "r" (live5), "r" (live6)
            );
            live5 = result;
        }
        
        /* Nested loop to increase pressure */
        int j;
        for (j = 0; j < 4; j++) {
            int temp = live7 + live8 + j;
            __asm__ volatile (
                "addl %1, %0"
                : "+r" (temp)
                : "ri" (data[(i + j) % size])
                : "cc"
            );
            data[i] += temp;
        }
        
        /* Switch between different asm patterns */
        switch (i % 3) {
            case 0:
                __asm__ volatile (
                    "movl %1, %%ecx\n\t"
                    "roll $3, %%ecx\n\t"
                    "movl %%ecx, %0"
                    : "=r" (live7)
                    : "r" (live8)
                    : "ecx", "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "bsrl %1, %0"
                    : "=r" (live8)
                    : "r" (live7)
                    : "cc"
                );
                break;
            case 2:
                __asm__ volatile (
                    "cmpl %1, %2\n\t"
                    "setg %b0"
                    : "=r" (result)
                    : "r" (live7), "r" (live8)
                    : "cc"
                );
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations > ARRAY_SIZE / 4) iterations = ARRAY_SIZE / 4;
    
    /* Initialize data arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5;
        float_data[i] = i * 0.7f;
        int_output[i] = 0;
        double_output[i] = 0.0;
        float_output[i] = 0.0f;
    }
    
    /* Run tests to trigger reload scenarios */
    test_primary_reloads(iterations, int_data, int_output);
    test_secondary_reloads(iterations / 2, double_data, double_output);
    test_optional_reloads(iterations, float_data, float_output);
    test_control_flow_reloads(mode, int_data, ARRAY_SIZE);
    
    /* Compute checksum to ensure all asm executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long long)double_output[i];
        checksum += (long long)float_output[i];
        checksum += int_data[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_data);
    free(int_output);
    free(double_data);
    free(double_output);
    free(float_data);
    free(float_output);
    
    return 0;
}
