/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    #endif
    
    /* ARM-specific if compiled for ARM */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 1, 0);
    #endif
    
    /* BPF-specific */
    #ifdef __bpf__
    result += __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* Test 2: Atomic operations on 128-bit values (requires libcall synthesis) */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit arithmetic - may require libcalls on some targets */
    result = a * b;
    result += a / (b + 1);
    
    /* Atomic operations on 128-bit - often requires helper functions */
    __int128 atomic_val = 0;
    __atomic_load(&a, &atomic_val, __ATOMIC_SEQ_CST);
    __atomic_compare_exchange(&atomic_val, &a, &b, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + atomic_val;
}

/* Test 3: Soft-float double operations on targets without native support */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex operations that often require libcalls */
    result = a / b;
    result += __builtin_sqrt(a * a + b * b);
    
    /* Complex number division - requires runtime support */
    _Complex double c1 = a + b * 1.0i;
    _Complex double c2 = b + a * 1.0i;
    _Complex double cdiv = c1 / c2;
    
    result += __real__(cdiv) + __imag__(cdiv);
    
    return result;
}

/* Test 4: OpenMP target region with data mapping */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * n;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] += __builtin_popcount(i) * n;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 5: Transactional memory and CPU feature detection */
NOOPT int test_tm_synthesis(int x) {
    volatile int result = x;
    
    /* Transactional memory - requires runtime support */
    #ifdef __TM__
    __transaction_atomic {
        result = result * 2 + 1;
    }
    #endif
    
    /* CPU feature detection - may synthesize resolver functions */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1000;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 0x2000;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int y = __builtin_constant_p(x) ? x * 2 : x + 1;
    result += y;
    
    return result;
}

/* Test 6: Mixed operations to maximize synthesis opportunities */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Combine multiple synthesis triggers */
    __int128 big_val = ((__int128)seed << 64) | seed;
    __int128 big_mul = test_libcall_synthesis(big_val, big_val + 1);
    
    result += (uint64_t)(big_mul >> 64) + (uint64_t)big_mul;
    
    /* Use atomic builtins with unusual sizes/orders */
    struct Unusual { char a[7]; } unusual;
    __atomic_store_n(&unusual, (struct Unusual){0}, __ATOMIC_RELAXED);
    
    /* Complex float operations */
    _Complex float cf = seed + seed * 2.0fi;
    cf = cf / (cf + 1.0fi);
    result += (uint64_t)(__real__(cf) * 1000);
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit values */
    __int128 big_a = 0x123456789ABCDEF0ULL;
    big_a = (big_a << 64) | 0xFEDCBA9876543210ULL;
    __int128 big_b = 0x5555555555555555ULL;
    big_b = (big_b << 64) | 0xAAAAAAAAAAAAAAAALL;
    accumulator += (uint64_t)test_libcall_synthesis(big_a, big_b);
    
    /* Test 3: Soft-float synthesis */
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(42);
    
    /* Test 5: Transactional memory synthesis */
    accumulator += test_tm_synthesis(12345);
    
    /* Test 6: Mixed synthesis */
    accumulator += test_mixed_synthesis(accumulator);
    
    /* Ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
