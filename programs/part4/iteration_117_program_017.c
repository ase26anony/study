/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Volatile accumulator to prevent optimization */
static volatile uint64_t global_acc = 0;

/* ========== Pattern 1: Target-specific built-ins ========== */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    /* RDTSC - time stamp counter */
    result += __builtin_ia32_rdtsc();
    
    /* CPUID leaf 0 */
    unsigned int eax, ebx, ecx, edx;
    __cpuid(0, eax, ebx, ecx, edx);
    result += eax + ebx + ecx + edx;
    
    /* SSE/AVX built-ins - some may require helper functions */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += _mm_extract_epi32(v3, 0);
    
    /* Memory barrier */
    __sync_synchronize();
    #endif
    
    /* ARM-specific if compiled for ARM */
    #ifdef __arm__
    /* MRC coprocessor register access */
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    struct Uncommon { char data[7]; } uncommon;
    __atomic_load(&uncommon, &uncommon, __ATOMIC_ACQUIRE);
    result += uncommon.data[0];
    
    return result;
}

/* ========== Pattern 2: 128-bit arithmetic on 32/64-bit target ========== */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit integer operations - may require libcalls on some targets */
    __int128 a = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA987654321ULL;
    __int128 b = ((__int128)0x111111111111111ULL << 64) | 0x222222222222222ULL;
    
    /* These operations often trigger libcall synthesis */
    __int128 mul = a * b;
    __int128 div = a / (b + 1);
    __int128 mod = a % (b + 2);
    
    result += (uint64_t)mul + (uint64_t)(mul >> 64);
    result += (uint64_t)div + (uint64_t)(div >> 64);
    result += (uint64_t)mod;
    
    /* Complex number division - often requires libcalls */
    _Complex double cd1 = 3.0 + 4.0 * _Complex_I;
    _Complex double cd2 = 1.0 + 2.0 * _Complex_I;
    _Complex double cd_div = cd1 / cd2;
    result += (uint64_t)__real__ cd_div + (uint64_t)__imag__ cd_div;
    
    /* Double precision on soft-float target simulation */
    double d1 = 3.141592653589793;
    double d2 = 2.718281828459045;
    double d3 = d1 * d2 / (d1 + d2);
    result += (uint64_t)d3;
    
    return result;
}

/* ========== Pattern 3: Transactional memory extensions ========== */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    volatile int x = 42;
    
    /* Transactional memory - may require runtime call synthesis */
    #ifdef __TM_supported
    __transaction_atomic {
        x++;
        result = x;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int y = rand();
    if (__builtin_constant_p(y)) {
        result += 1;
    } else {
        result += y & 0xFF;
    }
    
    /* CPU feature detection - may trigger resolver synthesis */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        result += 100;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 200;
    }
    __builtin_cpu_init();
    #endif
    
    return result;
}

/* ========== Pattern 4: OpenMP target region ========== */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    
    #ifdef _OPENMP
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - triggers runtime function synthesis */
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        result += data[i];
    }
    
    /* OpenACC-like pragma simulation */
    #pragma omp parallel
    {
        #pragma omp single
        {
            result += omp_get_num_threads();
        }
    }
    #endif
    
    return result;
}

/* ========== Pattern 5: Mixed synthesis triggers ========== */
NOOPT uint64_t test_mixed_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Mix 128-bit atomics (complex case) */
    typedef struct { __int128 val; } Atomic128;
    Atomic128 atomic_var = {0};
    
    /* Atomic exchange on 128-bit - may require libcall */
    __atomic_exchange(&atomic_var.val, &atomic_var.val, &atomic_var.val, __ATOMIC_SEQ_CST);
    
    /* Mixed precision floating point */
    float f = 1.5f;
    double d = 2.5;
    long double ld = 3.5L;
    
    result += (uint64_t)(f * d * ld);
    
    /* Vector operations that may require synthesis */
    #ifdef __SSE2__
    __m128i vec1 = _mm_set1_epi32(0x12345678);
    __m128i vec2 = _mm_set1_epi32(0x9ABCDEF0);
    __m128i vec3 = _mm_xor_si128(vec1, vec2);
    result += _mm_extract_epi32(vec3, 0);
    #endif
    
    /* Unaligned atomic access */
    char buffer[16] = {0};
    int* unaligned_ptr = (int*)(buffer + 1);
    __atomic_store_n(unaligned_ptr, 0xDEADBEEF, __ATOMIC_RELAXED);
    result += __atomic_load_n(unaligned_ptr, __ATOMIC_RELAXED) & 0xFF;
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    uint64_t accumulator = 0;
    
    /* Call all synthesis-triggering functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_tm_synthesis();
    accumulator += test_omp_synthesis();
    accumulator += test_mixed_synthesis();
    
    /* Store to volatile to ensure all operations are observable */
    global_acc = accumulator;
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: 0x%016llX\n", (unsigned long long)global_acc);
    
    /* Use results in branching to prevent dead code elimination */
    if (global_acc != 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
