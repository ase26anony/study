/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create inter-procedural data flow */
static unsigned long global_counter = 0;
static __int128 large_value = 0;

/* Function with loop that may trigger a_high.sgt(max_r) */
void test_high_part_greater_zero(int n) {
    /* Loop with induction variable that may have high part > 0 */
    for (long long i = 0; i < n; i++) {
        /* Create value that when analyzed as fixed-point might have high part > 0 */
        __int128 val = (__int128)i * (1LL << 60);
        
        /* Comparison that forces range analysis */
        if (val > (__int128)(1LL << 62)) {
            global_counter++;
        }
    }
}

/* Function targeting a_high == 0 && a_low.ugt(max_s) */
void test_zero_high_large_low(unsigned int bits) {
    /* Create value with high part = 0, low part large */
    unsigned long long mask = (1ULL << bits) - 1;
    
    /* Value that just exceeds the mask when bits = i_f_bits */
    unsigned long long val = mask + 1;
    
    /* Operations that might be analyzed as fixed-point */
    unsigned long long scaled = val << 2;
    
    /* Complex comparison to engage range analysis */
    if (scaled > mask && (scaled & mask) == 0) {
        global_counter += 2;
    }
}

/* Function with bitwise operations near boundaries */
void test_bit_boundary(unsigned int shift) {
    unsigned long long x = 1ULL << shift;
    unsigned long long y = x - 1;
    
    /* These operations create values at bit boundaries */
    unsigned long long a = x | y;      /* All lower bits set plus the high bit */
    unsigned long long b = a + 1;      /* Crosses power-of-two boundary */
    
    /* Comparisons that require precise range tracking */
    if (b > x && (b & (b - 1)) == 0) {
        global_counter += 4;
    }
}

/* Function with multiplication that may overflow into high part */
void test_multiplication_overflow(int a, int b) {
    /* Multiplication that may produce high part > 0 */
    long long result = (long long)a * b;
    
    /* Check if multiplication overflowed beyond 32 bits */
    if (result > INT_MAX || result < INT_MIN) {
        global_counter += 8;
    }
}

/* Function using __builtin_expect to influence branch prediction */
void test_builtin_expect(int limit) {
    int i;
    for (i = 0; __builtin_expect(i < limit, 1); i++) {
        /* Create value that grows large */
        large_value += i;
        
        /* Comparison at boundary */
        if (large_value > (__int128)(1ULL << 63)) {
            global_counter += 16;
            break;
        }
    }
}

/* Pure function for aggressive propagation */
__attribute__((pure))
unsigned long long pure_range(unsigned long long x, unsigned long long y) {
    /* Operation that creates specific bit pattern */
    return (x & ~y) | (y & ~x);  /* XOR equivalent */
}

/* Main test driver */
int main() {
    /* Test 1: Values that may have high part > 0 */
    test_high_part_greater_zero(100);
    
    /* Test 2: Values with high part = 0 but large low part */
    /* Use 31 bits to match potential i_f_bits in the uncovered code */
    test_zero_high_large_low(31);
    
    /* Test 3: Bit boundary cases */
    test_bit_boundary(30);
    test_bit_boundary(31);
    test_bit_boundary(32);
    
    /* Test 4: Multiplication overflow cases */
    test_multiplication_overflow(0x10000, 0x10000);  /* 2^16 * 2^16 = 2^32 */
    test_multiplication_overflow(INT_MAX, 2);
    test_multiplication_overflow(INT_MIN, -2);
    
    /* Test 5: Builtin expect with large values */
    test_builtin_expect(1000);
    
    /* Test 6: Pure function with boundary values */
    unsigned long long x = pure_range(0xFFFFFFFFULL, 0x7FFFFFFFULL);
    if (x > 0x80000000ULL) {
        global_counter += 32;
    }
    
    /* Additional complex loop with nested conditions */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            /* Create value that grows and may trigger range analysis */
            __int128 val = (__int128)i * j * (1LL << 30);
            
            /* Multiple comparison points */
            if (val > (__int128)(1LL << 40)) {
                if ((val & ((1LL << 40) - 1)) == 0) {
                    global_counter++;
                }
            }
        }
    }
    
    printf("Global counter: %lu\n", global_counter);
    return 0;
}
