/* Compile with: gcc -O2 -fopenmp -march=x86-64 -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-ins requiring synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may need synthesized declarations */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    /* Memory barrier built-in */
    __builtin_ia32_mfence();
    /* SIMD built-in - may require synthesis on some optimization levels */
    result += __builtin_ia32_crc32qi(result, 0xAB);
    #endif
    
    /* ARM-specific if compiled for ARM */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_load_n(&atomic_val, __ATOMIC_ACQUIRE);
    
    return result;
}

/* Test 2: Operations requiring libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit multiplication - often requires libcall */
    result = a * b;
    
    /* 128-bit division - almost always requires libcall */
    if (b != 0) {
        result = a / b;
    }
    
    /* Complex division - may require libcall */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double cdiv = c1 / c2;
    
    /* Use the result to prevent optimization */
    result += (__int128)(__real__ cdiv + __imag__ cdiv);
    
    return result;
}

/* Test 3: Double-precision on soft-float target */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Multiple double operations that may require soft-float libcalls */
    result = a * b;
    result += a / b;
    result = __builtin_sqrt(result);
    result = __builtin_sin(result);
    result = __builtin_cos(result);
    
    return result;
}

/* Test 4: OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] *= 2;
        }
        
        /* Use a built-in inside OpenMP region */
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFF;
        #endif
    }
    
    /* Sum results */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 5: Transactional memory and CPU dispatch */
NOOPT int test_tm_cpu_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize TM runtime functions */
    #ifdef __TM__
    __transaction_atomic {
        result = x * 2;
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
    __builtin_cpu_init();
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    if (__builtin_constant_p(x)) {
        result += 50;
    } else {
        /* Force runtime path */
        volatile int* ptr = &x;
        result += *ptr * 3;
    }
    
    return result;
}

/* Test 6: Atomic operations on unusual sizes */
NOOPT long double test_atomic_synthesis(long double* ptr) {
    volatile long double result = 0.0L;
    
    /* Atomic operations on long double (often 80 or 128 bits) */
    __atomic_load_n(ptr, __ATOMIC_RELAXED);
    __atomic_store_n(ptr, 3.14159L, __ATOMIC_RELAXED);
    
    /* Compare exchange on large type */
    long double expected = 1.0L;
    long double desired = 2.0L;
    __atomic_compare_exchange_n(ptr, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    result = *ptr;
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char** argv) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit values */
    __int128 large1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 large2 = 0x1000000000000000ULL;
    accumulator += (uint64_t)test_libcall_synthesis(large1, large2);
    
    /* Test 3: Soft-float synthesis */
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(argc > 1 ? atoi(argv[1]) : 42);
    
    /* Test 5: TM and CPU dispatch synthesis */
    accumulator += test_tm_cpu_synthesis(argc);
    
    /* Test 6: Atomic synthesis */
    long double ld_val = 1.5L;
    accumulator += (uint64_t)test_atomic_synthesis(&ld_val);
    
    /* Print result to ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
