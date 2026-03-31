/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-ins forcing synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __sync_synchronize();
    
    /* Atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, 1, __ATOMIC_SEQ_CST);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    /* Operations that typically require libcalls on many targets */
    __int128 mul = a * b;          /* 128-bit multiplication */
    __int128 div = a / (b + 1);    /* 128-bit division */
    __int128 mod = a % (b + 2);    /* 128-bit modulus */
    
    return mul + div + mod;
}

/* Test 3: Floating-point operations requiring soft-float */
NOOPT double test_softfloat_synthesis(double a, double b) {
    /* Complex operations that may require libcalls */
    _Complex double c1 = a + b * I;
    _Complex double c2 = b - a * I;
    _Complex double cdiv = c1 / c2;  /* Complex division */
    
    /* Transcendental functions */
    double trig = __builtin_sin(a) * __builtin_cos(b);
    
    return __real__ cdiv + __imag__ cdiv + trig;
}

/* Test 4: OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    int sum = 0;
    
    #pragma omp target map(tofrom: sum) if(0)  /* if(0) ensures host fallback */
    {
        /* Use some built-ins inside target region */
        for (int i = 0; i < n; i++) {
            sum += i;
        }
        __sync_synchronize();
    }
    
    return sum;
}

/* Test 5: Transactional memory extensions */
NOOPT int test_tm_synthesis(int *ptr) {
    int result = 0;
    
    /* Transactional memory - may require runtime synthesis */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* Test 6: Uncommon atomic operations */
NOOPT int test_atomic_synthesis(void) {
    struct Uncommon {
        char a;
        int b;
        char c;
    } __attribute__((packed));
    
    volatile struct Uncommon data = {0};
    struct Uncommon expected = {1, 2, 3};
    struct Uncommon desired = {4, 5, 6};
    
    /* Atomic compare-exchange on uncommon size */
    __atomic_compare_exchange(&data, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return data.b;
}

int main(void) {
    volatile uint64_t accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 a = ((__int128)0x12345678 << 64) | 0x9ABCDEF0;
    __int128 b = ((__int128)0xFEDCBA98 << 64) | 0x76543210;
    __int128 res128 = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)res128 + (uint64_t)(res128 >> 64);
    
    /* Test 3: Soft-float synthesis */
    double fp_res = test_softfloat_synthesis(3.14159, 2.71828);
    accumulator += (uint64_t)fp_res;
    
    /* Test 4: OpenMP synthesis */
    int omp_res = test_omp_synthesis(100);
    accumulator += omp_res;
    
    /* Test 5: Transactional memory */
    int tm_var = 42;
    int tm_res = test_tm_synthesis(&tm_var);
    accumulator += tm_res + tm_var;
    
    /* Test 6: Atomic synthesis */
    int atomic_res = test_atomic_synthesis();
    accumulator += atomic_res;
    
    /* Additional direct built-in usage in main */
    #ifdef __x86_64__
    accumulator += __builtin_ia32_rdtscp(&(unsigned){0});
    #endif
    
    /* Use __builtin_constant_p with runtime fallback */
    int dynamic_value = accumulator & 0xFF;
    if (!__builtin_constant_p(dynamic_value)) {
        accumulator += __builtin_popcountll(accumulator);
    }
    
    printf("Result: %llu\n", (unsigned long long)accumulator);
    return (int)(accumulator & 0x7FFFFFFF);
}
