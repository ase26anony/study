#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions prototypes */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_control_flow_reloads(int iterations, long *input, long *output, int mode);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;

/* Function to create register pressure with many live variables */
__attribute__((noinline))
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Create many live scalar variables */
    register int r0 asm("r12") = input[0];
    register int r1 asm("r13") = input[1];
    register int r2 asm("r14") = input[2];
    register int r3 asm("r15") = input[3];
    int v4 = input[4], v5 = input[5], v6 = input[6], v7 = input[7];
    int v8 = input[8], v9 = input[9], v10 = input[10], v11 = input[11];
    int v12 = input[12], v13 = input[13], v14 = input[14], v15 = input[15];
    
    /* Mixed mode constraints to trigger different inmode/outmode assignments */
    for (int i = 0; i < iterations; i++) {
        int temp1, temp2, temp3, temp4;
        char byte_temp;
        short word_temp;
        long double fp_temp;
        
        /* Complex asm with 5+ operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (temp1),      /* General register */
            "=q" (byte_temp),  /* Byte register (a,b,c,d) */
            "=t" (fp_temp),    /* Top of FP stack */
            "=a" (temp2),      /* Accumulator */
            "=d" (temp3),      /* Data register */
            
            /* Inputs with mixed constraints */
            : "0" (r0),        /* Matching constraint with temp1 */
              "r" (r1),        /* General register */
              "i" (0x1234),    /* Immediate */
              "m" (v4),        /* Memory */
              "g" (v5),        /* General (register or memory) */
              "a" (r2),        /* Accumulator */
              "b" (r3),        /* Base register */
              
            /* Clobber list */
            : "memory", "cc", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
        
        /* Earlyclobber constraint to prevent reuse */
        __asm__ volatile (
            "movl %[in1], %[out1]\n\t"
            "addl %[in2], %[out1]\n\t"
            "imull %[in3], %[out1]"
            : [out1] "=&r" (temp4)  /* Earlyclobber */
            : [in1] "r" (v6),
              [in2] "rm" (v7),      /* Register or memory */
              [in3] "i" (42)        /* Immediate */
            : "cc"
        );
        
        /* Update live variables to keep them in use */
        r0 = temp1 + i;
        r1 = temp2 ^ v8;
        r2 = temp3 | v9;
        r3 = temp4 & v10;
        
        /* Store result with memory constraint */
        __asm__ volatile (
            "movl %1, %0"
            : "=m" (output[i])
            : "r" (r0)
            : /* empty */
        );
        
        /* Rotate values to create dependencies */
        v4 = v5; v5 = v6; v6 = v7;
        v7 = v8; v8 = v9; v9 = v10;
        v10 = v11; v11 = v12; v12 = v13;
        v13 = v14; v14 = v15; v15 = r0;
    }
    
    /* Prevent dead code elimination */
    global_counter += r0 + r1 + r2 + r3;
}

/* Function to trigger secondary reloads */
__attribute__((noinline))
void test_secondary_reloads(int iterations, double *input, double *output) {
    /* Use AVX registers alongside scalar registers */
    __m256d vec0 = _mm256_set1_pd(input[0]);
    __m256d vec1 = _mm256_set1_pd(input[1]);
    __m256d vec2 = _mm256_set1_pd(input[2]);
    __m256d vec3 = _mm256_set1_pd(input[3]);
    
    double scalar0 = input[4];
    double scalar1 = input[5];
    double scalar2 = input[6];
    double scalar3 = input[7];
    
    for (int i = 0; i < iterations; i++) {
        double result1, result2;
        long double fp_result;
        
        /* Asm requiring specific register classes that may need secondary reloads */
        __asm__ volatile (
            /* Force use of legacy register constraint */
            "movq %[vec_elem], %%rax\n\t"
            "addq %[scalar], %%rax\n\t"
            "movq %%rax, %[result]"
            : [result] "=R" (result1)  /* Legacy register constraint */
            : [vec_elem] "x" (vec0[0]), /* XMM register */
              [scalar] "rm" (scalar0)   /* Register or memory */
            : "rax", "cc"
        );
        
        /* Mismatched constraints requiring secondary reload */
        __asm__ volatile (
            "fldl %[in1]\n\t"
            "faddl %[in2]\n\t"
            "fstpt %[out]"
            : [out] "=t" (fp_result)    /* Top of FP stack */
            : [in1] "rm" (scalar1),     /* May need secondary reload if in memory */
              [in2] "rm" (scalar2)
            : "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
        );
        
        /* Convert back from FP stack to regular register */
        __asm__ volatile (
            "fldt %[in]\n\t"
            "fstpl %[out]"
            : [out] "=m" (result2)
            : [in] "t" (fp_result)
            : "st(0)"
        );
        
        /* Update vectors and scalars */
        vec0 = _mm256_add_pd(vec0, _mm256_set1_pd(result1));
        vec1 = _mm256_mul_pd(vec1, _mm256_set1_pd(result2));
        
        scalar0 = scalar1 + i;
        scalar1 = scalar2 * 1.1;
        scalar2 = scalar3 / 2.0;
        scalar3 = result1 + result2;
        
        /* Store with complex addressing mode */
        __asm__ volatile (
            "vmovapd %1, %0"
            : "=m" (output[i * 4])
            : "x" (vec0)  /* XMM/YMM register */
            : /* empty */
        );
    }
    
    /* Extract elements to prevent optimization */
    double temp[4];
    _mm256_storeu_pd(temp, vec0);
    global_accumulator += (long)(temp[0] + temp[1] + temp[2] + temp[3]);
}

/* Function to test optional reloads and nocombine behavior */
__attribute__((noinline))
void test_optional_reloads(int iterations, float *input, float *output) {
    float accum = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        float temp1, temp2, temp3;
        int opt_result;
        
        /* Optional constraint with '?' modifier */
        __asm__ volatile (
            "movss %[in1], %[out1]\n\t"
            "addss %[in2], %[out1]"
            : [out1] "=?r" (temp1)      /* Optional output */
            : [in1] "rm" (input[i]),
              [in2] "rm" (accum)
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "mulss %[in1], %[out1]"
            : [out1] "=r" (temp2)
            : "0" (temp1),              /* Matching constraint */
              [in1] "rm" (3.14159f)
            : "cc"
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Third asm with different clobber to prevent combine */
        __asm__ volatile (
            "subss %[in1], %[out1]"
            : [out1] "=r" (temp3)
            : "0" (temp2),
              [in1] "rm" (2.71828f)
            : "xmm0", "xmm1", "xmm2"  /* Different clobbers inhibit combination */
        );
        
        accum = temp3;
        
        /* Store with volatile to ensure execution */
        __asm__ volatile (
            "movss %1, %0"
            : "=m" (output[i])
            : "x" (accum)
            : /* empty */
        );
        
        /* Optional input that might be omitted */
        if (i % 3 == 0) {
            __asm__ volatile (
                "cvtsi2ssl %[count], %[out]"
                : [out] "=x" (temp1)
                : [count] "?r" (i)      /* Optional input */
                : /* empty */
            );
            accum += temp1;
        }
    }
    
    global_counter += (int)accum;
}

/* Function with control flow dependent reloads */
__attribute__((noinline))
void test_control_flow_reloads(int iterations, long *input, long *output, int mode) {
    long a = input[0], b = input[1], c = input[2], d = input[3];
    long e = input[4], f = input[5], g = input[6], h = input[7];
    
    for (int i = 0; i < iterations; i++) {
        long result;
        
        /* Different asm blocks on different control flow paths */
        if (mode == 0) {
            __asm__ volatile (
                "movq %[in1], %%rax\n\t"
                "addq %[in2], %%rax\n\t"
                "movq %%rax, %[out]"
                : [out] "=r" (result)
                : [in1] "r" (a),
                  [in2] "r" (b)
                : "rax", "cc"
            );
        } else if (mode == 1) {
            __asm__ volatile (
                "imulq %[in1], %[out]"
                : [out] "=r" (result)
                : "0" (c),              /* Matching constraint */
                  [in1] "rm" (d)        /* Register or memory */
                : "cc"
            );
        } else {
            __asm__ volatile (
                "xorq %[in1], %[out]\n\t"
                "rorq $13, %[out]"
                : [out] "=r" (result)
                : "0" (e),              /* Matching constraint */
                  [in1] "r" (f)
                : "cc"
            );
        }
        
        /* Complex loop-carried dependencies */
        switch (i % 4) {
            case 0:
                a = result + g;
                __asm__ volatile (
                    "movq %1, %0"
                    : "=m" (output[i])
                    : "r" (a)
                );
                break;
            case 1:
                b = result ^ h;
                __asm__ volatile (
                    "movq %1, %0"
                    : "=m" (output[i])
                    : "r" (b)
                );
                break;
            case 2:
                c = result | a;
                __asm__ volatile (
                    "movq %1, %0"
                    : "=m" (output[i])
                    : "r" (c)
                );
                break;
            case 3:
                d = result & b;
                __asm__ volatile (
                    "movq %1, %0"
                    : "=m" (output[i])
                    : "r" (d)
                );
                break;
        }
        
        /* Rotate values */
        e = f; f = g; g = h;
        h = result + i;
    }
    
    global_accumulator += a + b + c + d + e + f + g + h;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <iterations> <mode>\n", argv[0]);
        printf("  iterations: Number of loop iterations (e.g., 100)\n");
        printf("  mode: Test mode (0-2)\n");
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    int mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > ARRAY_SIZE) iterations = ARRAY_SIZE;
    
    /* Initialize test arrays with mixed data */
    int int_data[ARRAY_SIZE];
    double double_data[ARRAY_SIZE];
    float float_data[ARRAY_SIZE];
    long long_data[ARRAY_SIZE];
    
    int int_output[ARRAY_SIZE] = {0};
    double double_output[ARRAY_SIZE] = {0};
    float float_output[ARRAY_SIZE] = {0};
    long long_output[ARRAY_SIZE] = {0};
    
    /* Fill arrays with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5 + 0.25;
        float_data[i] = i * 0.75f + 0.125f;
        long_data[i] = i * 5L + 2L;
    }
    
    /* Run all test functions to trigger different reload patterns */
    test_primary_reloads(iterations, int_data, int_output);
    test_secondary_reloads(iterations / 2, double_data, double_output);
    test_optional_reloads(iterations, float_data, float_output);
    test_control_flow_reloads(iterations, long_data, long_output, mode);
    
    /* Compute checksum to ensure all code executed */
    long checksum = global_counter + global_accumulator;
    
    for (int i = 0; i < iterations; i++) {
        checksum += int_output[i];
        checksum += (long)double_output[i];
        checksum += (long)float_output[i];
        checksum += long_output[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %ld\n", global_accumulator);
    
    return 0;
}
