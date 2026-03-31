#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Function to generate NaN
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

// Function to generate infinity
static double make_inf(void) {
    volatile double large = 1e308;
    return large * large;
}

// UNORDERED/ORDERED patterns
__attribute__((noinline))
void test_unordered_ordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    volatile double inf = make_inf();
    
    int result = 0;
    
    // UNORDERED: using isunordered()
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    // UNORDERED: direct NaN comparison
    if (nan1 != nan1) {
        result |= 2;
    }
    
    // ORDERED: using isordered()
    if (isordered(normal, inf)) {
        result |= 4;
    }
    
    // ORDERED: direct comparison
    if (normal == normal) {
        result |= 8;
    }
    
    // Complex unordered check with inline asm
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unordered_label
    );
    
    // Not unordered path
    result |= 16;
    goto after_unordered;
    
unordered_label:
    result |= 32;
    
after_unordered:
    checksum += result;
    sink(result);
}

// UNEQ (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    volatile double a = make_nan();
    volatile double b = 2.0;
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    int result = 0;
    
    // UNEQ: !(a > b) && !(a < b) which is a == b || unordered
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    // UNEQ with normal equal values
    if (!(c > d) && !(c < d)) {
        result |= 2;
    }
    
    // Using inline asm to force condition code
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[uneq_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : uneq_label
    );
    
    result |= 4;
    goto after_uneq;
    
uneq_label:
    result |= 8;
    
after_uneq:
    checksum += result;
    sink(result);
}

// UNGE (not less than): !(a < b)
__attribute__((noinline))
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // UNGE: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // UNGE with NaN (always true for unordered)
    if (!(nan < a)) {
        result |= 2;
    }
    
    // Complex expression
    volatile double x = 7.0;
    volatile double y = 7.0;
    if (!(x < y) || (x != x)) {
        result |= 4;
    }
    
    // Using memory operands
    volatile double arr[2] = {10.0, 5.0};
    if (!(arr[0] < arr[1])) {
        result |= 8;
    }
    
    checksum += result;
    sink(result);
}

// UNGT (not less or equal): !(a <= b)
__attribute__((noinline))
void test_ungt(void) {
    volatile double a = 8.0;
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // UNGT: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // UNGT with equal values
    if (!(c <= b)) {
        result |= 2;
    }
    
    // UNGT with NaN
    if (!(nan <= a)) {
        result |= 4;
    }
    
    // Nested in ternary
    result |= (!(a <= b)) ? 8 : 16;
    
    checksum += result;
    sink(result);
}

// UNLE (unordered or less or equal)
__attribute__((noinline))
void test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // UNLE: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // UNLE with equal values
    if (!(c > b)) {
        result |= 2;
    }
    
    // UNLE with NaN
    if (!(nan > a)) {
        result |= 4;
    }
    
    // Complex expression with memory
    volatile struct {
        double x;
        double y;
    } s = {2.0, 4.0};
    
    if (!(s.x > s.y)) {
        result |= 8;
    }
    
    checksum += result;
    sink(result);
}

// UNLT (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 4.0;
    volatile double c = 4.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // UNLT: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // UNLT with equal values (false)
    if (!(c >= b)) {
        result |= 2;
    }
    
    // UNLT with NaN (true)
    if (!(nan >= a)) {
        result |= 4;
    }
    
    // Using long double
    volatile long double ld1 = 1.5L;
    volatile long double ld2 = 3.5L;
    if (!(ld1 >= ld2)) {
        result |= 8;
    }
    
    checksum += result;
    sink(result);
}

// LTGT (less than or greater than - not equal and ordered)
__attribute__((noinline))
void test_ltgt(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // LTGT with equal values (false)
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    // LTGT with NaN (false - unordered)
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    // Using float type
    volatile float f1 = 2.0f;
    volatile float f2 = 6.0f;
    if ((f1 < f2) || (f1 > f2)) {
        result |= 8;
    }
    
    // Complex nested expression
    volatile double x = 7.0;
    volatile double y = 9.0;
    volatile double z = 8.0;
    result |= ((x < y) || (x > y)) ? 16 : 32;
    result |= ((z < y) && !(z > x)) ? 64 : 128;
    
    checksum += result;
    sink(result);
}

// Combined test with mixed operations
__attribute__((noinline))
void test_combined(void) {
    volatile double nan = make_nan();
    volatile double inf = make_inf();
    volatile double normal = 42.0;
    
    int result = 0;
    
    // Mix of different comparisons
    result |= isunordered(nan, normal) ? 1 : 0;
    result |= isordered(normal, inf) ? 2 : 0;
    result |= !(nan < normal) ? 4 : 0;
    result |= !(normal <= inf) ? 8 : 0;
    result |= !(inf > normal) ? 16 : 0;
    result |= !(nan >= normal) ? 32 : 0;
    result |= ((normal < inf) || (normal > inf)) ? 64 : 0;
    
    // Array access with volatile
    volatile double arr[4] = {nan, inf, -inf, normal};
    for (int i = 0; i < 3; i++) {
        if (isunordered(arr[i], arr[i+1])) {
            result += (1 << (i + 7));
        }
    }
    
    checksum += result;
    sink(result);
}

int main(void) {
    // Initialize volatile seed
    volatile int seed = 0x12345678;
    
    // Call all test functions
    test_unordered_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_combined();
    
    // Print checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
