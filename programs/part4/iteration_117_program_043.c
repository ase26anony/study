/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Target-specific built-in synthesis */
NOOPT unsigned long long test_builtin_synthesis(void) {
    volatile unsigned long long result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    #endif
    
    /* ARM-specific built-in (if compiled for ARM) */
    #ifdef __arm__
    unsigned int coproc = 15, opc1 = 0, crn = 0, crm = 0, opc2 = 0;
    result = __builtin_arm_mrc(coproc, opc1, crn, crm, opc2);
    #endif
    
    /* Atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0, atomic_new = 1;
    __atomic_compare_exchange(&atomic_val, &atomic_val, &atomic_new, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    return result + (unsigned long long)atomic_val;
}

/* Library call synthesis for unsupported operations */
NOOPT unsigned long long test_libcall_synthesis(void) {
    volatile unsigned long long result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = (unsigned __int128)0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 b = a * a;  /* May require __multi3 library call */
    result = (unsigned long long)(b >> 64) + (unsigned long long)b;
    
    /* Complex division requiring library call */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double c3 = c1 / c2;
    result += (unsigned long long)__real__(c3) + (unsigned long long)__imag__(c3);
    
    /* Double precision on soft-float target */
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    double z = x * y;  /* May require soft-float library call */
    result += (unsigned long long)z;
    
    return result;
}

/* OpenMP synthesis requiring runtime functions */
NOOPT unsigned long long test_omp_synthesis(void) {
    volatile unsigned long long result = 0;
    int data[100];
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: data[0:100])
    {
        for (int i = 0; i < 100; i++) {
            data[i] *= 2;
        }
    }
    
    /* Collect results */
    for (int i = 0; i < 100; i++) {
        result += data[i];
    }
    
    return result;
}

/* Advanced extensions requiring runtime support */
NOOPT unsigned long long test_advanced_synthesis(void) {
    volatile unsigned long long result = 0;
    
    /* Transactional memory (if supported) */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result = 42;
    }
    #endif
    
    /* CPU feature detection - may synthesize resolver functions */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 2;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = 0;
    if (__builtin_constant_p(x)) {
        result += 10;
    } else {
        result += 20;  /* Runtime path */
    }
    
    return result;
}

/* Combined synthesis triggers */
NOOPT unsigned long long test_combined_synthesis(void) {
    volatile unsigned long long result = 0;
    
    /* 128-bit atomic inside OpenMP region */
    __int128 atomic_128 = 0;
    
    #pragma omp parallel
    {
        __int128 local = 1;
        __atomic_fetch_add(&atomic_128, local, __ATOMIC_RELAXED);
    }
    
    result = (unsigned long long)atomic_128;
    
    /* Mix with target-specific built-in */
    #ifdef __x86_64__
    result ^= __builtin_ia32_rdtsc();
    #endif
    
    return result;
}

int main(void) {
    volatile unsigned long long accumulator = 0;
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_omp_synthesis();
    accumulator += test_advanced_synthesis();
    accumulator += test_combined_synthesis();
    
    /* Print result to ensure observability */
    printf("Result: %llu\n", accumulator);
    
    return 0;
}
