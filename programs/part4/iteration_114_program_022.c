/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *input, int *output);
void test_secondary_reloads(int iterations, double *input, double *output);
void test_optional_reloads(int iterations, float *input, float *output);
void test_register_pressure(int iterations, long *input, long *output);

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_sum = 0;

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *input, int *output) {
    int i, j;
    long temp1, temp2, temp3, temp4, temp5;
    int a, b, c, d, e;
    char byte_val;
    short word_val;
    
    /* Unrolled loop to create register pressure */
    for (i = 0; i < iterations; i += UNROLL_FACTOR) {
        /* Load many values into registers */
        a = input[i];
        b = input[i + 1];
        c = input[i + 2];
        d = input[i + 3];
        e = input[i + 4];
        
        /* Complex asm with 8 operands, mixing constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (temp1),      /* General register */
            "=&r" (temp2),     /* Earlyclobber */
            "=q" (byte_val),   /* Byte register (a,b,c,d) */
            "=r" (temp3),
            "=t" (temp4),      /* Top of FP stack */
            "=a" (temp5),      /* Accumulator */
            
            /* Inputs with various constraints */
            : "0" (a),         /* Matching constraint with temp1 */
              "r" (b),
              "rm" (c),        /* Register or memory */
              "i" (12345),     /* Immediate */
              "g" (d),         /* General (register, memory, or immediate) */
              "r" (e),
              "m" (input[i + 5])  /* Memory operand */
            
            /* Clobbers */
            : "cc", "memory", "ebx", "ecx", "edx", "st", "st(1)"
        );
        
        /* More asm statements with different modes */
        for (j = 0; j < 4; j++) {
            int idx = i + j;
            __asm__ volatile (
                "movl %1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                "movb %%al, %2\n\t"
                : "+r" (output[idx]), "=r" (temp1), "=q" (byte_val)
                : "r" (input[idx]), "i" (global_counter)
                : "eax", "cc"
            );
        }
        
        /* Use the results to prevent dead code elimination */
        output[i] = temp1 + temp2 + byte_val;
        global_sum += temp3 + temp4 + temp5;
    }
}

/* Force secondary reloads with mismatched constraints */
void test_secondary_reloads(int iterations, double *input, double *output) {
    int i;
    double acc = 0.0;
    __m128d vec_acc = _mm_setzero_pd();
    __m256d vec_acc256 = _mm256_setzero_pd();
    
    for (i = 0; i < iterations; i++) {
        double x = input[i];
        double y = input[i + 1];
        
        /* Force secondary reload by using 'a' constraint then 'b' constraint */
        long ax_val, bx_val;
        
        __asm__ volatile (
            "movq %2, %%rax\n\t"
            "addq $1, %%rax\n\t"
            : "=a" (ax_val)
            : "0" ((long)x), "m" (input[i + 2])
            : "cc"
        );
        
        /* Now force move from rax to rbx through secondary reload */
        __asm__ volatile (
            "movq %1, %%rbx\n\t"
            "imulq $3, %%rbx\n\t"
            : "=b" (bx_val)
            : "a" (ax_val), "r" ((long)y)
            : "cc"
        );
        
        /* AVX operations to increase register pressure */
        __m128d v1 = _mm_set_pd(x, y);
        __m128d v2 = _mm_loadu_pd(&input[i % ARRAY_SIZE]);
        vec_acc = _mm_add_pd(vec_acc, _mm_mul_pd(v1, v2));
        
        /* Conditional asm to create control flow dependent reloads */
        if (i % 3 == 0) {
            __asm__ volatile (
                "mov %1, %%r8\n\t"
                "mov %2, %%r9\n\t"
                "add %%r9, %%r8\n\t"
                "mov %%r8, %0\n\t"
                : "=R" (output[i])  /* Legacy register constraint */
                : "r" (bx_val), "r" (ax_val)
                : "r8", "r9", "cc"
            );
        } else if (i % 3 == 1) {
            __asm__ volatile (
                "xchg %1, %0\n\t"
                : "+r" (output[i]), "+r" (bx_val)
                :
                : "cc"
            );
        } else {
            /* Memory barrier to prevent reload combination */
            __asm__ volatile ("" ::: "memory");
            
            __asm__ volatile (
                "lea (%1, %2, 2), %0\n\t"
                : "=r" (output[i])
                : "r" (bx_val), "r" (ax_val)
                : "cc"
            );
        }
        
        /* Use AVX-256 to consume more registers */
        if (i % 8 == 0) {
            __m256d v256_1 = _mm256_set_pd(x, y, x*2, y*2);
            __m256d v256_2 = _mm256_loadu_pd(&input[(i * 2) % (ARRAY_SIZE - 3)]);
            vec_acc256 = _mm256_add_pd(vec_acc256, _mm256_mul_pd(v256_1, v256_2));
        }
        
        acc += (double)bx_val;
    }
    
    /* Store vector results to memory */
    _mm_storeu_pd(&output[iterations], vec_acc);
    _mm256_storeu_pd(&output[iterations + 2], vec_acc256);
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(int iterations, float *input, float *output) {
    int i;
    
    for (i = 0; i < iterations; i++) {
        float f1 = input[i];
        float f2 = input[i + 1];
        float f3 = input[i + 2];
        int opt_result;
        
        /* Optional output constraint */
        __asm__ volatile (
            "test %2, %2\n\t"
            "jz 1f\n\t"
            "movl $1, %0\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "movl $0, %0\n\t"
            "2:\n\t"
            : "=?r" (opt_result)  /* Optional output */
            : "r" ((int)f1), "r" ((int)f2)
            : "cc"
        );
        
        /* Memory barrier between similar asm statements */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to prevent combination */
        __asm__ volatile (
            "addl %1, %0\n\t"
            : "+r" (opt_result)
            : "r" ((int)f3)
            : "cc", "eax"  /* Different clobber list */
        );
        
        /* Another barrier */
        __asm__ volatile ("" ::: "memory");
        
        /* Third asm that could combine but won't due to barriers */
        __asm__ volatile (
            "imull $7, %0\n\t"
            : "+r" (opt_result)
            :
            : "cc"
        );
        
        output[i] = (float)opt_result;
        
        /* Complex asm with many operands in conditional */
        if (opt_result > 100) {
            long r8_val, r9_val, r10_val;
            __asm__ volatile (
                "mov %3, %%r8\n\t"
                "mov %4, %%r9\n\t"
                "mov %5, %%r10\n\t"
                "add %%r9, %%r8\n\t"
                "add %%r10, %%r8\n\t"
                "mov %%r8, %0\n\t"
                "mov %%r9, %1\n\t"
                "mov %%r10, %2\n\t"
                : "=r" (r8_val), "=r" (r9_val), "=r" (r10_val)
                : "r" ((long)f1 * 2), 
                  "r" ((long)f2 * 3), 
                  "r" ((long)f3 * 4),
                  "m" (input[i + 3])  /* Extra memory operand */
                : "r8", "r9", "r10", "cc", "memory"
            );
            
            output[i + 1] = (float)(r8_val + r9_val + r10_val);
        }
    }
}

/* Maximum register pressure test */
void test_register_pressure(int iterations, long *input, long *output) {
    int i, j;
    
    /* Many live variables to force spilling */
    long v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    long v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
    __m256d ymm0, ymm1, ymm2;
    
    for (i = 0; i < iterations; i++) {
        /* Initialize many variables */
        v1 = input[i * 10];
        v2 = input[i * 10 + 1];
        v3 = input[i * 10 + 2];
        v4 = input[i * 10 + 3];
        v5 = input[i * 10 + 4];
        v6 = input[i * 10 + 5];
        v7 = input[i * 10 + 6];
        v8 = input[i * 10 + 7];
        v9 = input[i * 10 + 8];
        v10 = input[i * 10 + 9];
        
        /* Use them in complex asm */
        for (j = 0; j < 4; j++) {
            __asm__ volatile (
                /* Multiple outputs with earlyclobber */
                "=r" (v11), "=&r" (v12), "=&r" (v13),
                "=r" (v14), "=r" (v15),
                
                /* Inputs - many live values */
                : "0" (v1 + j), "1" (v2), "2" (v3),
                  "r" (v4), "r" (v5), "r" (v6),
                  "r" (v7), "r" (v8), "r" (v9),
                  "r" (v10), "i" (j),
                  "m" (input[(i + j) % ARRAY_SIZE])
                
                /* Extensive clobber list */
                : "cc", "memory",
                  "rax", "rbx", "rcx", "rdx",
                  "rsi", "rdi", "r8", "r9", "r10",
                  "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7",
                  "ymm0", "ymm1", "ymm2", "ymm3"
            );
            
            /* Use vector intrinsics to consume more registers */
            xmm0 = _mm_set_epi64x(v11, v12);
            xmm1 = _mm_set_epi64x(v13, v14);
            xmm2 = _mm_add_epi64(xmm0, xmm1);
            
            ymm0 = _mm256_set_pd(v1, v2, v3, v4);
            ymm1 = _mm256_set_pd(v5, v6, v7, v8);
            ymm2 = _mm256_add_pd(ymm0, ymm1);
            
            /* Store results */
            _mm_storeu_si128((__m128i*)&output[i * 4 + j * 2], xmm2);
            if (j % 2 == 0) {
                _mm256_storeu_pd((double*)&output[i * 4 + j * 2 + 4], ymm2);
            }
        }
        
        /* Chain computations to keep values live */
        v16 = v11 + v12;
        v17 = v13 + v14;
        v18 = v15 + v16;
        v19 = v17 + v18;
        v20 = v19 * 2;
        
        output[i] = v20;
        
        /* Conditional asm with many live values */
        if (v20 > 1000) {
            __asm__ volatile (
                "mov %1, %%rax\n\t"
                "mov %2, %%rbx\n\t"
                "mov %3, %%rcx\n\t"
                "mov %4, %%rdx\n\t"
                "add %%rbx, %%rax\n\t"
                "add %%rcx, %%rax\n\t"
                "add %%rdx, %%rax\n\t"
                "mov %%rax, %0\n\t"
                : "=rm" (output[i + 1])  /* Register or memory output */
                : "r" (v16), "r" (v17), "r" (v18), "r" (v19),
                  "m" (input[(i + 5) % ARRAY_SIZE])
                : "rax", "rbx", "rcx", "rdx", "cc", "memory"
            );
        }
    }
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > ARRAY_SIZE / 2) iterations = ARRAY_SIZE / 2;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Allocate and initialize arrays with mixed data */
    int *int_data = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    long *long_data = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    
    int *int_output = (int*)calloc(ARRAY_SIZE, sizeof(int));
    double *double_output = (double*)calloc(ARRAY_SIZE, sizeof(double));
    float *float_output = (float*)calloc(ARRAY_SIZE, sizeof(float));
    long *long_output = (long*)calloc(ARRAY_SIZE, sizeof(long));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i * 3 + 1;
        double_data[i] = i * 1.5;
        float_data[i] = i * 0.75f;
        long_data[i] = i * 5L;
    }
    
    /* Run tests based on mode */
    switch (mode) {
        case 1:
            test_primary_reloads(iterations, int_data, int_output);
            break;
        case 2:
            test_secondary_reloads(iterations, double_data, double_output);
            break;
        case 3:
            test_optional_reloads(iterations, float_data, float_output);
            break;
        case 4:
            test_register_pressure(iterations, long_data, long_output);
            break;
        default:
            /* Run all tests */
            test_primary_reloads(iterations / 4, int_data, int_output);
            test_secondary_reloads(iterations / 4, double_data, double_output);
            test_optional_reloads(iterations / 4, float_data, float_output);
            test_register_pressure(iterations / 4, long_data, long_output);
            break;
    }
    
    /* Compute checksum to ensure all asm executed */
    long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (long)double_output[i];
        checksum += (long)float_output[i];
        checksum += long_output[i];
    }
    
    checksum += global_sum;
    
    printf("Checksum: %ld\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(float_data);
    free(long_data);
    free(int_output);
    free(double_output);
    free(float_output);
    free(long_output);
    
    return 0;
}
