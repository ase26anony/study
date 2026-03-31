/* test_synthesis.c - Program to trigger built-in function synthesis in GCC */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins (will trigger synthesis on x86 targets) */
#if defined(__x86_64__) || defined(__i386__)
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
#endif
    
    /* ARM-specific built-ins */
#if defined(__arm__) || defined(__aarch64__)
    unsigned int coproc = 15, opc1 = 0, crn = 0, crm = 0, opc2 = 0;
    result += __builtin_arm_mrc(coproc, opc1, crn, crm, opc2);
#endif
    
    /* Generic atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_val;
}

/* Test 2: Operations requiring library calls */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x10000000000000001ULL;
    __int128 c = a * b;  /* May require __multi3 library call */
    __int128 d = a / b;  /* May require __divti3 library call */
    
    result = (uint64_t)(c >> 64) ^ (uint64_t)d;
    
    /* Complex number division - often requires library calls */
    volatile _Complex double z1 = 3.0 + 4.0i;
    volatile _Complex double z2 = 1.0 + 2.0i;
    volatile _Complex double z3 = z1 / z2;
    
    result += (uint64_t)__real__ z3 + (uint64_t)__imag__ z3;
    
    return result;
}

/* Test 3: CPU feature detection and transactional memory */
NOOPT uint64_t test_advanced_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* CPU feature detection - may synthesize resolver functions */
#if defined(__x86_64__) || defined(__i386__)
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 2;
    }
#endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = result;
    if (__builtin_constant_p(x)) {
        result += 4;
    } else {
        result += 8;  /* Runtime path */
    }
    
    return result;
}

/* Test 4: OpenMP target region (if supported) */
#ifdef _OPENMP
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[100];
    
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = i * i;
        }
        
        /* Use a built-in inside OpenMP region */
        result = __builtin_popcount(data[42]);
    }
    
    return result;
}
#endif

/* Main function that calls all tests */
int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Call all synthesis-triggering functions */
    accumulator ^= test_builtin_synthesis();
    accumulator ^= test_libcall_synthesis();
    accumulator ^= test_advanced_synthesis();
    
#ifdef _OPENMP
    accumulator ^= test_omp_synthesis();
#endif
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
