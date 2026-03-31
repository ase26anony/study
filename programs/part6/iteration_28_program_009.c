/* test_fixed_value.c - Test program for GCC fixed-value range analysis */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function with loop where index may reach maximum value */
__attribute__((noinline))
void test_high_part_greater_than_max(int n) {
    /* This should trigger a_high.sgt(max_r) when n is negative */
    for (int i = 0; i < n; i++) {
        /* Complex exit condition to force range analysis */
        if (i * i < 0) { /* Always false, but compiler doesn't know */
            break;
        }
        global_counter += i;
    }
}

/* Function targeting a_high == 0 && a_low > max_s */
__attribute__((noinline))
void test_low_part_greater_than_max_s(unsigned long long limit) {
    /* Create value with high part = 0, low part > max_s */
    /* max_s after zext(i_f_bits) becomes a large unsigned value */
    
    /* Use bitwise operations to create specific patterns */
    unsigned long long mask = ~0ULL;
    unsigned long long value = 0;
    
    /* Build a value that might exceed max_s in low part */
    for (unsigned long long i = 0; i < limit; i++) {
        /* Shift operations that might create interesting ranges */
        value = (value << 1) | 1;
        
        /* Conditional that depends on value range */
        if (value > (mask >> 2)) {
            /* This branch should be taken for some values */
            global_counter++;
        }
    }
}

/* Function with arithmetic near power-of-two boundaries */
__attribute__((noinline, const))
unsigned long long near_power_of_two(unsigned bits) {
    /* Create values just below/above power-of-two boundaries */
    unsigned long long max_val = 1ULL << bits;
    unsigned long long near_max = max_val - 1;
    
    /* Operations that might produce zero high part but large low part */
    unsigned long long result = near_max;
    
    /* Multiply to potentially exceed the boundary */
    result = result * 2;  /* This overflows for some bit widths */
    
    return result;
}

/* Function with signed/unsigned mixing */
__attribute__((noinline))
void test_mixed_signed_unsigned(int start, unsigned increment) {
    int val = start;
    
    /* Loop with mixed signed/unsigned arithmetic */
    for (unsigned i = 0; i < 100; i++) {
        val += increment;
        
        /* Comparison that might trigger range analysis */
        if (val > INT_MAX / 2) {
            global_counter += val;
        }
    }
}

/* Function using 128-bit integers (if available) */
#ifdef __SIZEOF_INT128__
__attribute__((noinline))
void test_128bit_range(void) {
    __int128 large_val = ((__int128)1 << 64) - 1;
    __int128 result = large_val * 2;
    
    /* This should create interesting range for fixed-value analysis */
    if (result > 0) {
        global_counter += (unsigned long)result;
    }
}
#endif

/* Function with complex exit conditions */
__attribute__((noinline))
int test_complex_exit(int n, int m) {
    int sum = 0;
    
    /* Nested loops with complex bounds */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += i * j;
            
            /* Early exit based on complex condition */
            if (sum > 1000 && i > 50) {
                return sum;
            }
        }
    }
    
    return sum;
}

/* Function that creates value with zero high part but large low part */
__attribute__((noinline))
unsigned long long create_zero_high_large_low(unsigned shift) {
    /* Create a value where high part is 0 but low part is large */
    unsigned long long val = ~0ULL;
    
    /* Right shift to clear high bits, then left shift to make low part large */
    val = val >> (64 - shift);
    val = val << (shift - 1);
    
    return val;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Test various scenarios to trigger different paths */
    
    /* 1. Test a_high.sgt(max_r) path - negative loop bound */
    test_high_part_greater_than_max(volatile_var - 10);
    
    /* 2. Test a_high == 0 && a_low > max_s path */
    /* Use values that create zero high part but large low part */
    for (int bits = 32; bits <= 63; bits += 8) {
        unsigned long long val = create_zero_high_large_low(bits);
        test_low_part_greater_than_max_s(val & 0xFF);
    }
    
    /* 3. Test near power-of-two boundaries */
    for (unsigned bits = 30; bits < 62; bits += 4) {
        unsigned long long result = near_power_of_two(bits);
        printf("Near pow2(%u): %llu\n", bits, result);
    }
    
    /* 4. Test mixed signed/unsigned */
    test_mixed_signed_unsigned(0, 1000000);
    test_mixed_signed_unsigned(INT_MAX - 100, 1);
    
    /* 5. Test complex exit conditions */
    int complex_result = test_complex_exit(100, 100);
    printf("Complex result: %d\n", complex_result);
    
    #ifdef __SIZEOF_INT128__
    /* 6. Test 128-bit integers if available */
    test_128bit_range();
    #endif
    
    /* 7. Additional test with bitwise operations */
    unsigned long long x = 0xFFFFFFFFFFFFFFFFULL;
    for (int i = 0; i < 10; i++) {
        x = (x >> 1) | (x << 63);  /* Rotate right */
        if (x > 0x8000000000000000ULL) {
            global_counter++;
        }
    }
    
    printf("Global counter: %lu\n", global_counter);
    
    return 0;
}
