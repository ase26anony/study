/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

/* Prevent interprocedural optimization */
#define NOINLINE __attribute__((noinline, noipa))

/* Target-specific built-in functions */
NOINLINE unsigned long long test_x86_intrinsics(void) {
    volatile unsigned long long result = 0;
    
    /* x86-specific intrinsics that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* MMX/SSE intrinsics */
    result += __builtin_ia32_comieq(__builtin_ia32_addps(
        __builtin_ia32_loadups((const float*)&result),
        __builtin_ia32_loadups((const float*)&result)
    ), 0);
    
    /* BMI intrinsics */
    result += __builtin_ia32_bextr_u64(result, 0x1F1F);
    
    return result;
}

/* 128-bit arithmetic forcing libcall synthesis */
NOINLINE unsigned long long test_128bit_arithmetic(unsigned long long a, 
                                                   unsigned long long b) {
    volatile __int128 x = (__int128)a * (__int128)b;
    volatile __int128 y = (__int128)a + (__int128)b;
    volatile __int128 z = x / (y ? y : 1);
    
    /* Complex division also often requires libcalls */
    volatile _Complex double c1 = a + b * I;
    volatile _Complex double c2 = b + a * I;
    volatile _Complex double c3 = c1 / c2;
    
    return (unsigned long long)z + (unsigned long long)creal(c3);
}

/* Atomic operations with uncommon sizes */
NOINLINE unsigned long long test_atomic_synthesis(unsigned long long *ptr) {
    volatile unsigned long long result = 0;
    
    /* 128-bit atomic operation (may require libcall) */
    __int128 atomic_val = 0;
    __int128 desired = 100;
    __int128 expected = 0;
    
    /* This may trigger helper function synthesis */
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Atomic load with uncommon memory order */
    result += __atomic_load_n(ptr, __ATOMIC_CONSUME);
    
    return result + (unsigned long long)atomic_val;
}

/* OpenMP target region triggering runtime function synthesis */
NOINLINE unsigned long long test_omp_synthesis(void) {
    volatile unsigned long long result = 0;
    int arr[100];
    
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] = i * i;
        }
        
        /* Use target-specific built-in inside OpenMP region */
        result = __builtin_ia32_rdtsc();
    }
    
    /* Sum array to ensure computation isn't optimized away */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Transactional memory extension */
NOINLINE unsigned long long test_transactional_memory(unsigned long long *ptr) {
    volatile unsigned long long result = 0;
    
    /* Transactional memory operations may require runtime support */
    __transaction_atomic {
        *ptr += 1;
        result = *ptr;
        
        /* Nested complex operation inside transaction */
        volatile _Complex float cf = (*ptr) + (*ptr) * I;
        cf = cf / (cf + 1.0f);
        result += (unsigned long long)crealf(cf);
    }
    
    return result;
}

/* CPU feature detection requiring runtime resolution */
NOINLINE unsigned long long test_cpu_dispatch(void) {
    volatile unsigned long long result = 0;
    
    /* CPU feature checks that may require synthesized resolver functions */
    if (__builtin_cpu_supports("avx512f")) {
        result |= 0x1;
    }
    if (__builtin_cpu_supports("avx2")) {
        result |= 0x2;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result |= 0x4;
    }
    
    /* Initialize CPU features (may trigger synthesis) */
    __builtin_cpu_init();
    
    return result;
}

/* Soft-float double operations on hypothetical target */
NOINLINE unsigned long long test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Operations that might require soft-float libcalls */
    result = a * b;
    result = result / (a + b);
    result = __builtin_sqrt(result);
    result = __builtin_sin(result) + __builtin_cos(result);
    
    /* Complex double division */
    volatile _Complex double cd = a + b * I;
    cd = cd / (cd + 1.0);
    
    return (unsigned long long)result + (unsigned long long)creal(cd);
}

/* Main function that exercises all synthesis paths */
int main(void) {
    volatile unsigned long long accumulator = 0;
    volatile unsigned long long dummy_var = 42;
    
    /* Test various synthesis triggers */
    accumulator += test_x86_intrinsics();
    accumulator += test_128bit_arithmetic(accumulator, 123456789);
    accumulator += test_atomic_synthesis(&dummy_var);
    accumulator += test_omp_synthesis();
    accumulator += test_transactional_memory(&dummy_var);
    accumulator += test_cpu_dispatch();
    accumulator += test_softfloat_synthesis(3.14159, 2.71828);
    
    /* Mix in some direct built-in usage in main */
    accumulator += __builtin_popcountll(accumulator);
    accumulator += __builtin_bswap64(accumulator);
    
    /* Use __builtin_constant_p with runtime fallback */
    if (!__builtin_constant_p(accumulator)) {
        accumulator = __builtin_ia32_rdtsc() ^ accumulator;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
