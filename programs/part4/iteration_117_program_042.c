/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-ins requiring synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesized declarations */
    #ifdef __x86_64__
    /* RDTSC - time stamp counter */
    result += __builtin_ia32_rdtsc();
    
    /* CPUID - processor identification */
    unsigned int eax, ebx, ecx, edx;
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2")) {
        /* Use AVX2 built-in that may need synthesis */
        result += __builtin_ia32_crc32di(result, 0x12345678);
    }
    
    /* Memory barrier built-ins */
    __builtin_ia32_mfence();
    __builtin_ia32_sfence();
    __builtin_ia32_lfence();
    #endif
    
    /* Atomic built-ins with uncommon sizes/orders */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    
    return result + (uint64_t)atomic_val;
}

/* Test 2: Operations requiring libcall synthesis */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    __int128 c = a * b;  /* May require __multi3 libcall */
    __int128 d = a / b;  /* May require __divti3 libcall */
    
    result += (uint64_t)c + (uint64_t)(c >> 64);
    result += (uint64_t)d + (uint64_t)(d >> 64);
    
    /* Complex number division - often requires libcalls */
    volatile _Complex double z1 = 3.0 + 4.0 * _Complex_I;
    volatile _Complex double z2 = 1.0 + 2.0 * _Complex_I;
    volatile _Complex double z3 = z1 / z2;
    
    result += (uint64_t)__real__ z3 + (uint64_t)__imag__ z3;
    
    /* Double precision on soft-float target */
    volatile double x = 3.141592653589793;
    volatile double y = 2.718281828459045;
    volatile double z = x * y + x / y - y * x;  /* May require soft-float libcalls */
    
    result += (uint64_t)z;
    
    return result;
}

/* Test 3: OpenMP/OpenACC requiring runtime synthesis */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[1024];
    
    /* Initialize data */
    for (int i = 0; i < 1024; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: data[0:1024])
    {
        #pragma omp parallel for
        for (int i = 0; i < 1024; i++) {
            data[i] *= 2;
        }
        
        /* Nested atomic operation in target region */
        #pragma omp atomic
        result += data[512];
    }
    
    /* Transactional memory - may synthesize TM runtime calls */
    #ifdef __TM_FEATURE_ABORT
    __transaction_atomic {
        for (int i = 0; i < 100; i++) {
            result += i;
        }
    }
    #endif
    
    return result;
}

/* Test 4: Mixed patterns to maximize synthesis opportunities */
NOOPT uint64_t test_mixed_synthesis(int seed) {
    volatile uint64_t result = seed;
    
    /* Combine atomic operations with 128-bit values */
    volatile __int128 atomic128 = 0;
    __int128 old_val, new_val;
    
    for (int i = 0; i < 10; i++) {
        old_val = atomic128;
        new_val = old_val + (((__int128)i << 64) | i);
        
        /* Atomic exchange on 128-bit - likely requires libcall */
        __atomic_compare_exchange(&atomic128, &old_val, &new_val,
                                  0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        
        /* Use builtin_constant_p with runtime fallback */
        if (!__builtin_constant_p(seed)) {
            /* This branch forces consideration of runtime path */
            result += __builtin_popcountll(atomic128);
        }
    }
    
    /* Architecture-specific built-in in loop */
    #ifdef __x86_64__
    for (int i = 0; i < 100; i++) {
        result ^= __builtin_ia32_crc32di(result, i);
    }
    #endif
    
    return result + (uint64_t)atomic128;
}

/* Main function that exercises all synthesis patterns */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Use command line argument to prevent constant folding */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    printf("Testing built-in synthesis...\n");
    accumulator += test_builtin_synthesis();
    
    printf("Testing libcall synthesis...\n");
    accumulator += test_libcall_synthesis();
    
    printf("Testing OpenMP synthesis...\n");
    accumulator += test_omp_synthesis();
    
    printf("Testing mixed synthesis...\n");
    accumulator += test_mixed_synthesis(seed);
    
    /* Print checksum to ensure all operations are observable */
    printf("Result checksum: %llu\n", (unsigned long long)accumulator);
    
    return (accumulator != 0) ? 0 : 1;
}
