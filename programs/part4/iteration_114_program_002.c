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
void test_secondary_reloads(double *dbl_in, double *dbl_out, int count);
void test_optional_reloads(float *flt_in, float *flt_out, int mode);
void test_control_flow_reloads(int argc, char **argv, int *results);

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.141592653589793;

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line arguments */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    /* Initialize arrays with mixed data types */
    int *int_array = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_output = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    double *dbl_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *dbl_output = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *flt_array = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *flt_output = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    int *flow_results = (int*)malloc(16 * sizeof(int));
    
    if (!int_array || !int_output || !dbl_array || !dbl_output || 
        !flt_array || !flt_output || !flow_results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        dbl_array[i] = i * 0.5 + 1.0;
        flt_array[i] = i * 0.25f + 0.5f;
    }
    
    /* Execute test functions to trigger reloads */
    test_primary_reloads(iterations, int_array, int_output);
    test_secondary_reloads(dbl_array, dbl_output, ARRAY_SIZE / 4);
    test_optional_reloads(flt_array, flt_output, mode);
    test_control_flow_reloads(argc, argv, flow_results);
    
    /* Compute checksum to ensure all assembly executed */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_output[i];
        checksum += (uint64_t)dbl_output[i];
        checksum += (uint64_t)flt_output[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += flow_results[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(int_output);
    free(dbl_array);
    free(dbl_output);
    free(flt_array);
    free(flt_output);
    free(flow_results);
    
    return 0;
}

/* Complex inline assembly with multiple operands and constraints */
void test_primary_reloads(int iterations, int *input, int *output) {
    /* Many live variables to create register pressure */
    int a = input[0], b = input[1], c = input[2], d = input[3];
    int e = input[4], f = input[5], g = input[6], h = input[7];
    int i = input[8], j = input[9], k = input[10], l = input[11];
    int m = input[12], n = input[13], o = input[14], p = input[15];
    
    /* Vector types to increase register pressure further */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(e, f, g, h);
    __m256d vd1 = _mm256_set_pd(d, c, b, a);
    __m256d vd2 = _mm256_set_pd(h, g, f, e);
    
    /* Unrolled loop with complex inline assembly */
    for (int idx = 0; idx < iterations; idx++) {
        /* Extended asm with 7 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (a),      /* General register */
            "=&r" (b),     /* Earlyclobber general register */
            "=q" (c),      /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (d),      /* Accumulator */
            "=d" (e),      /* Data register */
            "=r" (f),      /* General register */
            "=m" (output[idx % 16])  /* Memory output */
            
            /* Inputs with various constraints */
            : "0" (a),     /* Matching constraint - same as output 0 */
            "r" (b),       /* General register input */
            "i" (12345),   /* Immediate constant */
            "m" (input[(idx * 7) % ARRAY_SIZE]),  /* Memory input */
            "r" (global_counter),  /* Global variable access */
            "t" (vd1[0]),  /* Top of FP stack */
            "x" (v1)       /* Any SSE register */
            
            /* Clobber list */
            : "memory", "cc", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Another asm with different constraints to prevent combining */
        __asm__ volatile ("" ::: "memory");  /* Memory barrier */
        
        __asm__ volatile (
            "=r" (g),
            "=r" (h),
            "=q" (i),      /* Byte register */
            "=b" (j),      /* Base register */
            "=c" (k),      /* Counter register */
            "=S" (l),      /* Source index */
            "=D" (m)       /* Destination index */
            : 
            "r" (a),
            "r" (b),
            "m" (input[(idx * 13 + 1) % ARRAY_SIZE]),
            "r" (c),
            "i" (idx),
            "X" (vd2),     /* Any SSE/AVX register */
            "f" (global_double)  /* Floating point register */
            : "memory", "cc"
        );
        
        /* Use all variables to keep them live */
        v1 = _mm_add_epi32(v1, _mm_set1_epi32(a + b + c));
        vd1 = _mm256_add_pd(vd1, _mm256_set1_pd(d + e + f));
        
        /* Store results to memory to force spills */
        output[(idx * 3) % ARRAY_SIZE] = a + g;
        output[(idx * 5 + 1) % ARRAY_SIZE] = b + h;
        output[(idx * 7 + 2) % ARRAY_SIZE] = c + i;
    }
    
    /* Final store to ensure side effects */
    global_counter += a + b + c + d + e + f + g + h;
}

/* Force secondary reloads through constraint mismatches */
void test_secondary_reloads(double *dbl_in, double *dbl_out, int count) {
    /* Use x86-specific legacy register constraints */
    for (int i = 0; i < count; i += 4) {
        double d1 = dbl_in[i];
        double d2 = dbl_in[i + 1];
        double d3 = dbl_in[i + 2];
        double d4 = dbl_in[i + 3];
        
        /* "R" constraint for legacy register (eax, ebx, ecx, edx, esi, edi, ebp) */
        /* May require secondary reload if allocated to R8-R15 */
        __asm__ volatile (
            "=R" (d1),     /* Legacy register output */
            "=R" (d2),
            "=m" (dbl_out[i]),
            "=m" (dbl_out[i + 1])
            :
            "R" (d3),      /* Legacy register input */
            "R" (d4),
            "m" (dbl_in[(i * 2) % ARRAY_SIZE]),
            "i" (0x3FF0000000000000UL)  /* Double 1.0 as immediate */
            : "memory", "cc", "rax", "rbx", "rcx", "rdx"
        );
        
        /* Mix register classes - "a" constraint then "b" constraint */
        int64_t int_val = (int64_t)d1;
        __asm__ volatile (
            "=a" (int_val)  /* Result in accumulator */
            :
            "b" (int_val),  /* Input in base register - may need move from rax to rbx */
            "m" (dbl_out[i + 2]),
            "r" (global_counter)
            : "memory", "cc"
        );
        
        dbl_out[i + 3] = (double)int_val;
    }
}

/* Test optional reloads and nocombine behavior */
void test_optional_reloads(float *flt_in, float *flt_out, int mode) {
    float f1 = flt_in[0], f2 = flt_in[1], f3 = flt_in[2], f4 = flt_in[3];
    
    /* Optional constraints with '?' modifier */
    for (int i = 0; i < ARRAY_SIZE / 8; i++) {
        __asm__ volatile (
            "=?r" (f1),    /* Optional output */
            "=r" (f2),
            "=?m" (flt_out[i * 2]),  /* Optional memory output */
            "=r" (f3)
            :
            "0" (f1),      /* Matching constraint with optional output */
            "r" (flt_in[i * 3 % ARRAY_SIZE]),
            "i" (mode),
            "m" (flt_in[i * 5 % ARRAY_SIZE]),
            "r" (i)
            : "memory", "cc"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers to force nocombine */
        __asm__ volatile (
            "=r" (f4),
            "=m" (flt_out[i * 2 + 1])
            :
            "r" (f2),
            "r" (f3),
            "m" (flt_in[i * 7 % ARRAY_SIZE])
            : "memory", "cc", "xmm0", "xmm1"  /* Different clobbers */
        );
        
        /* Use volatile asm with side effects */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0"
            : "=r" (flt_out[i * 3])
            : "r" ((int)f1), "r" ((int)f4)
            : "eax", "cc"
        );
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int argc, char **argv, int *results) {
    int x = argc * 10;
    int y = 0, z = 0, w = 0;
    
    /* Complex control flow with inline assembly */
    for (int i = 0; i < 16; i++) {
        if (i % 3 == 0) {
            /* Branch 1 - specific constraints */
            __asm__ volatile (
                "=r" (x),
                "=a" (y)
                :
                "r" (x),
                "i" (i),
                "m" (results[i])
                : "cc"
            );
        } else if (i % 3 == 1) {
            /* Branch 2 - different constraints */
            __asm__ volatile (
                "=b" (z),
                "=c" (w)
                :
                "r" (y),
                "m" (results[(i + 1) % 16]),
                "r" (argv[0] ? argv[0][0] : 'A')  /* Runtime dependent */
                : "cc", "rbx", "rcx"
            );
        } else {
            /* Branch 3 - yet another set of constraints */
            __asm__ volatile (
                "=r" (x),
                "=r" (y),
                "=q" (z)
                :
                "0" (x),
                "1" (y),
                "m" (results[(i * 2) % 16])
                : "cc"
            );
        }
        
        /* Store result based on control flow */
        results[i] = x + y + z + w + i;
        
        /* Nested loop with more asm */
        for (int j = 0; j < 4; j++) {
            int temp = results[i] + j;
            __asm__ volatile (
                "=r" (temp)
                :
                "r" (temp),
                "i" (j),
                "m" (global_counter)
                : "cc"
            );
            results[i] = temp;
        }
    }
}
