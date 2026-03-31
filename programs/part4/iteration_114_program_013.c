/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(double *dinput, float *foutput, int count);
void test_optional_reloads(long *linput, long *loutput, int mode);
void test_control_flow_reloads(unsigned char *data, int size, int threshold);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.141592653589793;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    printf("Running reload tests with iterations=%d, mode=%d\n", 
           iterations, mode);
    
    /* Allocate and initialize arrays with mixed data types */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long *long_array = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_output = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    unsigned char *byte_array = (unsigned char*)aligned_alloc(64, ARRAY_SIZE);
    
    if (!int_array || !int_output || !double_array || !float_output || 
        !long_array || !long_output || !byte_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 0.5 + 1.0;
        long_array[i] = i * 7L + 3L;
        byte_array[i] = (i * 13) & 0xFF;
        int_output[i] = 0;
        float_output[i] = 0.0f;
        long_output[i] = 0L;
    }
    
    /* Run test functions to trigger reloads */
    test_primary_reloads(iterations, int_array, int_output);
    test_secondary_reloads(double_array, float_output, ARRAY_SIZE / 4);
    test_optional_reloads(long_array, long_output, mode);
    test_control_flow_reloads(byte_array, ARRAY_SIZE, 128);
    
    /* Compute checksum to ensure all assembly executed */
    unsigned long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (unsigned long long)float_output[i];
        checksum += long_output[i];
        checksum += byte_array[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_array);
    free(int_output);
    free(double_array);
    free(float_output);
    free(long_array);
    free(long_output);
    free(byte_array);
    
    return 0;
}

/* Test function 1: Primary reloads with register pressure */
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Create many live variables to exhaust registers */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Use vector types to increase register pressure */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, f, g, h);
    __m256d vd1 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d vd2 = _mm256_set_pd(5.0, 6.0, 7.0, 8.0);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Unrolled loop with complex inline assembly */
        for (int idx = 0; idx < ARRAY_SIZE; idx += UNROLL_FACTOR) {
            /* Complex asm with 8 operands, mixed constraints */
            __asm__ volatile (
                /* Outputs with different constraints */
                "=r" (output[idx + 0]),   /* general register */
                "=&r" (output[idx + 1]),  /* earlyclobber */
                "=q" (output[idx + 2]),   /* byte register */
                "=a" (output[idx + 3]),   /* accumulator */
                "=d" (output[idx + 4]),   /* data register */
                "=t" (output[idx + 5]),   /* top of stack (x87) */
                "=m" (output[idx + 6]),   /* memory */
                "=c" (output[idx + 7])    /* counter */
                :
                /* Inputs with mixed constraints */
                "r" (input[idx + 0]),     /* register */
                "rm" (input[idx + 1]),    /* register or memory */
                "i" (256),                /* immediate */
                "g" (input[idx + 3]),     /* general */
                "m" (input[idx + 4]),     /* memory */
                "a" (iter),               /* accumulator */
                "d" (idx),                /* data register */
                "0" (output[idx + 0])     /* matching constraint */
                :
                /* Clobber list */
                "cc", "memory",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "ymm4", "ymm5", "ymm6", "ymm7"
            );
            
            /* Another asm with different constraints to prevent combining */
            __asm__ volatile ("" ::: "memory"); /* Memory barrier */
            
            __asm__ volatile (
                "movl %[in1], %%eax\n\t"
                "imull %[in2], %%eax\n\t"
                "addl %[in3], %%eax\n\t"
                "movl %%eax, %[out1]\n\t"
                "movq %[in4], %%mm0\n\t"  /* MMX register */
                "movq %%mm0, %[out2]"
                : [out1] "=r" (output[idx + 8]),
                  [out2] "=m" (output[idx + 9])
                : [in1] "rm" (input[idx + 8]),
                  [in2] "rm" (input[idx + 9]),
                  [in3] "i" (0x1234),
                  [in4] "x" (v1)
                : "eax", "mm0", "cc"
            );
            
            /* Update many live variables to keep them in registers */
            a += output[idx]; b += output[idx + 1];
            c += output[idx + 2]; d += output[idx + 3];
            e += output[idx + 4]; f += output[idx + 5];
            g += output[idx + 6]; h += output[idx + 7];
            
            /* Use vector operations to maintain vector register pressure */
            v1 = _mm_add_epi32(v1, v2);
            vd1 = _mm256_add_pd(vd1, vd2);
        }
        
        /* Additional asm with optional constraints */
        int opt_result;
        __asm__ volatile (
            "testl %[val], %[val]\n\t"
            "cmovnzl %[src], %[dst]"
            : [dst] "=?r" (opt_result)    /* Optional output */
            : [val] "r" (iter),
              [src] "rm" (input[iter % ARRAY_SIZE])
            : "cc"
        );
        
        global_counter += opt_result;
    }
    
    /* Force use of all variables to prevent optimization */
    __asm__ volatile ("" 
        : 
        : "r" (a), "r" (b), "r" (c), "r" (d),
          "r" (e), "r" (f), "r" (g), "r" (h),
          "x" (v1), "x" (vd1)
        : "memory"
    );
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(double *dinput, float *foutput, int count) {
    /* Variables that will force secondary reloads */
    register double d1 asm("xmm8");
    register double d2 asm("xmm9");
    register int r1 asm("r12");
    register int r2 asm("r13");
    
    d1 = 1.0;
    d2 = 2.0;
    r1 = 0x12345678;
    r2 = 0x87654321;
    
    for (int i = 0; i < count; i++) {
        /* Asm requiring specific register classes that may need secondary reloads */
        double temp;
        __asm__ volatile (
            /* Force a secondary reload by using 'R' constraint with R8-R15 */
            "movsd %[in1], %%xmm0\n\t"
            "addsd %[in2], %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=m" (foutput[i])  /* Memory output */
            : [in1] "R" (dinput[i]),   /* Legacy register constraint */
              [in2] "x" (d1)           /* SSE register */
            : "xmm0"
        );
        
        /* Another pattern: mixing register classes */
        __asm__ volatile (
            "mov %[reg1], %%eax\n\t"
            "add %[reg2], %%eax\n\t"
            "cvtsi2ss %%eax, %%xmm0\n\t"
            "movss %%xmm0, %[out]"
            : [out] "=m" (foutput[i + count])
            : [reg1] "a" (r1),         /* Must be in eax */
              [reg2] "r" (r2)          /* Any general register */
            : "eax", "xmm0", "cc"
        );
        
        /* Force register moves between different register sets */
        __asm__ volatile (
            "movd %[mmx], %%mm1\n\t"
            "movd %%mm1, %[out]"
            : [out] "=r" (r1)
            : [mmx] "y" (i)            /* MMX register */
            : "mm1"
        );
        
        /* Update variables to keep them live */
        d1 += dinput[i];
        r2 += i;
        
        /* Complex asm with multiple constraints that don't match */
        __asm__ volatile (
            "mov %[in1], %%ebx\n\t"
            "mov %[in2], %%ecx\n\t"
            "imull %%ecx, %%ebx\n\t"
            "mov %%ebx, %[out1]\n\t"
            "movsd %[in3], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm1\n\t"
            "movsd %%xmm1, %[out2]"
            : [out1] "=r" (r1),
              [out2] "=m" (temp)
            : [in1] "b" (r1),          /* Must be in ebx */
              [in2] "c" (r2),          /* Must be in ecx */
              [in3] "x" (d2)           /* SSE register */
            : "ebx", "ecx", "xmm1", "cc"
        );
        
        d2 = temp;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(long *linput, long *loutput, int mode) {
    volatile int barrier = 0;  /* Prevent optimization */
    
    for (int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Asm with optional output constraint */
        long opt1, opt2;
        
        __asm__ volatile (
            "mov %[in], %%rax\n\t"
            "test %[mode], %[mode]\n\t"
            "jz 1f\n\t"
            "add $100, %%rax\n\t"
            "1:\n\t"
            "mov %%rax, %[out]"
            : [out] "=?r" (opt1)       /* Optional output */
            : [in] "rm" (linput[i]),
              [mode] "r" (mode)
            : "rax", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could be combined but won't due to barrier */
        __asm__ volatile (
            "mov %[in], %%rbx\n\t"
            "sub $50, %%rbx\n\t"
            "mov %%rbx, %[out]"
            : [out] "=r" (opt2)
            : [in] "rm" (linput[i + 1])
            : "rbx", "cc"
        );
        
        /* Use both results to ensure they're computed */
        loutput[i] = opt1 + opt2;
        barrier = opt1;  /* Volatile use */
        
        /* Another pattern with 'nocombine' effect */
        if (i % 3 == 0) {
            __asm__ volatile (
                "cpuid\n\t"            /* Serializing instruction */
                ::: "rax", "rbx", "rcx", "rdx", "memory"
            );
        }
        
        /* Complex asm with many clobbers to prevent combining */
        __asm__ volatile (
            "movq %[in1], %%mm2\n\t"
            "paddq %[in2], %%mm2\n\t"
            "movq %%mm2, %[out]"
            : [out] "=m" (loutput[i + ARRAY_SIZE/2])
            : [in1] "y" ((long long)i),
              [in2] "y" ((long long)mode)
            : "mm2"
        );
    }
}

/* Test function 4: Control flow dependent reloads */
void test_control_flow_reloads(unsigned char *data, int size, int threshold) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    
    /* Multiple live variables with control flow dependencies */
    for (int i = 0; i < size; i++) {
        /* Conditional asm blocks */
        if (data[i] > threshold) {
            __asm__ volatile (
                "movzbl %[byte], %%eax\n\t"
                "addl %%eax, %[sum]"
                : [sum] "+r" (sum1)
                : [byte] "m" (data[i])
                : "eax", "cc"
            );
            
            /* Additional computation in this path */
            temp1 = data[i] * 2;
            __asm__ volatile (
                "imull $3, %[val], %%ecx\n\t"
                "movl %%ecx, %[out]"
                : [out] "=r" (temp2)
                : [val] "r" (temp1)
                : "ecx", "cc"
            );
        } else {
            __asm__ volatile (
                "movzbl %[byte], %%ebx\n\t"
                "subl %%ebx, %[sum]"
                : [sum] "+r" (sum2)
                : [byte] "m" (data[i])
                : "ebx", "cc"
            );
            
            temp3 = data[i] / 2;
        }
        
        /* Loop-dependent asm with varying constraints */
        switch (i % 4) {
            case 0:
                __asm__ volatile (
                    "movl %[idx], %%edx\n\t"
                    "andl $0xF, %%edx\n\t"
                    "movl %%edx, %[out]"
                    : [out] "=r" (temp4)
                    : [idx] "r" (i)
                    : "edx", "cc"
                );
                break;
            case 1:
                __asm__ volatile (
                    "leal (%[idx], %[idx], 2), %%esi\n\t"
                    "movl %%esi, %[out]"
                    : [out] "=r" (temp4)
                    : [idx] "r" (i)
                    : "esi", "cc"
                );
                break;
            case 2:
                /* Use different register class */
                __asm__ volatile (
                    "movl %[idx], %%edi\n\t"
                    "shrl $2, %%edi\n\t"
                    "movl %%edi, %[out]"
                    : [out] "=r" (temp4)
                    : [idx] "r" (i)
                    : "edi", "cc"
                );
                break;
            default:
                /* Memory operand forcing reload */
                __asm__ volatile (
                    "movl %[idx], %%ebp\n\t"
                    "negl %%ebp\n\t"
                    "movl %%ebp, %[out]"
                    : [out] "=m" (data[i])  /* Modify input array */
                    : [idx] "m" (i)
                    : "ebp", "cc"
                );
                break;
        }
        
        /* Nested loop to increase complexity */
        for (int j = 0; j < 4; j++) {
            int inner_temp = i + j;
            __asm__ volatile (
                "addl %[a], %[b]"
                : [b] "+r" (sum3)
                : [a] "r" (inner_temp)
                : "cc"
            );
            
            /* Use global variable to force memory accesses */
            global_counter++;
        }
        
        /* Final asm using all accumulated values */
        if (i % 16 == 0) {
            __asm__ volatile (
                "movl %[s1], %%eax\n\t"
                "addl %[s2], %%eax\n\t"
                "addl %[s3], %%eax\n\t"
                "addl %[s4], %%eax\n\t"
                "movl %%eax, %[out]"
                : [out] "=m" (sum4)
                : [s1] "r" (sum1),
                  [s2] "r" (sum2),
                  [s3] "r" (sum3),
                  [s4] "r" (temp4)
                : "eax", "cc", "memory"
            );
        }
    }
    
    /* Ensure all results are used */
    global_counter += sum1 + sum2 + sum3 + sum4 + temp1 + temp2 + temp3 + temp4;
}
