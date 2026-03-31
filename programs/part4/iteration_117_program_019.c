/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S */
/* For ARM: gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fopenmp */

#include <stdint.h>
#include <stdio.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* 1. Target-specific built-in synthesis */
NOOPT uint64_t test_x86_builtins(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* MMX/SSE built-ins - some may require synthesized declarations */
    result += __builtin_ia32_comieq(__builtin_ia32_addps(
        __builtin_ia32_loadups((const float*)&result),
        __builtin_ia32_loadups((const float*)&result)
    ), 0);
    #endif
    
    return result;
}

/* 2. 128-bit arithmetic forcing libcall synthesis */
NOOPT __int128 test_128bit_ops(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* Operations that often require libcalls on 64-bit targets */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % (b + 1);    /* 128-bit modulo */
    
    return result;
}

/* 3. Atomic operations with uncommon sizes */
NOOPT uint64_t test_atomic_synthesis(void) {
    struct UnusualAtomic {
        char data[7];  /* Unusual size for atomic operations */
    } volatile atom;
    
    struct UnusualAtomic old = {0}, new = {1};
    uint64_t result = 0;
    
    /* __atomic_compare_exchange with unusual size may require helper */
    __atomic_compare_exchange(&atom, &old, &new, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
    
    /* __atomic_load with unusual size */
    __atomic_load(&atom, &old, __ATOMIC_ACQUIRE);
    
    result = *(uint64_t*)&atom;
    return result;
}

/* 4. Double precision on soft-float target */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex floating operations that may require libcalls */
    result = a * b;
    result += a / b;
    result += __builtin_sqrt(a * a + b * b);
    
    /* Complex number division - often requires runtime support */
    _Complex double c1 = a + b * 1.0i;
    _Complex double c2 = b + a * 1.0i;
    _Complex double cdiv = c1 / c2;
    
    result += __real__(cdiv) + __imag__(cdiv);
    return result;
}

/* 5. OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100] = {0};
    
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] = i * n;
        }
        
        /* Use some built-ins inside target region */
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFF;
        #endif
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* 6. CPU feature detection requiring runtime resolution */
NOOPT int test_cpu_dispatch(void) {
    volatile int features = 0;
    
    #ifdef __x86_64__
    /* These may trigger synthesis of CPU dispatch functions */
    if (__builtin_cpu_supports("avx512f")) features |= 1;
    if (__builtin_cpu_supports("avx2")) features |= 2;
    if (__builtin_cpu_supports("sse4.2")) features |= 4;
    
    /* Initialize CPU features (may synthesize init function) */
    __builtin_cpu_init();
    #endif
    
    return features;
}

/* 7. Transactional memory extensions */
NOOPT int test_transactional_memory(int x) {
    volatile int result = 0;
    
    /* Transactional memory may require runtime support synthesis */
    __transaction_atomic {
        result = x * 2;
        result += x / 2;
    }
    
    return result;
}

/* 8. BPF built-ins (if targeting BPF) */
NOOPT unsigned long test_bpf_builtins(void) {
    volatile unsigned long result = 0;
    
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    result ^= __builtin_bpf_get_smp_processor_id();
    #endif
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Seed with argument to prevent constant folding */
    int seed = argc > 1 ? argv[1][0] : 42;
    
    /* Test 1: x86 built-ins */
    accumulator += test_x86_builtins();
    
    /* Test 2: 128-bit operations */
    __int128 a128 = ((__int128)seed << 64) | seed;
    __int128 b128 = ((__int128)seed << 32) | seed;
    __int128 r128 = test_128bit_ops(a128, b128);
    accumulator += (uint64_t)r128 + (uint64_t)(r128 >> 64);
    
    /* Test 3: Atomic synthesis */
    accumulator += test_atomic_synthesis();
    
    /* Test 4: Soft-float/complex operations */
    accumulator += (uint64_t)test_softfloat_synthesis(seed * 1.0, seed * 0.5);
    
    /* Test 5: OpenMP synthesis */
    accumulator += test_omp_synthesis(seed);
    
    /* Test 6: CPU dispatch */
    accumulator += test_cpu_dispatch();
    
    /* Test 7: Transactional memory */
    accumulator += test_transactional_memory(seed);
    
    /* Test 8: BPF built-ins */
    accumulator += test_bpf_builtins();
    
    /* Print result to ensure observability */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
