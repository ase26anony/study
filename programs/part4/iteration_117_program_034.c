/* test_synthesis.c - Program to trigger built-in function synthesis in GCC */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimizations that might eliminate synthesis */
#define NOOPT __attribute__((noinline, noipa, used))

/* Volatile accumulator to prevent dead code elimination */
static volatile uint64_t accumulator = 0;

/* ==================== Target-Specific Built-in Synthesis ==================== */

NOOPT uint64_t test_x86_intrinsics(void) {
    uint64_t result = 0;
    
    /* x86-specific intrinsics that may require synthesis */
#ifdef __x86_64__
    /* RDTSC - timestamp counter */
    result += __builtin_ia32_rdtsc();
    
    /* CPUID-like functionality */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* MMX/SSE/AVX intrinsics */
    __m128i vec = _mm_setzero_si128();
    vec = _mm_add_epi32(vec, _mm_set1_epi32(42));
    result += _mm_extract_epi32(vec, 0);
#endif
    
    return result;
}

NOOPT uint64_t test_atomic_operations(void) {
    uint64_t result = 0;
    
    /* 128-bit atomic operations often require libcall synthesis */
    __int128 large_val = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 large_inc = 1;
    
    /* Atomic exchange on 128-bit - may synthesize __atomic_exchange_16 */
    __atomic_exchange(&large_val, &large_inc, &large_val, __ATOMIC_SEQ_CST);
    result += (uint64_t)large_val;
    
    /* Atomic compare-exchange on misaligned/special-sized data */
    struct { char a; uint64_t b; } __attribute__((packed)) misaligned_struct = {0, 0xDEADBEEF};
    uint64_t expected = 0xDEADBEEF;
    uint64_t desired = 0xCAFEBABE;
    
    __atomic_compare_exchange(&misaligned_struct.b, &expected, &desired, 
                              0, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    result += misaligned_struct.b;
    
    return result;
}

/* ==================== Library Call Synthesis ==================== */

NOOPT uint64_t test_128bit_arithmetic(void) {
    uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x10000000000000001ULL;
    
    /* These operations may synthesize library calls */
    unsigned __int128 sum = a + b;
    unsigned __int128 diff = a - b;
    unsigned __int128 prod = a * b;
    unsigned __int128 quot = a / (b + 1);  /* Avoid division by zero */
    
    result += (uint64_t)sum;
    result += (uint64_t)diff;
    result += (uint64_t)prod;
    result += (uint64_t)quot;
    
    return result;
}

NOOPT uint64_t test_soft_float_operations(void) {
    double result = 0.0;
    
    /* Complex operations that may require libcalls */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    
    /* Complex division often requires library calls */
    _Complex double cdiv = c1 / c2;
    
    /* Transcendental functions */
    result += __builtin_sin(cdiv);
    result += __builtin_cos(cdiv);
    
    /* Double-precision math on soft-float targets */
    double a = 3.141592653589793;
    double b = 2.718281828459045;
    
    result += a * b;
    result += a / b;
    result += __builtin_pow(a, b);
    
    return (uint64_t)result;
}

/* ==================== OpenMP Runtime Synthesis ==================== */

NOOPT uint64_t test_omp_synthesis(void) {
    uint64_t result = 0;
    
#ifdef _OPENMP
    /* OpenMP target region - may synthesize data mapping routines */
    int data[1024];
    
    #pragma omp target teams distribute parallel for map(tofrom: data[0:1024])
    for (int i = 0; i < 1024; i++) {
        data[i] = i * i;
    }
    
    for (int i = 0; i < 1024; i++) {
        result += data[i];
    }
    
    /* OpenMP atomic capture with 64-bit */
    uint64_t atomic_var = 0;
    #pragma omp atomic capture
    {
        result = atomic_var;
        atomic_var += 0x12345678;
    }
#endif
    
    return result;
}

/* ==================== Transactional Memory ==================== */

NOOPT uint64_t test_transactional_memory(void) {
    uint64_t result = 0;
    
    /* Transactional memory - may synthesize runtime calls */
    __transaction_atomic {
        static uint64_t tm_var = 0;
        tm_var++;
        result = tm_var;
    }
    
    return result;
}

/* ==================== Mixed Synthesis Patterns ==================== */

NOOPT uint64_t test_mixed_synthesis(void) {
    uint64_t result = 0;
    
    /* Mix 128-bit atomics with target built-ins */
    __int128 atomic_128 = 0;
    __int128 desired = 1;
    
    __atomic_store(&atomic_128, &desired, __ATOMIC_RELAXED);
    result += (uint64_t)atomic_128;
    
#ifdef __x86_64__
    /* Mix with CPU feature detection */
    if (__builtin_cpu_supports("sse4.2")) {
        __m128i v = _mm_set1_epi64x(result);
        result += _mm_extract_epi64(v, 0);
    }
#endif
    
    /* Add some soft-float operations */
    _Complex float cf = 1.0f + 2.0fi;
    cf = cf / (0.5f + 0.25fi);
    result += (uint64_t)__real__ cf;
    
    return result;
}

/* ==================== Main Function ==================== */

int main(void) {
    printf("Starting synthesis test...\n");
    
    /* Call all synthesis triggers */
    accumulator += test_x86_intrinsics();
    accumulator += test_atomic_operations();
    accumulator += test_128bit_arithmetic();
    accumulator += test_soft_float_operations();
    accumulator += test_omp_synthesis();
    accumulator += test_transactional_memory();
    accumulator += test_mixed_synthesis();
    
    /* Ensure all operations are observable */
    printf("Result: 0x%016llX\n", (unsigned long long)accumulator);
    
    return 0;
}
