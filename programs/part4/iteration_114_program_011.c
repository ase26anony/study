/* reload_test.c - Complex inline assembly to trigger GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x86intrin.h>

#define UNROLL_FACTOR 16
#define ARRAY_SIZE 256

/* Test function for primary reload patterns */
void test_primary_reloads(int iterations, int *input, int *output, 
                         double *dinput, double *doutput) {
    volatile int i, j, k;
    volatile long long ll1, ll2, ll3;
    volatile double d1, d2, d3;
    volatile float f1, f2, f3;
    volatile __m128i v1, v2, v3;
    volatile __m256d av1, av2, av3;
    
    /* Create many live variables to increase register pressure */
    int live_vars[UNROLL_FACTOR];
    double live_doubles[UNROLL_FACTOR];
    long long live_llongs[UNROLL_FACTOR];
    
    for (i = 0; i < UNROLL_FACTOR; i++) {
        live_vars[i] = input[i] + i;
        live_doubles[i] = dinput[i] * (i + 1);
        live_llongs[i] = (long long)input[i] << i;
    }
    
    /* Complex inline assembly with multiple operands and constraints */
    for (i = 0; i < iterations; i++) {
        /* Force different machine modes and register classes */
        __asm__ volatile (
            /* Output operands with different constraints */
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "mov %[in3], %%bl\n\t"
            "mov %%bl, %[out2]\n\t"
            "fld %[din1]\n\t"
            "fadd %[din2]\n\t"
            "fstp %[dout1]\n\t"
            : [out1] "=r" (live_vars[i % UNROLL_FACTOR]),    /* word reg */
              [out2] "=q" (live_vars[(i + 1) % UNROLL_FACTOR]), /* byte reg */
              [dout1] "=t" (live_doubles[i % UNROLL_FACTOR])   /* top of FP stack */
            : [in1] "r" (live_vars[(i + 2) % UNROLL_FACTOR]),
              [in2] "rm" (live_vars[(i + 3) % UNROLL_FACTOR]), /* can be mem or reg */
              [in3] "i" (0x7F),                               /* immediate */
              [din1] "m" (live_doubles[(i + 1) % UNROLL_FACTOR]),
              [din2] "m" (live_doubles[(i + 2) % UNROLL_FACTOR])
            : "eax", "ebx", "st", "memory", "cc"
        );
        
        /* Another asm with earlyclobber and matching constraints */
        int tmp1 = live_vars[i % UNROLL_FACTOR];
        int tmp2 = live_vars[(i + 1) % UNROLL_FACTOR];
        __asm__ volatile (
            "imul %[mul], %[val]\n\t"
            "add %%ecx, %[val]\n\t"
            : [val] "=&r" (tmp1), "=&a" (tmp2)  /* earlyclobber */
            : "0" (tmp1), 
              [mul] "r" (live_vars[(i + 4) % UNROLL_FACTOR]),
              "1" (tmp2)
            : "ecx", "cc"
        );
        live_vars[i % UNROLL_FACTOR] = tmp1;
        live_vars[(i + 1) % UNROLL_FACTOR] = tmp2;
        
        /* Use vector intrinsics alongside scalar operations */
        v1 = _mm_set_epi32(live_vars[0], live_vars[1], 
                          live_vars[2], live_vars[3]);
        av1 = _mm256_set_pd(live_doubles[0], live_doubles[1],
                           live_doubles[2], live_doubles[3]);
        
        /* More complex asm with 5+ operands */
        __asm__ volatile (
            "mov %[a], %%eax\n\t"
            "mov %[b], %%ebx\n\t"
            "mov %[c], %%ecx\n\t"
            "lea (%%eax, %%ebx, 2), %%edx\n\t"
            "imul %%ecx, %%edx\n\t"
            "mov %%edx, %[x]\n\t"
            "mov %%eax, %[y]\n\t"
            : [x] "=r" (live_vars[(i + 5) % UNROLL_FACTOR]),
              [y] "=r" (live_vars[(i + 6) % UNROLL_FACTOR])
            : [a] "r" (live_vars[(i + 7) % UNROLL_FACTOR]),
              [b] "rm" (live_vars[(i + 8) % UNROLL_FACTOR]),
              [c] "r" (live_vars[(i + 9) % UNROLL_FACTOR])
            : "eax", "ebx", "ecx", "edx", "cc"
        );
    }
    
    /* Store results to output arrays */
    for (i = 0; i < UNROLL_FACTOR && i < ARRAY_SIZE; i++) {
        output[i] = live_vars[i];
        doutput[i] = live_doubles[i];
    }
}

/* Test function for secondary reload patterns */
void test_secondary_reloads(int iterations, int *input, int *output) {
    volatile int i;
    int tmp[UNROLL_FACTOR];
    
    for (i = 0; i < UNROLL_FACTOR; i++) {
        tmp[i] = input[i] * 2;
    }
    
    for (i = 0; i < iterations; i++) {
        /* Force secondary reloads with mismatched constraints */
        int idx = i % UNROLL_FACTOR;
        
        /* 'a' constraint (accumulator) followed by 'b' constraint */
        __asm__ volatile (
            "mov %[in], %%eax\n\t"
            "add $1, %%eax\n\t"
            : "=a" (tmp[idx])        /* Must go in EAX */
            : [in] "rm" (tmp[(idx + 1) % UNROLL_FACTOR])
            : "cc"
        );
        
        /* Now use result with 'b' constraint (EBX) */
        int tmp2 = tmp[idx];
        __asm__ volatile (
            "mov %[in], %%ebx\n\t"
            "shl $2, %%ebx\n\t"
            : "=b" (tmp2)            /* Must go in EBX */
            : [in] "r" (tmp2)        /* This may need secondary reload */
            : "cc"
        );
        tmp[idx] = tmp2;
        
        /* Legacy register constraint that may need secondary reload */
        __asm__ volatile (
            "push %%rbx\n\t"
            "mov %[val], %%ebx\n\t"
            "ror $4, %%ebx\n\t"
            "mov %%ebx, %[out]\n\t"
            "pop %%rbx\n\t"
            : [out] "=R" (tmp[(idx + 2) % UNROLL_FACTOR])  /* Legacy register */
            : [val] "rm" (tmp[(idx + 3) % UNROLL_FACTOR])
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
    }
    
    for (i = 0; i < UNROLL_FACTOR && i < ARRAY_SIZE; i++) {
        output[i + UNROLL_FACTOR] = tmp[i];
    }
}

/* Test function for optional reloads */
void test_optional_reloads(int iterations, int *input, int *output) {
    volatile int i;
    int optional_results[UNROLL_FACTOR];
    
    for (i = 0; i < UNROLL_FACTOR; i++) {
        optional_results[i] = input[i + UNROLL_FACTOR];
    }
    
    for (i = 0; i < iterations; i++) {
        int idx = i % UNROLL_FACTOR;
        
        /* Optional output constraint */
        int opt1, opt2;
        __asm__ volatile (
            "test %[flag], %[flag]\n\t"
            "jz 1f\n\t"
            "mov $1, %[res1]\n\t"
            "jmp 2f\n\t"
            "1:\n\t"
            "mov $0, %[res1]\n\t"
            "2:\n\t"
            : [res1] "=?r" (opt1),   /* Optional output */
              [res2] "=r" (opt2)     /* Required output */
            : [flag] "r" (optional_results[idx]),
              [base] "r" (optional_results[(idx + 1) % UNROLL_FACTOR])
            : "cc"
        );
        
        optional_results[idx] = opt1 + opt2;
        
        /* Another asm that could be combined but won't due to volatile */
        __asm__ volatile (
            "add $1, %[val]\n\t"
            : [val] "+r" (optional_results[(idx + 2) % UNROLL_FACTOR])
            :
            : "cc"
        );
        
        /* Memory barrier to prevent combination */
        __asm__ volatile ("" ::: "memory");
        
        /* Similar asm that won't combine due to different clobbers */
        __asm__ volatile (
            "add $2, %[val]\n\t"
            : [val] "+r" (optional_results[(idx + 3) % UNROLL_FACTOR])
            :
            : "cc", "memory"  /* Different clobber list */
        );
    }
    
    for (i = 0; i < UNROLL_FACTOR && i < ARRAY_SIZE; i++) {
        output[i + 2 * UNROLL_FACTOR] = optional_results[i];
    }
}

/* Control flow dependent reloads */
void test_control_flow_reloads(int mode, int *input, int *output) {
    volatile int i, j;
    int tmp[UNROLL_FACTOR * 2];
    
    for (i = 0; i < UNROLL_FACTOR * 2; i++) {
        tmp[i] = input[i];
    }
    
    /* Complex control flow with inline assembly */
    for (i = 0; i < UNROLL_FACTOR; i++) {
        if (mode & (1 << (i % 8))) {
            /* Path with register-intensive operations */
            __asm__ volatile (
                "mov %[a], %%eax\n\t"
                "mov %[b], %%ebx\n\t"
                "mov %[c], %%ecx\n\t"
                "add %%ebx, %%eax\n\t"
                "imul %%ecx, %%eax\n\t"
                "mov %%eax, %[out]\n\t"
                : [out] "=r" (tmp[i])
                : [a] "r" (tmp[i]),
                  [b] "rm" (tmp[i + 1]),
                  [c] "r" (tmp[i + 2])
                : "eax", "ebx", "ecx", "cc"
            );
        } else {
            /* Alternative path with different constraints */
            __asm__ volatile (
                "mov %[in], %%edx\n\t"
                "shr $1, %%edx\n\t"
                "mov %%edx, %[out]\n\t"
                : [out] "=r" (tmp[i])
                : [in] "rm" (tmp[i + 3])
                : "edx", "cc"
            );
        }
        
        /* Loop with nested conditionals */
        for (j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                __asm__ volatile (
                    "inc %[val]\n\t"
                    : [val] "+r" (tmp[i + j])
                    :
                    : "cc"
                );
            }
        }
    }
    
    for (i = 0; i < UNROLL_FACTOR && i < ARRAY_SIZE; i++) {
        output[i + 3 * UNROLL_FACTOR] = tmp[i];
    }
}

int main(int argc, char **argv) {
    int iterations = 100;
    int mode = 2;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    /* Initialize data arrays */
    int input[ARRAY_SIZE];
    int output[ARRAY_SIZE];
    double dinput[ARRAY_SIZE];
    double doutput[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i] = i * 3 + 1;
        output[i] = 0;
        dinput[i] = i * 0.5;
        doutput[i] = 0.0;
    }
    
    /* Execute test functions with complex inline assembly */
    test_primary_reloads(iterations, input, output, dinput, doutput);
    test_secondary_reloads(iterations, input, output);
    test_optional_reloads(iterations, input, output);
    test_control_flow_reloads(mode, input, output);
    
    /* Compute checksum to ensure all assembly executed */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i];
        checksum += (long long)doutput[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Test completed with iterations=%d, mode=%d\n", iterations, mode);
    
    return 0;
}
