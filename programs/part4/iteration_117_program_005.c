/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, used))

/* Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* AVX/SSE built-ins */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Library call synthesis for unsupported operations */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x1000000000000000ULL;
    unsigned __int128 c = a * b;  /* May require __multi3 library call */
    unsigned __int128 d = a / b;  /* May require __udivti3 library call */
    
    result = (uint64_t)(c ^ d);
    
    /* Double-precision operations on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    volatile double z = x * y;  /* May require soft-float library call */
    
    result += (uint64_t)z;
    
    /* Complex number division */
    _Complex double cx = 3.0 + 4.0 * _Complex_I;
    _Complex double cy = 1.0 + 2.0 * _Complex_I;
    _Complex double cz = cx / cy;  /* May require library call */
    
    result += (uint64_t)(__real__(cz) + __imag__(cz));
    
    return result;
}

/* OpenMP synthesis */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[1024];
    
    /* Initialize data */
    for (int i = 0; i < 1024; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:1024])
    for (int i = 0; i < 1024; i++) {
        data[i] *= 2;
    }
    
    /* Collect result */
    for (int i = 0; i < 1024; i++) {
        result += data[i];
    }
    
    return result;
}

/* Transactional memory synthesis */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    int x = 0, y = 0;
    
    /* Transactional memory operations */
    __transaction_atomic {
        x = 42;
        y = x * 2;
    }
    
    result = x + y;
    
    /* __builtin_constant_p with runtime fallback */
    int arr[10] = {0};
    for (int i = 0; i < 10; i++) {
        if (__builtin_constant_p(i)) {
            arr[i] = i * 2;
        } else {
            arr[i] = i * 3;
        }
        result += arr[i];
    }
    
    return result;
}

/* Combined synthesis triggers */
NOOPT uint64_t test_combined_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Mix 128-bit atomics with built-ins */
    __int128 atomic128 = 0;
    __atomic_store_n(&atomic128, 0x123456789ABCDEFULL, __ATOMIC_RELEASE);
    
    #ifdef __x86_64__
    /* Use CPUID-like built-in */
    if (__builtin_cpu_is("intel")) {
        result |= 0x8000;
    }
    #endif
    
    /* Complex arithmetic */
    _Complex float cf1 = 1.5f + 2.5f * _Complex_I;
    _Complex float cf2 = 0.5f + 1.5f * _Complex_I;
    _Complex float cf3 = cf1 * cf2;
    
    result += (uint64_t)(__real__(cf3) * 1000);
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Call all synthesis test functions */
    accumulator ^= test_builtin_synthesis();
    accumulator ^= test_libcall_synthesis();
    accumulator ^= test_omp_synthesis();
    accumulator ^= test_tm_synthesis();
    accumulator ^= test_combined_synthesis();
    
    /* Make results observable */
    printf("Synthesis test result: 0x%016llx\n", 
           (unsigned long long)accumulator);
    
    /* Additional volatile operations to prevent optimization */
    volatile int check = (accumulator != 0);
    if (!check) {
        abort();
    }
    
    return 0;
}
