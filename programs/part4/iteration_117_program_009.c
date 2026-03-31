/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))

/* 128-bit integer type */
typedef unsigned __int128 uint128_t;

/* ========== Pattern 1: Target-specific built-ins ========== */
NOINLINE uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* SSE/AVX built-ins */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    #endif
    
    /* ARM specific built-ins */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* BPF built-ins */
    #ifdef __bpf__
    result += __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* ========== Pattern 2: Unsupported operations requiring libcalls ========== */
NOINLINE uint64_t test_libcall_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on 64-bit target (may still need libcalls for some ops) */
    uint128_t x = ((uint128_t)a << 64) | b;
    uint128_t y = ((uint128_t)b << 64) | a;
    uint128_t z = x * y;  /* Multiplication may need libcall */
    result = (uint64_t)(z >> 64) ^ (uint64_t)z;
    
    /* Complex number division (often requires libcalls) */
    _Complex double c1 = a + b * 1.0i;
    _Complex double c2 = b + a * 1.0i;
    _Complex double c3 = c1 / c2;
    result += (uint64_t)__real__(c3) + (uint64_t)__imag__(c3);
    
    /* Atomic operations with unusual sizes */
    struct UnusualAtomic {
        uint64_t data[3];
    } atomic_var = {0};
    
    __atomic_load(&atomic_var, &atomic_var, __ATOMIC_ACQUIRE);
    __atomic_store(&atomic_var, &atomic_var, __ATOMIC_RELEASE);
    
    return result;
}

/* ========== Pattern 3: Advanced extensions requiring runtime ========== */
NOINLINE uint64_t test_advanced_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Transactional memory (if supported) */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result ^= 0xABCD;
        result *= 3;
    }
    #endif
    
    /* Variable-length array with complex initialization */
    int n = (seed % 64) + 1;
    int vla[n];
    for (int i = 0; i < n; i++) {
        vla[i] = (result + i) % 256;
        result += vla[i];
    }
    
    /* Builtin constant evaluation with runtime fallback */
    int is_const = __builtin_constant_p(seed);
    if (!is_const) {
        /* Force runtime path */
        result = __builtin_popcountll(result);
    }
    
    return result;
}

/* ========== Pattern 4: OpenMP target region synthesis ========== */
NOINLINE uint64_t test_omp_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = 0;
    
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result) map(to: a, b)
    {
        /* Use target-specific operations inside OpenMP region */
        result = a ^ b;
        
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFF;
        #endif
        
        /* Additional computation to ensure synthesis */
        for (int i = 0; i < 100; i++) {
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    #endif
    
    return result;
}

/* ========== Pattern 5: Mixed synthesis triggers ========== */
NOINLINE uint64_t test_mixed_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = 0;
    
    /* Combine multiple patterns */
    
    /* 1. Atomic on 128-bit value */
    __int128 atomic_128 = 0;
    __int128 desired = ((__int128)a << 64) | b;
    __atomic_compare_exchange(&atomic_128, &(__int128){0}, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* 2. Complex arithmetic */
    _Complex float cf1 = a + b * 1.0if;
    _Complex float cf2 = b + a * 1.0if;
    _Complex float cf3 = cf1 / cf2;
    result += (uint64_t)__real__(cf3) + (uint64_t)__imag__(cf3);
    
    /* 3. Target built-in in loop */
    #ifdef __x86_64__
    for (int i = 0; i < 10; i++) {
        result ^= __builtin_ia32_rdtsc();
    }
    #endif
    
    return result;
}

/* ========== Main function ========== */
int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Use command line arguments to prevent constant folding */
    uint64_t seed1 = (uint64_t)(argv[0] ? argv[0][0] : 'A');
    uint64_t seed2 = (uint64_t)(argc > 1 ? argv[1][0] : 'B');
    
    /* Call all synthesis test functions */
    accumulator ^= test_builtin_synthesis();
    accumulator += test_libcall_synthesis(seed1, seed2);
    accumulator ^= test_advanced_synthesis(seed1 ^ seed2);
    accumulator += test_omp_synthesis(seed1, seed2);
    accumulator ^= test_mixed_synthesis(seed1, seed2);
    
    /* Ensure all results are observable */
    printf("Result: 0x%016llx\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0xFF);
}
