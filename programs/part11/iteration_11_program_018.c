/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Array of volatile values */
volatile double arr[8] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};

/* Struct with volatile members */
struct volatile_doubles {
    volatile double a;
    volatile double b;
    volatile double c;
};

/* Function to generate UNORDERED condition code */
__attribute__((noinline))
int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    int result = 0;
    
    /* Use isunordered() which should generate UNORDERED comparison */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Direct NaN comparison for unordered */
    volatile double c = 0.0/0.0;
    if (c != c) {  /* NaN != NaN is true */
        result |= 2;
    }
    
    /* Use inline assembly to force condition code usage */
    volatile double d = arr[0];
    volatile double e = arr[1];
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unordered_label
    );
    
    result |= 4;
    goto end;
    
unordered_label:
    result |= 8;
    
end:
    sink(result);
    return result;
}

/* Function to generate ORDERED condition code */
__attribute__((noinline))
int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    int result = 0;
    
    /* Use isordered() which should generate ORDERED comparison */
    if (isordered(a, b)) {
        result |= 1;
    }
    
    /* Direct ordered comparison */
    volatile double c = arr[2];
    volatile double d = arr[3];
    if (c == c && d == d) {  /* Both are not NaN */
        result |= 2;
    }
    
    /* Complex expression with ordered check */
    result |= (isordered(g_nan, g_one) ? 0 : 4);
    result |= (isordered(g_one, g_two) ? 8 : 0);
    
    sink(result);
    return result;
}

/* Function to generate UNEQ (unordered or equal) condition code */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* UNEQ: !(a > b) && !(a < b) which covers equal and unordered */
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    /* Using NaN to trigger unordered case */
    if (!(c > b) && !(c < b)) {
        result |= 2;
    }
    
    /* Complex ternary expression */
    struct volatile_doubles vs = {g_nan, g_one, g_two};
    result |= ((!(vs.a > vs.b) && !(vs.a < vs.b)) ? 4 : 0);
    
    sink(result);
    return result;
}

/* Function to generate UNGE (not less than) condition code */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* UNGE: !(a < b) - not less than (greater, equal, or unordered) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* Nested in larger expression */
    result = (result * 3) + (!(arr[0] < arr[1]) ? 4 : 0);
    
    /* Use inline assembly with condition code */
    volatile double x = arr[4];
    volatile double y = arr[5];
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[not_less_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : not_less_label
    );
    
    result |= 8;
    goto unge_end;
    
not_less_label:
    result |= 16;
    
unge_end:
    sink(result);
    return result;
}

/* Function to generate UNGT (not less than or equal) condition code */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* UNGT: !(a <= b) - not less than or equal (greater or unordered) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    volatile double x = arr[0];
    volatile double y = arr[1];
    if ((x > y) || isunordered(x, y)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Function to generate UNLE (unordered or less than or equal) condition code */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* UNLE: !(a > b) - not greater than (less, equal, or unordered) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c > b)) {
        result |= 2;
    }
    
    /* Using different floating point types */
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    if (!(f1 > f2)) {
        result |= 4;
    }
    
    /* Long double comparison */
    volatile long double ld1 = 1.5L;
    volatile long double ld2 = 2.5L;
    if (!(ld1 > ld2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Function to generate UNLT (unordered or less than) condition code */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* UNLT: !(a >= b) - not greater than or equal (less or unordered) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(c >= b)) {
        result |= 2;
    }
    
    /* Complex expression with memory operands */
    result |= (!(arr[6] >= arr[7]) ? 4 : 0);
    
    sink(result);
    return result;
}

/* Function to generate LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) - ordered and not equal */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Should be false for NaN */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* Complex nested expression */
    volatile double x = arr[2];
    volatile double y = arr[3];
    result |= (((x < y) || (x > y)) ? 4 : 0);
    result |= (((y < x) || (y > x)) ? 8 : 0);
    
    /* Use inline assembly to check condition */
    volatile double p = arr[4];
    volatile double q = arr[5];
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : ltgt_label
    );
    
    result |= 16;
    goto ltgt_end;
    
ltgt_label:
    result |= 32;
    
ltgt_end:
    sink(result);
    return result;
}

/* Function combining multiple condition codes in complex expressions */
__attribute__((noinline))
int test_combined(void) {
    volatile double a = arr[0];
    volatile double b = arr[1];
    volatile double c = arr[2];
    volatile double d = arr[3];
    
    int result = 0;
    
    /* Complex ternary with multiple comparisons */
    result = (a != a) ? 1 : ((b > c) ? 2 : 3);
    
    /* Nested logical expressions */
    if ((!(a < b) && (c > d)) || isunordered(a, c)) {
        result += 4;
    }
    
    /* Multiple condition codes in one expression */
    int temp = 0;
    temp |= (!(a >= b) ? 1 : 0);      /* UNLT */
    temp |= ((c < d) || (c > d) ? 2 : 0); /* LTGT */
    temp |= (!(a > b) ? 4 : 0);       /* UNLE */
    
    result += temp * 8;
    
    sink(result);
    return result;
}

/* Main function that calls all test functions */
int main(void) {
    int checksum = 0;
    
    /* Initialize array with special values */
    arr[0] = 0.0/0.0;  /* NaN */
    arr[1] = 1.0/0.0;  /* +Inf */
    arr[2] = -1.0/0.0; /* -Inf */
    arr[3] = 0.0;
    arr[4] = 1.0;
    arr[5] = 2.0;
    arr[6] = 3.0;
    arr[7] = 4.0;
    
    printf("Starting condition code tests...\n");
    
    /* Call each test function and accumulate results */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_combined();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int dummy = 0;
    dummy = x;
}

void sink_ptr(void* p) {
    volatile static void* dummy = NULL;
    dummy = p;
}
