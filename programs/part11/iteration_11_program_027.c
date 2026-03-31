#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
extern void sink_ptr(const void*);

// Force no inlining to isolate patterns
#define NOINLINE __attribute__((noinline))

// Global volatile variables to prevent constant folding
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

// Volatile arrays for memory operands
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[2] = {0.0L, 1.0L};

// Struct with volatile members
struct volatile_fp {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct volatile_fp fp_struct = {0.0f, 0.0, 0.0L};

// Pattern 1: UNORDERED and ORDERED comparisons
NOINLINE int test_unordered_ordered(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile float f1 = farr[0];
    volatile long double ld1 = ldarr[0];
    
    int result = 0;
    
    // UNORDERED: using isunordered()
    if (isunordered(d1, d2)) {
        result |= 1;
    }
    
    // UNORDERED: direct NaN comparison
    if (d1 != d1) {  // NaN != NaN is true
        result |= 2;
    }
    
    // ORDERED: using isordered()
    if (isordered(g_one, g_two)) {
        result |= 4;
    }
    
    // ORDERED: direct comparison of normal numbers
    if (g_one == g_one) {  // Normal == Normal is true
        result |= 8;
    }
    
    // Mixed types with memory operands
    if (isunordered(fp_struct.d, darr[1])) {
        result |= 16;
    }
    
    if (isordered(farr[0], farr[1])) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

// Pattern 2: UNEQ (unordered or equal)
NOINLINE int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    
    int result = 0;
    
    // Using inline assembly to force condition code
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ordered_eq]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ordered_eq
    );
    
    // Fall through for unordered
    result = 1;
    goto done;
    
ordered_eq:
    // Equal case
    result = 2;
    
done:
    // Another UNEQ pattern: !(a != b) which is a == b OR unordered
    if (!(a != b)) {
        result |= 4;
    }
    
    // Normal numbers equal
    if (!(c != b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

// Pattern 3: UNGE (not less than) = !(a < b)
NOINLINE int test_unge(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    // Direct UNGE: !(x < y)
    if (!(x < y)) {  // 1 < 2 is true, so !(true) = false
        result |= 1;
    }
    
    // UNGE with NaN
    if (!(nan < y)) {  // NaN < 2 is false, so !(false) = true
        result |= 2;
    }
    
    // Using inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[not_less]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : not_less
    );
    
    result |= 4;
    goto done2;
    
not_less:
    result |= 8;
    
done2:
    // Memory operand version
    volatile double* px = &darr[0];
    volatile double* py = &darr[1];
    if (!(*px < *py)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

// Pattern 4: UNGT (not less or equal) = !(a <= b)
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double nan = g_nan;
    
    int result = 0;
    
    // Direct UNGT: !(a <= b)
    if (!(a <= b)) {  // 2 <= 1 is false, so !(false) = true
        result |= 1;
    }
    
    // With NaN
    if (!(nan <= b)) {  // NaN <= 1 is false, so !(false) = true
        result |= 2;
    }
    
    // Complex expression
    result += ((!(a <= b)) && (b == b)) ? 4 : 0;
    
    // Using struct member
    if (!(fp_struct.d <= darr[0])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

// Pattern 5: UNLE (unordered or less or equal)
NOINLINE int test_unle(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    // UNLE: !(a > b)
    if (!(x > y)) {  // 1 > 2 is false, so !(false) = true
        result |= 1;
    }
    
    if (!(nan > y)) {  // NaN > 2 is false, so !(false) = true
        result |= 2;
    }
    
    // Inline assembly forcing condition code
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[not_greater]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : not_greater
    );
    
    result |= 4;
    goto done3;
    
not_greater:
    result |= 8;
    
done3:
    sink(result);
    return result;
}

// Pattern 6: UNLT (unordered or less than)
NOINLINE int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    // UNLT: !(a >= b)
    if (!(a >= b)) {  // 1 >= 2 is false, so !(false) = true
        result |= 1;
    }
    
    if (!(nan >= b)) {  // NaN >= 2 is false, so !(false) = true
        result |= 2;
    }
    
    // Complex nested expression
    int temp = (!(a >= b)) ? 4 : 0;
    temp += (!(b >= a)) ? 8 : 0;
    result |= temp;
    
    // Memory operand
    if (!(darr[0] >= darr[2])) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

// Pattern 7: LTGT (not equal and ordered) = (a < b) || (a > b)
NOINLINE int test_ltgt(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile double nan = g_nan;
    volatile double same = g_one;
    
    int result = 0;
    
    // LTGT: (x < y) || (x > y)
    if ((x < y) || (x > y)) {  // 1 < 2 is true, so true
        result |= 1;
    }
    
    // With NaN - should be false
    if ((nan < y) || (nan > y)) {  // Both comparisons false with NaN
        result |= 2;
    }
    
    // Equal numbers - should be false
    if ((x < same) || (x > same)) {  // Both false
        result |= 4;
    }
    
    // Complex expression mixing types
    volatile float fx = farr[0];
    volatile float fy = farr[1];
    result += ((fx < fy) || (fx > fy)) ? 8 : 0;
    
    // Using inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ltgt_label
    );
    
    result |= 16;
    goto done4;
    
ltgt_label:
    result |= 32;
    
done4:
    sink(result);
    return result;
}

// Pattern 8: Mixed condition codes in complex expressions
NOINLINE int test_mixed_conditions(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    volatile double d = g_zero;
    
    int result = 0;
    
    // Complex ternary with multiple comparisons
    result = (a != a) ? 1 : 
             ((!(b < c)) ? 2 : 
             ((!(c > d)) ? 3 : 
             ((b < c) || (b > c)) ? 4 : 5));
    
    // Nested logical expressions
    if ((!(a >= b)) && (isunordered(a, b) || (!(c <= d)))) {
        result |= 8;
    }
    
    // Multiple condition codes combined
    int r1 = (!(b >= c)) ? 16 : 0;
    int r2 = (!(c <= b)) ? 32 : 0;
    int r3 = ((b < c) || (b > c)) ? 64 : 0;
    
    result |= r1 | r2 | r3;
    
    // Memory operands in complex expression
    volatile double* pa = &darr[0];
    volatile double* pb = &darr[1];
    volatile double* pc = &darr[2];
    
    if ((!(*pa >= *pb)) || ((*pb < *pc) || (*pb > *pc))) {
        result |= 128;
    }
    
    sink(result);
    return result;
}

// Pattern 9: Long double comparisons
NOINLINE int test_long_double(void) {
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    volatile long double ldnan = g_nan;
    
    int result = 0;
    
    // Various condition codes with long double
    if (isunordered(ldnan, ld1)) {
        result |= 1;
    }
    
    if (!(ld1 >= ld2)) {
        result |= 2;
    }
    
    if (!(ld2 <= ld1)) {
        result |= 4;
    }
    
    if ((ld1 < ld2) || (ld1 > ld2)) {
        result |= 8;
    }
    
    // Inline assembly with long double
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ld_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ld_label
    );
    
    result |= 16;
    goto done5;
    
ld_label:
    result |= 32;
    
done5:
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    // Initialize some NaN values
    volatile double local_nan = 0.0/0.0;
    volatile float local_nanf = 0.0f/0.0f;
    sink_ptr(&local_nan);
    sink_ptr(&local_nanf);
    
    // Call all test functions
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_conditions();
    checksum += test_long_double();
    
    // Print checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

// Dummy sink functions to prevent optimization
void sink(int x) {
    volatile static int buffer;
    buffer = x;
}

void sink_ptr(const void* p) {
    volatile static const void* buffer;
    buffer = p;
}
