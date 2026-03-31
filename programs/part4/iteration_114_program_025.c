#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test function prototypes */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput);
void test_secondary_reloads(int iterations, long long *input, long long *output, float *finput, float *foutput);
void test_optional_reloads(int iterations, unsigned char *input, unsigned char *output, __m128i *vinput, __m128i *voutput);
void test_control_flow_reloads(int mode, int iterations, int *data, int *result);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.141592653589793;

/* Complex inline assembly with multiple operands and constraints */
static inline void complex_asm_operation(int a, int b, int c, int d, int e, 
                                         int *out1, int *out2, int *out3) {
    int tmp1, tmp2, tmp3;
    
    /* Extended asm with 7 operands, mixed constraints */
    __asm__ volatile (
        "movl %[a], %%eax\n\t"           /* Use accumulator constraint */
        "addl %[b], %%eax\n\t"
        "imull %[c], %%eax\n\t"
        "movl %%eax, %[tmp1]\n\t"
        "movl %[d], %%ebx\n\t"           /* Use base register constraint */
        "subl %[e], %%ebx\n\t"
        "movl %%ebx, %[tmp2]\n\t"
        "xorl %%eax, %%ebx\n\t"
        "movl %%ebx, %[tmp3]\n\t"
        : [tmp1] "=&r" (tmp1),           /* Early clobber */
          [tmp2] "=&r" (tmp2),           /* Early clobber */
          [tmp3] "=r" (tmp3)
        : [a] "rm" (a),                  /* Register or memory */
          [b] "rm" (b),
          [c] "i" (15),                  /* Immediate */
          [d] "r" (d),                   /* Register only */
          [e] "rm" (e)
        : "eax", "ebx", "cc", "memory"
    );
    
    /* Second asm with matching constraints to prevent combination */
    __asm__ volatile (
        "leal (%[t1], %[t2], 2), %%ecx\n\t"
        "movl %%ecx, %[o1]\n\t"
        "leal (%[t3], %[t1], 4), %%edx\n\t"
        "movl %%edx, %[o2]\n\t"
        : [o1] "=rm" (*out1),
          [o2] "=rm" (*out2)
        : [t1] "0" (tmp1),               /* Matching constraint */
          [t2] "r" (tmp2),
          [t3] "r" (tmp3)
        : "ecx", "edx", "cc"
    );
    
    *out3 = tmp3;
}

/* Function with register pressure through unrolled loops */
void test_primary_reloads(int iterations, int *input, int *output, double *dinput, double *doutput) {
    int i, j;
    
    /* Many live variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8;
    double d1, d2, d3, d4;
    __m128i vec1, vec2, vec3;
    
    for (i = 0; i < iterations; i++) {
        /* Initialize many live variables */
        v1 = input[i * 8 + 0];
        v2 = input[i * 8 + 1];
        v3 = input[i * 8 + 2];
        v4 = input[i * 8 + 3];
        v5 = input[i * 8 + 4];
        v6 = input[i * 8 + 5];
        v7 = input[i * 8 + 6];
        v8 = input[i * 8 + 7];
        
        d1 = dinput[i * 4 + 0];
        d2 = dinput[i * 4 + 1];
        d3 = dinput[i * 4 + 2];
        d4 = dinput[i * 4 + 3];
        
        /* Use vector intrinsics alongside scalar operations */
        vec1 = _mm_set_epi32(v1, v2, v3, v4);
        vec2 = _mm_set_epi32(v5, v6, v7, v8);
        vec3 = _mm_add_epi32(vec1, vec2);
        
        /* Complex asm with mixed constraints */
        int out1, out2, out3;
        complex_asm_operation(v1, v2, v3, v4, v5, &out1, &out2, &out3);
        
        /* More asm with different mode constraints */
        unsigned char byte_result;
        __asm__ volatile (
            "movb %[in1], %%al\n\t"
            "addb %[in2], %%al\n\t"
            "movb %%al, %[out]\n\t"
            : [out] "=q" (byte_result)   /* Byte register constraint */
            : [in1] "q" ((unsigned char)v6),
              [in2] "i" (42)             /* Immediate */
            : "al"
        );
        
        /* Double precision asm with x87 stack */
        double dresult;
        __asm__ volatile (
            "fldl %[d1]\n\t"
            "faddl %[d2]\n\t"
            "fstpl %[result]\n\t"
            : [result] "=t" (dresult)    /* Top of x87 stack */
            : [d1] "m" (d1),
              [d2] "m" (d2)
            : "st", "st(1)"
        );
        
        /* Store results, creating more register pressure */
        output[i * 8 + 0] = out1 + _mm_extract_epi32(vec3, 0);
        output[i * 8 + 1] = out2 + _mm_extract_epi32(vec3, 1);
        output[i * 8 + 2] = out3 + _mm_extract_epi32(vec3, 2);
        output[i * 8 + 3] = byte_result + _mm_extract_epi32(vec3, 3);
        doutput[i * 2 + 0] = dresult + d3;
        doutput[i * 2 + 1] = d4;
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Function to trigger secondary reloads */
void test_secondary_reloads(int iterations, long long *input, long long *output, 
                           float *finput, float *foutput) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        long long a = input[i * 2];
        long long b = input[i * 2 + 1];
        float f1 = finput[i * 3];
        float f2 = finput[i * 3 + 1];
        float f3 = finput[i * 3 + 2];
        
        /* Asm with constraints requiring secondary reloads */
        long long result1, result2;
        
        /* Force secondary reload by using 'a' constraint with memory operand */
        __asm__ volatile (
            "movq %[input], %%rax\n\t"
            "addq $0x12345678, %%rax\n\t"
            "movq %%rax, %[result]\n\t"
            : [result] "=rm" (result1)
            : [input] "a" (a)            /* Accumulator constraint */
            : "rax"
        );
        
        /* Mix register classes to force moves */
        __asm__ volatile (
            "movq %[b], %%r8\n\t"        /* Force use of R8-R15 */
            "subq %%rax, %%r8\n\t"
            "movq %%r8, %[out]\n\t"
            : [out] "=r" (result2)
            : [b] "R" (b),               /* Legacy register constraint */
              "a" (result1)              /* In accumulator from previous */
            : "r8"
        );
        
        /* Float operations with mismatched constraints */
        float fresult;
        __asm__ volatile (
            "movss %[f1], %%xmm0\n\t"
            "addss %[f2], %%xmm0\n\t"
            "mulss %[f3], %%xmm0\n\t"
            "movss %%xmm0, %[out]\n\t"
            : [out] "=x" (fresult)       /* XMM register */
            : [f1] "xm" (f1),
              [f2] "xm" (f2),
              [f3] "xm" (f3)
            : "xmm0"
        );
        
        output[i * 2] = result1;
        output[i * 2 + 1] = result2;
        foutput[i] = fresult;
    }
}

/* Function with optional reloads and nocombine patterns */
void test_optional_reloads(int iterations, unsigned char *input, unsigned char *output,
                          __m128i *vinput, __m128i *voutput) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        unsigned char b1 = input[i * 4];
        unsigned char b2 = input[i * 4 + 1];
        unsigned char b3 = input[i * 4 + 2];
        unsigned char b4 = input[i * 4 + 3];
        
        __m128i v1 = vinput[i];
        
        /* Asm with optional constraints */
        unsigned char opt1, opt2;
        __asm__ volatile (
            "movb %[in1], %%al\n\t"
            "cmpb $0, %%al\n\t"
            "je 1f\n\t"
            "addb %[in2], %%al\n\t"
            "1:\n\t"
            "movb %%al, %[out1]\n\t"
            : [out1] "=?r" (opt1)        /* Optional output */
            : [in1] "q" (b1),
              [in2] "q" (b2)
            : "al", "cc"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Second asm that could combine but won't due to barrier */
        __asm__ volatile (
            "movb %[in3], %%bl\n\t"
            "xorb %[in4], %%bl\n\t"
            "movb %%bl, %[out2]\n\t"
            : [out2] "=q" (opt2)
            : [in3] "q" (b3),
              [in4] "q" (b4)
            : "bl"
        );
        
        /* Vector asm with complex constraints */
        __m128i vresult;
        __asm__ volatile (
            "movdqa %[vec], %%xmm0\n\t"
            "psllw $4, %%xmm0\n\t"
            "movdqa %%xmm0, %[result]\n\t"
            : [result] "=x" (vresult)
            : [vec] "xm" (v1)
            : "xmm0"
        );
        
        output[i * 4] = opt1;
        output[i * 4 + 1] = opt2;
        output[i * 4 + 2] = b1 ^ b2;
        output[i * 4 + 3] = b3 & b4;
        voutput[i] = vresult;
    }
}

/* Function with control-flow dependent reloads */
void test_control_flow_reloads(int mode, int iterations, int *data, int *result) {
    int i, temp;
    
    for (i = 0; i < iterations; i++) {
        int x = data[i];
        
        /* Control flow affects which asm gets executed */
        if (mode == 0) {
            __asm__ volatile (
                "movl %[x], %%eax\n\t"
                "shrl $3, %%eax\n\t"
                "movl %%eax, %[t]\n\t"
                : [t] "=r" (temp)
                : [x] "rm" (x)
                : "eax"
            );
        } else if (mode == 1) {
            __asm__ volatile (
                "movl %[x], %%ebx\n\t"
                "imull $7, %%ebx\n\t"
                "movl %%ebx, %[t]\n\t"
                : [t] "=r" (temp)
                : [x] "rm" (x)
                : "ebx"
            );
        } else {
            __asm__ volatile (
                "movl %[x], %%ecx\n\t"
                "xorl $0xFFFF, %%ecx\n\t"
                "movl %%ecx, %[t]\n\t"
                : [t] "=r" (temp)
                : [x] "rm" (x)
                : "ecx"
            );
        }
        
        /* Nested loop with more asm */
        int j;
        for (j = 0; j < 4; j++) {
            int y = temp + j;
            int z;
            
            __asm__ volatile (
                "leal (%[y], %[y], 2), %[z]\n\t"
                : [z] "=r" (z)
                : [y] "r" (y)
            );
            
            temp = z;
        }
        
        result[i] = temp;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]) % 3;
    }
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_input = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_input = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *double_output = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long long *ll_input = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    long long *ll_output = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    float *float_input = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *float_output = (float*)malloc(ARRAY_SIZE * sizeof(float));
    unsigned char *byte_input = (unsigned char*)malloc(ARRAY_SIZE);
    unsigned char *byte_output = (unsigned char*)malloc(ARRAY_SIZE);
    __m128i *vec_input = (__m128i*)_mm_malloc(ARRAY_SIZE * sizeof(__m128i), 16);
    __m128i *vec_output = (__m128i*)_mm_malloc(ARRAY_SIZE * sizeof(__m128i), 16);
    int *control_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *control_result = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with mixed patterns */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_input[i] = i * 3 + 1;
        double_input[i] = i * 0.5;
        ll_input[i] = (long long)i * 1000000LL;
        float_input[i] = i * 0.25f;
        byte_input[i] = (unsigned char)(i ^ 0x55);
        control_data[i] = i * 7 - 3;
        
        /* Initialize vectors */
        vec_input[i] = _mm_set_epi32(i, i+1, i+2, i+3);
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_input, int_output, 
                        double_input, double_output);
    
    test_secondary_reloads(iterations, ll_input, ll_output,
                          float_input, float_output);
    
    test_optional_reloads(iterations, byte_input, byte_output,
                         vec_input, vec_output);
    
    test_control_flow_reloads(mode, iterations, control_data, control_result);
    
    /* Compute checksum to ensure all asm executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (unsigned long long)double_output[i];
        checksum += ll_output[i];
        checksum += (unsigned long long)float_output[i];
        checksum += byte_output[i];
        
        int *vec_elems = (int*)&vec_output[i];
        checksum += vec_elems[0] + vec_elems[1] + vec_elems[2] + vec_elems[3];
        
        checksum += control_result[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global double: %f\n", global_double);
    
    /* Cleanup */
    free(int_input);
    free(int_output);
    free(double_input);
    free(double_output);
    free(ll_input);
    free(ll_output);
    free(float_input);
    free(float_output);
    free(byte_input);
    free(byte_output);
    _mm_free(vec_input);
    _mm_free(vec_output);
    free(control_data);
    free(control_result);
    
    return 0;
}
