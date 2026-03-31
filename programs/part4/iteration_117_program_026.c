/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    /* CPU feature detection - may synthesize resolver */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Transactional memory - may require runtime support */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result += 2;
    }
    #endif
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations often require libcalls on 64-bit targets */
    result = a * b;                     /* May synthesize __multi3 */
    result += a / b;                    /* May synthesize __divti3 */
    result += a % b;                    /* May synthesize __modti3 */
    
    /* Atomic operations on 128-bit values */
    __int128 expected = a;
    __atomic_compare_exchange_n(&result, &expected, b, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Test 3: Soft-float double operations */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex operations that may require libcalls */
    __complex__ double c1 = a + b * 1.0i;
    __complex__ double c2 = b + a * 1.0i;
    __complex__ double cdiv = c1 / c2;  /* Complex division libcall */
    
    result = __real__(cdiv) + __imag__(cdiv);
    
    /* Transcendental functions */
    result += __builtin_sin(a);
    result += __builtin_cos(b);
    
    return result;
}

/* Test 4: OpenMP target region synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100]) map(tofrom: result)
    {
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < 100; i++) {
            result += arr[i];
        }
        
        /* Use built-in inside target region */
        result += __builtin_popcount(result);
    }
    
    return result;
}

/* Test 5: Mixed synthesis triggers */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* 128-bit value */
    __int128 big_val = ((__int128)seed << 64) | seed;
    
    /* Atomic exchange on 128-bit */
    __int128 old = big_val;
    __atomic_exchange(&big_val, &old, &old, __ATOMIC_ACQ_REL);
    
    /* Use RDTSC */
    result ^= __builtin_ia32_rdtsc();
    
    /* CPU feature check */
    if (__builtin_cpu_supports("sse4.2")) {
        /* Use SSE4.2 built-in */
        result = __builtin_ia32_crc32di(result, seed);
    }
    
    /* Memory model operation */
    __atomic_signal_fence(__ATOMIC_RELEASE);
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    __int128 res128 = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)res128 + (uint64_t)(res128 >> 64);
    
    /* Test 3: Soft-float synthesis */
    double dres = test_softfloat_synthesis(3.1415926535, 2.7182818284);
    accumulator += (uint64_t)dres;
    
    /* Test 4: OpenMP synthesis */
    int omp_res = test_omp_synthesis(100);
    accumulator += omp_res;
    
    /* Test 5: Mixed synthesis */
    accumulator += test_mixed_synthesis(accumulator);
    
    /* Ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
