/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* 128-bit integer type for libcall synthesis */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== Pattern 1: Target-specific built-in synthesis ========== */
NO_OPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    /* RDTSC - time stamp counter */
    result += __builtin_ia32_rdtsc();
    
    /* CPUID leaf 0 */
    unsigned int eax, ebx, ecx, edx;
    __builtin_cpu_init();
    if (__builtin_cpu_supports("sse2")) {
        /* Use some SSE2 built-ins */
        __m128i a = _mm_setzero_si128();
        __m128i b = _mm_set1_epi32(42);
        __m128i c = _mm_add_epi32(a, b);
        result += _mm_extract_epi32(c, 0);
    }
    
    /* Uncommon atomic built-in with specific memory order */
    unsigned long atomic_var = 42;
    __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
    #endif
    
    return result;
}

/* ========== Pattern 2: Libcall synthesis for unsupported operations ========== */
NO_OPT uint64_t test_libcall_synthesis(int128_t a, int128_t b) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic - may require libcall synthesis on some targets */
    int128_t sum = a + b;
    int128_t product = a * b;  /* This often requires libcall */
    int128_t quotient = a / (b ? b : 1);  /* Division definitely requires libcall */
    
    /* Use results to prevent optimization */
    result += (uint64_t)(sum >> 64);
    result += (uint64_t)(product >> 64);
    result += (uint64_t)(quotient >> 64);
    
    /* Double-precision operations on potential soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    volatile double z = x * y + x / y;  /* May require soft-float libcalls */
    
    /* Complex number division - often requires libcall */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double c3 = c1 / c2;
    
    result += (uint64_t)(__real__ c3 * 1000);
    result += (uint64_t)(__imag__ c3 * 1000);
    
    return result;
}

/* ========== Pattern 3: Transactional memory synthesis ========== */
NO_OPT uint64_t test_tm_synthesis(int *ptr) {
    volatile uint64_t result = 0;
    
    /* Transactional memory - may require runtime call synthesis */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        *ptr += 1;
        result = *ptr;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = rand();
    if (__builtin_constant_p(x)) {
        result += 1;
    } else {
        result += x & 0xFF;
    }
    
    return result;
}

/* ========== Pattern 4: OpenMP runtime synthesis ========== */
NO_OPT uint64_t test_omp_synthesis(int n) {
    volatile uint64_t result = 0;
    int i;
    
    /* OpenMP target region - may synthesize data mapping routines */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Use some built-ins inside OpenMP region */
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFFFF;
        #endif
        
        /* Simple computation */
        for (i = 0; i < n; i++) {
            result += i * 2;
        }
    }
    #endif
    
    return result;
}

/* ========== Pattern 5: Combined synthesis triggers ========== */
NO_OPT uint64_t test_combined_synthesis(int128_t val) {
    volatile uint64_t result = 0;
    
    /* Atomic operation on 128-bit value */
    __int128 atomic128 = 0;
    __int128 expected = 0;
    __int128 desired = val;
    
    /* This may synthesize atomic helper functions */
    __atomic_compare_exchange_n(&atomic128, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result += (uint64_t)atomic128;
    
    /* Mix with transactional memory */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        atomic128 += 1;
        result += (uint64_t)(atomic128 >> 64);
    }
    #endif
    
    return result;
}

/* ========== Main function ========== */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit values */
    int128_t a = ((int128_t)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    int128_t b = ((int128_t)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    accumulator += test_libcall_synthesis(a, b);
    
    /* Test 3: Transactional memory synthesis */
    int tm_var = 100;
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(100);
    
    /* Test 5: Combined synthesis */
    accumulator += test_combined_synthesis(a);
    
    /* Print result to ensure observability */
    printf("Accumulator result: %llu\n", (unsigned long long)accumulator);
    
    /* Also use some atomic built-ins in main */
    volatile int atomic_counter = 0;
    __atomic_add_fetch(&atomic_counter, accumulator & 0xFFFFFFFF, __ATOMIC_RELAXED);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
