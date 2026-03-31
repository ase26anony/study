/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Force non-inlined functions */
#define NOINLINE __attribute__((noinline))

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
struct volatile_floats {
    volatile float f;
    volatile double d;
    volatile long double ld;
};
volatile struct volatile_floats vf = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons with NaN */
NOINLINE int test_unordered(void) {
    volatile double d = g_nan;
    volatile float f = farr[0];
    volatile long double ld = ldarr[0];
    int result = 0;
    
    /* Using isunordered() */
    if (isunordered(d, f)) {
        result |= 1;
    }
    
    /* Direct NaN comparison */
    if (d != d) {  /* Should generate UNORDERED */
        result |= 2;
    }
    
    /* Complex expression with unordered */
    result = (d != d) ? (result | 4) : (result & ~4);
    
    /* Inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    result |= 8;
    return result;
    
unordered_label:
    result |= 16;
    return result;
}

/* Pattern 2: ORDERED comparisons */
NOINLINE int test_ordered(void) {
    volatile double d1 = g_one;
    volatile double d2 = g_two;
    volatile double d_nan = g_nan;
    int result = 0;
    
    /* Using isordered() */
    if (isordered(d1, d2)) {
        result |= 1;
    }
    
    /* Direct ordered comparison */
    if (d1 == d1) {  /* Should generate ORDERED */
        result |= 2;
    }
    
    /* Complex ordered expression */
    result = (d1 == d1 && d2 == d2) ? (result | 4) : (result & ~4);
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    result |= 8;
    return result;
    
ordered_label:
    result |= 16;
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
NOINLINE int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double nan = g_nan;
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Using memory operands from array */
    if (!(darr[0] != darr[1])) {
        result |= 2;
    }
    
    /* Complex expression */
    result = (!(vf.d != darr[2])) ? (result | 4) : (result & ~4);
    
    /* Force condition code with inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : uneq_label
    );
    result |= 8;
    return result;
    
uneq_label:
    result |= 16;
    return result;
}

/* Pattern 4: UNGE (not less than) */
NOINLINE int test_unge(void) {
    volatile double x = g_two;
    volatile double y = g_one;
    int result = 0;
    
    /* UNGE: !(x < y) */
    if (!(x < y)) {
        result |= 1;
    }
    
    /* Using struct member */
    if (!(vf.d < darr[3])) {
        result |= 2;
    }
    
    /* Nested in larger expression */
    int temp = (!(x < y)) ? 10 : 20;
    result += temp;
    
    /* Inline asm */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unge_label
    );
    result |= 32;
    return result;
    
unge_label:
    result |= 64;
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With float type */
    volatile float f1 = farr[2];
    volatile float f2 = farr[1];
    if (!(f1 <= f2)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    result = (!(a <= b) && (a == a)) ? (result | 4) : (result & ~4);
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ungt_label
    );
    result |= 8;
    return result;
    
ungt_label:
    result |= 16;
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
NOINLINE int test_unle(void) {
    volatile double p = g_one;
    volatile double q = g_two;
    int result = 0;
    
    /* UNLE: !(p > q) */
    if (!(p > q)) {
        result |= 1;
    }
    
    /* Using long double */
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    if (!(ld1 > ld2)) {
        result |= 2;
    }
    
    /* In ternary expression */
    result = (!(p > q)) ? (result | 4) : (result & ~4);
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unle_label
    );
    result |= 8;
    return result;
    
unle_label:
    result |= 16;
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
NOINLINE int test_unlt(void) {
    volatile double m = g_one;
    volatile double n = g_two;
    int result = 0;
    
    /* UNLT: !(m >= n) */
    if (!(m >= n)) {
        result |= 1;
    }
    
    /* With array elements */
    if (!(darr[0] >= darr[2])) {
        result |= 2;
    }
    
    /* Combined with other operations */
    result = (!(m >= n) || (m != m)) ? (result | 4) : (result & ~4);
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unlt_label
    );
    result |= 8;
    return result;
    
unlt_label:
    result |= 16;
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
NOINLINE int test_ltgt(void) {
    volatile double u = g_one;
    volatile double v = g_two;
    volatile double nan = g_nan;
    int result = 0;
    
    /* LTGT: (u < v) || (u > v) */
    if ((u < v) || (u > v)) {
        result |= 1;
    }
    
    /* Alternative: !(u == v) && !(u != u) */
    if (!(u == v) && (u == u)) {
        result |= 2;
    }
    
    /* With mixed types */
    volatile float fu = farr[1];
    volatile float fv = farr[3];
    result = ((fu < fv) || (fu > fv)) ? (result | 4) : (result & ~4);
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ltgt_label
    );
    result |= 8;
    return result;
    
ltgt_label:
    result |= 16;
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
NOINLINE int test_mixed(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    int result = 0;
    
    /* Complex nested conditionals */
    if (x != x) {  /* UNORDERED */
        result |= 1;
        if (!(y >= z)) {  /* UNLT */
            result |= 2;
        }
    } else if (isordered(y, z)) {  /* ORDERED */
        result |= 4;
        if (!(y <= z)) {  /* UNGT */
            result |= 8;
        }
    }
    
    /* Ternary with multiple conditions */
    int r = (x != x) ? 1 : ((y < z) || (y > z)) ? 2 : 3;
    result += r;
    
    /* Logical AND/OR combining conditions */
    if ((!(y == z)) && (y == y)) {  /* LTGT components */
        result |= 16;
    }
    
    return result;
}

/* Pattern 10: Memory-intensive comparisons */
NOINLINE int test_memory_ops(void) {
    volatile struct volatile_floats local_vf;
    local_vf.f = farr[0];
    local_vf.d = darr[1];
    local_vf.ld = ldarr[0];
    
    int result = 0;
    
    /* Compare struct members with array elements */
    if (!(local_vf.d < darr[2])) {  /* UNGE */
        result |= 1;
    }
    
    if (!(farr[1] > farr[3])) {  /* UNLE */
        result |= 2;
    }
    
    if ((local_vf.d < darr[3]) || (local_vf.d > darr[3])) {  /* LTGT */
        result |= 4;
    }
    
    /* Chain of comparisons */
    result = (!(local_vf.f == farr[2])) ? (result | 8) : (result & ~8);
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    volatile double seed = 0.0;
    for (int i = 0; i < 3; i++) {
        seed = seed / seed;  /* Generate NaN */
    }
    g_nan = seed;
    
    /* Call all test functions */
    checksum += test_unordered();
    sink(checksum);
    
    checksum += test_ordered();
    sink(checksum);
    
    checksum += test_uneq();
    sink(checksum);
    
    checksum += test_unge();
    sink(checksum);
    
    checksum += test_ungt();
    sink(checksum);
    
    checksum += test_unle();
    sink(checksum);
    
    checksum += test_unlt();
    sink(checksum);
    
    checksum += test_ltgt();
    sink(checksum);
    
    checksum += test_mixed();
    sink(checksum);
    
    checksum += test_memory_ops();
    sink(checksum);
    
    /* Print final checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int buffer[1024];
    static int idx = 0;
    buffer[idx++ % 1024] = x;
}

void sink_ptr(void* p) {
    volatile static void* buffer[1024];
    static int idx = 0;
    buffer[idx++ % 1024] = p;
}
