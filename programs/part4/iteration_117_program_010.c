/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S */
/* For ARM: gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fopenmp */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* 128-bit integer type */
typedef unsigned __int128 uint128_t;

/* Function 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* SSE/AVX built-ins - some may require synthesized declarations */
    result += __builtin_ia32_crc32qi(result, 0xAB);
    result ^= __builtin_ia32_crc32hi(result, 0xCD);
    #endif
    
    /* ARM-specific built-ins */
    #ifdef __arm__
    unsigned int coproc = 15, opc1 = 0, CRn = 0, CRm = 0, opc2 = 0;
    result = __builtin_arm_mrc(coproc, opc1, CRn, CRm, opc2);
    result ^= __builtin_arm_rsr("cpuid");
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    volatile uint128_t atomic_var = 0;
    uint128_t expected = 0, desired = 1;
    
    /* 128-bit atomic compare-exchange - may require helper function */
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_var;
}

/* Function 2: Library call synthesis for unsupported operations */
NOOPT uint64_t test_libcall_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    uint128_t x = ((uint128_t)a << 64) | b;
    uint128_t y = ((uint128_t)b << 64) | a;
    
    /* These operations often require library calls on 64-bit targets */
    uint128_t mul = x * y;          /* May call __multi3 */
    uint128_t div = x / (y | 1);    /* May call __udivti3 */
    uint128_t mod = x % (y | 1);    /* May call __umodti3 */
    
    result = (uint64_t)mul ^ (uint64_t)div ^ (uint64_t)mod;
    
    /* Double-precision math on soft-float target */
    volatile double d1 = (double)a / 3.14159;
    volatile double d2 = (double)b / 2.71828;
    volatile double d3 = d1 * d2 + d1 / d2;
    
    /* Complex number division - often requires library call */
    volatile _Complex double c1 = d1 + d2 * I;
    volatile _Complex double c2 = d2 - d1 * I;
    volatile _Complex double c3 = c1 / c2;
    
    result += (uint64_t)d3 + (uint64_t)__real__ c3;
    
    return result;
}

/* Function 3: OpenMP runtime synthesis */
NOOPT uint64_t test_omp_synthesis(uint64_t *data, int n) {
    volatile uint64_t result = 0;
    
    #pragma omp target map(tofrom: data[0:n]) map(tofrom: result)
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 6364136223846793005ULL + 1442695040888963407ULL;
        }
        
        /* Use target-specific built-in inside OpenMP region */
        #ifdef __x86_64__
        result = __builtin_ia32_rdtsc();
        #endif
        
        #ifdef __arm__
        result = __builtin_arm_rsr("cpsr");
        #endif
    }
    
    /* Sum results */
    for (int i = 0; i < n; i++) {
        result ^= data[i];
    }
    
    return result;
}

/* Function 4: Transactional memory and CPU feature synthesis */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Transactional memory - may synthesize runtime functions */
    #ifdef __TM_supported
    __transaction_atomic {
        result = 0xDEADBEEF;
    }
    #endif
    
    /* CPU feature detection - may synthesize resolver functions */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result ^= 0x512;
    }
    if (__builtin_cpu_supports("avx2")) {
        result ^= 0x256;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = rand();
    if (__builtin_constant_p(x)) {
        result += 1;
    } else {
        result += 2;  /* Runtime path */
    }
    
    return result;
}

/* Function 5: Mixed synthesis patterns */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Combine multiple patterns */
    
    /* 1. Atomic 128-bit operation */
    volatile uint128_t atomic_val = seed;
    uint128_t old_val = atomic_val;
    __atomic_fetch_add(&atomic_val, ((uint128_t)seed << 64), __ATOMIC_ACQ_REL);
    result ^= (uint64_t)atomic_val;
    
    /* 2. Complex arithmetic */
    volatile _Complex float cf = result + (result * 2) * I;
    cf = cf / (cf + 1.0f);
    result += (uint64_t)__real__ cf;
    
    /* 3. Target built-in */
    #ifdef __x86_64__
    result ^= __builtin_ia32_crc32di(result, seed);
    #endif
    
    /* 4. Unaligned atomic (may require helper) */
    volatile char unaligned_buffer[64] __attribute__((aligned(1)));
    __atomic_store_n((uint64_t*)(unaligned_buffer + 3), result, __ATOMIC_RELEASE);
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize some data for OpenMP test */
    int data_size = 100;
    uint64_t *data = (uint64_t*)malloc(data_size * sizeof(uint64_t));
    for (int i = 0; i < data_size; i++) {
        data[i] = i + argc;
    }
    
    /* Call all synthesis-triggering functions */
    accumulator ^= test_builtin_synthesis();
    accumulator ^= test_libcall_synthesis(argc, accumulator);
    accumulator ^= test_omp_synthesis(data, data_size);
    accumulator ^= test_tm_synthesis();
    accumulator ^= test_mixed_synthesis(accumulator);
    
    /* Use results to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)accumulator);
    
    free(data);
    return (int)(accumulator & 0xFF);
}
