/* Test program to trigger target hook flags in targhooks.cc */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin main.c helper.c -o test_program */

#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

/* Inline function that will be used in multiple translation units */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This should trigger __divti3 helper generation */
    return a / b;
}

/* Function with nothrow attribute */
__attribute__((nothrow)) 
static void atomic_128bit_op(_Atomic __int128 *ptr, __int128 val) {
    /* Atomic operation on 128-bit type */
    __int128 expected = *ptr;
    __int128 desired;
    do {
        desired = expected + val;
    } while (!atomic_compare_exchange_strong(ptr, &expected, desired));
}

/* Function to prevent dead code elimination */
static void print_hash(__int128 value) {
    unsigned long long hi = (unsigned long long)(value >> 64);
    unsigned long long lo = (unsigned long long)value;
    printf("Result hash: 0x%016llx%016llx\n", hi, lo);
}

int main(void) {
    volatile __int128 volatile_var = 100;
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 1000;
    _Atomic __int128 atomic_var = 0;
    
    /* 1. 128-bit division operation - triggers helper generation */
    __int128 result = do_128bit_division(a, b);
    
    /* 2. Use volatile variable in computation */
    result += volatile_var;
    
    /* 3. Atomic operation on 128-bit type */
    atomic_128bit_op(&atomic_var, result);
    
    /* 4. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation inside target region */
        result = result * 2 - 1;
    }
    #endif
    
    /* 5. Additional complex 128-bit operations */
    __int128 c = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    __int128 d = do_128bit_division(c, b);
    
    /* 6. Modulo operation - may trigger __modti3 helper */
    __int128 mod_result = c % b;
    
    /* 7. Multiplication - may trigger __multi3 helper */
    __int128 mul_result = result * d;
    
    /* Combine results to prevent optimization */
    __int128 final_result = atomic_var + result + d + mod_result + mul_result;
    
    /* Print hash to prevent dead code elimination */
    print_hash(final_result);
    
    return 0;
}
