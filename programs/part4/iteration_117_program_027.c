/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __sync_synchronize();
    
    /* Atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __atomic_load_n(&atomic_val, __ATOMIC_ACQUIRE);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Test 2: Operations requiring library calls */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    /* 128-bit arithmetic on targets without native support */
    __int128 mul = a * b;      /* May generate __multi3 call */
    __int128 div = a / b;      /* May generate __divti3 call */
    __int128 mod = a % b;      /* May generate __modti3 call */
    
    /* Complex number division (often uses library calls) */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double cdiv = c1 / c2;
    
    /* Double precision on soft-float target */
    volatile double d1 = 3.141592653589793;
    volatile double d2 = 2.718281828459045;
    volatile double ddiv = d1 / d2;  /* May generate __divdf3 call */
    
    return mul + div + mod + (__int128)(__real__ cdiv) + (__int128)ddiv;
}

/* Test 3: OpenMP runtime function synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int i;
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: result)
    {
        /* Use some built-ins inside OpenMP region */
        for (i = 0; i < n; i++) {
            result += __builtin_popcount(i);  /* Population count */
        }
    }
    
    /* OpenMP atomic with capture */
    int capture;
    #pragma omp atomic capture
    capture = result++;
    
    return result + capture;
}

/* Test 4: Transactional memory extensions */
NOOPT int test_tm_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize _ITM_* runtime calls */
    __transaction_atomic {
        result = x * 2;
        /* Nested atomic operation */
        __atomic_add_fetch(&result, 1, __ATOMIC_SEQ_CST);
    }
    
    return result;
}

/* Test 5: Mixed patterns to maximize synthesis chances */
NOOPT uint64_t test_mixed_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Combination of different synthesis triggers */
    
    /* 1. Atomic on 128-bit value */
    volatile __int128 atomic128 = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange_n(&atomic128, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* 2. Target-specific built-in with unusual memory order */
    result += __builtin_ia32_rdtscp(&atomic128);  /* RDTSCP with parameter */
    
    /* 3. Floating point operation that may need libcall */
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ld_div = ld1 / ld2;  /* May generate __divtf3 */
    
    /* 4. Built-in with constant propagation fallback */
    if (!__builtin_constant_p(result)) {
        result += __builtin_bswap64(0x123456789ABCDEF0ULL);
    }
    
    return result + (uint64_t)ld_div + (uint64_t)atomic128;
}

int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    
    __int128 a = ((__int128)0x12345678 << 64) | 0x9ABCDEF0;
    __int128 b = ((__int128)0xFEDCBA98 << 64) | 0x76543210;
    accumulator += (uint64_t)test_libcall_synthesis(a, b);
    
    accumulator += test_omp_synthesis(100);
    accumulator += test_tm_synthesis(argc > 1 ? argv[1][0] : 42);
    accumulator += test_mixed_synthesis();
    
    /* Print result to ensure observability */
    printf("Accumulator: %llu\n", (unsigned long long)accumulator);
    
    /* Additional volatile store to prevent dead code elimination */
    volatile uint64_t sink = accumulator;
    (void)sink;
    
    return 0;
}
