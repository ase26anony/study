/* reload_test.c - Complex inline assembly to trigger reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <stdint.h>

#define UNROLL_FACTOR 32
#define ARRAY_SIZE 1024

/* Test functions */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout);
void test_secondary_reloads(int iterations, long *in, long *out, float *fin, float *fout);
void test_optional_reloads(int iterations, uint64_t *in, uint64_t *out, __m128i *vin, __m128i *vout);
void test_control_flow_reloads(int mode, int count, int *data, int *result);

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_counter = 0;

/* Complex inline assembly with many operands and constraints */
void test_primary_reloads(int iterations, int *in, int *out, double *din, double *dout) {
    int i, j;
    long temp1, temp2, temp3, temp4, temp5;
    double dtemp1, dtemp2;
    int r1, r2, r3, r4, r5;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < iterations; i++) {
        /* Force many values to be live simultaneously */
        temp1 = in[i * 4 + 0];
        temp2 = in[i * 4 + 1];
        temp3 = in[i * 4 + 2];
        temp4 = in[i * 4 + 3];
        temp5 = in[i] + global_counter;
        
        dtemp1 = din[i * 2];
        dtemp2 = din[i * 2 + 1];
        
        /* Complex asm with 7 operands, mixed constraints */
        __asm__ volatile (
            /* Outputs with different constraints */
            "=r" (r1),      /* General register */
            "=&r" (r2),     /* Early clobber */
            "=q" (r3),      /* Byte-addressable register (eax, ebx, ecx, edx) */
            "=a" (r4),      /* Accumulator */
            "=d" (r5),      /* Data register */
            
            /* Inputs with various constraints */
            : "0" (temp1),  /* Matching constraint for r1 */
              "r" (temp2),  /* General register */
              "rm" (temp3), /* Register or memory */
              "i" (0x1234), /* Immediate */
              "g" (temp4),  /* General (register, memory, or immediate) */
              "r" (temp5),
              "m" (dtemp1)  /* Memory */
            
            /* Clobber many registers */
            : "rcx", "rbx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory", "cc"
        );
        
        /* Use results to prevent dead code elimination */
        out[i * 4 + 0] = r1 + r2;
        out[i * 4 + 1] = r3 ^ r4;
        out[i * 4 + 2] = r5;
        out[i * 4 + 3] = (int)dtemp2;
        
        /* Another asm with floating point constraints */
        __asm__ volatile (
            "movq %1, %%xmm0\n\t"
            "movq %2, %%xmm1\n\t"
            "addpd %%xmm1, %%xmm0\n\t"
            "movq %%xmm0, %0"
            : "=m" (dout[i])
            : "m" (dtemp1), "m" (dtemp2)
            : "xmm0", "xmm1", "memory"
        );
        
        /* Memory barrier to prevent reload combination */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Force secondary reloads with mismatched constraints */
void test_secondary_reloads(int iterations, long *in, long *out, float *fin, float *fout) {
    int i;
    long a, b, c, d;
    float fa, fb;
    
    for (i = 0; i < iterations; i++) {
        a = in[i * 2];
        b = in[i * 2 + 1];
        c = a + b + global_seed;
        d = a * b;
        
        fa = fin[i];
        fb = fin[i + ARRAY_SIZE/2];
        
        /* asm requiring specific register classes that may need secondary reloads */
        __asm__ volatile (
            /* Output must be in rdi (R constraint) - may need secondary reload */
            "=R" (a),
            /* Output in rsi (S constraint) */
            "=S" (b)
            
            : "0" (c),      /* Matching constraint - same as first output */
              "r" (d),      /* General register input */
              "a" (i),      /* Must be in eax/rax */
              "b" (global_counter)  /* Must be in ebx/rbx */
            
            /* Clobber flags */
            : "cc"
        );
        
        /* Another asm that could combine but won't due to memory barrier */
        __asm__ volatile ("" ::: "memory");
        
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "imul %2, %%eax\n\t"
            "mov %%eax, %0"
            : "=r" (c)
            : "r" (a), "rm" (b)  /* 'rm' constraint may need secondary reload if memory */
            : "rax", "cc"
        );
        
        /* Use floating point with integer constraints forcing moves */
        __asm__ volatile (
            "cvtsi2ss %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "movss %%xmm0, %0"
            : "=m" (fout[i])
            : "r" ((int)fa), "m" (fb)  /* Integer register for float input */
            : "xmm0", "memory"
        );
        
        out[i * 2] = a + c;
        out[i * 2 + 1] = b;
    }
}

/* Test optional reloads and nocombine scenarios */
void test_optional_reloads(int iterations, uint64_t *in, uint64_t *out, __m128i *vin, __m128i *vout) {
    int i;
    uint64_t opt1, opt2, opt3;
    __m128i v1, v2;
    
    for (i = 0; i < iterations; i++) {
        opt1 = in[i];
        opt2 = in[i + 256];
        opt3 = in[i + 512];
        
        v1 = vin[i];
        v2 = vin[i + 128];
        
        /* asm with optional constraints */
        __asm__ volatile (
            "=?r" (opt1),   /* Optional output */
            "=r" (opt2),
            "=&r" (opt3)
            
            : "0" (opt1),   /* Matching constraint */
              "1" (opt2),   /* Another matching constraint */
              "rm" (opt3),  /* Register or memory */
              "i" (0xFF),   /* Immediate */
              "g" (global_seed)  /* General */
            
            : "cc", "memory"
        );
        
        /* Volatile barrier prevents combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that could combine but won't */
        __asm__ volatile (
            "mov %1, %%rax\n\t"
            "add %2, %%rax\n\t"
            "mov %%rax, %0"
            : "=?r" (opt1)  /* Optional output */
            : "r" (opt2), "rm" (opt3)
            : "rax", "cc"
        );
        
        /* Vector operations alongside scalar */
        __asm__ volatile (
            "movdqu %1, %%xmm0\n\t"
            "movdqu %2, %%xmm1\n\t"
            "paddq %%xmm1, %%xmm0\n\t"
            "movdqu %%xmm0, %0"
            : "=m" (vout[i])
            : "m" (v1), "m" (v2)
            : "xmm0", "xmm1", "memory"
        );
        
        out[i] = opt1 ^ opt2 ^ opt3;
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int count, int *data, int *result) {
    int i, temp;
    
    for (i = 0; i < count; i++) {
        temp = data[i];
        
        /* Different asm blocks based on control flow */
        if (mode & 1) {
            __asm__ volatile (
                "lea (%1, %2), %0\n\t"
                "add $0x1234, %0"
                : "=r" (temp)
                : "r" (temp), "r" (i)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "imul $0x5678, %1, %0\n\t"
                "sub $0x9ABC, %0"
                : "=r" (temp)
                : "r" (temp)
                : "cc"
            );
        }
        
        if (i % 3 == 0) {
            /* Another asm in different control flow path */
            __asm__ volatile (
                "ror $8, %0"
                : "+r" (temp)
                :: "cc"
            );
        }
        
        /* Complex condition with asm */
        __asm__ volatile (
            "test %1, %1\n\t"
            "cmovg %2, %0"
            : "+r" (temp)
            : "r" (global_counter), "r" (temp + 1000)
            : "cc"
        );
        
        result[i] = temp;
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 1;
    
    /* Parse command line */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) mode = atoi(argv[2]);
    
    if (iterations > ARRAY_SIZE/4) iterations = ARRAY_SIZE/4;
    
    /* Allocate and initialize arrays with mixed types */
    int *int_in = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int *int_out = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    long *long_in = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    long *long_out = (long*)aligned_alloc(64, ARRAY_SIZE * sizeof(long));
    float *float_in = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float *float_out = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double *double_in = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double *double_out = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    uint64_t *uint64_in = (uint64_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(uint64_t));
    uint64_t *uint64_out = (uint64_t*)aligned_alloc(64, ARRAY_SIZE * sizeof(uint64_t));
    __m128i *vec_in = (__m128i*)aligned_alloc(64, (ARRAY_SIZE/4) * sizeof(__m128i));
    __m128i *vec_out = (__m128i*)aligned_alloc(64, (ARRAY_SIZE/4) * sizeof(__m128i));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_in[i] = i * 3 + 1;
        long_in[i] = i * 5 + 2;
        float_in[i] = i * 0.1f;
        double_in[i] = i * 0.01;
        uint64_in[i] = (uint64_t)i * 7 + 3;
        if (i < ARRAY_SIZE/4) {
            vec_in[i] = _mm_set_epi32(i, i+1, i+2, i+3);
        }
    }
    
    /* Run tests to trigger reloads */
    test_primary_reloads(iterations, int_in, int_out, double_in, double_out);
    test_secondary_reloads(iterations/2, long_in, long_out, float_in, float_out);
    test_optional_reloads(iterations/4, uint64_in, uint64_out, vec_in, vec_out);
    test_control_flow_reloads(mode, iterations, int_in + 256, int_out + 256);
    
    /* Compute checksum to ensure all asm executed */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_out[i];
        checksum += long_out[i];
        checksum += *(uint32_t*)&float_out[i];
        checksum += *(uint64_t*)&double_out[i];
        checksum += uint64_out[i];
        if (i < ARRAY_SIZE/4) {
            checksum += ((uint64_t*)&vec_out[i])[0];
            checksum += ((uint64_t*)&vec_out[i])[1];
        }
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Cleanup */
    free(int_in);
    free(int_out);
    free(long_in);
    free(long_out);
    free(float_in);
    free(float_out);
    free(double_in);
    free(double_out);
    free(uint64_in);
    free(uint64_out);
    free(vec_in);
    free(vec_out);
    
    return 0;
}
