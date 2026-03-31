/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization of values */
extern void sink(int);
extern void sink_ptr(void*);

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Array of volatile values */
volatile double arr[10] = {0.0, 1.0, 2.0, 0.0/0.0, 1.0/0.0};

/* Struct with volatile members */
struct volatile_doubles {
    volatile double a;
    volatile double b;
    volatile double c;
};

/* Function declarations with noinline to isolate patterns */
__attribute__((noinline)) int test_unordered(void);
__attribute__((noinline)) int test_ordered(void);
__attribute__((noinline)) int test_uneq(void);
__attribute__((noinline)) int test_unge(void);
__attribute__((noinline)) int test_ungt(void);
__attribute__((noinline)) int test_unle(void);
__attribute__((noinline)) int test_unlt(void);
__attribute__((noinline)) int test_ltgt(void);
__attribute__((noinline)) int test_mixed_types(void);
__attribute__((noinline)) int test_complex_expressions(void);
__attribute__((noinline)) int test_memory_operands(void);

/* Helper to generate NaN */
static volatile double make_nan(void) {
    volatile double x = 0.0;
    return x / x;
}

/* Helper to generate infinity */
static volatile double make_inf(void) {
    volatile double x = 1.0;
    return x / 0.0;
}

/* Test UNORDERED condition codes */
__attribute__((noinline)) int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    /* Direct NaN comparison */
    if (nan1 != nan1) {
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(nan1, normal)) {
        result |= 2;
    }
    
    /* Inline assembly to force condition code use */
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
    
    result |= 4;
    goto skip_unordered;
    
unordered_label:
    result |= 8;
    
skip_unordered:
    sink(result);
    return result;
}

/* Test ORDERED condition codes */
__attribute__((noinline)) int test_ordered(void) {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct ordered comparison */
    if (normal1 == normal1) {
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(normal1, normal2)) {
        result |= 2;
    }
    
    /* Complex ordered check */
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
    
    result |= 4;
    goto skip_ordered;
    
ordered_label:
    result |= 8;
    
skip_ordered:
    sink(result);
    return result;
}

/* Test UNEQ (unordered or equal) */
__attribute__((noinline)) int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Generate UNEQ via !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan != nan)) {
        result |= 2;
    }
    
    /* Force condition code with inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[uneq_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : uneq_label
    );
    
    result |= 4;
    goto skip_uneq;
    
uneq_label:
    result |= 8;
    
skip_uneq:
    sink(result);
    return result;
}

/* Test UNGE (not less than) */
__attribute__((noinline)) int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* Generate UNGE via !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(b < b)) {
        result |= 2;
    }
    
    /* With NaN (should be true for unordered) */
    if (!(c < b)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unge_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : unge_label
    );
    
    result |= 8;
    goto skip_unge;
    
unge_label:
    result |= 16;
    
skip_unge:
    sink(result);
    return result;
}

/* Test UNGT (not less than or equal) */
__attribute__((noinline)) int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* Generate UNGT via !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(b <= a)) {  /* b <= a is true, so !(b <= a) is false */
        result |= 0;  /* This won't execute */
    } else {
        result |= 2;
    }
    
    /* With NaN */
    if (!(c <= b)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ungt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : ungt_label
    );
    
    result |= 8;
    goto skip_ungt;
    
ungt_label:
    result |= 16;
    
skip_ungt:
    sink(result);
    return result;
}

/* Test UNLE (unordered or less than or equal) */
__attribute__((noinline)) int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Generate UNLE via !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(b > b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(c > b)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unle_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : unle_label
    );
    
    result |= 8;
    goto skip_unle;
    
unle_label:
    result |= 16;
    
skip_unle:
    sink(result);
    return result;
}

/* Test UNLT (unordered or less than) */
__attribute__((noinline)) int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Generate UNLT via !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With equal values */
    if (!(b >= a)) {  /* b >= a is true, so !(b >= a) is false */
        result |= 0;  /* This won't execute */
    } else {
        result |= 2;
    }
    
    /* With NaN */
    if (!(c >= b)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unlt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : unlt_label
    );
    
    result |= 8;
    goto skip_unlt;
    
unlt_label:
    result |= 16;
    
skip_unlt:
    sink(result);
    return result;
}

/* Test LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline)) int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Generate LTGT via (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With equal values (should be false) */
    if ((b < b) || (b > b)) {
        result |= 0;  /* This won't execute */
    } else {
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((c < b) || (c > b)) {
        result |= 0;  /* This won't execute */
    } else {
        result |= 4;
    }
    
    /* Alternative formulation: !(a == b) && !isunordered(a, b) */
    if (!(a == b) && !isunordered(a, b)) {
        result |= 8;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st"
        : ltgt_label
    );
    
    result |= 16;
    goto skip_ltgt;
    
ltgt_label:
    result |= 32;
    
skip_ltgt:
    sink(result);
    return result;
}

/* Test with mixed floating-point types */
__attribute__((noinline)) int test_mixed_types(void) {
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    volatile float f_nan = 0.0f/0.0f;
    
    volatile double d1 = 1.5;
    volatile double d2 = 2.5;
    
    volatile long double ld1 = 1.5L;
    volatile long double ld2 = 2.5L;
    
    int result = 0;
    
    /* Float comparisons */
    if (!(f1 < f2)) {  /* UNGE */
        result |= 1;
    }
    
    if (!(f_nan == f_nan)) {  /* LTGT or UNORD */
        result |= 2;
    }
    
    /* Double comparisons */
    if ((d1 < d2) || (d1 > d2)) {  /* LTGT */
        result |= 4;
    }
    
    /* Long double comparisons */
    if (!(ld1 > ld2)) {  /* UNLE */
        result |= 8;
    }
    
    /* Mixed type comparison */
    if (!((double)f1 < d2)) {  /* UNGE with conversion */
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Test complex nested expressions */
__attribute__((noinline)) int test_complex_expressions(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    int result = 0;
    
    /* Nested ternary with floating comparisons */
    int r1 = (x != x) ? 1 : ((y > z) ? 2 : 3);
    result ^= r1;
    
    /* Logical AND/OR combination */
    if ((!(x < y) && !(y > z)) || (z == z)) {
        result ^= 4;
    }
    
    /* Complex condition */
    int r2 = (!(x == x) || !(y <= z)) ? 5 : 6;
    result ^= r2;
    
    /* Multiple comparisons in expression */
    int r3 = ((x > y) ? 7 : 8) + ((!(z < y)) ? 9 : 10);
    result ^= r3;
    
    sink(result);
    return result;
}

/* Test with memory operands (arrays and structs) */
__attribute__((noinline)) int test_memory_operands(void) {
    struct volatile_doubles vs = {.a = 1.0, .b = 2.0, .c = 0.0/0.0};
    int result = 0;
    
    /* Array element comparisons */
    if (!(arr[0] < arr[1])) {  /* UNGE */
        result |= 1;
    }
    
    if ((arr[2] < arr[1]) || (arr[2] > arr[1])) {  /* LTGT */
        result |= 2;
    }
    
    /* Struct member comparisons */
    if (!(vs.a > vs.b)) {  /* UNLE */
        result |= 4;
    }
    
    if (!(vs.c == vs.c)) {  /* LTGT or UNORD */
        result |= 8;
    }
    
    /* Array with NaN */
    if (!(arr[3] >= arr[0])) {  /* UNLT */
        result |= 16;
    }
    
    /* Pointer dereference */
    volatile double* ptr = &arr[4];
    if (!(*ptr < arr[0])) {  /* UNGE */
        result |= 32;
    }
    
    sink(result);
    sink_ptr(&vs);
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing i386 condition codes...\n");
    
    /* Initialize global volatile values */
    g_nan = make_nan();
    g_inf = make_inf();
    g_zero = 0.0;
    g_one = 1.0;
    g_two = 2.0;
    
    /* Initialize array */
    arr[0] = 0.0;
    arr[1] = 1.0;
    arr[2] = 2.0;
    arr[3] = make_nan();
    arr[4] = make_inf();
    
    /* Run all tests and accumulate checksum */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed_types();
    checksum ^= test_complex_expressions();
    checksum ^= test_memory_operands();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int dummy;
    dummy = x;
}

void sink_ptr(void* p) {
    volatile static void* dummy;
    dummy = p;
}
