/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp((unsigned int*)&result);
    #endif
    
    /* ARM-specific if compiled for ARM */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 1, 0);
    #endif
    
    /* BPF-specific */
    #ifdef __bpf__
    result += __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT unsigned __int128 test_128bit_synthesis(unsigned __int128 a, unsigned __int128 b) {
    /* These operations often require libcalls on 64-bit targets */
    volatile unsigned __int128 mul = a * b;
    volatile unsigned __int128 div = a / (b + 1);
    volatile unsigned __int128 mod = a % (b + 2);
    
    return mul + div + mod;
}

/* Test 3: Atomic operations with uncommon sizes */
NOOPT uint64_t test_atomic_synthesis(uint64_t* ptr, uint64_t val) {
    volatile uint64_t result = 0;
    
    /* __atomic built-ins may synthesize helper functions */
    __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
    result = __atomic_compare_exchange_n(ptr, &val, val + 1, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Try with 128-bit atomics if supported */
    #ifdef __SIZEOF_INT128__
    unsigned __int128* ptr128 = (unsigned __int128*)ptr;
    unsigned __int128 val128 = val;
    __atomic_load_n(ptr128, __ATOMIC_RELAXED);
    #endif
    
    return result;
}

/* Test 4: Complex arithmetic forcing libcalls */
NOOPT _Complex double test_complex_synthesis(_Complex double a, _Complex double b) {
    /* Complex division often requires libcalls */
    volatile _Complex double div = a / b;
    volatile _Complex double sqrt = __builtin_complex_sqrt(a);
    
    return div + sqrt;
}

/* Test 5: OpenMP target region synthesis */
NOOPT int test_omp_synthesis(int* data, int n) {
    volatile int result = 0;
    
    #pragma omp target map(tofrom: data[0:n]) map(to: n)
    {
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < n; i++) {
            data[i] = i * 2;
            result += data[i];
        }
    }
    
    return result;
}

/* Test 6: CPU feature detection synthesis */
NOOPT int test_cpu_synthesis(void) {
    volatile int features = 0;
    
    #ifdef __x86_64__
    /* These may synthesize CPU dispatch functions */
    if (__builtin_cpu_supports("avx2")) features |= 1;
    if (__builtin_cpu_supports("avx512f")) features |= 2;
    if (__builtin_cpu_supports("sse4.2")) features |= 4;
    __builtin_cpu_init();
    #endif
    
    return features;
}

/* Test 7: Transactional memory synthesis */
NOOPT int test_tm_synthesis(int* counter) {
    volatile int result = 0;
    
    #ifdef __TM_synthesis__
    __transaction_atomic {
        (*counter)++;
        result = *counter;
    }
    #endif
    
    return result;
}

/* Test 8: Soft-float double operations (for ARM without hardware FPU) */
NOOPT double test_softfloat_synthesis(double a, double b) {
    /* Force soft-float libcalls */
    volatile double div = a / b;
    volatile double sin = __builtin_sin(a);
    volatile double exp = __builtin_exp(b);
    
    return div + sin + exp;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char** argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize test data */
    uint64_t atomic_var = 12345;
    int omp_data[100];
    unsigned __int128 big_num = ((unsigned __int128)1 << 64) + 123;
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_128bit_synthesis(big_num, big_num + 1);
    accumulator += test_atomic_synthesis(&atomic_var, 67890);
    accumulator += __real__(test_complex_synthesis(c1, c2));
    accumulator += test_omp_synthesis(omp_data, 100);
    accumulator += test_cpu_synthesis();
    
    int counter = 0;
    accumulator += test_tm_synthesis(&counter);
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Use argc to make results unpredictable */
    accumulator *= argc;
    
    /* Print result to prevent optimization */
    printf("Synthesis test accumulator: %llu\n", (unsigned long long)accumulator);
    
    return (accumulator > 0) ? 0 : 1;
}
