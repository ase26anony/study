/* test_synthesis.c
 * 
 * This program is designed to trigger the synthesis of built-in function
 * declarations in GCC's backend, specifically aiming to exercise the
 * uncovered lines in targhooks.cc that set special attributes on synthesized
 * tree nodes.
 *
 * Compilation suggestions:
 *   x86:    gcc -O2 -march=x86-64 -fdump-tree-all -S test_synthesis.c
 *   ARM:    gcc -O2 -march=armv7-a -mfloat-abi=softfp -mfpu=neon test_synthesis.c
 *   OpenMP: gcc -O2 -fopenmp -fdump-tree-omplower test_synthesis.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* ========== Pattern 1: Target-specific built-in synthesis ========== */

NOOPT
uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
#ifdef __x86_64__
    /* rdtsc intrinsic - commonly synthesized */
    result += __builtin_ia32_rdtsc();
    
    /* SSE/AVX built-ins that might not have predefined declarations */
    result += __builtin_ia32_crc32qi(0x12345678, 0x9A);
    result += __builtin_ia32_rdpid();
#endif
    
    /* ARM-specific built-ins */
#ifdef __arm__
    /* ARM MRC coprocessor instruction */
    result += __builtin_arm_mrc(15, 0, 0, 13, 0, 3);
    
    /* ARM DSP extension */
    result += __builtin_arm_qadd(1000, 2000);
#endif
    
    /* BPF built-ins (if targeting BPF backend) */
#ifdef __bpf__
    result += __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
#endif
    
    /* Generic atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange(&atomic_val, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result + (uint64_t)atomic_val;
}

/* ========== Pattern 2: Library call synthesis via unsupported ops ========== */

NOOPT
__int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* 128-bit arithmetic on targets without native support */
    result = a * b;           /* May synthesize __multi3 */
    result += a / b;          /* May synthesize __divti3 */
    result += a % (b + 1);    /* May synthesize __modti3 */
    
    /* Double-precision math on soft-float target */
    volatile double x = 3.141592653589793;
    volatile double y = 2.718281828459045;
    result += (__int128)(x * y);      /* May synthesize __muldf3 */
    result += (__int128)(x / y);      /* May synthesize __divdf3 */
    
    /* Complex number division */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double c3 = c1 / c2;
    result += (__int128)(__real__ c3 * 1000);
    
    return result;
}

/* ========== Pattern 3: Language extensions requiring runtime support ========== */

NOOPT
int test_extension_synthesis(int x) {
    volatile int result = 0;
    
    /* Transactional memory (requires runtime support) */
#ifdef __TM__
    __transaction_atomic {
        result = x * 2;
    }
#endif
    
    /* CPU feature detection (may synthesize resolver) */
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx512f")) {
        result += 100;
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += 200;
    }
#endif
    
    /* __builtin_constant_p with runtime fallback */
    if (__builtin_constant_p(x)) {
        result = x * 3;
    } else {
        /* Force runtime path */
        volatile int* ptr = &x;
        result = *ptr * 4;
    }
    
    return result;
}

/* ========== Pattern 4: OpenMP target region synthesis ========== */

NOOPT
int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + n;
    }
    
    /* OpenMP target region - may synthesize data mapping and runtime functions */
#ifdef _OPENMP
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        for (int i = 0; i < 100; i++) {
            arr[i] = arr[i] * 2 + n;
        }
    }
#endif
    
    /* Sum results */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* ========== Pattern 5: Combined synthesis triggers ========== */

NOOPT
__int128 test_combined_synthesis(__int128 a, int b) {
    volatile __int128 result = 0;
    
    /* Mix 128-bit atomics with built-ins */
    volatile __int128 atomic_val = a;
    __int128 expected = a, desired = a + 1;
    
    __atomic_compare_exchange(&atomic_val, &expected, &desired,
                              0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    
    result = atomic_val;
    
    /* Add target-specific built-in */
#ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
#endif
    
    /* Add soft-float operation */
    volatile double x = (double)b;
    result += (__int128)(x * 3.14159);
    
    return result;
}

/* ========== Main function ========== */

int main(int argc, char** argv) {
    volatile uint64_t accumulator = 0;
    
    /* Use command-line arguments to make values unpredictable */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Pattern 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Pattern 2: Libcall synthesis */
    __int128 a = ((__int128)rand() << 64) | rand();
    __int128 b = ((__int128)rand() << 32) | rand();
    if (b == 0) b = 1;  /* Avoid division by zero */
    accumulator += (uint64_t)test_libcall_synthesis(a, b);
    
    /* Pattern 3: Extension synthesis */
    accumulator += test_extension_synthesis(rand() % 100);
    
    /* Pattern 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(rand() % 50);
    
    /* Pattern 5: Combined synthesis */
    accumulator += (uint64_t)test_combined_synthesis(a / 2, rand() % 1000);
    
    /* Ensure all operations are observable */
    printf("Result checksum: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator % 256);
}
