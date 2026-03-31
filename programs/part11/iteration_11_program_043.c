/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
static volatile int checksum = 0;

/* Helper to generate NaN */
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

/* Helper to generate infinity */
static double make_inf(void) {
    volatile double large = 1e308;
    return large * large;
}

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
static int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14159;
    int result = 0;
    
    /* Direct unordered check */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unordered_label
    );
    result |= 2;
    
unordered_label:
    /* Compare NaN with itself */
    if (nan1 != nan1) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
static int test_ordered(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct ordered check */
    if (isordered(a, b)) {
        result |= 1;
    }
    
    /* Using inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ordered_label
    );
    result |= 2;
    
ordered_label:
    /* NaN should not be ordered with normal number */
    if (!isunordered(a, nan)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which becomes unordered or equal */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Using complex expression */
    if ((isunordered(a, nan) || (a == b))) {
        result |= 2;
    }
    
    /* Inline assembly forcing UNEQ condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : uneq_label
    );
    result |= 4;
    
uneq_label:
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
static int test_unge(void) {
    volatile double a = 7.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Using different types */
    volatile float fa = 7.0f;
    volatile float fb = 3.0f;
    if (!(fa < fb)) {
        result |= 2;
    }
    
    /* With NaN operand */
    if (!(nan < b)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
static int test_ungt(void) {
    volatile double a = 9.0;
    volatile double b = 4.0;
    int result = 0;
    
    /* Generate UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Using long double */
    volatile long double la = 9.0L;
    volatile long double lb = 4.0L;
    if (!(la <= lb)) {
        result |= 2;
    }
    
    /* Complex expression */
    result = (a > b) ? (result | 4) : (result & ~4);
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
static int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 8.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Using memory operands */
    volatile double arr[3] = {2.0, 8.0, make_nan()};
    if (!(arr[0] > arr[1])) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > b)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
static int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 10.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Nested in larger expression */
    int temp = ((a < b) || isunordered(a, nan)) ? 2 : 0;
    result |= temp;
    
    /* Using struct member */
    struct { volatile double x; volatile double y; } s = {1.0, 10.0};
    if (!(s.x >= s.y)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 6.0;
    volatile double b = 6.0;
    volatile double c = 7.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate LTGT: (a < b) || (a > b) */
    if ((a < c) || (a > c)) {
        result |= 1;
    }
    
    /* Equivalent to !(a == b) for ordered values */
    if (!(a == b)) {
        result |= 2;
    }
    
    /* Complex ternary expression */
    result = ((a < b) || (a > b)) ? (result | 4) : (result & ~4);
    
    /* With NaN - should be false */
    if ((nan < a) || (nan > a)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 9: Mixed condition codes in complex expressions */
__attribute__((noinline))
static int test_mixed(void) {
    volatile double x = make_nan();
    volatile double y = 5.0;
    volatile double z = 10.0;
    int result = 0;
    
    /* Complex nested conditionals */
    if (isunordered(x, y)) {
        if (!(y >= z)) {
            result |= 1;
        }
    } else if (!(y <= z)) {
        result |= 2;
    }
    
    /* Multiple condition codes in one expression */
    result = (x != x) ? 1 : ((y > z) ? 2 : 3);
    
    /* Logical combination */
    int r1 = !(y < z);  /* UNGE */
    int r2 = !(y > z);  /* UNLE */
    int r3 = (y < z) || (y > z);  /* LTGT */
    result += r1 + r2 + r3;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 10: Memory-intensive comparisons */
__attribute__((noinline))
static int test_memory_ops(void) {
    /* Use arrays to force memory operands */
    volatile double arr1[4] = {1.0, make_nan(), 3.0, 4.0};
    volatile double arr2[4] = {5.0, 6.0, make_nan(), 8.0};
    int result = 0;
    
    /* Various comparisons with array elements */
    for (int i = 0; i < 3; i++) {
        if (isunordered(arr1[i], arr2[i])) {
            result ^= (1 << i);
        }
        if (!(arr1[i] < arr2[i])) {
            result ^= (1 << (i + 4));
        }
    }
    
    sink(result);
    checksum += result;
    return result;
}

int main(void) {
    printf("Testing i386 condition codes...\n");
    
    /* Initialize volatile seed */
    volatile double seed = 42.0;
    
    /* Call all test patterns */
    int r1 = test_unordered();
    int r2 = test_ordered();
    int r3 = test_uneq();
    int r4 = test_unge();
    int r5 = test_ungt();
    int r6 = test_unle();
    int r7 = test_unlt();
    int r8 = test_ltgt();
    int r9 = test_mixed();
    int r10 = test_memory_ops();
    
    /* Accumulate results */
    int total = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Force checksum computation */
    total += checksum;
    
    printf("Result checksum: %d\n", total);
    printf("Condition codes tested:\n");
    printf("  UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT\n");
    
    return 0;
}
