/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>
#include <immintrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 1024

/* Global arrays to create register pressure */
static int int_array[ARRAY_SIZE];
static double double_array[ARRAY_SIZE];
static float float_array[ARRAY_SIZE];
static __m128i vec128_array[ARRAY_SIZE/4];
static __m256d vec256_array[ARRAY_SIZE/8];

/* Test function 1: Primary reloads with mixed constraints */
void test_primary_reloads(int iterations, int mode) {
    volatile int a, b, c, d, e, f, g, h;
    volatile long la, lb, lc, ld;
    volatile float fa, fb, fc, fd;
    volatile double da, db, dc, dd;
    
    /* Create many live variables to exhaust registers */
    int v1 = int_array[0];
    int v2 = int_array[1];
    int v3 = int_array[2];
    int v4 = int_array[3];
    int v5 = int_array[4];
    int v6 = int_array[5];
    int v7 = int_array[6];
    int v8 = int_array[7];
    int v9 = int_array[8];
    int v10 = int_array[9];
    int v11 = int_array[10];
    int v12 = int_array[11];
    int v13 = int_array[12];
    int v14 = int_array[13];
    int v15 = int_array[14];
    int v16 = int_array[15];
    
    /* Mixed mode constraints with earlyclobber */
    for (int i = 0; i < iterations; i++) {
        /* Complex asm with 5+ operands, different modes */
        __asm__ volatile (
            /* Outputs with different register classes and modes */
            "=r" (a),     /* General register, word mode */
            "=&q" (b),    /* Byte register (earlyclobber) */
            "=t" (fa),    /* Top of FP stack */
            "=a" (c),     /* Accumulator */
            "=d" (d)      /* Data register */
            :
            /* Inputs with mixed constraints */
            : "r" (v1),           /* Register */
              "m" (int_array[i]), /* Memory */
              "i" (123),          /* Immediate */
              "r" (v2),
              "0" (v3)            /* Matching constraint */
            : "memory", "cc", "fpsr"
        );
        
        /* Another asm with different constraints */
        __asm__ volatile (
            "=r" (e),
            "=m" (int_array[i+1]),
            "=q" (f)
            : "r" (a),
              "i" (456),
              "m" (double_array[i]),
              "r" (v4),
              "r" (v5)
            : "cc"
        );
        
        /* Use results to prevent optimization */
        v1 = a + b;
        v2 = c * d;
        v3 = e | f;
    }
    
    /* Nested loop with vector operations */
    for (int i = 0; i < iterations/2; i++) {
        for (int j = 0; j < UNROLL_FACTOR; j++) {
            __m128i vec1 = vec128_array[i];
            __m128i vec2 = vec128_array[i+1];
            
            /* Mix vector and scalar operations */
            __asm__ volatile (
                "=x" (vec1),      /* SSE register */
                "=r" (g),
                "=r" (h)
                : "x" (vec2),
                  "r" (v6 + j),
                  "m" (float_array[i*UNROLL_FACTOR + j]),
                  "i" (789),
                  "0" (v7)        /* Matching constraint */
                : "xmm0", "xmm1"
            );
            
            vec128_array[i] = vec1;
            v6 = g;
            v7 = h;
        }
    }
}

/* Test function 2: Secondary reload patterns */
void test_secondary_reloads(int iterations, int mode) {
    volatile int result1, result2, result3;
    volatile long long ll_result;
    
    /* Force secondary reloads with mismatched constraints */
    for (int i = 0; i < iterations; i++) {
        int temp1 = int_array[i];
        int temp2 = int_array[i+1];
        
        /* Constraint requiring register but operand might be in memory */
        __asm__ volatile (
            "=a" (result1),      /* Must be in accumulator */
            "=b" (result2),      /* Must be in base register */
            "=R" (result3)       /* Legacy register constraint */
            : "rm" (temp1),      /* Could be register or memory */
              "r" (temp2),
              "i" (mode)         /* Immediate */
            : "rax", "rbx", "rcx"
        );
        
        /* Use result in another asm with different register class */
        __asm__ volatile (
            "=r" (ll_result)
            : "a" (result1),     /* From accumulator */
              "b" (result2),     /* From base register */
              "m" (double_array[i])
            : "rdx"
        );
        
        /* Store with memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        int_array[i] = result1 + result2 + (int)ll_result;
    }
    
    /* AVX operations alongside scalar */
    for (int i = 0; i < iterations/4; i++) {
        __m256d vec1 = vec256_array[i];
        __m256d vec2 = vec256_array[i+1];
        double scalar = double_array[i];
        
        /* Mix AVX and general purpose registers */
        __asm__ volatile (
            "=x" (vec1),        /* AVX register */
            "=r" (result1)
            : "x" (vec2),
              "r" ((int)scalar),
              "m" (float_array[i*4]),
              "m" (float_array[i*4 + 1])
            : "ymm0", "ymm1", "ymm2"
        );
        
        vec256_array[i] = vec1;
    }
}

/* Test function 3: Optional and non-combine reloads */
void test_optional_reloads(int iterations, int mode) {
    volatile int opt1, opt2, opt3;
    volatile int forced1, forced2;
    
    for (int i = 0; i < iterations; i++) {
        /* Optional constraints with '?' modifier */
        __asm__ volatile (
            "=r" (forced1),
            "=?r" (opt1),       /* Optional output */
            "=?r" (opt2)        /* Optional output */
            : "r" (int_array[i]),
              "m" (double_array[i]),
              "i" (i & 0xFF)
            : "cc"
        );
        
        /* Memory barrier to prevent combination with next asm */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm but with different clobbers */
        __asm__ volatile (
            "=r" (forced2),
            "=?r" (opt3)
            : "r" (int_array[i+1]),
              "0" (forced1)     /* Matching constraint */
            : "memory", "cc"    /* Different clobber list */
        );
        
        /* Use optional results if they were allocated */
        if (opt1) int_array[i] += opt1;
        if (opt2) int_array[i] += opt2;
        if (opt3) int_array[i] += opt3;
        
        /* Conditional asm based on runtime value */
        if (mode & 1) {
            __asm__ volatile (
                "=r" (forced1),
                "=m" (float_array[i])
                : "r" (forced2),
                  "i" (mode)
                : "cc"
            );
        } else {
            __asm__ volatile (
                "=r" (forced2),
                "=q" (opt1)     /* Byte register constraint */
                : "r" (forced1),
                  "m" (int_array[i])
                : "cc"
            );
        }
    }
}

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3 + 1;
        double_array[i] = i * 1.5;
        float_array[i] = i * 0.75f;
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        vec128_array[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
    }
    
    for (int i = 0; i < ARRAY_SIZE/8; i++) {
        vec256_array[i] = _mm256_set_pd(i*8+3, i*8+2, i*8+1, i*8);
    }
}

/* Compute checksum to ensure all asm executes */
unsigned long long compute_checksum(void) {
    unsigned long long checksum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_array[i];
        checksum += (unsigned long long)(double_array[i] * 1000);
        checksum += (unsigned int)(float_array[i] * 1000);
    }
    
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        int* v = (int*)&vec128_array[i];
        checksum += v[0] + v[1] + v[2] + v[3];
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    printf("Running reload tests with iterations=%d, mode=%d\n", 
           iterations, mode);
    
    /* Initialize data */
    init_arrays();
    
    /* Run tests to trigger reload logic */
    test_primary_reloads(iterations, mode);
    test_secondary_reloads(iterations, mode);
    test_optional_reloads(iterations, mode);
    
    /* Additional loop with mixed operations */
    for (int outer = 0; outer < 3; outer++) {
        volatile int temp = 0;
        for (int i = 0; i < iterations/10; i++) {
            /* Complex asm in inner loop with many live values */
            int live1 = int_array[i];
            int live2 = int_array[i+1];
            int live3 = int_array[i+2];
            int live4 = int_array[i+3];
            int live5 = int_array[i+4];
            double live_d = double_array[i];
            
            __asm__ volatile (
                "=r" (temp),
                "=r" (live1),
                "=m" (int_array[i+5])
                : "r" (live2),
                  "r" (live3),
                  "m" (live_d),
                  "i" (i),
                  "0" (live4),    /* Matching constraint */
                  "r" (live5)
                : "cc", "memory"
            );
            
            /* Use all live variables */
            int_array[i] = live1 + live2 + live3 + live4 + live5 + temp;
        }
    }
    
    /* Compute and print checksum */
    unsigned long long checksum = compute_checksum();
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
