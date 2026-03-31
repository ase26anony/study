/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and inlining */
#define NOINLINE_NOIPA __attribute__((noinline, noipa, noclone))

/* Target-specific built-in synthesis */
NOINLINE_NOIPA
uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();          /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    /* CPU feature detection - may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result += __builtin_ia32_crc32di(result, 0x12345678);
    }
    
    /* Uncommon atomic operation */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, (__int128)result, __ATOMIC_RELAXED);
    
    return result;
}

/* Library call synthesis for unsupported operations */
NOINLINE_NOIPA
__int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations on x86-64 may generate library calls */
    result = a * b;                    /* __multi3 */
    result += a / b;                   /* __divti3 */
    result += a % b;                   /* __modti3 */
    
    /* Complex number division - often uses library calls */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double c3 = c1 / c2;
    
    /* Use the complex result to prevent optimization */
    result += (__int128)(__real__ c3 + __imag__ c3);
    
    return result;
}

/* OpenMP synthesis for runtime functions */
NOINLINE_NOIPA
int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] += n;
        }
    }
    
    /* Sum results */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Transactional memory - may synthesize TM runtime functions */
NOINLINE_NOIPA
int test_tm_synthesis(int *ptr) {
    int result = 0;
    
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* Soft-float synthesis (if compiled without hardware FPU support) */
NOINLINE_NOIPA
double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Double precision operations that may use library calls
       if compiled with -msoft-float or on targets without FPU */
    result = a * b;                    /* __muldf3 */
    result += a / b;                   /* __divdf3 */
    result += __builtin_sqrt(a);       /* sqrt */
    result += __builtin_sin(b);        /* sin */
    
    return result;
}

/* Combined synthesis triggers */
NOINLINE_NOIPA
uint64_t test_combined_synthesis(__int128 a, __int128 b) {
    volatile uint64_t result = 0;
    
    /* Mix of different synthesis triggers */
    result += __builtin_ia32_rdtsc();
    
    /* 128-bit atomic operation */
    __int128 atomic_val = a;
    __atomic_compare_exchange_n(&atomic_val, &a, b, 0, 
                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Complex arithmetic */
    _Complex float cf = (3.0f + 4.0fi) / (1.0f + 2.0fi);
    result += (uint64_t)(__real__ cf + __imag__ cf);
    
    /* CPU dispatch */
    if (__builtin_cpu_supports("sse4.2")) {
        result = __builtin_ia32_crc32di(result, b & 0xFFFFFFFF);
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Force synthesis of various built-in/library functions */
    accumulator += test_builtin_synthesis();
    
    /* Test 128-bit library call synthesis */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x10000000000000001ULL;
    accumulator += (uint64_t)test_libcall_synthesis(a, b);
    
    /* Test OpenMP synthesis */
    accumulator += test_omp_synthesis(argc);
    
    /* Test transactional memory synthesis */
    int tm_var = 42;
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Test soft-float synthesis */
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Test combined synthesis */
    accumulator += test_combined_synthesis(a, b);
    
    /* Print result to ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
