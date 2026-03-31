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
    volatile double normal = 3.14;
    volatile double inf = make_inf();
    
    int result = 0;
    
    /* Direct unordered checks */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    if (isunordered(nan1, nan2)) {
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unordered_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 4;
    goto skip_unordered;
    
unordered_label:
    result |= 8;
    
skip_unordered:
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
static int test_ordered(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    if (isordered(a, b)) {
        result |= 1;
    }
    
    if (isordered(a, a)) {
        result |= 2;
    }
    
    /* Complex expression with ordered check */
    result += (isordered(a, nan) ? 0 : 4);
    
    /* Inline assembly for ordered */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ordered_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 8;
    goto skip_ordered;
    
ordered_label:
    result |= 16;
    
skip_ordered:
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
    
    /* Generate UNEQ: !(a != b) which becomes UNEQ for floating point */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan != nan)) {
        result |= 2;
    }
    
    /* Complex expression */
    volatile double x = 3.0, y = 3.0;
    result += ((x == y) || isunordered(x, y)) ? 4 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
static int test_unge(void) {
    volatile double a = 10.0;
    volatile double b = 5.0;
    volatile double c = 10.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    if (!(c < b)) {
        result |= 2;
    }
    
    /* With NaN operand */
    if (!(nan < a)) {
        result |= 4;
    }
    
    /* Using memory operands */
    volatile double arr[3] = {1.0, 2.0, 3.0};
    if (!(arr[1] < arr[0])) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
static int test_ungt(void) {
    volatile double a = 7.0;
    volatile double b = 7.0;
    volatile double c = 8.0;
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    if (!(b <= a)) {
        result |= 2;
    }
    
    if (!(a <= c)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    volatile double x = 5.0, y = 6.0;
    result += (!(x <= y) && !(y <= x)) ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
static int test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 4.0;
    volatile double c = 4.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Using different floating types */
    volatile float f1 = 2.5f;
    volatile float f2 = 3.5f;
    if (!(f1 > f2)) {
        result |= 1;
    }
    
    volatile long double ld1 = 10.0L;
    volatile long double ld2 = 10.0L;
    if (!(ld1 > ld2)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > a)) {
        result |= 4;
    }
    
    /* Memory operand */
    struct { volatile double d1; volatile double d2; } s = {1.0, 2.0};
    if (!(s.d1 > s.d2)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
static int test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    if (!(b >= c)) {
        result |= 2;
    }
    
    /* Ternary expression */
    volatile double x = 5.0, y = 6.0;
    int temp = (!(x >= y)) ? 4 : 0;
    result |= temp;
    
    /* Logical OR with other condition */
    if (!(x >= y) || (x < y)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 8: LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 1.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal values should not trigger */
    if ((c < a) || (c > a)) {
        result |= 2;
    }
    
    /* With NaN */
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    volatile double x = 3.0, y = 4.0, z = 5.0;
    result += (((x < y) || (x > y)) && ((y < z) || (y > z))) ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 9: Mixed condition codes in complex expressions */
__attribute__((noinline))
static int test_mixed(void) {
    volatile double d1 = 1.5;
    volatile double d2 = 2.5;
    volatile double nan = make_nan();
    volatile double inf = make_inf();
    
    int result = 0;
    
    /* Complex expression with multiple condition codes */
    result = (d1 != d1) ? 1 : 
             ((d1 == d1) ? 2 : 
             ((!(d1 < d2)) ? 4 : 
             ((!(d2 <= d1)) ? 8 : 
             (((d1 < d2) || (d1 > d2)) ? 16 : 32))));
    
    /* Another complex expression */
    volatile double arr[4] = {1.0, nan, inf, 0.0};
    for (int i = 0; i < 3; i++) {
        if (isunordered(arr[i], arr[i+1])) {
            result += 64;
        } else if (!(arr[i] >= arr[i+1])) {
            result += 128;
        } else if ((arr[i] < arr[i+1]) || (arr[i] > arr[i+1])) {
            result += 256;
        }
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 10: Inline assembly with explicit condition codes */
__attribute__((noinline))
static int test_asm_conditions(void) {
    volatile double a = 10.0;
    volatile double b = 20.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Force generation of specific condition codes through inline asm */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : "=@ccueq"(result)
        : "m"(a), "m"(b)
        : "st", "cc"
    );
    
    result = result ? 1 : 0;
    
    /* Test with unordered */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : "=@ccunord"(result)
        : "m"(nan), "m"(a)
        : "st", "cc"
    );
    
    result |= (result ? 2 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

int main(void) {
    printf("Testing i386 condition codes...\n");
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    srand(seed);
    
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
    int r10 = test_asm_conditions();
    
    /* Accumulate results */
    int total = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    printf("Total: %d\n", total);
    
    return 0;
}
