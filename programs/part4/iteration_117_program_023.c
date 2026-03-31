/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* 128-bit integer type */
typedef unsigned __int128 uint128_t;

/* Function 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    result ^= __builtin_ia32_rdtscp(&result);   /* RDTSCP with processor ID */
    
    /* SSE/AVX built-ins - some may require helper functions */
    if (__builtin_cpu_supports("sse2")) {
        __m128i v = _mm_setzero_si128();
        v = _mm_add_epi64(v, v);
        result += ((uint64_t*)&v)[0];
    }
    
    /* Transactional memory extension */
    #ifdef __TM__
    __transaction_atomic {
        result += 1;
    }
    #endif
    #endif
    
    return result;
}

/* Function 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT uint64_t test_libcall_synthesis(uint64_t a, uint64_t b) {
    volatile uint64_t result = 0;
    
    /* 128-bit multiplication - may require __multi3 libcall */
    uint128_t x = ((uint128_t)a << 64) | b;
    uint128_t y = ((uint128_t)b << 64) | a;
    uint128_t product = x * y;  /* This often calls __multi3 */
    
    /* 128-bit division - may require __udivti3/__umodti3 */
    if (y != 0) {
        uint128_t quotient = x / y;
        uint128_t remainder = x % y;
        result = (uint64_t)quotient + (uint64_t)remainder;
    }
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = product;
    
    /* 16-byte atomic compare-exchange - may require helper */
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result += (uint64_t)atomic_val;
    return result;
}

/* Function 3: Floating-point operations forcing soft-float libcalls */
NOOPT double test_float_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex division - often calls library functions */
    __complex__ double c1 = a + b * 1.0i;
    __complex__ double c2 = b + a * 1.0i;
    __complex__ double cdiv = c1 / c2;
    
    result += __real__ cdiv + __imag__ cdiv;
    
    /* Transcendental functions that may use libm */
    result += __builtin_sin(a) * __builtin_cos(b);
    
    /* Fused multiply-add - may use libm or special builtin */
    result = __builtin_fma(a, b, result);
    
    return result;
}

/* Function 4: OpenMP target region synthesis */
NOOPT uint64_t test_omp_synthesis(uint64_t *data, int n) {
    volatile uint64_t result = 0;
    
    #pragma omp target map(tofrom: data[0:n]) map(tofrom: result)
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 1103515245 + 12345;
            result += data[i];
        }
        
        /* Atomic operation in target region */
        #pragma omp atomic
        result += 1;
    }
    
    return result;
}

/* Function 5: Mixed operations for maximum synthesis chance */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Use __builtin_constant_p with non-constant argument */
    int is_const = __builtin_constant_p(seed);
    result += is_const ? 0 : 1;
    
    /* Variable-length array with complex size calculation */
    int size = (seed % 100) + 1;
    uint64_t vla[size];
    for (int i = 0; i < size; i++) {
        vla[i] = result + i;
        result ^= vla[i];
    }
    
    /* Memory barrier that may synthesize to target-specific instruction */
    __atomic_thread_fence(__ATOMIC_ACQ_REL);
    
    /* Stack protection check (if enabled) */
    volatile uint64_t canary = 0xDEADBEEF;
    result ^= canary;
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize some data for OpenMP test */
    int data_size = 100;
    uint64_t *omp_data = (uint64_t*)malloc(data_size * sizeof(uint64_t));
    for (int i = 0; i < data_size; i++) {
        omp_data[i] = i + argc;
    }
    
    /* Run all synthesis tests */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis(argc, accumulator);
    accumulator += (uint64_t)test_float_synthesis(argc * 1.0, accumulator * 0.1);
    accumulator += test_omp_synthesis(omp_data, data_size);
    accumulator += test_mixed_synthesis(accumulator);
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%016llx\n", (unsigned long long)accumulator);
    
    free(omp_data);
    return (int)(accumulator & 0x7FFFFFFF);
}
