/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S -o test.s test.c */

/* Prevent optimizations from removing our test functions */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test functions that should trigger built-in synthesis */
NOOPT unsigned long long test_builtin_synthesis(void) {
    volatile unsigned long long result = 0;
    
    /* x86-specific builtins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier builtins */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    /* CPU feature detection - may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result += __builtin_ia32_crc32qi(result, 0xAA);
    }
    
    /* Uncommon atomic operation */
    unsigned char byte = 0x55;
    __atomic_fetch_add(&byte, 1, __ATOMIC_RELAXED);
    result += byte;
    
    return result;
}

NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    /* 128-bit operations often require libcalls on many targets */
    volatile __int128 result;
    
    /* Multiplication - often requires helper function */
    result = a * b;
    
    /* Division - almost always requires libcall */
    if (b != 0) {
        result = result / b;
    }
    
    /* Complex floating point - may require libcalls */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double c3 = c1 / c2;
    
    /* Use the complex result to affect the 128-bit result */
    result += (__int128)(__real__ c3 * 1000);
    
    return result;
}

NOOPT double test_softfloat_synthesis(double a, double b) {
    /* Force soft-float library calls on targets without native double support */
    volatile double result = 0.0;
    
    /* Transcendental functions often require libcalls */
    result += __builtin_sin(a);
    result += __builtin_cos(b);
    
    /* Complex power function */
    volatile _Complex double c = a + b * 1.0i;
    result += __real__ __builtin_cpow(c, 2.0);
    
    return result;
}

#ifdef _OPENMP
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int i;
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: result)
    {
        /* Use target-specific builtin inside OpenMP region */
        #ifdef __x86_64__
        result += __builtin_ia32_rdtsc() & 0xFF;
        #endif
        
        /* Simple computation to ensure code isn't optimized away */
        for (i = 0; i < n; i++) {
            result += i * i;
        }
    }
    
    /* OpenMP atomic with uncommon memory order */
    #pragma omp atomic seq_cst
    result += 1;
    
    return result;
}
#endif

/* Transactional Memory - if supported */
#ifdef __TM_FUNCTIONS
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}
#endif

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile unsigned long long accumulator = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 big1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 big2 = 0x1000000000000000ULL;
    __int128 big_result = test_libcall_synthesis(big1, big2);
    accumulator += (unsigned long long)big_result;
    accumulator += (unsigned long long)(big_result >> 64);
    
    /* Test 3: Soft-float libcall synthesis */
    double d1 = 3.141592653589793;
    double d2 = 2.718281828459045;
    double d_result = test_softfloat_synthesis(d1, d2);
    accumulator += (unsigned long long)d_result;
    
    /* Test 4: OpenMP synthesis (if available) */
    #ifdef _OPENMP
    int omp_result = test_omp_synthesis(100);
    accumulator += omp_result;
    #endif
    
    /* Test 5: Transactional memory (if available) */
    #ifdef __TM_FUNCTIONS
    int tm_var = 42;
    int tm_result = test_tm_synthesis(&tm_var);
    accumulator += tm_result;
    #endif
    
    /* Additional: Uncommon atomic size that might require helper */
    volatile __int128 atomic_var = 0;
    __atomic_fetch_add(&atomic_var, 1, __ATOMIC_RELAXED);
    accumulator += (unsigned long long)atomic_var;
    
    /* Print result to ensure all operations are observable */
    printf("Result: %llu\n", accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
