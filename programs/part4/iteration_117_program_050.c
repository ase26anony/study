/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, used))

/* 1. Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    /* CPU feature detection - may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    /* Transactional memory extension */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        result += 2;
    }
    #endif
    
    return result;
}

/* 2. Library call synthesis for unsupported operations */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit operations on x86-64 may generate library calls */
    result = a * b;                    /* __multi3 */
    result += a / b;                   /* __divti3 */
    result += a % b;                   /* __modti3 */
    
    /* Complex division - often uses library calls */
    _Complex double c1 = 3.0 + 4.0i;
    _Complex double c2 = 1.0 + 2.0i;
    _Complex double c3 = c1 / c2;
    
    /* Use the complex result to prevent optimization */
    result += (__int128)(__real__ c3 + __imag__ c3);
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, result, __ATOMIC_RELAXED);
    
    return result + atomic_val;
}

/* 3. OpenMP runtime function synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping and runtime functions */
    #pragma omp target map(tofrom: arr[0:100]) map(tofrom: result)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] *= 2;
        }
        result = arr[n % 100];
    }
    
    /* OpenMP atomic with uncommon memory order */
    int atomic_var = 0;
    #pragma omp atomic seq_cst
    atomic_var += result;
    
    return result + atomic_var;
}

/* 4. Soft-float synthesis (if compiled without hardware FPU support) */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Operations that may require soft-float library calls */
    result = a / b;                    /* __divdf3 */
    result += __builtin_sqrt(a);       /* sqrt */
    result += __builtin_sin(b);        /* sin */
    result += __builtin_pow(a, b);     /* pow */
    
    /* Fused multiply-add - may be synthesized */
    result = __builtin_fma(a, b, result);
    
    return result;
}

/* 5. Combined synthesis triggers */
NOOPT __int128 test_combined_synthesis(__int128 val) {
    volatile __int128 result = val;
    
    /* Mix of different synthesis triggers */
    
    /* Atomic 128-bit operation */
    __int128 atomic_val = 0;
    __atomic_compare_exchange_n(&atomic_val, &result, val + 1, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Target built-in inside computation */
    uint64_t ts = __builtin_ia32_rdtsc();
    result += (__int128)ts;
    
    /* Complex operation */
    _Complex float cf = (2.0f + 3.0fi) / (1.0f + 1.0fi);
    result += (__int128)(__real__ cf + __imag__ cf);
    
    /* OpenMP region with atomic */
    int tmp = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        tmp++;
    }
    result += tmp;
    
    return result + atomic_val;
}

int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Use command-line arguments to make values unpredictable */
    int seed = argc > 1 ? argv[1][0] : 42;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: Libcall synthesis with 128-bit values */
    __int128 a = ((__int128)seed << 64) | seed;
    __int128 b = ((__int128)(seed + 1) << 32) | (seed + 2);
    __int128 res128 = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)res128 + (uint64_t)(res128 >> 64);
    
    /* Test 3: OpenMP synthesis */
    accumulator += test_omp_synthesis(seed);
    
    /* Test 4: Soft-float synthesis */
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    double dres = test_softfloat_synthesis(d1, d2);
    accumulator += (uint64_t)dres;
    
    /* Test 5: Combined synthesis */
    __int128 combined_res = test_combined_synthesis(a);
    accumulator += (uint64_t)combined_res + (uint64_t)(combined_res >> 64);
    
    /* Also test __atomic built-ins with uncommon sizes */
    struct Uncommon { char data[7]; } uncommon;
    __atomic_store_n(&uncommon.data, seed, __ATOMIC_RELAXED);
    accumulator += uncommon.data[0];
    
    /* Print result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)accumulator);
    
    return (int)(accumulator % 256);
}
