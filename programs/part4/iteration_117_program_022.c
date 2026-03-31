/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific built-ins that may require synthesis */
    result += __builtin_ia32_rdtsc();           /* RDTSC instruction */
    
    /* Memory barrier built-ins */
    __sync_synchronize();
    
    /* Atomic built-ins with uncommon sizes */
    volatile __int128 atomic_val = 0;
    __int128 expected = 0, desired = 1;
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* CPU feature detection built-ins */
    if (__builtin_cpu_supports("avx2")) {
        result += 1;
    }
    
    return result;
}

/* Test 2: Operations requiring library calls */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result;
    
    /* 128-bit operations on x86-64 may require libcalls */
    result = a * b;                    /* __multi3 */
    result += a / b;                   /* __divti3 */
    result += a % b;                   /* __modti3 */
    
    /* Complex number division (often requires libcalls) */
    volatile _Complex double c1 = 1.0 + 2.0i;
    volatile _Complex double c2 = 3.0 + 4.0i;
    volatile _Complex double cdiv = c1 / c2;
    
    /* Use the complex result to prevent optimization */
    result += (__int128)(__real__ cdiv * 1000);
    
    return result;
}

/* Test 3: Double-precision on soft-float target simulation */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result;
    
    /* Operations that might require soft-float libcalls */
    result = a * b;                    /* __muldf3 */
    result += a / b;                   /* __divdf3 */
    result = __builtin_sqrt(a);        /* sqrt */
    result += __builtin_sin(b);        /* sin */
    
    return result;
}

/* Test 4: OpenMP target region triggering runtime synthesis */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int i;
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: result)
    {
        #pragma omp parallel for reduction(+:result)
        for (i = 0; i < n; i++) {
            result += i * i;
        }
    }
    
    return result;
}

/* Test 5: Transactional memory extensions */
NOOPT int test_tm_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize TM runtime calls */
    __transaction_atomic {
        result = *ptr;
        *ptr = result + 1;
    }
    
    return result;
}

/* Test 6: BPF built-ins (if compiled for BPF target) */
NOOPT unsigned long test_bpf_synthesis(void) {
    volatile unsigned long result = 0;
    
    /* These may synthesize helper function declarations */
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    
    return result;
}

/* Test 7: ARM-specific built-ins */
NOOPT unsigned int test_arm_synthesis(void) {
    volatile unsigned int result = 0;
    
    /* ARM system register access - may require synthesis */
    #ifdef __arm__
    result = __builtin_arm_mrc(15, 0, 0, 0, 0);  /* Example: MIDR */
    #endif
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Force synthesis of various built-in/library functions */
    accumulator += test_builtin_synthesis();
    
    /* 128-bit arithmetic - may synthesize __multi3, __divti3, etc. */
    __int128 a = 1000000000000000000LL;
    __int128 b = 3;
    accumulator += test_libcall_synthesis(a, b);
    
    /* Floating-point operations */
    accumulator += (uint64_t)test_softfloat_synthesis(3.14159, 2.71828);
    
    /* OpenMP synthesis */
    accumulator += test_omp_synthesis(100);
    
    /* Transactional memory */
    int tm_var = 42;
    accumulator += test_tm_synthesis(&tm_var);
    
    /* Architecture-specific built-ins */
    #ifdef __bpf__
    accumulator += test_bpf_synthesis();
    #endif
    
    #ifdef __arm__
    accumulator += test_arm_synthesis();
    #endif
    
    /* Print result to prevent dead code elimination */
    printf("Synthesis test result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator & 0x7FFFFFFF);
}
