/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in functions forcing synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* SIMD built-ins that might trigger synthesis */
    __m128i a = _mm_setzero_si128();
    __m128i b = _mm_set1_epi32(42);
    __m128i c = _mm_add_epi32(a, b);
    result += _mm_extract_epi32(c, 0);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, (__int128)result, __ATOMIC_SEQ_CST);
    __int128 loaded = __atomic_load_n(&atomic_val, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)loaded;
}

/* Test 2: Operations requiring library call synthesis */
NOOPT uint64_t test_libcall_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)seed << 64) | seed;
    unsigned __int128 b = ((unsigned __int128)seed << 32) | seed;
    unsigned __int128 c = a * b;  /* May require __multi3 library call */
    unsigned __int128 d = a / (b + 1);  /* May require __udivti3 library call */
    
    result += (uint64_t)(c >> 64);
    result += (uint64_t)d;
    
    /* Complex number operations often require library calls */
    _Complex double z1 = seed + seed * 1.0i;
    _Complex double z2 = (seed + 1) + (seed + 1) * 1.0i;
    _Complex double z3 = z1 / z2;  /* May require library call */
    
    result += (uint64_t)__real__(z3);
    result += (uint64_t)__imag__(z3);
    
    /* Double precision on soft-float target */
    double x = (double)seed;
    double y = x * 3.14159;
    double z = y / x;
    result += (uint64_t)z;
    
    return result;
}

/* Test 3: Transactional memory requiring runtime support */
NOOPT uint64_t test_tm_synthesis(uint64_t val) {
    volatile uint64_t result = val;
    
    #ifdef __TM_FENCE__
    /* Transactional memory operations */
    __transaction_atomic {
        result = result * 3 + 1;
        if (result % 2 == 0) {
            result /= 2;
        }
    }
    #endif
    
    /* Atomic operations on 128-bit values */
    __int128 atomic128 = 0;
    __int128 expected = 0;
    __int128 desired = ((__int128)val << 64) | val;
    
    __atomic_compare_exchange_n(&atomic128, &expected, desired, 
                                0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    
    result += (uint64_t)atomic128;
    return result;
}

/* Test 4: OpenMP target region requiring runtime synthesis */
NOOPT uint64_t test_omp_synthesis(uint64_t input) {
    volatile uint64_t result = input;
    
    #ifdef _OPENMP
    int data[1024];
    
    #pragma omp target map(tofrom: data[0:1024])
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < 1024; i++) {
            data[i] = (i * input) % 1024;
        }
        
        /* Use target-specific built-in inside OpenMP region */
        #ifdef __x86_64__
        unsigned int aux;
        unsigned long long tsc = __builtin_ia32_rdtscp(&aux);
        data[0] ^= (int)(tsc & 0xFFFFFFFF);
        #endif
    }
    
    /* Checksum of results */
    for (int i = 0; i < 1024; i++) {
        result ^= data[i];
    }
    #endif
    
    return result;
}

/* Test 5: Mixed patterns for maximum coverage */
NOOPT uint64_t test_mixed_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = a ^ b;
    
    /* 128-bit atomic operation */
    __int128 atomic_val = ((__int128)a << 64) | b;
    __atomic_fetch_add(&atomic_val, 1, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    /* Complex division */
    _Complex float cf = a + b * 1.0if;
    _Complex float cf2 = cf / (cf + 1.0if);
    result += (uint64_t)__real__(cf2);
    
    /* CPU feature detection with runtime fallback */
    volatile int use_avx = 0;
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx")) {
        use_avx = 1;
        /* Force synthesis of CPU dispatch functions */
        __builtin_cpu_init();
    }
    #endif
    
    result += use_avx;
    
    /* Transactional memory if available */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result = result * 7 + 3;
    }
    #endif
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    uint64_t seed = 0xDEADBEEFCAFEBABEULL;
    
    /* Call all synthesis tests */
    accumulator ^= test_builtin_synthesis();
    accumulator += test_libcall_synthesis(seed);
    accumulator ^= test_tm_synthesis(accumulator);
    accumulator += test_omp_synthesis(accumulator);
    accumulator ^= test_mixed_synthesis(accumulator, seed);
    
    /* Ensure all operations are observable */
    printf("Result: 0x%016llX\n", (unsigned long long)accumulator);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile uint64_t *dummy = &accumulator;
    *dummy = accumulator;
    
    return (int)(accumulator & 0x7FFFFFFF);
}
