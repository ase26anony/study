/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa, used))

/* Test 1: Target-specific built-in functions */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86 specific built-ins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result += __builtin_ia32_rdtscp(&result);
    result += __builtin_cpu_supports("avx2");
    result += __builtin_cpu_supports("sse4.2");
    #endif
    
    /* ARM specific built-ins */
    #ifdef __arm__
    unsigned int coproc = 15, opc1 = 0, crn = 0, crm = 0, opc2 = 0;
    result += __builtin_arm_mrc(coproc, opc1, crn, crm, opc2);
    #endif
    
    /* Generic atomic built-ins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 atomic_expected = 0;
    __int128 atomic_desired = 1;
    __atomic_compare_exchange(&atomic_val, &atomic_expected, &atomic_desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing library calls */
NOOPT __int128 test_libcall_synthesis(__int128 a, __int128 b) {
    volatile __int128 result = 0;
    
    /* Operations that often require library calls */
    result = a * b;           /* 128-bit multiplication */
    result += a / b;          /* 128-bit division */
    result += a % b;          /* 128-bit modulo */
    
    /* Complex number division - often requires library calls */
    _Complex double c1 = 1.0 + 2.0i;
    _Complex double c2 = 3.0 + 4.0i;
    _Complex double cdiv = c1 / c2;
    result += (__int128)(creal(cdiv) * 1000);
    
    return result;
}

/* Test 3: Soft-float operations */
NOOPT double test_softfloat_synthesis(double a, double b) {
    volatile double result = 0.0;
    
    /* Operations that may require soft-float library calls */
    result = a * b;           /* Double multiplication */
    result += a / b;          /* Double division */
    result += __builtin_sqrt(a); /* Square root */
    result += __builtin_sin(a);  /* Trigonometric function */
    
    return result;
}

/* Test 4: OpenMP target region with data mapping */
NOOPT int test_omp_synthesis(int n) {
    volatile int result = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i + n;
    }
    
    /* OpenMP target region - may synthesize data mapping routines */
    #pragma omp target map(tofrom: arr[0:100])
    {
        for (int i = 0; i < 100; i++) {
            arr[i] *= 2;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 5: Transactional memory extensions */
NOOPT int test_transactional_synthesis(int *ptr) {
    volatile int result = 0;
    
    /* Transactional memory - may synthesize runtime calls */
    __transaction_atomic {
        *ptr += 1;
        result = *ptr;
    }
    
    return result;
}

/* Test 6: BPF built-ins (if compiling for BPF target) */
NOOPT unsigned long test_bpf_synthesis(void) {
    volatile unsigned long result = 0;
    
    #ifdef __bpf__
    result = __builtin_bpf_packet_data();
    result += __builtin_bpf_packet_end();
    #endif
    
    return result;
}

/* Main function that exercises all synthesis paths */
int main(int argc, char *argv[]) {
    volatile uint64_t accumulator = 0;
    volatile int shared_var = 0;
    
    /* Test 1: Built-in synthesis */
    accumulator += test_builtin_synthesis();
    
    /* Test 2: 128-bit libcall synthesis */
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = ((__int128)0x1111111111111111 << 64) | 0x2222222222222222;
    __int128 res128 = test_libcall_synthesis(a, b);
    accumulator += (uint64_t)res128 + (uint64_t)(res128 >> 64);
    
    /* Test 3: Soft-float synthesis */
    double d1 = 3.141592653589793;
    double d2 = 2.718281828459045;
    double dres = test_softfloat_synthesis(d1, d2);
    accumulator += (uint64_t)dres;
    
    /* Test 4: OpenMP synthesis */
    accumulator += test_omp_synthesis(argc);
    
    /* Test 5: Transactional memory synthesis */
    accumulator += test_transactional_synthesis(&shared_var);
    
    /* Test 6: BPF synthesis (if applicable) */
    accumulator += test_bpf_synthesis();
    
    /* Additional: CPU feature detection synthesis */
    #ifdef __x86_64__
    accumulator += __builtin_cpu_init();
    accumulator += __builtin_cpu_supports("avx512f");
    accumulator += __builtin_cpu_supports("bmi2");
    #endif
    
    /* Additional: Atomic operations on unusual sizes */
    struct Unusual { char a; long long b; } unusual = {0};
    __atomic_store_n(&unusual.b, accumulator, __ATOMIC_RELEASE);
    accumulator += __atomic_load_n(&unusual.b, __ATOMIC_ACQUIRE);
    
    /* Ensure all operations are observable */
    printf("Result: %llu\n", (unsigned long long)accumulator);
    
    return (int)(accumulator % 256);
}
