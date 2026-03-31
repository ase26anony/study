/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S -o test.s test.c */
/* For ARM: gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fopenmp -fdump-tree-all -S -o test.s test.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent interprocedural optimization */
#define NOINLINE_NOIPA __attribute__((noinline, noipa))

/* Test 1: Target-specific built-in synthesis */
NOINLINE_NOIPA
uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
    /* Memory barrier built-in */
    __builtin_ia32_mfence();
    #endif
    
    /* ARM-specific built-ins */
    #ifdef __arm__
    unsigned int coproc = 15, opc1 = 0, CRn = 0, CRm = 0, opc2 = 0;
    result = __builtin_arm_mrc(coproc, opc1, CRn, CRm, opc2);
    __builtin_arm_dmb(0xF); /* Full system barrier */
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    return result + (uint64_t)atomic_val;
}

/* Test 2: Library call synthesis for unsupported operations */
NOINLINE_NOIPA
__int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations that may require library calls */
    result = a * b;           /* Multiplication may need libcall */
    result += a / b;          /* Division definitely needs libcall */
    result += a % b;          /* Modulo needs libcall */
    
    /* Double precision on soft-float target */
    volatile double d1 = 3.141592653589793;
    volatile double d2 = 2.718281828459045;
    volatile double d3 = d1 * d2;  /* May need soft-float libcall */
    
    result += (__int128)(d3 * 1000);
    
    /* Complex division - often requires libcall */
    volatile _Complex float cf1 = 1.0f + 2.0if;
    volatile _Complex float cf2 = 3.0f + 4.0if;
    volatile _Complex float cf3 = cf1 / cf2;
    
    result += (__int128)(__real__ cf3 * 1000);
    
    return result;
}

/* Test 3: OpenMP runtime synthesis */
NOINLINE_NOIPA
int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + n;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:100])
    for (int i = 0; i < 100; i++) {
        arr[i] = arr[i] * 2 + 1;
    }
    
    /* Collect results */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 4: Transactional memory and CPU feature detection */
NOINLINE_NOIPA
int test_tm_cpu_synthesis(int x) {
    volatile int result = x;
    
    /* Transactional memory - may synthesize runtime calls */
    #ifdef __TM_FEATURE_AVAILABLE
    __transaction_atomic {
        result = result * 3 + 1;
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
    
    /* Uncommon atomic memory order */
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELEASE);
    result = __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    
    return result;
}

/* Test 5: Mixed synthesis triggers */
NOINLINE_NOIPA
__int128 test_mixed_synthesis(__int128 a, int b) {
    volatile __int128 result = a;
    
    /* Combine 128-bit atomic with built-in */
    __int128 atomic_val = a;
    __int128 expected = a, desired = a + 1;
    
    __atomic_compare_exchange(&atomic_val, &expected, &desired,
                              0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    result += atomic_val;
    
    /* Use __builtin_constant_p with runtime fallback */
    if (!__builtin_constant_p(b)) {
        /* Force runtime path */
        volatile int* ptr = (volatile int*)&b;
        result += *ptr;
    }
    
    /* Double precision operation */
    volatile double d = (double)b;
    for (int i = 0; i < 10; i++) {
        d = d * 1.1 - 0.5;
    }
    result += (__int128)d;
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis */
    __int128 large_val = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 libcall_result = test_libcall_synthesis(large_val, large_val / 3);
    accumulator += (uint64_t)libcall_result + (uint64_t)(libcall_result >> 64);
    
    /* Test 3: OpenMP synthesis */
    accumulator += test_omp_synthesis(accumulator % 100);
    
    /* Test 4: TM and CPU synthesis */
    accumulator += test_tm_cpu_synthesis(accumulator % 1000);
    
    /* Test 5: Mixed synthesis */
    __int128 mixed_result = test_mixed_synthesis(large_val, accumulator % 100);
    accumulator += (uint64_t)mixed_result + (uint64_t)(mixed_result >> 64);
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator % 256);
}
