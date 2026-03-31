/* test_fixed_value.c - Test program for GCC fixed-value.cc coverage */
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

/* Force compiler to consider all code paths */
static volatile int sink;

/* Helper to prevent optimization */
static void use(void *p) {
    sink = *(int*)p;
}

int main(void) {
    /* 1. Large integer arithmetic with overflow/underflow */
    /* Use __int128 operations that require double-int representation */
    __int128 a = (__int128)LLONG_MAX * 4;  /* Exceeds 64-bit range */
    __int128 b = (__int128)LLONG_MIN * 3;
    unsigned __int128 c = (unsigned __int128)ULLONG_MAX * 2;
    
    /* 2. Complex comparisons mimicking a_high.sgt(max_r) logic */
    /* Compare high parts of 128-bit values */
    if (a > ((__int128)LLONG_MAX << 32)) {
        sink = 1;
    }
    
    /* Chain comparisons: high part equal, then compare low part */
    __int128 val1 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 32) | 0xFFFFFFFF;
    __int128 val2 = ((__int128)0x7FFFFFFFFFFFFFFFLL << 32) | 0xFFFFFFFE;
    
    if ((val1 >> 64) > (val2 >> 64) || 
        ((val1 >> 64) == (val2 >> 64) && 
         ((unsigned long long)val1 > (unsigned long long)val2))) {
        sink = 2;
    }
    
    /* 3. Loop bounds with complex exit conditions */
    long long limit = LLONG_MAX / 4;
    long long factor = 3;
    
    /* Loop where exit condition requires range analysis */
    for (long long i = 0; i < limit * factor; i += (limit / 1000)) {
        /* Nested loop with different type to force range merging */
        for (int j = 0; j < 10; j++) {
            /* Operations that might overflow */
            __int128 product = (__int128)i * j;
            if (product > ((__int128)LLONG_MAX >> 2)) {
                sink = 3;
                break;
            }
        }
        if (i > limit) break; /* Prevent infinite loop */
    }
    
    /* 4. Bit-field operations and masking */
    unsigned __int128 wide_mask = ~((unsigned __int128)0);
    unsigned __int128 shifted = wide_mask >> 63;
    
    /* Extract bit-fields requiring double-int analysis */
    int shift_amount = 65; /* More than 64 bits */
    unsigned __int128 masked = (c >> shift_amount) & 0x1FFFFFFFFULL;
    
    /* Compare masked value against boundary */
    if (masked > 0xFFFFFFFFULL) {
        sink = 4;
    }
    
    /* 5. Compiler built-ins for overflow checking */
    long long x = LLONG_MAX;
    long long y = 2;
    long long result;
    
    /* Use overflow builtins that may invoke fixed-value logic */
    if (__builtin_mul_overflow(x, y, &result)) {
        sink = 5;
    }
    
    /* 128-bit overflow check */
    __int128 big_x = ((__int128)LLONG_MAX << 10);
    __int128 big_y = 1024;
    __int128 big_result = big_x * big_y;
    
    /* Complex comparison chain similar to uncovered code */
    /* This should trigger a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
    __int128 boundary = ((__int128)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    if (big_result > boundary ||
        (big_result == boundary && (unsigned __int128)big_result > 0)) {
        sink = 6;
    }
    
    /* 6. Mixed signed/unsigned comparisons */
    unsigned long long high_part = 0x8000000000000000ULL;
    long long signed_low = -1;
    
    /* Create implicit double-int comparison */
    if ((high_part > 0x7FFFFFFFFFFFFFFFULL) && 
        ((signed)high_part > 0 || signed_low > 0)) {
        sink = 7;
    }
    
    /* 7. Division with large divisors - forces range analysis */
    __int128 dividend = ((__int128)LLONG_MAX << 32);
    __int128 divisor = 3;
    __int128 quotient = dividend / divisor;
    
    if (quotient > ((__int128)LLONG_MAX << 30)) {
        sink = 8;
    }
    
    /* 8. Modulo operation near boundaries */
    unsigned __int128 mod_val = (unsigned __int128)ULLONG_MAX * 3;
    unsigned __int128 mod_result = mod_val % 0x10000000000000000ULL;
    
    if (mod_result > 0xFFFFFFFFFFFFFFFFULL) {
        sink = 9;
    }
    
    /* 9. Shift operations that might overflow */
    __int128 left_shifted = (__int128)1 << 120;  /* Near maximum */
    __int128 right_shifted = left_shifted >> 60;
    
    if (right_shifted > (LLONG_MAX >> 10)) {
        sink = 10;
    }
    
    /* 10. Final checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    checksum ^= (unsigned long long)a;
    checksum ^= (unsigned long long)b;
    checksum ^= (unsigned long long)c;
    checksum ^= (unsigned long long)wide_mask;
    checksum ^= (unsigned long long)big_result;
    checksum ^= (unsigned long long)quotient;
    checksum ^= (unsigned long long)mod_result;
    
    printf("Checksum: %llu\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
