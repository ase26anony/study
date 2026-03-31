/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Helper to prevent dead code elimination */
static volatile int sink;

/* Pattern 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    /* x86-specific built-ins that may require synthesis */
    uint64_t result = 0;
    
    /* __builtin_ia32_rdtsc - often synthesized */
    result ^= __builtin_ia32_rdtsc();
    
    /* __builtin_ia32_rdtscp - may require synthesis */
    unsigned int aux;
    result ^= __builtin_ia32_rdtscp(&aux);
    
    /* __builtin_cpu_supports - may trigger resolver synthesis */
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x1;
    }
    
    /* Memory barrier built-in */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    return result;
}

/* Pattern 2: 128-bit operations requiring libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    /* 128-bit multiplication - often requires libcall on many targets */
    __int128 mul = a * b;
    
    /* 128-bit division - almost always requires libcall */
    __int128 div = 0;
    if (b != 0) {
        div = a / b;
    }
    
    /* Atomic 128-bit operation - may require helper synthesis */
    __int128 old_val = a;
    __int128 new_val = b;
    __atomic_compare_exchange_n(&a, &old_val, new_val, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    return mul ^ div ^ a;
}

/* Pattern 3: Soft-float operations requiring libcall synthesis */
NOOPT double test_softfloat_synthesis(double a, double b) {
    /* Complex division often requires libcall */
    __complex__ double c1 = a + b * 1.0i;
    __complex__ double c2 = b + a * 1.0i;
    __complex__ double cdiv = c1 / c2;
    
    /* Transcendental functions often require libcalls */
    double trig_result = __builtin_sin(a) * __builtin_cos(b);
    
    /* Fused multiply-add may require synthesis on some targets */
    double fma_result = __builtin_fma(a, b, trig_result);
    
    return __real__ cdiv + __imag__ cdiv + fma_result;
}

/* Pattern 4: OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    int result = 0;
    
    /* OpenMP target region may trigger runtime function synthesis */
    #pragma omp target map(tofrom: result)
    {
        /* Use some built-ins inside target region */
        for (int i = 0; i < n; i++) {
            result += __builtin_popcount(i);
        }
    }
    
    return result;
}

/* Pattern 5: Transactional memory requiring runtime synthesis */
NOOPT int test_tm_synthesis(int *ptr) {
    int result = 0;
    
    /* Transactional memory built-in */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* Pattern 6: BPF-style built-in (if compiled for BPF target) */
NOOPT unsigned long test_bpf_synthesis(void) {
    unsigned long result = 0;
    
    /* These may trigger synthesis when not natively supported */
    result = __builtin_bpf_pseudo_call(1);
    
    return result;
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize with some non-zero values */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    
    /* Test 1: Built-in synthesis */
    accumulator ^= test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis for 128-bit */
    __int128 int128_result = test_libcall_synthesis(a, b);
    accumulator ^= (uint64_t)int128_result ^ (uint64_t)(int128_result >> 64);
    
    /* Test 3: Soft-float libcall synthesis */
    double fp_result = test_softfloat_synthesis(3.1415926535, 2.7182818284);
    accumulator ^= *(uint64_t*)&fp_result;
    
    /* Test 4: OpenMP synthesis */
    int omp_result = test_omp_synthesis(100);
    accumulator ^= omp_result;
    
    /* Test 5: Transactional memory synthesis */
    int tm_var = 42;
    int tm_result = test_tm_synthesis(&tm_var);
    accumulator ^= tm_result ^ tm_var;
    
    /* Test 6: Various atomic operations that may require helpers */
    struct LargeStruct {
        uint64_t data[4];
    } atomic_struct = {0};
    
    struct LargeStruct desired = {{1, 2, 3, 4}};
    struct LargeStruct expected = {0};
    
    /* Large atomic compare-exchange may require helper */
    __atomic_compare_exchange(&atomic_struct, &expected, &desired,
                              0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    for (int i = 0; i < 4; i++) {
        accumulator ^= atomic_struct.data[i];
    }
    
    /* Use sink to prevent optimization */
    sink = accumulator;
    
    /* Print result to ensure observability */
    printf("Result: 0x%016lx\n", (unsigned long)accumulator);
    
    return 0;
}
