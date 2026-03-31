/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S */
/* For ARM: gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fopenmp */

#include <stdio.h>
#include <stdint.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific builtins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    /* Memory barrier builtin */
    __builtin_ia32_mfence();
    #endif
    
    /* ARM-specific builtins */
    #ifdef __arm__
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    __builtin_arm_dmb(0xF); /* Full system DMB */
    #endif
    
    /* BPF builtins */
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    #endif
    
    /* Generic atomic builtins with uncommon sizes */
    __int128 atomic_val = 0;
    __atomic_load_n(&atomic_val, __ATOMIC_ACQUIRE);
    
    return result;
}

/* Test 2: Library call synthesis for unsupported operations */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations that may require libcalls */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result = a % b;           /* 128-bit modulo */
    
    /* Complex number division (often requires libcalls) */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double cdiv = c1 / c2;
    
    /* Double precision on soft-float target */
    volatile double d1 = 3.141592653589793;
    volatile double d2 = 2.718281828459045;
    volatile double ddiv = d1 / d2;
    volatile double dmul = d1 * d2;
    
    /* Use result to prevent optimization */
    return result + (__int128)((int64_t)cdiv) + (__int128)ddiv + (__int128)dmul;
}

/* Test 3: OpenMP runtime function synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * n;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] += n;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    /* OpenMP atomic with uncommon memory order */
    #pragma omp atomic seq_cst
    result += 1;
    
    return result;
}

/* Test 4: Transactional memory and CPU feature detection */
NOOPT int test_tm_synthesis(void) {
    volatile int result = 0;
    
    /* Transactional memory (requires runtime support) */
    #ifdef __TM__
    __transaction_atomic {
        result = 42;
    }
    #endif
    
    /* CPU feature detection (may synthesize resolver functions) */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 2;
    }
    __builtin_cpu_init();
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int x = result;
    if (__builtin_constant_p(x)) {
        result += 10;
    } else {
        result += 20;  /* Runtime path */
    }
    
    return result;
}

/* Test 5: Combined synthesis triggers */
NOOPT __int128 test_combined_synthesis(__int128 a, __int128 b, int n) {
    volatile __int128 result = 0;
    
    /* Atomic operation on 128-bit value */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = a + b;
    
    /* 128-bit atomic compare-exchange (likely requires libcall) */
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* Use target builtin inside computation */
    #ifdef __x86_64__
    uint64_t tsc = __builtin_ia32_rdtsc();
    result = (__int128)tsc * atomic_val;
    #endif
    
    /* Complex division in the mix */
    volatile _Complex float cf = (1.0f + 2.0fi) / (3.0f + 4.0fi);
    result += (__int128)((int)__real__ cf);
    
    /* OpenMP with atomic */
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < n; i++) {
        result += i;
    }
    
    return result;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    accumulator += (uint64_t)test_libcall_synthesis(a, b);
    accumulator += (uint64_t)(test_libcall_synthesis(a, b) >> 64);
    
    /* Test 3: OpenMP synthesis */
    accumulator += test_omp_synthesis(42);
    
    /* Test 4: TM and CPU feature synthesis */
    accumulator += test_tm_synthesis();
    
    /* Test 5: Combined synthesis */
    accumulator += (uint64_t)test_combined_synthesis(a, b, 100);
    accumulator += (uint64_t)(test_combined_synthesis(a, b, 100) >> 64);
    
    /* Ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
