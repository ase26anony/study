/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* __atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Library call synthesis for unsupported operations */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    __int128 a = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA987654321ULL;
    __int128 b = 0x1000000000000001ULL;
    __int128 c = a * b;      /* May require __multi3 library call */
    result += (uint64_t)(c >> 64) + (uint64_t)c;
    
    /* Complex number division (often requires library calls) */
    _Complex double cd1 = 3.0 + 4.0i;
    _Complex double cd2 = 1.0 + 2.0i;
    _Complex double cd3 = cd1 / cd2;
    result += (uint64_t)__real__ cd3 + (uint64_t)__imag__ cd3;
    
    /* Double-precision math on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    double z = x * y;        /* May require __muldf3 on soft-float */
    result += (uint64_t)z;
    
    return result;
}

/* OpenMP synthesis for runtime functions */
NOOPT uint64_t test_omp_synthesis(void) {
    volatile uint64_t result = 0;
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    /* Sum results */
    for (int i = 0; i < 100; i++) {
        result += data[i];
    }
    
    return result;
}

/* Transactional memory synthesis */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    int x = 0;
    
    /* Transactional memory - may synthesize TM runtime functions */
    __transaction_atomic {
        x = 42;
        result = x;
    }
    
    /* Use __builtin_constant_p with runtime fallback */
    if (__builtin_constant_p(x)) {
        result += 1;
    } else {
        result += 2;  /* Runtime path */
    }
    
    return result;
}

/* Combined synthesis triggers */
NOOPT uint64_t test_combined_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Mix 128-bit atomics with built-ins */
    __int128 atomic128 = 0;
    __atomic_store_n(&atomic128, 
                     ((__int128)__builtin_ia32_rdtsc() << 64) | __builtin_ia32_rdtsc(),
                     __ATOMIC_RELEASE);
    
    result += (uint64_t)atomic128;
    
    /* Complex math with OpenMP */
    _Complex float cf[10];
    #pragma omp simd
    for (int i = 0; i < 10; i++) {
        cf[i] = (i + 1) + (i * 2)i;
    }
    
    for (int i = 0; i < 10; i++) {
        result += (uint64_t)__real__ cf[i] + (uint64_t)__imag__ cf[i];
    }
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_omp_synthesis();
    accumulator += test_tm_synthesis();
    accumulator += test_combined_synthesis();
    
    /* Print result to ensure observability */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
