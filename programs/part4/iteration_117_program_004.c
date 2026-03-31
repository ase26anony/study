/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    result ^= __builtin_ia32_rdtscp(&result);   /* RDTSCP with parameter */
    
    /* MMX/SSE built-ins that might need declaration synthesis */
    __m128i a = _mm_setzero_si128();
    __m128i b = _mm_set1_epi32(0x12345678);
    __m128i c = _mm_add_epi32(a, b);
    result += c[0];
    
    /* AVX built-in with uncommon operation */
    __m256i v = _mm256_setzero_si256();
    v = _mm256_xor_si256(v, v);
    result += v[0];
    #endif
    
    /* ARM-style built-in if compiled for ARM */
    #ifdef __arm__
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    /* BPF built-in if compiled for BPF */
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    #endif
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT unsigned __int128 test_libcall_synthesis(unsigned __int128 a, unsigned __int128 b) {
    volatile unsigned __int128 result = 0;
    
    /* 128-bit operations that often require library calls */
    result = a * b;                     /* 128-bit multiplication */
    result += a / (b + 1);              /* 128-bit division */
    result = result << 64;              /* 128-bit shift */
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __atomic_load(&atomic_val, &result, __ATOMIC_SEQ_CST);
    __atomic_compare_exchange(&atomic_val, &result, &a, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Test 3: Soft-float double operations */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex operations that may require libcalls */
    result = a / b;                     /* Division might use soft-float */
    result = __builtin_sqrt(result);    /* Square root */
    
    /* Complex number division */
    _Complex double c1 = a + b * _Complex_I;
    _Complex double c2 = b + a * _Complex_I;
    _Complex double c3 = c1 / c2;
    result += __real__(c3) + __imag__(c3);
    
    return result;
}

/* Test 4: Transactional memory extensions */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory - may require runtime support */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    /* CPU feature detection built-ins */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result |= 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 2;
    }
    #endif
    
    return result;
}

/* Test 5: OpenMP target region synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - creates outlined functions */
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] += n;
        }
        
        /* Mix with built-ins inside target region */
        #ifdef __x86_64__
        unsigned long long tsc = __builtin_ia32_rdtsc();
        arr[0] ^= (tsc & 0xFFFFFFFF);
        #endif
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 6: Uncommon atomic operations */
NOOPT long test_atomic_synthesis(long *ptr, int size) {
    volatile long result = 0;
    
    /* Atomic operations with uncommon sizes/orders */
    switch (size) {
        case 1:
            __atomic_load_n((char *)ptr, __ATOMIC_ACQUIRE);
            break;
        case 2:
            __atomic_store_n((short *)ptr, result, __ATOMIC_RELEASE);
            break;
        case 4:
            result = __atomic_exchange_n((int *)ptr, result, __ATOMIC_ACQ_REL);
            break;
        case 8:
            result = __atomic_fetch_add((long *)ptr, 1, __ATOMIC_SEQ_CST);
            break;
        case 16:
            /* 128-bit atomic - may require helper */
            __int128 val128 = 0;
            __atomic_compare_exchange((__int128 *)ptr, &val128, &val128, 
                                      0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
            break;
    }
    
    /* Memory barrier synthesis */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    return result;
}

/* Test 7: __builtin_constant_p with runtime fallback */
NOOPT int test_constant_p_synthesis(int x) {
    volatile int result = 0;
    
    /* This may create both compile-time and runtime paths */
    if (__builtin_constant_p(x)) {
        result = x * 2;
    } else {
        /* Force runtime computation with libcall */
        result = x / 3;  /* Division might use libcall */
    }
    
    /* Another constant_p test with built-in */
    if (__builtin_constant_p(result)) {
        #ifdef __x86_64__
        result ^= __builtin_ia32_rdtsc() & 0xFF;
        #endif
    }
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile unsigned long long accumulator = 0;
    
    /* Force external input to prevent optimization */
    int seed = argc > 1 ? argv[1][0] : 42;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    unsigned __int128 a = ((unsigned __int128)seed << 64) | seed;
    unsigned __int128 b = ((unsigned __int128)seed << 32) | seed;
    unsigned __int128 res128 = test_libcall_synthesis(a, b);
    accumulator += (unsigned long long)res128;
    accumulator += (unsigned long long)(res128 >> 64);
    
    /* Test 3: Soft-float synthesis */
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    double dres = test_softfloat_synthesis(d1, d2);
    accumulator += (unsigned long long)dres;
    
    /* Test 4: TM synthesis */
    int tm_var = seed;
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test 5: OpenMP synthesis */
    accumulator += test_omp_synthesis(seed);
    
    /* Test 6: Atomic synthesis */
    long atomic_var = seed;
    accumulator += test_atomic_synthesis(&atomic_var, 8);
    
    /* Test 7: Constant_p synthesis */
    accumulator += test_constant_p_synthesis(seed);
    
    /* Print result to ensure all operations are observable */
    printf("Result: %llu\n", accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
