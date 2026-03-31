/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    /* Memory barrier built-in */
    __builtin_ia32_mfence();
    /* CRC32 built-in */
    result += __builtin_ia32_crc32di(result, 0x12345678);
    #endif
    
    /* ARM-style built-in (will be ignored on x86 but triggers different paths) */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 1, 0);
    #endif
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* Operations that often require libcalls on 64-bit targets */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % (b + 1);    /* 128-bit modulo */
    
    /* Atomic operations on 128-bit values */
    __int128 expected = result;
    __atomic_compare_exchange(&a, &expected, &result, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Test 3: Floating-point operations requiring soft-float */
NOOPT double test_float_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex operations that may require libcalls */
    __complex__ double c1 = a + b * 1.0i;
    __complex__ double c2 = b + a * 1.0i;
    __complex__ double c3 = c1 / c2;  /* Complex division often uses libcall */
    
    result = __real__ c3 + __imag__ c3;
    
    /* Transcendental functions */
    result += __builtin_sin(a) * __builtin_cos(b);
    
    return result;
}

/* Test 4: OpenMP target region synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * n;
    }
    
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] += i * 2;
        }
        
        /* Use a built-in inside OpenMP region */
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFF;
        #endif
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 5: Transactional memory and CPU feature detection */
NOOPT int test_tm_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory (may synthesize TM runtime functions) */
    #ifdef __TM__
    __transaction_atomic {
        result = x * 2;
    }
    #endif
    
    /* CPU feature detection (may synthesize resolver functions) */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        result += 100;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 200;
    }
    #endif
    
    return result;
}

/* Test 6: Atomic operations with unusual sizes/orders */
NOOPT long test_atomic_synthesis(long *ptr) {
    volatile long result = 0;
    
    /* Atomic operations that may require helper functions */
    __int128 large_atomic = 0;
    result += __atomic_load_n(&large_atomic, __ATOMIC_ACQUIRE);
    
    /* Atomic exchange with unusual memory order */
    long old = __atomic_exchange_n(ptr, result, __ATOMIC_SEQ_CST);
    result += old;
    
    /* Atomic fetch-add with large type */
    result += __atomic_fetch_add(ptr, 42, __ATOMIC_RELAXED);
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    accumulator += (uint64_t)test_libcall_synthesis(a, b);
    
    /* Test 3: Float libcall synthesis */
    accumulator += (uint64_t)test_float_synthesis(3.14159, 2.71828);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(42);
    
    /* Test 5: TM and CPU feature synthesis */
    accumulator += test_tm_synthesis(100);
    
    /* Test 6: Atomic synthesis */
    long atomic_var = 12345;
    accumulator += test_atomic_synthesis(&atomic_var);
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
