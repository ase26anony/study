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
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    result ^= __builtin_ia32_crc32di(result, result);
    #endif
    
    /* ARM-specific built-ins (if compiled for ARM) */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* BPF built-ins */
    #ifdef __bpf__
    result += __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* 128-bit integer operations forcing libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* Operations that often require libcalls on 64-bit targets */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % b;          /* 128-bit modulo */
    
    /* Atomic operations on 128-bit values */
    __int128 expected = a;
    __atomic_compare_exchange(&result, &expected, &b, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Double-precision operations on targets with only single-precision FPU */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex operations that often require libcalls */
    result = a / b;           /* Division may need soft-float */
    result += __builtin_sqrt(a * b);  /* Square root */
    
    /* Complex number division */
    __complex__ double c1 = a + b * 1.0i;
    __complex__ double c2 = b + a * 1.0i;
    __complex__ double cdiv = c1 / c2;
    result += __real__(cdiv) + __imag__(cdiv);
    
    return result;
}

/* OpenMP target region triggering runtime function synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int *array = (int*)malloc(n * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping functions */
    #pragma omp target map(tofrom: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Collect results */
    for (int i = 0; i < n; i++) {
        result += array[i];
    }
    
    free(array);
    return result;
}

/* Transactional memory extensions */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory operation */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* CPU feature detection - may synthesize resolver functions */
NOOPT int test_cpu_dispatch_synthesis(void) {
    volatile int result = 0;
    
    /* CPU feature checks */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("avx2")) {
        result |= 2;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 4;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = rand();
    if (__builtin_constant_p(x)) {
        result += 100;
    } else {
        result += x % 100;  /* Runtime path */
    }
    
    return result;
}

/* Combined synthesis triggers in one function */
NOOPT uint64_t test_combined_synthesis(__int128 a, __int128 b, double x, double y) {
    volatile uint64_t result = 0;
    
    /* Mix different synthesis triggers */
    result += test_builtin_synthesis();
    
    __int128 int_result = test_libcall_synthesis(a, b);
    result += (uint64_t)(int_result >> 64) + (uint64_t)int_result;
    
    double float_result = test_softfloat_synthesis(x, y);
    result += (uint64_t)float_result;
    
    int tm_val = 42;
    result += test_tm_synthesis(&tm_val);
    
    result += test_cpu_dispatch_synthesis();
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize with some values */
    __int128 int128_a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 int128_b = ((__int128)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    double x = 3.141592653589793;
    double y = 2.718281828459045;
    
    /* Test individual synthesis paths */
    accumulator += test_builtin_synthesis();
    
    __int128 int_result = test_libcall_synthesis(int128_a, int128_b);
    accumulator += (uint64_t)(int_result >> 64) + (uint64_t)int_result;
    
    double float_result = test_softfloat_synthesis(x, y);
    accumulator += (uint64_t)float_result;
    
    /* Test OpenMP synthesis if available */
    #ifdef _OPENMP
    accumulator += test_omp_synthesis(100);
    #endif
    
    int tm_val = 100;
    accumulator += test_tm_synthesis(&tm_val);
    
    accumulator += test_cpu_dispatch_synthesis();
    
    /* Combined test */
    accumulator += test_combined_synthesis(int128_a, int128_b, x, y);
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
