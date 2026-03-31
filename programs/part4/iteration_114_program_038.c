/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int *input, int *output, int iterations);
void test_secondary_reloads(double *input, double *output, int iterations);
void test_optional_reloads(float *input, float *output, int iterations, int mode);
void test_control_flow_reloads(long *input, long *output, int size, int threshold);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Function with heavy register pressure and complex inline assembly */
void test_primary_reloads(int *input, int *output, int iterations) {
    /* Many live variables to create register pressure */
    register int r0 asm("eax") = input[0];
    register int r1 asm("ebx") = input[1];
    register int r2 asm("ecx") = input[2];
    register int r3 asm("edx") = input[3];
    register int r4 asm("esi") = input[4];
    register int r5 asm("edi") = input[5];
    int r6 = input[6], r7 = input[7], r8 = input[8], r9 = input[9];
    int r10 = input[10], r11 = input[11], r12 = input[12], r13 = input[13];
    
    /* Vector variables to compete for registers */
    __m128i v0 = _mm_set_epi32(r0, r1, r2, r3);
    __m128i v1 = _mm_set_epi32(r4, r5, r6, r7);
    __m256d vd0 = _mm256_set_pd(r8, r9, r10, r11);
    
    for (int i = 0; i < iterations; i++) {
        /* Complex inline assembly with many operands and constraints */
        __asm__ volatile (
            /* Multiple output operands with different constraints */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            
            /* Byte register constraint */
            "movb %b[in4], %%cl\n\t"
            "xorb %%cl, %b[out2]\n\t"
            
            /* Memory operand with offset */
            "addl %[mem1], %%ebx\n\t"
            
            /* Top of stack constraint */
            "fldl %[double_in]\n\t"
            "fstpl %[double_out]\n\t"
            
            /* Outputs */
            : [out1] "=r" (r0),           /* General register */
              [out2] "=q" (r1),           /* Byte register (a,b,c,d) */
              [double_out] "=t" (global_accumulator)  /* Top of FP stack */
            
            /* Inputs with mixed constraints */
            : [in1] "r" (r2),             /* Register */
              [in2] "rm" (r3),            /* Register or memory */
              [in3] "i" (0x1234),         /* Immediate */
              [in4] "q" (r4),             /* Byte register */
              [mem1] "m" (input[i % ARRAY_SIZE]),  /* Memory */
              [double_in] "m" (vd0[0])    /* Memory for FP */
            
            /* Clobber many registers */
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "memory", "cc", "st", "st(1)", "st(2)", "st(3)"
        );
        
        /* Another asm with earlyclobber and matching constraints */
        int temp1, temp2;
        __asm__ volatile (
            "movl %2, %0\n\t"
            "leal (%0, %0, 2), %1\n\t"    /* %1 = 3 * %0 */
            : "=&r" (temp1), "=r" (temp2)  /* & = earlyclobber */
            : "0" (r5)                     /* Matching constraint */
            : "cc"
        );
        
        /* Use results to prevent dead code elimination */
        output[i % ARRAY_SIZE] = r0 + r1 + temp1 + temp2;
        
        /* Unrolled computation to increase register pressure */
        #pragma unroll(UNROLL_FACTOR)
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            int idx = (i * UNROLL_FACTOR + j) % ARRAY_SIZE;
            
            /* Complex asm with 5+ operands */
            __asm__ volatile (
                "imull %%eax, %%ebx\n\t"
                "addl %%ecx, %%edx\n\t"
                "xorl %%esi, %%edi\n\t"
                : "=a" (r0), "=b" (r1), "=c" (r2), "=d" (r3)
                : "S" (r4), "D" (r5), "m" (input[idx]),
                  "i" (j), "r" (global_counter)
                : "cc"
            );
            
            output[idx] += r0 + r1 + r2 + r3;
        }
        
        /* Rotate register values */
        int tmp = r0;
        r0 = r1; r1 = r2; r2 = r3; r3 = r4;
        r4 = r5; r5 = r6; r6 = r7; r7 = r8;
        r8 = r9; r9 = r10; r10 = r11; r11 = r12;
        r12 = r13; r13 = tmp;
    }
    
    /* Store final values */
    output[0] = r0; output[1] = r1; output[2] = r2;
}

/* Function to trigger secondary reloads */
void test_secondary_reloads(double *input, double *output, int iterations) {
    /* Variables that will need secondary reloads */
    double d0 = input[0], d1 = input[1], d2 = input[2], d3 = input[3];
    double d4 = input[4], d5 = input[5], d6 = input[6], d7 = input[7];
    
    /* Mixed precision operations */
    float f0 = (float)d0, f1 = (float)d1, f2 = (float)d2, f3 = (float)d3;
    
    for (int i = 0; i < iterations; i++) {
        /* Asm requiring specific register classes that may need secondary reloads */
        
        /* Force accumulator constraint then base register constraint */
        int acc_val;
        __asm__ volatile (
            "movl $0x1, %%eax\n\t"
            "cpuid\n\t"                    /* Requires specific registers */
            : "=a" (acc_val)               /* Output in accumulator */
            : "a" (0x1), "c" (0), "d" (0)  /* Inputs in specific registers */
            : "ebx", "cc"
        );
        
        /* Now use the result with a different constraint */
        int base_val;
        __asm__ volatile (
            "movl %1, %%ebx\n\t"
            "leal (%%ebx, %%ebx, 4), %0\n\t"  /* %0 = 5 * %1 */
            : "=r" (base_val)
            : "b" (acc_val)                 /* Input in base register */
            : "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Another asm that could combine but won't due to barrier */
        __asm__ volatile (
            "addl $1, %0\n\t"
            : "+r" (base_val)
            :: "cc"
        );
        
        /* Complex FP asm with mismatched constraints */
        double result;
        __asm__ volatile (
            /* "f" constraint for x87 register, may need secondary reload */
            "fldl %1\n\t"
            "fmull %2\n\t"
            "fstpl %0\n\t"
            : "=m" (result)                /* Memory output */
            : "f" (d0), "fm" (d1)          /* f = x87 register, fm = register/memory */
            : "st", "st(1)"
        );
        
        /* Use R constraint (legacy register) which may need secondary reload
           if allocated to R8-R15 */
        long legacy_reg;
        __asm__ volatile (
            "mov %1, %0\n\t"
            "ror $8, %0\n\t"
            : "=R" (legacy_reg)            /* R = legacy register (eax-edi) */
            : "r" ((long)base_val)
            : "cc"
        );
        
        /* Store results with complex addressing */
        int idx = i % (ARRAY_SIZE / 2);
        output[idx] = result + legacy_reg;
        output[idx + ARRAY_SIZE/2] = d0 + d1;
        
        /* Rotate values to keep them live */
        double tmp = d0;
        d0 = d1; d1 = d2; d2 = d3; d3 = d4;
        d4 = d5; d5 = d6; d6 = d7; d7 = tmp;
    }
}

/* Function with optional reloads */
void test_optional_reloads(float *input, float *output, int iterations, int mode) {
    /* Variables with optional constraints */
    float f0 = input[0], f1 = input[1], f2 = input[2], f3 = input[3];
    float f4 = input[4], f5 = input[5], f6 = input[6], f7 = input[7];
    
    for (int i = 0; i < iterations; i++) {
        /* Optional output constraint */
        float opt_result;
        __asm__ volatile (
            "movss %1, %0\n\t"
            "addss %2, %0\n\t"
            : "=?r" (opt_result)          /* ? = optional */
            : "rm" (f0), "rm" (f1)
            : "cc"
        );
        
        /* Another optional with different mode */
        char byte_opt;
        __asm__ volatile (
            "movb %1, %b0\n\t"            /* %b0 = low byte */
            : "=?q" (byte_opt)            /* Optional byte register */
            : "ri" ((char)(i & 0xFF))
            : "cc"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't combine due to barrier */
        __asm__ volatile (
            "addss %1, %0\n\t"
            : "+r" (opt_result)
            : "rm" (f2)
            : "cc"
        );
        
        /* Control flow dependent asm */
        if (mode & 1) {
            __asm__ volatile (
                "mulss %1, %0\n\t"
                : "+r" (opt_result)
                : "rm" (f3)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "subss %1, %0\n\t"
                : "+r" (opt_result)
                : "rm" (f4)
                : "cc"
            );
        }
        
        /* Store with complex index */
        int idx = (i * 7) % ARRAY_SIZE;
        output[idx] = opt_result + byte_opt;
        
        /* Update live variables */
        float tmp = f0;
        f0 = f1; f1 = f2; f2 = f3; f3 = f4;
        f4 = f5; f5 = f6; f6 = f7; f7 = tmp;
    }
}

/* Function with control flow dependent reloads */
void test_control_flow_reloads(long *input, long *output, int size, int threshold) {
    long accum = 0;
    
    for (int i = 0; i < size; i++) {
        long val = input[i];
        
        /* Different asm blocks in different control flow paths */
        if (val > threshold) {
            /* Path 1: Complex asm with many clobbers */
            __asm__ volatile (
                "movq %1, %%rax\n\t"
                "imulq $0x12345678, %%rax\n\t"
                "movq %%rax, %0\n\t"
                : "=rm" (val)
                : "rm" (val)
                : "rax", "rdx", "cc"
            );
        } else if (val < -threshold) {
            /* Path 2: Different constraints */
            __asm__ volatile (
                "negq %0\n\t"
                "shrq $3, %0\n\t"
                : "+r" (val)
                :: "cc"
            );
        } else {
            /* Path 3: Yet another pattern */
            __asm__ volatile (
                "leaq (%1, %1, 2), %0\n\t"  /* %0 = 3 * %1 */
                : "=r" (val)
                : "r" (val)
                : "cc"
            );
        }
        
        /* Nested loop with register pressure */
        for (int j = 0; j < 4; j++) {
            long temp = val + j;
            
            /* Asm inside nested loop - hard to optimize */
            __asm__ volatile (
                "addq %1, %0\n\t"
                "rorq $5, %0\n\t"
                : "+r" (temp)
                : "ri" (accum)
                : "cc"
            );
            
            accum ^= temp;
        }
        
        output[i] = val;
    }
    
    /* Final complex asm */
    __asm__ volatile (
        "movq %1, %%rax\n\t"
        "xorq %%rdx, %%rdx\n\t"
        "movq $100, %%rcx\n\t"
        "divq %%rcx\n\t"
        "movq %%rdx, %0\n\t"
        : "=r" (output[0])
        : "r" (accum)
        : "rax", "rcx", "rdx", "cc"
    );
}

int main(int argc, char **argv) {
    /* Parse command line arguments */
    int iterations = 100;
    int mode = 1;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations <= 0) iterations = 100;
    if (iterations > 10000) iterations = 10000;
    
    printf("Running reload tests with iterations=%d, mode=%d\n", iterations, mode);
    
    /* Initialize arrays with mixed data */
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
        int_data[i] = (i * 37) & 0xFFF;
        double_data[i] = (i * 0.12345);
        float_data[i] = (i * 0.54321f);
        long_data[i] = (long)i * 1000000;
    }
    
    /* Run tests to trigger different reload patterns */
    test_primary_reloads(int_data, int_output, iterations);
    test_secondary_reloads(double_data, double_output, iterations / 2);
    test_optional_reloads(float_data, float_output, iterations / 4, mode);
    test_control_flow_reloads(long_data, long_output, ARRAY_SIZE / 8, 500000);
    
    /* Compute checksum to ensure all code executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long long)double_output[i];
        checksum += (long long)float_output[i];
        checksum += long_output[i];
    }
    
    /* Use checksum to prevent dead code elimination */
    __asm__ volatile (
        "addq %1, %0\n\t"
        : "+r" (checksum)
        : "r" (global_counter)
        : "cc"
    );
    
    printf("Checksum: %lld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
