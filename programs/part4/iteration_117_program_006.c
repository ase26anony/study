/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Force declaration synthesis patterns */

/* Pattern 1: Target-specific built-ins requiring synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require declaration synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
    
    /* AVX/SSE built-ins - some may require synthesized declarations */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i v3 = _mm_add_epi32(v1, v2);
    result += ((uint64_t*)&v3)[0];
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, (__int128)0x123456789ABCDEF, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Pattern 2: Operations requiring library call synthesis */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)0x12345678 << 64) | 0x9ABCDEF0;
    unsigned __int128 b = ((unsigned __int128)0xFEDCBA98 << 64) | 0x76543210;
    unsigned __int128 c = a * b;  /* May require __multi3 library call */
    unsigned __int128 d = a / (b >> 120);  /* May require __udivti3 */
    
    result = (uint64_t)c + (uint64_t)d;
    
    /* Double-precision operations on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    volatile double z = x * y + x / y;  /* May require soft-float libcalls */
    
    result += (uint64_t)z;
    
    /* Complex number division */
    _Complex double comp1 = 3.0 + 4.0 * _Complex_I;
    _Complex double comp2 = 1.0 + 2.0 * _Complex_I;
    _Complex double comp3 = comp1 / comp2;  /* May require library call */
    
    result += (uint64_t)__real__(comp3) + (uint64_t)__imag__(comp3);
    
    return result;
}

/* Pattern 3: Transactional memory extensions */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t counter = 0;
    
    /* Transactional memory - may require runtime function synthesis */
    #ifdef __TM_supported
    __transaction_atomic {
        counter++;
        counter *= 2;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int dynamic_value = rand() % 100;
    if (__builtin_constant_p(dynamic_value)) {
        counter += 1000;
    } else {
        counter += dynamic_value;
    }
    
    return counter;
}

/* Pattern 4: OpenMP target region requiring runtime synthesis */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[1024];
    
    /* Initialize data */
    for (int i = 0; i < 1024; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping runtime functions */
    #pragma omp target teams distribute parallel for map(tofrom: data[0:1024])
    for (int i = 0; i < 1024; i++) {
        data[i] = data[i] * 2 + 1;
    }
    
    /* Collect result */
    for (int i = 0; i < 1024; i++) {
        result += data[i];
    }
    
    return result;
}

/* Pattern 5: Mixed synthesis triggers */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Atomic operation on 128-bit value */
    __int128 atomic_128 = 0;
    __int128 expected = 0;
    __int128 desired = ((__int128)seed << 64) | seed;
    
    __atomic_compare_exchange_n(&atomic_128, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result += (uint64_t)atomic_128;
    
    /* Architecture-specific built-in inside computation */
    #ifdef __x86_64__
    unsigned int aux;
    uint64_t tsc = __builtin_ia32_rdtscp(&aux);
    result ^= tsc;
    result += aux;
    #endif
    
    /* 128-bit division */
    unsigned __int128 big_num = ((unsigned __int128)result << 64) | result;
    unsigned __int128 quotient = big_num / (result | 1);
    result += (uint64_t)quotient;
    
    return result;
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Use command-line argument as seed to prevent constant folding */
    uint64_t seed = (argc > 1) ? (uint64_t)atoi(argv[1]) : 12345;
    
    printf("Testing built-in function synthesis...\n");
    accumulator += test_builtin_synthesis();
    
    printf("Testing library call synthesis...\n");
    accumulator += test_libcall_synthesis();
    
    printf("Testing transactional memory synthesis...\n");
    accumulator += test_tm_synthesis();
    
    printf("Testing OpenMP synthesis...\n");
    accumulator += test_omp_synthesis();
    
    printf("Testing mixed synthesis...\n");
    accumulator += test_mixed_synthesis(seed);
    
    /* Make result observable */
    printf("Result checksum: 0x%016llx\n", (unsigned long long)accumulator);
    
    /* Use result to affect return value */
    return (accumulator == 0) ? 1 : 0;
}

/* Additional architecture-specific tests */
#ifdef __ARM_ARCH
NOOPT uint64_t test_arm_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* ARM-specific system register access */
    #ifdef __arm__
    unsigned int val;
    __asm__ volatile("mrc p15, 0, %0, c0, c0, 0" : "=r"(val));
    result += val;
    #endif
    
    /* ARM NEON intrinsics */
    uint32x4_t v1 = vdupq_n_u32(0x12345678);
    uint32x4_t v2 = vdupq_n_u32(0x9ABCDEF0);
    uint32x4_t v3 = vaddq_u32(v1, v2);
    result += vgetq_lane_u32(v3, 0);
    
    return result;
}
#endif

/* BPF target synthesis */
#ifdef __bpf__
NOOPT uint64_t test_bpf_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* BPF built-ins for packet access */
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    
    return result;
}
#endif
