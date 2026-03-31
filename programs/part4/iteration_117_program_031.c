/* Compile with: gcc -O2 -march=x86-64 -fopenmp -fdump-tree-all -o test_synthesis test_synthesis.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and interprocedural analysis */
#define NOOPT __attribute__((noinline, noipa, noclone))

/* Test 1: Target-specific built-in synthesis */
NOOPT uint64_t test_builtin_synthesis(void) {
    volatile uint64_t result = 0;
    
    /* x86-specific builtins that may require synthesis */
    #ifdef __x86_64__
    result += __builtin_ia32_rdtsc();
    result ^= __builtin_ia32_rdtscp(&result);
    
    /* CPU feature detection builtins */
    if (__builtin_cpu_supports("avx2")) {
        result += __builtin_ia32_crc32di(result, result);
    }
    if (__builtin_cpu_supports("sse4.2")) {
        result += __builtin_ia32_crc32qi((uint8_t)result, (uint8_t)result);
    }
    #endif
    
    /* Generic atomic builtins with uncommon sizes */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = 1;
    
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    result += (uint64_t)atomic_val;
    
    return result;
}

/* Test 2: 128-bit arithmetic forcing libcall synthesis */
NOOPT uint64_t test_libcall_synthesis(uint64_t a, uint64_t b) {
    volatile __int128 x = ((__int128)a << 64) | b;
    volatile __int128 y = ((__int128)b << 64) | a;
    
    /* Operations that often require libcalls on 64-bit targets */
    __int128 mul = x * y;           /* 128x128 -> 128 multiplication */
    __int128 div = x / (y | 1);     /* 128/128 division */
    __int128 mod = x % (y | 1);     /* 128/128 modulus */
    
    /* Complex division - often requires libcall */
    volatile _Complex double c1 = a + b * I;
    volatile _Complex double c2 = b + a * I;
    volatile _Complex double cdiv = c1 / c2;
    
    /* Double precision on soft-float target simulation */
    volatile double d1 = (double)a * 3.14159;
    volatile double d2 = (double)b * 2.71828;
    volatile double ddiv = d1 / d2;
    
    return (uint64_t)mul ^ (uint64_t)div ^ (uint64_t)mod ^ 
           (uint64_t)(__real__ cdiv) ^ (uint64_t)(__imag__ cdiv) ^
           (uint64_t)ddiv;
}

/* Test 3: OpenMP target region triggering runtime synthesis */
NOOPT uint64_t test_omp_synthesis(uint64_t *data, int n) {
    volatile uint64_t sum = 0;
    
    #pragma omp target map(tofrom: sum) map(to: data[0:n])
    {
        #pragma omp teams distribute parallel for reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
        
        /* Additional builtin usage inside target region */
        #ifdef __x86_64__
        sum ^= __builtin_ia32_rdtsc();
        #endif
    }
    
    return sum;
}

/* Test 4: Transactional memory extensions */
NOOPT uint64_t test_tm_synthesis(uint64_t *ptr) {
    volatile uint64_t result = 0;
    
    #ifdef __TM_supported
    __transaction_atomic {
        *ptr += 1;
        result = *ptr;
        
        /* Nested synthesis triggers */
        __int128 tmp = (__int128)result * result;
        result ^= (uint64_t)tmp;
    }
    #endif
    
    return result;
}

/* Test 5: Mixed synthesis patterns */
NOOPT uint64_t test_mixed_synthesis(uint64_t seed) {
    volatile uint64_t result = seed;
    
    /* Atomic on 128-bit */
    __int128 atomic128 = seed;
    __atomic_fetch_add(&atomic128, (__int128)seed, __ATOMIC_SEQ_CST);
    result ^= (uint64_t)atomic128;
    
    /* CPU dispatch pattern */
    #ifdef __x86_64__
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx")) {
        result = __builtin_ia32_crc32di(result, result);
    } else if (__builtin_cpu_supports("sse4.2")) {
        result = __builtin_ia32_crc32si((unsigned)result, (unsigned)result);
    }
    #endif
    
    /* Complex arithmetic */
    volatile _Complex float cf = result + result * I;
    cf = cf / (cf + 1.0f);
    result ^= (uint64_t)(__real__ cf * 1000);
    
    return result;
}

int main(int argc, char **argv) {
    volatile uint64_t accumulator = 0;
    
    /* Initialize some data for OpenMP test */
    int n = 100;
    uint64_t *data = (uint64_t*)malloc(n * sizeof(uint64_t));
    for (int i = 0; i < n; i++) {
        data[i] = i + argc;
    }
    
    /* Run all synthesis tests */
    accumulator ^= test_builtin_synthesis();
    accumulator ^= test_libcall_synthesis(argc, accumulator);
    accumulator ^= test_omp_synthesis(data, n);
    
    uint64_t tm_var = 0;
    accumulator ^= test_tm_synthesis(&tm_var);
    accumulator ^= test_mixed_synthesis(accumulator);
    
    /* Ensure results are observable */
    printf("Synthesis test checksum: 0x%016lx\n", (unsigned long)accumulator);
    
    free(data);
    return (int)(accumulator & 0x7FFFFFFF);
}
