/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOINLINE uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific builtins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    result += __builtin_cpu_supports("avx2");
    result += __builtin_cpu_supports("sse4.2");
    #endif
    
    /* ARM-specific builtins */
    #ifdef __arm__
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* BPF builtins */
    #ifdef __bpf__
    result += __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOINLINE unsigned __int128 test_128bit_synthesis(unsigned __int128 a, 
                                                 unsigned __int128 b) {
    /* These operations often require libcalls on 64-bit targets */
    volatile unsigned __int128 mul = a * b;
    volatile unsigned __int128 div = mul / (b + 1);
    volatile unsigned __int128 mod = div % (a + 1);
    
    return mul + div + mod;
}

/* Test 3: Atomic operations with uncommon sizes */
NOINLINE uint64_t test_atomic_synthesis(void) {
    volatile __int128 atomic_var = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    
    /* 128-bit atomic operations require libcalls */
    __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    __atomic_store_n(&atomic_var, desired, __ATOMIC_RELEASE);
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    
    return (uint64_t)atomic_var;
}

/* Test 4: Complex arithmetic requiring libcalls */
NOINLINE _Complex double test_complex_synthesis(_Complex double a,
                                                _Complex double b) {
    /* Complex division often requires libcalls */
    volatile _Complex double div = a / b;
    volatile _Complex double mul = a * b;
    
    return div + mul;
}

/* Test 5: OpenMP target region triggering runtime synthesis */
NOINLINE int test_omp_synthesis(int n) {
    volatile int result = 0;
    
    #pragma omp target map(tofrom: result)
    {
        #pragma omp parallel for reduction(+:result)
        for (int i = 0; i < n; i++) {
            /* Mix in some builtins inside OpenMP region */
            #ifdef __x86_64__
            result += __builtin_popcount(i);
            #endif
            result += i * i;
        }
    }
    
    return result;
}

/* Test 6: Transactional memory extensions */
NOINLINE int test_transactional_synthesis(int *ptr) {
    int result = 0;
    
    #ifdef __GNUC__
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    #endif
    
    return result;
}

/* Test 7: Soft-float double operations (for targets without hardware FPU) */
NOINLINE double test_softfloat_synthesis(double a, double b) {
    /* These may trigger libcalls on soft-float targets */
    volatile double div = a / b;
    volatile double sin_result = __builtin_sin(a);
    volatile double exp_result = __builtin_exp(b);
    
    return div + sin_result + exp_result;
}

/* Test 8: Variable-length array with complex operations */
NOINLINE int test_vla_synthesis(int n) {
    volatile int sum = 0;
    
    /* VLA with operations that may require libcalls */
    double vla[n];
    _Complex double cvla[n];
    
    for (int i = 0; i < n && i < 100; i++) {
        vla[i] = i * 1.5;
        cvla[i] = vla[i] + vla[i] * 1.0i;
        sum += (int)(cvla[i] * cvla[i]);
    }
    
    return sum;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit arithmetic */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x10000000000000001ULL;
    accumulator += (uint64_t)test_128bit_synthesis(a, b);
    
    /* Test 3: Atomic operations */
    accumulator += test_atomic_synthesis();
    
    /* Test 4: Complex arithmetic */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 - 2.0i;
    accumulator += (uint64_t)__real__(test_complex_synthesis(c1, c2));
    
    /* Test 5: OpenMP synthesis */
    accumulator += test_omp_synthesis(100);
    
    /* Test 6: Transactional memory */
    int tm_var = 42;
    accumulator += test_transactional_synthesis(&tm_var);
    
    /* Test 7: Soft-float operations */
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Test 8: VLA with complex ops */
    accumulator += test_vla_synthesis(50);
    
    /* Print result to prevent optimization */
    printf("Accumulator: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
