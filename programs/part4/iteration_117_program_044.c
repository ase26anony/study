/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent interprocedural optimization */
#define NOINLINE __attribute__((noinline, noipa))

/* Function using x86-specific builtins that may need synthesis */
NOINLINE uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Use various x86 builtins that might not have predefined declarations */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* CPU feature detection builtins that may synthesize runtime helpers */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Memory barrier builtins */
    __builtin_ia32_mfence();
    
    /* Some SSE/AVX builtins */
    __m128i v1 = _mm_setzero_si128();
    __m128i v2 = _mm_set1_epi32(42);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    return result;
}

/* Function requiring 128-bit arithmetic (may synthesize library calls) */
NOINLINE uint64_t test_libcall_synthesis(void) {
    volatile __int128 a = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    volatile __int128 b = 0x1000000000000000ULL;
    volatile __int128 c;
    
    /* 128-bit operations that may require library calls */
    c = a + b;
    c = c * 3;
    c = c / 5;
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, c, __ATOMIC_SEQ_CST);
    __int128 loaded = __atomic_load_n(&atomic_val, __ATOMIC_RELAXED);
    
    return (uint64_t)(loaded >> 64) + (uint64_t)loaded;
}

/* Function using double-precision math on hypothetical soft-float target */
NOINLINE uint64_t test_float_synthesis(double x, double y) {
    volatile double result = 0.0;
    
    /* Complex floating-point operations that may synthesize library calls */
    result = x * y;
    result = result / (x + y);
    result = __builtin_sqrt(result);
    
    /* Complex number division */
    _Complex double z1 = x + y * _Complex_I;
    _Complex double z2 = y + x * _Complex_I;
    _Complex double z3 = z1 / z2;
    
    result += __real__(z3) + __imag__(z3);
    
    /* Convert to integer in a way that might need runtime support */
    return (uint64_t)(result * 1000.0);
}

/* OpenMP function that may synthesize runtime helpers */
NOINLINE uint64_t test_omp_synthesis(void) {
    volatile uint64_t sum = 0;
    int array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping and runtime functions */
    #pragma omp target teams distribute parallel for map(tofrom: array[0:100])
    for (int i = 0; i < 100; i++) {
        array[i] *= 2;
    }
    
    /* Collect results */
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    return sum;
}

/* Transactional memory (if supported) */
NOINLINE uint64_t test_tm_synthesis(void) {
    volatile uint64_t counter = 0;
    
    /* Transactional memory block - may synthesize runtime functions */
    __transaction_atomic {
        counter++;
        counter *= 2;
    }
    
    return counter;
}

/* Main function that exercises all synthesis paths */
int main(void) {
    volatile uint64_t accumulator = 0;
    
    printf("Starting synthesis tests...\n");
    
    /* Test 1: Builtin synthesis */
    accumulator += test_builtin_synthesis();
    printf("Builtin test completed: %llu\n", (unsigned long long)accumulator);
    
    /* Test 2: 128-bit libcall synthesis */
    accumulator += test_libcall_synthesis();
    printf("128-bit test completed: %llu\n", (unsigned long long)accumulator);
    
    /* Test 3: Float synthesis */
    accumulator += test_float_synthesis(3.14159, 2.71828);
    printf("Float test completed: %llu\n", (unsigned long long)accumulator);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis();
    printf("OpenMP test completed: %llu\n", (unsigned long long)accumulator);
    
    /* Test 5: Transactional memory (if available) */
    #ifdef __TM_FEATURE_AVAILABLE
    accumulator += test_tm_synthesis();
    printf("TM test completed: %llu\n", (unsigned long long)accumulator);
    #endif
    
    /* Final checksum */
    uint64_t checksum = accumulator;
    for (int i = 0; i < 64; i++) {
        checksum = (checksum >> 1) ^ ((checksum & 1) ? 0xEDB88320UL : 0);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    printf("All tests completed.\n");
    
    return (int)(checksum & 0x7FFFFFFF);
}
