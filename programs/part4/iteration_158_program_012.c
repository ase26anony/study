/* test_double_int_cmp.c
 * Compile with: gcc -O3 -fdump-tree-all -c test_double_int_cmp.c -o test.o
 */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to use wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function to force comparison in different contexts */
int compare_int128(int128_t a, int128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int compare_uint128(uint128_t a, uint128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Test case 1: High part comparisons (unsigned) */
void test_high_part_comparisons(void) {
    /* Create values where high parts differ */
    uint128_t a = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    uint128_t b = ((uint128_t)0x123456789ABCDEF1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    uint128_t c = ((uint128_t)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL;
    
    /* Exercise high part less */
    sink = compare_uint128(a, b);  /* high(a) < high(b) */
    
    /* Exercise high part greater */
    sink = compare_uint128(b, a);  /* high(b) > high(a) */
    
    /* Exercise high part equal, low part less */
    sink = compare_uint128(a, c);  /* high equal, low(a) < low(c) */
    
    /* Exercise high part equal, low part greater */
    sink = compare_uint128(c, a);  /* high equal, low(c) > low(a) */
}

/* Test case 2: Low part comparisons when high parts are zero */
void test_low_part_only(void) {
    uint128_t small1 = 0x1111111111111111ULL;
    uint128_t small2 = 0x2222222222222222ULL;
    
    sink = compare_uint128(small1, small2);  /* low part less */
    sink = compare_uint128(small2, small1);  /* low part greater */
    sink = compare_uint128(small1, small1);  /* equality */
}

/* Test case 3: Mixed signed/unsigned comparisons */
void test_mixed_signedness(void) {
    /* Signed negative value (high bit set) */
    int128_t s1 = -((int128_t)1 << 120);
    /* Large unsigned value */
    uint128_t u1 = ((uint128_t)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* This should trigger unsigned comparison of high parts */
    if (s1 < u1) {
        sink = 1;
    } else {
        sink = 0;
    }
}

/* Test case 4: Array indexing with wide integers */
void test_array_indexing(void) {
    char buffer[256];
    
    /* Loop with wide integer counter */
    for (uint128_t i = ((uint128_t)0x1ULL << 64) | 0x0ULL; 
         i < ((uint128_t)0x1ULL << 64) | 0x10ULL; 
         i++) {
        /* Bounds check requires comparison */
        if (i < 256) {
            buffer[i] = (char)i;
        }
    }
    
    /* Use as array index with comparison */
    uint128_t idx = ((uint128_t)0x1ULL << 64) | 0x5ULL;
    if (idx < 256) {
        sink = buffer[idx];
    }
}

/* Test case 5: Switch statement with wide constants */
void test_switch_wide(void) {
    uint128_t val = ((uint128_t)0x12345678ULL << 64) | 0x9ABCDEF0ULL;
    int result = 0;
    
    /* Switch on comparison result */
    switch (compare_uint128(val, ((uint128_t)0x12345678ULL << 64) | 0x9ABCDEF1ULL)) {
        case -1: result = 1; break;  /* less than */
        case 0:  result = 2; break;  /* equal */
        case 1:  result = 3; break;  /* greater than */
    }
    sink = result;
}

/* Test case 6: Compiler builtins with overflow */
void test_builtins(void) {
    uint128_t a = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    uint128_t b = 1;
    uint128_t result;
    
    /* Overflow check requires comparison */
    if (__builtin_add_overflow(a, b, &result)) {
        sink = 1;  /* overflow occurred */
    }
    
    /* Multiplication with potential overflow */
    uint128_t c = ((uint128_t)0x8000000000000000ULL << 64);
    if (__builtin_mul_overflow(c, 2, &result)) {
        sink = 2;
    }
}

/* Test case 7: Ternary operator with wide comparison */
void test_ternary(void) {
    uint128_t x = ((uint128_t)0x1ULL << 64);
    uint128_t y = ((uint128_t)0x2ULL << 64);
    
    /* Ternary condition requires comparison */
    uint128_t z = (x < y) ? x : y;
    sink = (int)z;
}

/* Test case 8: Constant folding scenarios */
void test_constant_folding(void) {
    /* Compile-time constants that should be folded */
    const uint128_t c1 = ((uint128_t)0xDEADBEEFULL << 64) | 0xCAFEBABEULL;
    const uint128_t c2 = ((uint128_t)0xDEADBEEFULL << 64) | 0xCAFEBABFULL;
    const uint128_t c3 = ((uint128_t)0xDEADBEE0ULL << 64) | 0xFFFFFFFFULL;
    
    /* These comparisons should be constant-folded */
    sink = (c1 < c2) ? 1 : 0;    /* high equal, low less */
    sink = (c2 > c1) ? 1 : 0;    /* high equal, low greater */
    sink = (c3 < c1) ? 1 : 0;    /* high less */
    sink = (c1 > c3) ? 1 : 0;    /* high greater */
    sink = (c1 == c1) ? 1 : 0;   /* equality */
}

/* Test case 9: Arithmetic operations that produce comparison */
void test_arithmetic_comparison(void) {
    uint128_t base = ((uint128_t)0x1000ULL << 64);
    
    /* Range checking after arithmetic */
    for (uint64_t i = 0; i < 100; i++) {
        uint128_t val = base + i;
        
        /* Multiple comparison points */
        if (val < base + 50) {
            sink = 1;  /* less than midpoint */
        } else if (val > base + 50) {
            sink = 2;  /* greater than midpoint */
        } else {
            sink = 3;  /* equal to midpoint */
        }
    }
}

/* Main function combining all test cases */
int main(void) {
    test_high_part_comparisons();      /* Exercises all 4 comparison outcomes */
    test_low_part_only();              /* Exercises low-part comparisons */
    test_mixed_signedness();           /* Exercises signed/unsigned conversion */
    test_array_indexing();             /* Exercises bounds checking */
    test_switch_wide();                /* Exercises switch lowering */
    test_builtins();                   /* Exercises builtin overflow checks */
    test_ternary();                    /* Exercises ternary operator */
    test_constant_folding();           /* Exercises compile-time folding */
    test_arithmetic_comparison();      /* Exercises runtime comparisons */
    
    return 0;
}
