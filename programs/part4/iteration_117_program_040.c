/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate synthesis */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-ins requiring synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    result += __builtin_ia32_rdpid();
    #endif
    
    /* ARM-specific if compiled for ARM */
    #ifdef __arm__
    uint32_t coproc = 15, opc1 = 0, crn = 0, crm = 0, opc2 = 0;
    result += __builtin_arm_mrc(coproc, opc1, crn, crm, opc2);
    #endif
    
    /* Atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 atomic_expected = 0;
    __int128 atomic_desired = 1;
    __atomic_compare_exchange(&atomic_val, &atomic_expected, &atomic_desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Test 2: Operations requiring libcall synthesis */
NOOPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL) << 64;
    unsigned __int128 b = 0xFEDCBA9876543210ULL;
    unsigned __int128 c = a * b;  /* May require __multi3 libcall */
    unsigned __int128 d = a / b;  /* May require __udivti3 libcall */
    
    result = (uint64_t)(c >> 64) + (uint64_t)d;
    
    /* Complex division requiring libcall */
    __complex__ double z1 = 3.0 + 4.0i;
    __complex__ double z2 = 1.0 + 2.0i;
    __complex__ double z3 = z1 / z2;
    result += (uint64_t)__real__ z3 + (uint64_t)__imag__ z3;
    
    return result;
}

/* Test 3: OpenMP requiring runtime function synthesis */
NOOPT uint64_t test_omp_synthesis(int size) {
    volatile uint64_t result = 0;
    int *array = (int*)malloc(size * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: array[0:size])
    {
        #pragma omp parallel for
        for (int i = 0; i < size; i++) {
            array[i] *= 2;
        }
    }
    
    /* Collect results */
    for (int i = 0; i < size; i++) {
        result += array[i];
    }
    
    free(array);
    return result;
}

/* Test 4: Transactional memory and CPU feature detection */
NOOPT uint64_t test_advanced_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Transactional memory (if supported) */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result = 42;
    }
    #endif
    
    /* CPU feature detection - may synthesize resolver functions */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        result += 100;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 200;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = rand();
    if (__builtin_constant_p(x)) {
        result += 1;
    } else {
        result += 2;  /* Runtime path */
    }
    
    return result;
}

/* Test 5: Combined synthesis triggers */
NOOPT uint64_t test_combined_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Mix 128-bit atomics with built-ins */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, 0x123456789ABCDEF0ULL, __ATOMIC_RELEASE);
    
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    #endif
    
    /* Complex operation inside */
    __complex__ float cf = 1.0f + 2.0fi;
    cf = cf / (2.0f + 3.0fi);
    result += (uint64_t)__real__ cf;
    
    return result + (uint64_t)atomic_val;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Seed random for variability */
    srand(42);
    
    /* Execute all synthesis tests */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_omp_synthesis(argc > 1 ? atoi(argv[1]) : 100);
    accumulator += test_advanced_synthesis();
    accumulator += test_combined_synthesis();
    
    /* Make results observable */
    printf("Synthesis test accumulator: %llu\n", 
           (unsigned long long)accumulator);
    
    return (int)(accumulator & 0xFF);
}
