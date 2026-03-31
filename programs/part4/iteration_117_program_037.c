/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Force synthesis of built-in declarations */
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
    
    return result;
}

/* Force 128-bit arithmetic library call synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result;
    
    /* Operations that often require library calls on 64-bit targets */
    result = a * b;      /* 128-bit multiplication */
    result += a / b;     /* 128-bit division */
    result += a % b;     /* 128-bit modulo */
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __atomic_load(&result, &atomic_val, __ATOMIC_ACQUIRE);
    __atomic_store(&result, &atomic_val, __ATOMIC_RELEASE);
    
    return result;
}

/* Force soft-float library call synthesis */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result;
    
    /* Complex operations that may require library calls */
    _Complex double c1 = a + b * I;
    _Complex double c2 = b + a * I;
    _Complex double c3 = c1 / c2;  /* Complex division often uses libcalls */
    
    result = __real__ c3 + __imag__ c3;
    
    /* Transcendental functions */
    result += __builtin_sin(a);
    result += __builtin_cos(b);
    result += __builtin_exp(a * b);
    
    return result;
}

/* OpenMP target region for runtime function synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * n;
    }
    
    #pragma omp target map(tofrom: arr) map(to: n)
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] += __builtin_popcount(i) * n;
        }
        
        /* Use target-specific built-ins inside OpenMP region */
        #ifdef __x86_64__
        unsigned int aux;
        result += __builtin_ia32_rdtscp(&aux);
        #endif
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Transactional memory for runtime helper synthesis */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    __transaction_atomic {
        *ptr += 1;
        result = *ptr;
        
        /* Nested atomic operation */
        __atomic_add_fetch(ptr, 1, __ATOMIC_SEQ_CST);
    }
    
    return result;
}

/* Mixed synthesis triggers */
NOOPT uint64_t test_mixed_synthesis(__int128 a, double b) {
    volatile uint64_t result = 0;
    
    /* 128-bit atomic with memory ordering */
    __int128 atomic_128 = a;
    __atomic_compare_exchange(&atomic_128, &a, &a, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Target built-in inside complex computation */
    #ifdef __x86_64__
    result = __builtin_ia32_rdtsc();
    #endif
    
    /* Soft-float operation */
    _Complex double c = b + (b * 2.0) * I;
    result += (uint64_t)__real__(c);
    
    /* Uncommon atomic size */
    struct Uncommon { char data[7]; } uncommon;
    __atomic_load(&uncommon, &uncommon, __ATOMIC_CONSUME);
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Use command line arguments to prevent constant folding */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 a = ((__int128)rand() << 64) | rand();
    __int128 b = ((__int128)rand() << 32) | rand();
    __int128 libcall_result = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)libcall_result + (uint64_t)(libcall_result >> 64);
    
    /* Test 3: Soft-float synthesis */
    double x = (double)rand() / RAND_MAX;
    double y = (double)rand() / RAND_MAX;
    accumulator += (uint64_t)test_softfloat_synthesis(x, y);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(rand() % 100);
    
    /* Test 5: Transactional memory synthesis */
    int tm_var = rand();
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test 6: Mixed synthesis */
    accumulator += test_mixed_synthesis(a, x);
    
    /* Additional architecture-specific tests */
    #ifdef __ARM_ARCH
    /* ARM-specific built-ins */
    unsigned int arm_result;
    __asm__ volatile ("mrc p15, 0, %0, c0, c0, 0" : "=r"(arm_result));
    accumulator += arm_result;
    #endif
    
    #ifdef __BPF__
    /* BPF-specific built-ins */
    accumulator += __builtin_bpf_packet_data();
    accumulator += __builtin_bpf_packet_end();
    #endif
    
    printf("Result: %llu\n", (unsigned long long)accumulator);
    return (int)(accumulator & 0x7FFFFFFF);
}
