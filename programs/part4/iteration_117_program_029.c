/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* SIMD built-ins that might need declaration synthesis */
    __m128i v1 = _mm_setzero_si128();
    __m128i v2 = _mm_set1_epi32(42);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 0x2000;
    }
    #endif
    
    /* Atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, (__int128)result, __ATOMIC_SEQ_CST);
    __int128 loaded = __atomic_load_n(&atomic_val, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)loaded;
}

/* Library call synthesis through unsupported operations */
NOOPT uint64_t test_libcall_synthesis(int seed) {
    volatile uint64_t result = seed;
    
    /* 128-bit arithmetic on targets without native support */
    __int128 a = ((__int128)seed << 64) | seed;
    __int128 b = ((__int128)seed << 32) | seed;
    __int128 c = a * b;      /* May generate __multi3 call */
    __int128 d = a / (b | 1); /* May generate __divti3 call */
    
    result += (uint64_t)(c >> 64);
    result += (uint64_t)d;
    
    /* Complex number division (often uses library calls) */
    _Complex double cd1 = seed + seed * 1.0i;
    _Complex double cd2 = (seed + 1) + (seed + 1) * 1.0i;
    _Complex double cd3 = cd1 / cd2;
    
    result += (uint64_t)__real__ cd3;
    result += (uint64_t)__imag__ cd3;
    
    /* Double precision on soft-float target */
    double x = seed * 3.14159;
    double y = x * x;
    result += (uint64_t)y;
    
    return result;
}

/* OpenMP runtime function synthesis */
NOOPT uint64_t test_omp_synthesis(int n) {
    volatile uint64_t result = 0;
    
    #pragma omp target map(tofrom: result) if(n > 100)
    {
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < n % 100; i++) {
            result += i * i;
        }
    }
    
    /* OpenMP atomic with capture */
    int atomic_var = 0;
    #pragma omp atomic capture
    result += (atomic_var = n);
    
    return result;
}

/* Transactional memory extension */
NOOPT uint64_t test_transactional_synthesis(int val) {
    volatile uint64_t result = 0;
    
    /* Transactional memory (requires runtime support) */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result = val * 2;
        if (val > 100) {
            __transaction_cancel;
        }
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        if (__builtin_constant_p(val)) {
            arr[i] = i * 2;
        } else {
            arr[i] = i * val;
        }
        result += arr[i];
    }
    
    return result;
}

/* Combined synthesis triggers */
NOOPT uint64_t test_combined_synthesis(int seed) {
    volatile uint64_t result = 0;
    
    /* Mix of patterns in one function */
    
    /* 1. Atomic on 128-bit value */
    __int128 atomic_128 = seed;
    __atomic_fetch_add(&atomic_128, (__int128)seed, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_128;
    
    /* 2. Target built-in inside loop */
    for (int i = 0; i < 3; i++) {
        #ifdef __x86_64__
        result ^= __builtin_ia32_rdtsc() >> (i * 8);
        #endif
    }
    
    /* 3. Complex math */
    _Complex float cf = seed + seed * 1.0if;
    cf = cf * cf / (cf + 1.0if);
    result += (uint64_t)__real__ cf;
    
    /* 4. Unaligned atomic (may require helper) */
    struct {
        char pad[3];
        uint32_t val;
    } __attribute__((packed)) unaligned_struct;
    
    unaligned_struct.val = seed;
    uint32_t old = __atomic_exchange_n(&unaligned_struct.val, seed * 2, __ATOMIC_ACQ_REL);
    result += old;
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis(seed);
    accumulator += test_omp_synthesis(seed);
    accumulator += test_transactional_synthesis(seed);
    accumulator += test_combined_synthesis(seed);
    
    /* Make results observable */
    printf("Result: %lu\n", (unsigned long)accumulator);
    
    return (int)(accumulator & 0xFF);
}
