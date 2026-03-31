/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -S -o test.s test.c */

/* Prevent interprocedural optimization */
#define NOINLINE __attribute__((noinline, noipa))

/* Test function for x86-specific builtins */
NOINLINE unsigned long long test_x86_builtins(void) {
    volatile unsigned long long result = 0;
    
    /* Use various x86 intrinsics that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier builtins */
    __sync_synchronize();
    
    /* CPU feature detection - may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Transactional memory - may synthesize runtime calls */
    #ifdef __TM__
    __transaction_atomic {
        result += 2;
    }
    #endif
    
    return result;
}

/* Test function for 128-bit operations (requires libcall synthesis on many targets) */
NOINLINE unsigned long long test_128bit_ops(void) {
    volatile unsigned __int128 a = 100;
    volatile unsigned __int128 b = 200;
    volatile unsigned __int128 c;
    
    /* 128-bit multiplication - often requires libcall */
    c = a * b;
    
    /* 128-bit division - almost always requires libcall */
    c = c / a;
    
    /* Return lower 64 bits to avoid 128-bit return value issues */
    return (unsigned long long)c;
}

/* Test function for atomic operations with uncommon sizes */
NOINLINE unsigned long long test_atomic_ops(void) {
    volatile __int128 atomic_var = 0;
    __int128 expected = 0;
    __int128 desired = 100;
    
    /* Atomic compare-exchange on 128-bit - may synthesize helper */
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Atomic load of 128-bit */
    __int128 loaded = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    return (unsigned long long)loaded;
}

/* Test function for floating-point operations requiring soft-float */
NOINLINE double test_softfloat_ops(double x, double y) {
    volatile double result = 0.0;
    
    /* Complex division often requires libcall */
    volatile _Complex double c1 = x + y * _Complex_I;
    volatile _Complex double c2 = y + x * _Complex_I;
    volatile _Complex double c3 = c1 / c2;
    
    result += __real__ c3 + __imag__ c3;
    
    /* Transcendental functions may require libcalls */
    result += __builtin_sin(x);
    result += __builtin_exp(y);
    
    return result;
}

/* Test function for OpenMP target region synthesis */
NOINLINE unsigned long long test_omp_synthesis(void) {
    volatile unsigned long long result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping and runtime functions */
    #pragma omp target map(tofrom: arr[0:100])
    {
        #pragma omp parallel for
        for (int i = 0; i < 100; i++) {
            arr[i] *= 2;
        }
    }
    
    /* Sum results */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test function for BPF builtins (if compiled for BPF target) */
NOINLINE unsigned long long test_bpf_builtins(void) {
    volatile unsigned long long result = 0;
    
    /* These would only work when targeting BPF */
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    #endif
    
    return result;
}

/* Test function for ARM-specific builtins */
NOINLINE unsigned long long test_arm_builtins(void) {
    volatile unsigned long long result = 0;
    
    /* ARM system register access - may require synthesis */
    #ifdef __arm__
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    #endif
    
    #ifdef __aarch64__
    result = __builtin_aarch64_get_fpcr();
    #endif
    
    return result;
}

/* Main function that calls all test functions */
int main(void) {
    volatile unsigned long long accumulator = 0;
    
    /* Call various synthesis-triggering functions */
    accumulator += test_x86_builtins();
    accumulator += test_128bit_ops();
    accumulator += test_atomic_ops();
    accumulator += (unsigned long long)test_softfloat_ops(1.0, 2.0);
    accumulator += test_omp_synthesis();
    accumulator += test_bpf_builtins();
    accumulator += test_arm_builtins();
    
    /* Print result to ensure no optimization */
    printf("Result: %llu\n", accumulator);
    
    return 0;
}
