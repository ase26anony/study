/* Test program to trigger target hook flags in targhooks.cc */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin test_targhooks.c test_targhooks_aux.c -o test_targhooks */

#include <stdio.h>
#include <stdint.h>

/* Inline function to force helper generation across TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This should trigger __divti3 helper generation */
    return a / b;
}

/* Function with nothrow attribute */
__attribute__((nothrow)) 
static void atomic_update(__int128 *ptr, __int128 val) {
    /* Atomic operation on 128-bit */
    __int128 expected = *ptr;
    __int128 desired;
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

/* OpenMP target region using 128-bit */
#ifdef _OPENMP
#pragma omp declare target
__int128 target_var = 0;
#pragma omp end declare target
#endif

int main() {
    volatile __int128 volatile_128 = 100;
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = ((__int128)0x1 << 64) | 0x1;
    __int128 result = 0;
    
    /* Division triggering helper generation */
    for (int i = 0; i < 10; i++) {
        result = do_128bit_division(a + i, b + i);
        volatile_128 = result;  /* Use volatile */
    }
    
    /* Atomic operation */
    __int128 atomic_var = 0;
    atomic_update(&atomic_var, result);
    
    /* OpenMP target region */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var)
    {
        target_var = atomic_var;
        atomic_var = target_var / 2;
    }
    #endif
    
    /* Modulo operation (another helper) */
    __int128 mod_result = a % b;
    
    /* Multiplication (yet another helper) */
    __int128 mul_result = a * b;
    
    /* Print hash to prevent optimization */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    uint64_t hash = high ^ low ^ (uint64_t)(mod_result >> 64) ^ (uint64_t)mod_result;
    
    printf("Result hash: 0x%016lx\n", hash);
    
    return 0;
}
