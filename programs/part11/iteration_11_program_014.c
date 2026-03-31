/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
volatile int checksum = 0;

/* Helper to generate NaN */
static inline double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

/* Helper to generate infinity */
static inline double make_inf(void) {
    volatile double large = 1e308;
    return large * large;
}

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    /* Direct unordered checks */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    /* Using inline assembly to force condition code */
    volatile double a = nan1;
    volatile double b = normal;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : unordered_label
    );
    
    result |= 2;
    goto skip_unordered;
    
unordered_label:
    result |= 4;
    
skip_unordered:
    
    /* Complex expression with unordered */
    volatile double x = nan1;
    volatile double y = 5.0;
    result |= (x != x) ? 8 : 0;
    result |= (isunordered(x, y) && (y > 0)) ? 16 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    if (isordered(normal1, normal2)) {
        result |= 1;
    }
    
    /* Force ordered condition code with assembly */
    volatile double a = normal1;
    volatile double b = normal2;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : ordered_label
    );
    
    result |= 2;
    goto skip_ordered;
    
ordered_label:
    result |= 4;
    
skip_ordered:
    
    /* Ordered in ternary expression */
    volatile double x = 3.0;
    volatile double y = 4.0;
    result |= (x == x && y == y) ? 8 : 0;
    result |= (isordered(x, y) || (x < y)) ? 16 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which becomes unordered or equal */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan != nan)) {
        result |= 2;
    }
    
    /* Complex expression that should generate UNEQ */
    volatile double x = 10.0;
    volatile double y = 10.0;
    result |= (!(x != y) && (x > 0)) ? 4 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = 7.0;
    volatile double b = 3.0;
    volatile double c = 7.0;
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(c < a)) {
        result |= 2;
    }
    
    /* With assembly to force condition code */
    volatile double x = 8.0;
    volatile double y = 2.0;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unge_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : unge_label
    );
    
    result |= 4;
    goto skip_unge;
    
unge_label:
    result |= 8;
    
skip_unge:
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = 9.0;
    volatile double b = 4.0;
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Using different types */
    volatile float f1 = 15.0f;
    volatile float f2 = 10.0f;
    if (!(f1 <= f2)) {
        result |= 2;
    }
    
    /* In complex expression */
    volatile double x = 20.0;
    volatile double y = 15.0;
    result |= (!(x <= y) ? 4 : 0) | ((x > y) ? 8 : 0);
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 8.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN (should be true) */
    if (!(nan > 5.0)) {
        result |= 2;
    }
    
    /* Equal case */
    volatile double c = 5.0;
    volatile double d = 5.0;
    if (!(c > d)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 6.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan >= 3.0)) {
        result |= 2;
    }
    
    /* Using long double */
    volatile long double ld1 = 10.0L;
    volatile long double ld2 = 20.0L;
    if (!(ld1 >= ld2)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 8: LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = 5.0;
    volatile double b = 10.0;
    volatile double c = 10.0;
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal case (should be false) */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* With assembly */
    volatile double x = 7.0;
    volatile double y = 3.0;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : ltgt_label
    );
    
    result |= 4;
    goto skip_ltgt;
    
ltgt_label:
    result |= 8;
    
skip_ltgt:
    
    /* Complex LTGT expression */
    volatile double p = 12.0;
    volatile double q = 8.0;
    result |= ((p < q) || (p > q)) ? 16 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 9: Mixed comparisons with memory operands */
__attribute__((noinline))
int test_memory_operands(void) {
    volatile double arr[4] = {1.0, make_nan(), 3.0, 4.0};
    volatile struct {
        double x;
        double y;
    } point = {2.5, make_inf()};
    
    int result = 0;
    
    /* Unordered with array access */
    if (isunordered(arr[0], arr[1])) {
        result |= 1;
    }
    
    /* Ordered with struct member */
    if (isordered(point.x, arr[2])) {
        result |= 2;
    }
    
    /* UNGE with memory operands */
    if (!(arr[3] < point.x)) {
        result |= 4;
    }
    
    /* LTGT with mixed operands */
    if ((arr[0] < point.y) || (arr[0] > point.y)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

/* Pattern 10: Nested condition codes in complex expressions */
__attribute__((noinline))
int test_nested_conditions(void) {
    volatile double a = make_nan();
    volatile double b = 5.0;
    volatile double c = 10.0;
    volatile double d = 10.0;
    
    int result = 0;
    
    /* Complex ternary with multiple condition codes */
    result = (a != a) ? 1 : 
             (!(b > c)) ? 2 :
             ((c < d) || (c > d)) ? 3 :
             (!(d <= b)) ? 4 : 5;
    
    /* Logical AND/OR of comparisons */
    if ((isunordered(a, b) || !(b >= c)) && (isordered(c, d) || (c != d))) {
        result |= 8;
    }
    
    /* Chained comparisons */
    volatile double x = 7.0;
    volatile double y = 3.0;
    volatile double z = 9.0;
    
    if (!(x < y) && !(y >= z) && ((z < x) || (z > x))) {
        result |= 16;
    }
    
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
    int r9 = test_memory_operands();
    int r10 = test_nested_conditions();
    
    /* Use results to prevent dead code elimination */
    int total = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    printf("Individual results: %d %d %d %d %d %d %d %d %d %d\n",
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Checksum: %d\n", checksum);
    printf("Total: %d\n", total);
    
    return total == 0 ? 0 : 1;
}
