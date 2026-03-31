/* test_synthesis.c - Program to trigger built-in function synthesis in GCC */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins - will trigger synthesis on x86 targets */
#ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
#endif
    
    /* ARM specific built-ins */
#ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
#endif
    
    /* BPF specific built-ins */
#ifdef __bpf__
    result += __builtin_bpf_packet_data();
#endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* These operations often require libcalls on 64-bit targets */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % b;          /* 128-bit modulo */
    
    /* Complex division - often requires libcall */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double cdiv = c1 / c2;
    result += (__int128)(__real__ cdiv * 1000);
    
    return result;
}

/* Test 3: Double precision on soft-float target */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* These may trigger soft-float libcalls */
    result = a * b;
    result += a / b;
    result += __builtin_sqrt(a);
    result += __builtin_sin(b);
    result += __builtin_pow(a, b);
    
    return result;
}

/* Test 4: Transactional memory extensions */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize runtime functions */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* Test 5: CPU feature detection */
NOOPT uint64_t test_cpu_synthesis(void) {
    volatile uint64_t result = 0;
    
#ifdef __x86_64__
    /* CPU feature detection - may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 2;
    }
    if (__builtin_cpu_supports("avx512f")) {
        result |= 4;
    }
#endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = result;
    if (__builtin_constant_p(x)) {
        result += 100;
    } else {
        result += 200;  /* Runtime path */
    }
    
    return result;
}

/* Test 6: OpenMP target region */
#ifdef _OPENMP
#include <omp.h>
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int data[100];
    
    #pragma omp target map(tofrom: data[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            data[i] = i * n;
        }
        
        /* Use some built-ins inside OpenMP region */
        for (int i = 0; i < 100; i++) {
            data[i] += __builtin_popcount(i);
        }
    }
    
    for (int i = 0; i < 100; i++) {
        result += data[i];
    }
    
    return result;
}
#endif

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit values */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 libcall_result = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)libcall_result + (uint64_t)(libcall_result >> 64);
    
    /* Test 3: Soft-float synthesis */
    double softfloat_result = test_softfloat_synthesis(3.14159, 2.71828);
    accumulator += (uint64_t)softfloat_result;
    
    /* Test 4: Transactional memory */
    int tm_var = 42;
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test 5: CPU feature synthesis */
    accumulator += test_cpu_synthesis();
    
    /* Test 6: OpenMP synthesis */
#ifdef _OPENMP
    accumulator += test_omp_synthesis(argc);
#endif
    
    /* Use atomic operations on accumulator */
    __atomic_add_fetch(&accumulator, 1, __ATOMIC_SEQ_CST);
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator % 256);
}
