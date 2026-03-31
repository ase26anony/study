/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, used))

/* Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    result ^= __builtin_ia32_crc32di(result, result);
    #endif
    
    /* Atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx512f")) {
        result |= 0x1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 0x2;
    }
    
    return result;
}

/* Library call synthesis for unsupported operations */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations that may require libcalls */
    result = a * b;           /* 128-bit multiplication */
    result += a / (b | 1);    /* 128-bit division */
    result += a % (b | 1);    /* 128-bit modulo */
    
    /* Complex number division (often requires libcalls) */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double cdiv = c1 / c2;
    
    /* Cast to avoid unused warning */
    result += (__int128)(__real__ cdiv + __imag__ cdiv);
    
    return result;
}

/* OpenMP synthesis for runtime functions */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int i;
    
    #pragma omp target map(tofrom: result) if(n > 1000)
    {
        #pragma omp parallel for reduction(+:result)
        for (i = 0; i < n % 100; i++) {
            result += i * i;
        }
    }
    
    /* OpenACC pragma for additional synthesis */
    #pragma acc parallel copy(result)
    {
        result += 1;
    }
    
    return result;
}

/* Transactional memory synthesis */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory operations */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    /* __builtin_constant_p with runtime fallback */
    if (__builtin_constant_p(*ptr)) {
        result += 100;
    } else {
        result += 200;
    }
    
    return result;
}

/* Soft-float synthesis (for targets without hardware double) */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Double precision operations that may require soft-float libcalls */
    result = a * b;          /* Multiplication */
    result += a / (b + 1.0); /* Division */
    result += __builtin_sqrt(a * a + b * b); /* Square root */
    result += __builtin_sin(a) * __builtin_cos(b); /* Transcendental */
    
    return result;
}

/* ARM-specific built-in synthesis */
NOOPT uint32_t test_arm_synthesis(void) {
    volatile uint32_t result = 0;
    
    #ifdef __arm__
    /* ARM system register access */
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    result ^= __builtin_arm_mrc(15, 0, 0, 1, 0);
    
    /* ARM DSP extensions */
    result += __builtin_arm_usad8(result, result ^ 0xFF);
    #endif
    
    #ifdef __aarch64__
    /* AArch64 system registers */
    result = __builtin_aarch64_get_fpcr();
    result ^= __builtin_aarch64_get_fpsr();
    #endif
    
    return result;
}

/* BPF-specific built-in synthesis */
NOOPT uint64_t test_bpf_synthesis(void) {
    volatile uint64_t result = 0;
    
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    result ^= __builtin_bpf_get_smp_processor_id();
    #endif
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    volatile __int128 big_val = 0;
    volatile double fp_val = 0.0;
    volatile int tm_var = 0;
    
    /* Seed values from command line to prevent constant folding */
    int seed = argc > 1 ? argv[1][0] : 42;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit arithmetic */
    big_val = ((__int128)seed << 64) | seed;
    accumulator += (uint64_t)test_libcall_synthesis(big_val, big_val + 1);
    
    /* Test 3: OpenMP synthesis */
    accumulator += test_omp_synthesis(seed);
    
    /* Test 4: Transactional memory synthesis */
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test 5: Soft-float synthesis */
    fp_val = seed * 1.234567;
    accumulator += (uint64_t)test_softfloat_synthesis(fp_val, fp_val * 2.0);
    
    /* Test 6: ARM-specific synthesis */
    accumulator += test_arm_synthesis();
    
    /* Test 7: BPF-specific synthesis */
    accumulator += test_bpf_synthesis();
    
    /* Additional atomic operations with uncommon memory orders */
    volatile __int128 atomic_128 = 0;
    __atomic_store_n(&atomic_128, big_val, __ATOMIC_RELAXED);
    accumulator += (uint64_t)__atomic_load_n(&atomic_128, __ATOMIC_ACQUIRE);
    
    /* Print result to ensure all operations are observable */
    printf("Synthesis test result: 0x%016llx\n", (unsigned long long)accumulator);
    printf("TM variable: %d\n", tm_var);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
