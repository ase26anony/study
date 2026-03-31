/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Library call synthesis through unsupported operations */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    __int128 a = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 c = a * b;      /* May generate __multi3 call */
    __int128 d = a / b;      /* May generate __divti3 call */
    
    result += (uint64_t)c + (uint64_t)d;
    
    /* Complex number division (often uses library calls) */
    _Complex double z1 = 3.0 + 4.0 * _Complex_I;
    _Complex double z2 = 1.0 + 2.0 * _Complex_I;
    _Complex double z3 = z1 / z2;
    
    result += (uint64_t)__real__ z3 + (uint64_t)__imag__ z3;
    
    return result;
}

/* OpenMP runtime synthesis */
NOOPT uint64_t test_omp_synthesis(int n) {
    volatile uint64_t result = 0;
    int i;
    
    #pragma omp target map(tofrom: result) if(n > 1000)
    {
        /* Use target-specific built-in inside OpenMP region */
        result = __builtin_ia32_rdtsc();
        
        /* Atomic operation inside target region */
        #pragma omp atomic
        result += 1;
    }
    
    /* OpenMP atomic with capture - may generate runtime calls */
    int capture;
    #pragma omp atomic capture
    capture = result++;
    
    return result + capture;
}

/* Transactional memory synthesis */
NOOPT uint64_t test_tm_synthesis(void) {
    volatile uint64_t result = 0;
    int x = 0;
    
    /* Transactional memory extension */
    __transaction_atomic {
        x++;
        result = __builtin_ia32_rdtsc();
    }
    
    return result + x;
}

/* Mixed synthesis triggers */
NOOPT uint64_t test_mixed_synthesis(__int128 *ptr) {
    volatile uint64_t result = 0;
    
    /* Atomic 128-bit operation (may need library helper) */
    __int128 old = *ptr;
    __int128 new_val = old + 1;
    
    while (!__atomic_compare_exchange_n(ptr, &old, new_val, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        new_val = old + 1;
    }
    
    result = (uint64_t)*ptr;
    
    /* CPU initialization built-in */
    __builtin_cpu_init();
    
    /* Constant detection with runtime fallback */
    if (!__builtin_constant_p(result)) {
        result += __builtin_ia32_rdtsc() & 0xFF;
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Force synthesis by calling functions with different patterns */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_omp_synthesis(argc);
    
    __int128 big_val = 0;
    accumulator += test_mixed_synthesis(&big_val);
    
    /* Transactional memory if supported */
    #ifdef __TM_FENCE__
    accumulator += test_tm_synthesis();
    #endif
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
