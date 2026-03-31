/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S */
/* For ARM: gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fopenmp */
/* For BPF: gcc -O2 -target bpf -fopenmp */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* 1. Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins - will force synthesis on x86 targets */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
    /* Memory barrier built-in */
    __builtin_ia32_mfence();
    #endif
    
    /* ARM specific built-ins */
    #ifdef __arm__
    result = __builtin_arm_mrc(15, 0, 0, 1, 0);
    #endif
    
    /* BPF specific built-ins */
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_val;
}

/* 2. Library call synthesis for unsupported operations */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result;
    
    /* 128-bit operations that often require libcalls */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % b;          /* 128-bit modulo */
    
    /* Double precision on soft-float target */
    volatile double d1 = 3.141592653589793;
    volatile double d2 = 2.718281828459045;
    volatile double d3 = d1 * d2;  /* May require soft-float libcall */
    
    result += (__int128)d3;
    
    /* Complex division - often requires libcall */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double c3 = c1 / c2;
    
    result += (__int128)__real__(c3);
    
    return result;
}

/* 3. Advanced extensions requiring runtime support */
NOOPT int test_extension_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory */
    __transaction_atomic {
        result = x * 2;
    }
    
    /* CPU feature detection */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result += 100;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 200;
    }
    #endif
    
    /* __builtin_constant_p with runtime fallback */
    int y = __builtin_constant_p(x) ? x * 3 : x * 4;
    result += y;
    
    return result;
}

/* 4. OpenMP synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int i;
    
    #pragma omp target map(tofrom: result)
    {
        #pragma omp parallel for reduction(+:result)
        for (i = 0; i < n; i++) {
            result += i * i;
        }
    }
    
    return result;
}

/* 5. Combined synthesis triggers */
NOOPT __int128 test_combined_synthesis(__int128 a, __int128 b, int n) {
    volatile __int128 result = 0;
    
    /* Atomic 128-bit operation */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, a, __ATOMIC_RELEASE);
    
    /* Mix with target built-in */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    #endif
    
    /* Complex operation */
    volatile _Complex float cf = (1.0f + 2.0fi) / (3.0f + 4.0fi);
    result += (__int128)__real__(cf) * 1000;
    
    /* OpenMP region with computation */
    #pragma omp target teams distribute parallel for reduction(+:result)
    for (int i = 0; i < n; i++) {
        result += i;
    }
    
    return result + atomic_val;
}

int main(int argc, char **argv) {
    volatile __int128 accumulator = 0;
    
    /* Force synthesis by calling all test functions */
    accumulator += test_builtin_synthesis();
    
    __int128 a = ((__int128)argc << 32) | argc;
    __int128 b = ((__int128)argc << 16) | argc;
    accumulator += test_libcall_synthesis(a, b);
    
    accumulator += test_extension_synthesis(argc);
    accumulator += test_omp_synthesis(argc);
    accumulator += test_combined_synthesis(a, b, argc);
    
    /* Print checksum to prevent optimization */
    uint64_t low = (uint64_t)accumulator;
    uint64_t high = (uint64_t)(accumulator >> 64);
    printf("Checksum: 0x%016lx%016lx\n", high, low);
    
    return (int)(low + high);
}
