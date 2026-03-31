/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
    
    /* SIMD built-ins that might trigger synthesis */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    #endif
    
    /* Atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, (__int128)result, __ATOMIC_RELAXED);
    __int128 loaded = __atomic_load_n(&atomic_val, __ATOMIC_ACQUIRE);
    result += (uint64_t)loaded;
    
    return result;
}

/* Library call synthesis through unsupported operations */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x10000000000000001ULL;
    unsigned __int128 c = a * b;  /* May require __multi3 library call */
    result = (uint64_t)(c >> 64) ^ (uint64_t)c;
    
    /* Complex number division (often requires library calls) */
    _Complex double cd1 = 3.0 + 4.0i;
    _Complex double cd2 = 1.0 + 2.0i;
    _Complex double cd3 = cd1 / cd2;
    result += (uint64_t)__real__ cd3;
    
    /* Double precision on soft-float target */
    double d1 = 3.141592653589793;
    double d2 = 2.718281828459045;
    double d3 = d1 * d2;  /* May become __muldf3 call */
    result += (uint64_t)d3;
    
    return result;
}

/* OpenMP runtime synthesis */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] += i * 2;
        }
    }
    
    /* Collect results */
    for (int i = 0; i < 100; i++) {
        result += data[i];
    }
    
    return result;
}

/* Transactional memory synthesis */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    int x = 42, y = 100;
    
    /* Transactional memory operations */
    __transaction_atomic {
        x = x * 2;
        y = y / 2;
        result = x + y;
    }
    
    /* Atomic with uncommon memory order */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    return result;
}

/* BPF built-in synthesis (if compiled for BPF target) */
NOOPT uint64_t test_bpf_synthesis(void) {
    volatile uint64_t result = 0;
    
    #ifdef __bpf__
    /* BPF-specific built-ins */
    result = __builtin_bpf_packet_data();
    result ^= __builtin_bpf_packet_end();
    #endif
    
    /* Generic atomic with 64-bit on 32-bit target */
    uint64_t atomic64 = 0;
    __atomic_fetch_add(&atomic64, 0x12345678, __ATOMIC_RELAXED);
    result += atomic64;
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Seed for pseudo-random operations */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Call each synthesis test function */
    accumulator ^= test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator ^= test_omp_synthesis();
    accumulator += test_tm_synthesis();
    accumulator ^= test_bpf_synthesis();
    
    /* Additional direct built-in usage in main */
    #ifdef __x86_64__
    unsigned int aux;
    accumulator += __builtin_ia32_rdtscp(&aux);
    #endif
    
    /* Force usage of result to prevent optimization */
    printf("Result: 0x%016llx\n", (unsigned long long)accumulator);
    
    return (accumulator == 0) ? 1 : 0;
}
