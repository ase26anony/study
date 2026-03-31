/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate synthesis */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Target-specific built-in synthesis */
NO_OPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* __atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    /* CPU feature detection that may synthesize resolver functions */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Library call synthesis through unsupported operations */
NO_OPT uint64_t test_libcall_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* 128-bit arithmetic on target without native support */
    __int128 a = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA987654321ULL;
    __int128 b = 0x1000000000000001ULL;
    __int128 c = a * b;      /* May synthesize __multi3 */
    __int128 d = a / b;      /* May synthesize __divti3 */
    __int128 e = a % b;      /* May synthesize __modti3 */
    
    result += (uint64_t)c + (uint64_t)d + (uint64_t)e;
    
    /* Complex number division (often uses library calls) */
    __complex__ double z1 = 3.0 + 4.0i;
    __complex__ double z2 = 1.0 + 2.0i;
    __complex__ double z3 = z1 / z2;
    
    result += (uint64_t)__real__ z3 + (uint64_t)__imag__ z3;
    
    return result;
}

/* OpenMP runtime function synthesis */
NO_OPT uint64_t test_omp_synthesis(int size) {
    volatile uint64_t result = 0;
    int *array = (int*)malloc(size * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: array[0:size])
    {
        #pragma omp parallel for
        for (int i = 0; i < size; i++) {
            array[i] *= 2;
        }
    }
    
    /* Collect results */
    for (int i = 0; i < size; i++) {
        result += array[i];
    }
    
    free(array);
    return result;
}

/* Combined synthesis triggers */
NO_OPT uint64_t test_combined_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Transactional memory (may synthesize runtime functions) */
    _Bool tx_success = 0;
    
    /* Use __transaction_atomic if supported, otherwise fallback */
    #ifdef __TM_FENCE__
    __transaction_atomic {
        static int counter = 0;
        counter++;
        result = counter;
    }
    tx_success = 1;
    #else
    /* Fallback: atomic operation on 128-bit value */
    __int128 atomic_counter = 0;
    __atomic_fetch_add(&atomic_counter, 1, __ATOMIC_SEQ_CST);
    result = (uint64_t)atomic_counter;
    #endif
    
    /* More x86 built-ins */
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    result += eax + ebx + ecx + edx;
    
    /* __builtin_constant_p with runtime fallback */
    int dynamic_value = rand() % 100;
    if (__builtin_constant_p(dynamic_value)) {
        result += 1000;
    } else {
        result += dynamic_value;
    }
    
    return result;
}

/* Force synthesis through soft-float operations */
NO_OPT uint64_t test_softfloat_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Double-precision operations that may use soft-float libcalls
       on targets without native double support */
    double a = 3.141592653589793;
    double b = 2.718281828459045;
    
    /* These operations may synthesize library calls */
    double c = a * b;      /* __muldf3 */
    double d = a / b;      /* __divdf3 */
    double e = a + b;      /* __adddf3 */
    double f = a - b;      /* __subdf3 */
    
    /* Use results to prevent optimization */
    result += (uint64_t)c + (uint64_t)d + (uint64_t)e + (uint64_t)f;
    
    /* Long double operations (often use libcalls) */
    long double ld1 = 1.234567890123456789L;
    long double ld2 = 9.876543210987654321L;
    long double ld3 = ld1 * ld2;
    
    result += (uint64_t)ld3;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Seed random for __builtin_constant_p test */
    srand(42);
    
    /* Call all synthesis test functions */
    accumulator += test_builtin_synthesis();
    accumulator += test_libcall_synthesis();
    accumulator += test_omp_synthesis(argc > 1 ? atoi(argv[1]) : 100);
    accumulator += test_combined_synthesis();
    accumulator += test_softfloat_synthesis();
    
    /* Print result to ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return 0;
}
