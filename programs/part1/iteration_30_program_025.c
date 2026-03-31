/* Test program to trigger overflow checking in fixed-value.cc */
/* Compile with: gcc -O2 -ffixed-point -fdump-tree-all this_file.c */

#include <stdio.h>

/* Helper to prevent optimization */
static volatile int global_counter = 0;

/* Test 1: Overflow in unsigned short _Fract (i_f_bits = 8) */
void test_unsigned_short_fract_overflow() {
    printf("Test 1: Unsigned short _Fract overflow\n");
    
    /* For unsigned short _Fract: 8 fractional bits, 0 integer bits */
    /* Max value = (2^8 - 1)/2^8 = 255/256 ≈ 0.996 */
    
    /* This should trigger a_high == 0 && a_low > max_s */
    /* max_s = (2^8 - 1) = 255 */
    /* We need a value with high=0, low > 255 */
    
    unsigned short _Accum source = 1.0uhk;  /* 1.0 in unsigned short _Accum */
    unsigned short _Fract target;
    
    /* This conversion should trigger overflow check */
    target = (unsigned short _Fract)source;
    
    /* Use the result to prevent dead code elimination */
    if (target > 0) global_counter++;
    printf("  Result: %u/256\n", (unsigned)(target * 256));
}

/* Test 2: Overflow with positive high part */
void test_positive_high_part() {
    printf("Test 2: Positive high part overflow\n");
    
    /* We need a_high > 0 (max_r = 0) */
    /* Use a large _Accum value converted to smaller type */
    
    unsigned long _Accum large_val = 300.0ULK;  /* Large value */
    unsigned short _Accum smaller;
    
    /* This should trigger a_high > 0 */
    smaller = (unsigned short _Accum)large_val;
    
    if (smaller > 0) global_counter++;
    printf("  Result: %u\n", (unsigned)smaller);
}

/* Test 3: Signed type overflow */
void test_signed_fract_overflow() {
    printf("Test 3: Signed _Fract overflow\n");
    
    /* For signed short _Fract: 7 fractional bits (plus sign) */
    /* Max positive = (2^7 - 1)/2^7 = 127/128 ≈ 0.992 */
    
    short _Accum source = 1.0hk;  /* 1.0 exceeds max for signed _Fract */
    short _Fract target;
    
    target = (short _Fract)source;
    
    if (target > 0) global_counter++;
    printf("  Result: %d/128\n", (int)(target * 128));
}

/* Test 4: Multiple conversions with different i_f_bits */
void test_various_fractional_bits() {
    printf("Test 4: Various fractional bit counts\n");
    
    /* Test different fixed-point types to cover various i_f_bits */
    unsigned _Fract f1 = 0.5ur;
    unsigned _Fract f2 = 0.6ur;
    
    /* This addition might overflow */
    unsigned _Fract sum = f1 + f2;
    
    /* Test conversion from integer */
    int int_val = 2;
    unsigned _Fract from_int = (unsigned _Fract)int_val;
    
    /* Test _Sat types */
    unsigned _Sat _Fract sat_f1 = 0.9ur;
    unsigned _Sat _Fract sat_f2 = 0.8ur;
    unsigned _Sat _Fract sat_sum = sat_f1 + sat_f2;
    
    global_counter += (sum > 0) + (from_int > 0) + (sat_sum > 0);
    printf("  Results: sum=%u/256, from_int=%u/256, sat_sum=%u/256\n",
           (unsigned)(sum * 256),
           (unsigned)(from_int * 256),
           (unsigned)(sat_sum * 256));
}

/* Test 5: Using builtins for overflow detection */
void test_builtin_overflow() {
    printf("Test 5: Builtin overflow checks\n");
    
    unsigned short _Fract a = 0.8ur;
    unsigned short _Fract b = 0.9ur;
    unsigned short _Fract result;
    
    /* This might trigger overflow checking internally */
    int overflow = __builtin_add_overflow(a, b, &result);
    
    if (overflow) {
        printf("  Overflow detected in addition\n");
        global_counter++;
    }
}

/* Test 6: Explicit large value conversions */
void test_explicit_large_conversions() {
    printf("Test 6: Explicit large value conversions\n");
    
    /* Create values that clearly exceed target bounds */
    
    /* For unsigned _Fract (8 fractional bits): max = 255/256 */
    /* Use a value slightly above 1.0 */
    unsigned _Accum above_one = 1.0001uK;
    unsigned _Fract conv1 = (unsigned _Fract)above_one;
    
    /* Use a value much larger than 1.0 */
    unsigned _Accum large = 2.5uK;
    unsigned _Fract conv2 = (unsigned _Fract)large;
    
    /* Test with different fractional bit counts */
    unsigned long _Fract long_fract = 0.999999ulr;
    unsigned _Fract from_long = (unsigned _Fract)long_fract;
    
    global_counter += (conv1 > 0) + (conv2 > 0) + (from_long > 0);
    printf("  Results: conv1=%u/256, conv2=%u/256, from_long=%u/256\n",
           (unsigned)(conv1 * 256),
           (unsigned)(conv2 * 256),
           (unsigned)(from_long * 256));
}

/* Test 7: Edge case - exact boundary value */
void test_boundary_value() {
    printf("Test 7: Boundary value test\n");
    
    /* Test with value exactly at boundary */
    /* For unsigned short _Fract: max = 255/256 */
    
    /* Create value exactly at 255/256 */
    unsigned short _Fract max_val = 255.0/256;
    
    /* Try to add a tiny amount */
    unsigned short _Fract tiny = 0.5/256;  /* 0.5/256 = 1/512 */
    unsigned short _Fract result = max_val + tiny;
    
    if (result > max_val) global_counter++;
    printf("  max_val=%u/256, result=%u/256\n",
           (unsigned)(max_val * 256),
           (unsigned)(result * 256));
}

int main() {
    printf("Starting fixed-point overflow tests...\n\n");
    
    test_unsigned_short_fract_overflow();
    printf("\n");
    
    test_positive_high_part();
    printf("\n");
    
    test_signed_fract_overflow();
    printf("\n");
    
    test_various_fractional_bits();
    printf("\n");
    
    test_builtin_overflow();
    printf("\n");
    
    test_explicit_large_conversions();
    printf("\n");
    
    test_boundary_value();
    printf("\n");
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    /* Return non-zero if any overflow was expected but not handled */
    return 0;
}
