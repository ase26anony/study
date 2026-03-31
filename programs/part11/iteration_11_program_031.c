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

/* Volatile arrays for memory operands */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[2] = {0.0L, 1.0L};

/* Struct with volatile members */
struct volatile_fp {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct volatile_fp vfp = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int test_unordered(void) {
    volatile double d = g_nan;
    volatile float f = farr[0];
    volatile long double ld = ldarr[0];
    int result = 0;
    
    /* Direct NaN comparison */
    if (d != d) {
        result |= 1;  /* unordered */
    }
    
    /* Using isunordered */
    if (isunordered(d, f)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    result |= (isunordered(ld, darr[1]) ? 4 : 0);
    
    /* Inline assembly to force condition code use */
    volatile double a = g_nan;
    volatile double b = g_one;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 8;
    goto end_unordered;
    
unordered_label:
    result |= 16;
    
end_unordered:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double d1 = g_one;
    volatile double d2 = g_two;
    volatile float f = farr[1];
    int result = 0;
    
    /* Direct ordered comparison */
    if (d1 == d1) {
        result |= 1;  /* ordered */
    }
    
    /* Using isordered */
    if (isordered(d1, d2)) {
        result |= 2;
    }
    
    /* Memory operand ordered check */
    if (isordered(vfp.d, darr[2])) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 8;
    goto end_ordered;
    
ordered_label:
    result |= 16;
    
end_ordered:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a == b (which includes unordered case) */
    if (a == b) {
        result |= 2;
    }
    
    /* Using memory operands */
    if (!(darr[0] < darr[1]) && !(darr[0] > darr[1])) {
        result |= 4;
    }
    
    /* Complex expression */
    result |= ((a == a) && (b == c)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Alternative formulation */
    if (a >= b || a != a) {
        result |= 2;
    }
    
    /* With NaN operand */
    volatile double nan = g_nan;
    if (!(nan < b)) {
        result |= 4;
    }
    
    /* Using struct member */
    if (!(vfp.d < darr[3])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Alternative: a > b || a != a */
    if (a > b || a != a) {
        result |= 2;
    }
    
    /* With memory operand */
    if (!(darr[2] <= darr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a <= b || a != a */
    if (a <= b || a != a) {
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(farr[2] > farr[1]) ? 4 : 0);
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Alternative: a < b || a != a */
    if (a < b || a != a) {
        result |= 2;
    }
    
    /* Using long double */
    volatile long double ld = ldarr[0];
    if (!(ld >= ldarr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With NaN (should not trigger) */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* Complex nested expression */
    result |= ((darr[0] < darr[1]) || (darr[0] > darr[1])) ? 4 : 0;
    
    /* Ternary operator usage */
    int r = (a < b) ? 8 : ((a > b) ? 16 : 0);
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
__attribute__((noinline))
int test_mixed(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    int result = 0;
    
    /* Complex expression combining multiple conditions */
    if ((x != x) && (y < z)) {
        result |= 1;
    }
    
    if ((isunordered(x, y) || (y > z)) && !(z <= y)) {
        result |= 2;
    }
    
    /* Nested ternary with different condition codes */
    result += (x != x) ? 4 : ((y < z) ? 8 : ((!(y >= z)) ? 16 : 32));
    
    /* Logical AND/OR combination */
    if ((!(x < y)) && ((z > y) || (x != x))) {
        result |= 64;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Memory-intensive comparisons */
__attribute__((noinline))
int test_memory_ops(void) {
    int result = 0;
    
    /* Load from different memory locations */
    volatile double d1 = darr[0];
    volatile double d2 = darr[1];
    volatile float f1 = farr[0];
    volatile float f2 = farr[1];
    volatile long double ld1 = ldarr[0];
    
    /* Various comparisons with memory operands */
    if (isunordered(d1, d2)) result |= 1;
    if (!(f1 < f2)) result |= 2;
    if ((darr[2] > darr[1]) || (darr[2] < darr[1])) result |= 4;
    if (!(vfp.d >= darr[0])) result |= 8;
    if (ld1 == ld1) result |= 16;
    
    /* Array element comparisons */
    for (int i = 0; i < 3; i++) {
        if (isunordered(darr[i], darr[i+1])) {
            result |= (1 << (i + 5));
        }
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    sink_ptr((void*)&seed);
    
    /* Call all pattern functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_memory_ops();
    
    /* Print checksum to ensure all code is live */
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
