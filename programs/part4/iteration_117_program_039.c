/* test_synthesis.c
 * 
 * This program is designed to trigger the built-in function synthesis
 * path in GCC's targhooks.cc (lines 981-990) by using various patterns
 * that force the compiler to create synthesized function declarations
 * with special attributes (static, public, external, hidden visibility, etc.).
 *
 * Compilation suggestions:
 *   x86:    gcc -O2 -march=x86-64 -fdump-tree-all -S test_synthesis.c
 *   ARM:    gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon -fdump-tree-all -S test_synthesis.c
 *   OpenMP: gcc -O2 -fopenmp -fdump-tree-omplower -S test_synthesis.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization and inlining */
#define NOINLINE_NOIPA __attribute__((noinline, noipa))

/* ========== Pattern 1: Target-specific built-in synthesis ========== */

NOINLINE_NOIPA
uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
#if defined(__x86_64__) || defined(__i386__)
    /* __builtin_ia32_rdtsc often requires synthesized declaration */
    result += __builtin_ia32_rdtsc();
    
    /* Memory barrier built-in */
    __builtin_ia32_mfence();
    
    /* SIMD built-in - may trigger synthesis on some optimization levels */
    __builtin_ia32_paddb128((__v16qi){0}, (__v16qi){0});
#endif
    
    /* ARM-specific built-ins */
#if defined(__arm__) || defined(__aarch64__)
    /* Coprocessor register access - often synthesized */
    result += __builtin_arm_mrc(15, 0, 0, 0, 0);
    
    /* Memory barrier */
    __builtin_arm_dmb(0xF);
#endif
    
    /* Generic atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_val;
}

/* ========== Pattern 2: Library call synthesis for unsupported ops ========== */

NOINLINE_NOIPA
__int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result;
    
    /* 128-bit multiplication on 32-bit targets often requires libcall */
    result = a * b;
    
    /* 128-bit division definitely requires libcall */
    if (b != 0) {
        result = result / b;
    }
    
    /* Double-precision math on soft-float target */
    volatile double x = 3.141592653589793;
    volatile double y = 2.718281828459045;
    volatile double z = x * y + x / y - y * y;
    
    /* Complex division often requires libcall */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double c3 = c1 / c2;
    
    return result + (__int128)((int64_t)z + (int64_t)__real__ c3);
}

/* ========== Pattern 3: Language extensions requiring runtime support ========== */

NOINLINE_NOIPA
int test_extension_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize __tm_* functions */
#ifdef __TM_FENCE__
    __transaction_atomic {
        result = x * 2;
    }
#endif
    
    /* CPU feature detection - may synthesize resolver functions */
#if defined(__x86_64__) || defined(__i386__)
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 2;
    }
#endif
    
    /* __builtin_constant_p with runtime fallback */
    int dynamic_value = x + rand();
    if (__builtin_constant_p(x)) {
        result += 100;
    } else {
        /* Force runtime path */
        result += dynamic_value;
    }
    
    return result;
}

/* ========== Pattern 4: OpenMP runtime synthesis ========== */

NOINLINE_NOIPA
int test_omp_synthesis(int n) {
    volatile int result = 0;
    
#ifdef _OPENMP
    /* OpenMP target region - may synthesize data mapping and runtime functions */
    #pragma omp target map(tofrom: result)
    {
        for (int i = 0; i < n; i++) {
            result += i * i;
        }
    }
    
    /* Teams construct - may synthesize more runtime functions */
    #pragma omp target teams distribute parallel for reduction(+:result)
    for (int i = 0; i < n; i++) {
        result += i % 7;
    }
#endif
    
    return result;
}

/* ========== Pattern 5: Combined synthesis triggers ========== */

NOINLINE_NOIPA
uint64_t test_combined_synthesis(__int128 a, __int128 b, int n) {
    volatile uint64_t result = 0;
    
    /* Combine 128-bit arithmetic with atomics */
    __int128 product = a * b;
    volatile __int128 atomic_var = product;
    
    __atomic_store_n(&atomic_var, product, __ATOMIC_RELEASE);
    __int128 loaded = __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    
    /* Mix with target-specific built-ins */
#if defined(__x86_64__) || defined(__i386__)
    result += __builtin_ia32_rdtsc();
#endif
    
    /* Add OpenMP synthesis if available */
#ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result += (uint64_t)loaded;
    }
#endif
    
    /* Complex division for libcall synthesis */
    volatile _Complex double cdiv = (1.0 + 2.0i) / (3.0 + 4.0i);
    result += (uint64_t)__real__ cdiv;
    
    return result;
}

/* ========== Main function ========== */

int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize with some non-zero values */
    __int128 big_val1 = ((__int128)0x12345678 << 64) | 0x9ABCDEF0;
    __int128 big_val2 = ((__int128)0xFEDCBA98 << 64) | 0x76543210;
    int test_size = 100;
    
    /* Call all synthesis patterns */
    accumulator += test_builtin_synthesis();
    accumulator += (uint64_t)test_libcall_synthesis(big_val1, big_val2);
    accumulator += test_extension_synthesis(argc);
    accumulator += test_omp_synthesis(test_size);
    accumulator += test_combined_synthesis(big_val1, big_val2, test_size);
    
    /* Ensure all operations are observable */
    printf("Synthesis test accumulator: %llu\n", 
           (unsigned long long)accumulator);
    
    /* Use result to prevent dead code elimination */
    if (accumulator != 0) {
        return 0;
    }
    return 1;
}
