/* test_fixed_value.c - Test program for GCC fixed-value range analysis */
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
    const long long max_ll = LLONG_MAX;
    const long long min_ll = LLONG_MIN;
    const unsigned long long max_ull = ULLONG_MAX;
    
    /* 128-bit arithmetic that requires double-int representation */
    __int128 a = (__int128)max_ll * 2;               /* Will overflow signed 64-bit */
    __int128 b = (__int128)max_ll * max_ll;          /* Large positive */
    __int128 c = (__int128)min_ll * 2;               /* Large negative */
    unsigned __int128 d = (unsigned __int128)max_ull * 3; /* Unsigned overflow */
    
    /* 2. Complex expressions for loop bounds */
    long long limit1 = (max_ll / 4) * 3;
    long long limit2 = (min_ll / 4) * 3;
    
    /* Loop with bounds near overflow */
    for (long long i = max_ll - 1000; i < max_ll - 500; i++) {
        /* 3. Conditional branches based on wide comparisons */
        __int128 prod = (__int128)i * i;
        
        /* This mimics: if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))) */
        if (prod > b) {
            sink = 1;
        }
        
        /* Chain comparisons */
        if (prod >= 0 && prod <= (__int128)max_ll * max_ll) {
            sink = 2;
        }
        
        /* Unsigned comparison with wrapping */
        unsigned __int128 uprod = (unsigned __int128)(i > 0 ? i : -i);
        if (uprod > d) {
            sink = 3;
        }
    }
    
    /* 4. Bit-field operations and masking */
    unsigned __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 mask_high = mask_low << 64;
    unsigned __int128 combined = mask_high | 0x8000000000000000ULL;
    
    for (int shift = 0; shift < 128; shift += 16) {
        unsigned __int128 shifted = combined >> shift;
        
        /* Comparisons that might trigger the specific double-int logic */
        if (shifted > mask_low) {
            sink = 4;
        }
        
        if ((shifted & mask_high) != 0) {
            sink = 5;
        }
    }
    
    /* 5. Compiler built-ins for overflow checking */
    long long x = max_ll - 100;
    long long y = 10;
    long long result;
    
    /* These built-ins may invoke fixed-value logic */
    if (__builtin_add_overflow(x, y, &result)) {
        sink = 6;
    }
    
    if (__builtin_mul_overflow(x, y, &result)) {
        sink = 7;
    }
    
    /* Complex nested loops with different types */
    for (int i = 0; i < 100; i++) {
        for (long long j = limit2; j < limit1; j += 1000000000000LL) {
            __int128 prod = (__int128)i * j;
            
            /* Multiple comparison forms */
            if (prod > a || prod < c) {
                sink = 8;
            }
            
            if (prod == 0) {
                sink = 9;
            }
            
            /* Simulate the exact condition structure from uncovered lines */
            unsigned __int128 uprod2 = (unsigned __int128)prod;
            if (prod > 0) {
                /* This structure mimics: high > max || (high == max && low > max_s) */
                if (uprod2 > (unsigned __int128)max_ll * 2) {
                    sink = 10;
                }
            }
        }
    }
    
    /* Additional boundary tests */
    __int128 boundary_cases[] = {
        (__int128)max_ll << 32,
        (__int128)min_ll << 32,
        (__int128)max_ull,
        -((__int128)max_ull >> 1)
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            __int128 cmp_result = boundary_cases[i] + boundary_cases[j];
            
            /* Various comparison patterns */
            if (cmp_result > boundary_cases[0]) {
                sink = 11 + i;
            }
            
            if (cmp_result < boundary_cases[1]) {
                sink = 12 + i;
            }
            
            /* Equality with subsequent unsigned comparison */
            if (cmp_result == boundary_cases[2]) {
                unsigned __int128 ucmp = (unsigned __int128)cmp_result;
                if (ucmp > (unsigned __int128)boundary_cases[3]) {
                    sink = 13 + i;
                }
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", sink);
    
    return 0;
}
