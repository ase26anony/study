/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Helper to make values volatile and prevent optimization */
static volatile int sink;

/* Pattern 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&sink);
    
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
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    
    /* This may require helper function synthesis */
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_val;
}

/* Pattern 2: Operations requiring library calls */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    result = a * b;      /* May generate __multi3 call */
    result += a / b;     /* May generate __divti3 call */
    result += a % b;     /* May generate __modti3 call */
    
    /* Complex number operations */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double c3 = c1 / c2;  /* May generate library call */
    
    /* Use the complex result */
    result += (__int128)__real__(c3);
    result += (__int128)__imag__(c3);
    
    /* Double precision on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    double z = x * y;  /* May generate soft-float library call */
    
    result += (__int128)z;
    
    return result;
}

/* Pattern 3: Transactional memory extensions */
NOOPT int test_transactional_synthesis(int *ptr) {
    int result = 0;
    
    #ifdef __TM_FENCE__
    /* Transactional memory operations */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    /* Transactional memory built-ins */
    if (__builtin_tx_start()) {
        result += 100;
        __builtin_tx_end();
    }
    #endif
    
    return result;
}

/* Pattern 4: OpenMP runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    int result = 0;
    int i;
    
    #ifdef _OPENMP
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: result)
    {
        #pragma omp parallel for reduction(+:result)
        for (i = 0; i < n; i++) {
            result += i * i;
        }
    }
    
    /* OpenMP atomic with capture - may synthesize runtime calls */
    int atomic_var = 0;
    #pragma omp atomic capture
    {
        result = atomic_var;
        atomic_var += n;
    }
    #endif
    
    return result;
}

/* Pattern 5: Mixed operations in single function */
NOOPT uint64_t test_mixed_synthesis(__int128 a, __int128 b, int *ptr) {
    uint64_t result = 0;
    
    /* Mix of different synthesis triggers */
    
    /* 1. Atomic on 128-bit value */
    __int128 atomic_128 = 0;
    __atomic_store_n(&atomic_128, a, __ATOMIC_RELEASE);
    
    /* 2. Target built-in inside conditional */
    #ifdef __x86_64__
    if (__builtin_cpu_is("intel")) {
        result += __builtin_ia32_rdtsc() & 0xFFFF;
    }
    #endif
    
    /* 3. Complex operation */
    _Complex float cf = (3.0f + 4.0fi) / (1.0f + 2.0fi);
    result += (uint64_t)__real__(cf);
    
    /* 4. Transactional memory if available */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        atomic_128 += b;
    }
    #endif
    
    /* 5. OpenMP parallel region */
    #ifdef _OPENMP
    #pragma omp parallel reduction(+:result)
    {
        result += (uint64_t)atomic_128;
    }
    #endif
    
    return result + (uint64_t)atomic_128;
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize with some values */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x10000000000000001ULL;
    
    int transactional_var = 42;
    int *ptr = &transactional_var;
    
    /* Call each synthesis pattern function */
    accumulator += test_builtin_synthesis();
    
    __int128 libcall_result = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)libcall_result + (uint64_t)(libcall_result >> 64);
    
    accumulator += test_transactional_synthesis(ptr);
    accumulator += test_omp_synthesis(100);
    accumulator += test_mixed_synthesis(a, b, ptr);
    
    /* Make result observable */
    printf("Result: 0x%016llx\n", (unsigned long long)accumulator);
    
    /* Additional forced synthesis via constant evaluation fallback */
    int dynamic_value = argc > 1 ? atoi(argv[1]) : 100;
    
    /* __builtin_constant_p with non-constant argument forces runtime path */
    if (__builtin_constant_p(dynamic_value)) {
        accumulator += 1;  /* Compile-time path */
    } else {
        accumulator += 2;  /* Runtime path - may synthesize helpers */
    }
    
    /* Use result to prevent dead code elimination */
    sink = (int)accumulator;
    
    return sink != 0 ? 0 : 1;
}
