/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimization */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Target-specific built-ins for x86 */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* Use x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-in */
    __sync_synchronize();
    
    /* Atomic built-in with specific memory order */
    uint64_t atomic_var = 0;
    __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    
    /* CPU feature detection built-in - may synthesize resolver */
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

/* 128-bit arithmetic forcing libcall synthesis */
NOOPT unsigned __int128 test_libcall_synthesis(unsigned __int128 a, unsigned __int128 b) {
    volatile unsigned __int128 result = 0;
    
    /* 128-bit operations that may require libcalls */
    result = a + b;                    /* May synthesize __addti3 */
    result = result * a;               /* May synthesize __multti3 */
    result = result / (b + 1);         /* May synthesize __udivti3 */
    
    /* Atomic operation on 128-bit value */
    unsigned __int128 atomic_128 = 0;
    __atomic_compare_exchange_n(&atomic_128, &a, b, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Floating-point operations forcing soft-float libcalls */
NOOPT double test_float_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Complex number division - often requires libcall */
    __complex__ double c1 = a + b * I;
    __complex__ double c2 = b + a * I;
    __complex__ double cdiv = c1 / c2;
    
    result = __real__ cdiv + __imag__ cdiv;
    
    /* Double precision math that might need libcalls */
    result += __builtin_sin(a);
    result += __builtin_cos(b);
    result += __builtin_exp(a * b);
    
    return result;
}

/* OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i * n;
    }
    
    #pragma omp target map(tofrom: arr[0:100]) map(to: n)
    {
        /* Use target-specific built-in inside OpenMP region */
        #ifdef __x86_64__
        unsigned long long tsc = __builtin_ia32_rdtsc();
        #endif
        
        for (int i = 0; i < 100; i++) {
            arr[i] += (i % 10) + n;
        }
        
        result = arr[50];
    }
    
    /* Sum array to ensure computation isn't optimized away */
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* BPF built-in if compiling for BPF target */
#ifdef __bpf__
NOOPT unsigned long test_bpf_synthesis(void) {
    volatile unsigned long result = 0;
    
    /* BPF-specific built-ins */
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    result += __builtin_bpf_skb_data();
    
    return result;
}
#endif

/* ARM-specific built-ins if compiling for ARM */
#ifdef __arm__
NOOPT unsigned int test_arm_synthesis(void) {
    volatile unsigned int result = 0;
    
    /* ARM system register access */
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);
    result += __builtin_arm_mrc(15, 0, 0, 1, 0);
    
    /* ARM DSP extensions */
    result += __builtin_arm_usad8(0x12345678, 0x87654321);
    
    return result;
}
#endif

int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    
    /* Use command-line arguments to make values unpredictable */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    unsigned __int128 a128 = ((unsigned __int128)rand() << 64) | rand();
    unsigned __int128 b128 = ((unsigned __int128)rand() << 64) | rand();
    unsigned __int128 res128 = test_libcall_synthesis(a128, b128);
    accumulator += (uint64_t)res128 + (uint64_t)(res128 >> 64);
    
    /* Test 3: Float libcall synthesis */
    double a = (double)rand() / RAND_MAX;
    double b = (double)rand() / RAND_MAX;
    accumulator += (uint64_t)test_float_synthesis(a, b);
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(rand() % 100);
    
    /* Target-specific tests */
    #ifdef __bpf__
    accumulator += test_bpf_synthesis();
    #endif
    
    #ifdef __arm__
    accumulator += test_arm_synthesis();
    #endif
    
    /* Use __builtin_constant_p with runtime fallback */
    int dynamic_value = rand();
    if (__builtin_constant_p(dynamic_value)) {
        accumulator += 1000;
    } else {
        /* This branch should be taken, may synthesize runtime check */
        accumulator += dynamic_value;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator % 256);
}
