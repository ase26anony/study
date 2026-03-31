/* test_synthesis.c
 * Designed to trigger built-in function synthesis in GCC's targhooks.cc
 * Specifically targets lines 981-990 which set attributes on synthesized tree nodes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Volatile accumulator to prevent optimization */
static volatile uint64_t accumulator = 0;

/* ============================================
 * Pattern 1: Target-specific built-in functions
 * ============================================ */

NOOPT
uint64_t test_builtin_synthesis(void) {
    uint64_t result = 0;
    
    /* x86-specific built-ins (will trigger synthesis on x86 targets) */
#ifdef __x86_64__
    /* rdtsc - commonly synthesized */
    result ^= __builtin_ia32_rdtsc();
    
    /* Various SSE/AVX built-ins */
    if (__builtin_cpu_supports("sse2")) {
        __m128i a = _mm_setzero_si128();
        __m128i b = _mm_set1_epi32(0x12345678);
        __m128i c = _mm_add_epi32(a, b);
        result ^= _mm_extract_epi32(c, 0);
    }
#endif
    
    /* ARM-specific built-ins */
#ifdef __arm__
    /* ARM system register access - often synthesized */
    result ^= __builtin_arm_mrc(15, 0, 0, 13, 0);
#endif
    
    /* BPF built-ins */
#ifdef __bpf__
    result ^= __builtin_bpf_packet_data();
#endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 atomic_new = 0x123456789ABCDEF0;
    __int128 atomic_expected = 0;
    
    /* 128-bit atomic operations often require synthesized helpers */
    __atomic_compare_exchange(&atomic_val, &atomic_expected, &atomic_new, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result ^= (uint64_t)atomic_val;
    
    return result;
}

/* ============================================
 * Pattern 2: Unsupported operations requiring libcalls
 * ============================================ */

NOOPT
uint64_t test_libcall_synthesis(void) {
    uint64_t result = 0;
    
    /* 128-bit integer arithmetic on 32-bit target */
    __int128 a = ((__int128)0x12345678 << 64) | 0x9ABCDEF012345678ULL;
    __int128 b = ((__int128)0xFEDCBA98 << 64) | 0x76543210FEDCBA98ULL;
    
    /* These operations often require library calls on 32-bit targets */
    __int128 mul_result = a * b;
    __int128 div_result = a / (b ? b : 1);
    
    result ^= (uint64_t)mul_result;
    result ^= (uint64_t)(mul_result >> 64);
    result ^= (uint64_t)div_result;
    
    /* Double-precision floating point on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    
    /* Complex operations often require libcalls */
    _Complex double c1 = x + y * _Complex_I;
    _Complex double c2 = y + x * _Complex_I;
    _Complex double cdiv = c1 / c2;
    
    result ^= *(uint64_t*)&x;
    result ^= *(uint64_t*)&y;
    result ^= *(uint64_t*)&cdiv;
    
    return result;
}

/* ============================================
 * Pattern 3: Language extensions requiring runtime support
 * ============================================ */

NOOPT
uint64_t test_extension_synthesis(void) {
    uint64_t result = 0;
    
    /* Transactional memory - requires runtime support */
    _Bool tx_success = 0;
    
    /* This may trigger synthesis of TM runtime functions */
    __transaction_atomic {
        tx_success = 1;
        result ^= 0xDEADBEEF;
    }
    
    result ^= tx_success;
    
    /* CPU feature detection - may synthesize resolver functions */
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result ^= 0x512;
    }
    if (__builtin_cpu_supports("avx2")) {
        result ^= 0x256;
    }
#endif
    
    /* __builtin_constant_p with runtime fallback */
    int dynamic_value = rand() % 100;
    if (__builtin_constant_p(dynamic_value)) {
        result ^= 0x1111;
    } else {
        /* Runtime path - may trigger synthesis */
        result ^= 0x2222 ^ dynamic_value;
    }
    
    return result;
}

/* ============================================
 * Pattern 4: OpenMP target region synthesis
 * ============================================ */

NOOPT
uint64_t test_omp_synthesis(void) {
    uint64_t result = 0;
    int data[1024];
    
    /* Initialize data */
    for (int i = 0; i < 1024; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping and runtime functions */
    #pragma omp target map(tofrom: data[0:1024])
    {
        #pragma omp parallel for
        for (int i = 0; i < 1024; i++) {
            data[i] *= 2;
        }
        
        /* Use a built-in inside OpenMP region */
        #ifdef __x86_64__
        result ^= __builtin_ia32_rdtsc();
        #endif
    }
    
    /* Accumulate results */
    for (int i = 0; i < 1024; i++) {
        result ^= data[i];
    }
    
    return result;
}

/* ============================================
 * Pattern 5: Combined synthesis triggers
 * ============================================ */

NOOPT
uint64_t test_combined_synthesis(void) {
    uint64_t result = 0;
    
    /* Combine 128-bit atomics with built-ins */
    __int128 atomic_val = 0;
    __int128 atomic_new = 1;
    
    for (int i = 0; i < 10; i++) {
        /* Atomic operation that may require synthesized helper */
        __atomic_store_n(&atomic_val, atomic_new, __ATOMIC_RELEASE);
        
        /* Mix with target built-in */
        #ifdef __x86_64__
        result ^= __builtin_ia32_rdtsc();
        #endif
        
        atomic_new++;
    }
    
    /* Complex division in loop */
    _Complex float cf1 = 1.0f + 2.0f * _Complex_I;
    _Complex float cf2 = 3.0f + 4.0f * _Complex_I;
    
    for (int i = 0; i < 5; i++) {
        cf1 = cf1 / cf2;
        result ^= *(uint32_t*)&cf1;
    }
    
    return result;
}

/* ============================================
 * Main function
 * ============================================ */

int main(void) {
    /* Initialize random seed for __builtin_constant_p test */
    srand(42);
    
    /* Call all synthesis test functions */
    accumulator ^= test_builtin_synthesis();
    accumulator ^= test_libcall_synthesis();
    accumulator ^= test_extension_synthesis();
    accumulator ^= test_omp_synthesis();
    accumulator ^= test_combined_synthesis();
    
    /* Print result to ensure observability */
    printf("Accumulator: 0x%016llX\n", (unsigned long long)accumulator);
    
    /* Also print to volatile variable to prevent optimization */
    volatile uint64_t final_result = accumulator;
    
    return (final_result != 0) ? 0 : 1;
}
