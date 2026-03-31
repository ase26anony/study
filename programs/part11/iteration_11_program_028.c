/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Arrays for memory operand variations */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct for complex memory access */
struct fp_pair {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct fp_pair fp_struct = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons */
__attribute__((noinline))
int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[1];
    
    int result = 0;
    
    /* Direct NaN comparison for unordered */
    if (a != a) {
        result |= 1;  /* unordered */
    }
    
    /* Using isunordered() */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    if (isunordered(c, (float)b) || (c < (float)b)) {
        result |= 4;
    }
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : unordered_label
    );
    
    result |= 8;
    goto skip_unordered;
    
unordered_label:
    result |= 16;
    
skip_unordered:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {  /* ordered */
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Compare with NaN to test ordered */
    if (!isunordered(a, nan)) {
        result |= 4;
    }
    
    /* Complex expression */
    result += (isordered(farr[1], farr[2]) ? 8 : 0);
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ordered_label
    );
    
    result |= 16;
    goto skip_ordered;
    
ordered_label:
    result |= 32;
    
skip_ordered:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a == b (includes unordered case) */
    if (!(a != b)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan < b) && !(nan > b)) {
        result |= 4;
    }
    
    /* Complex expression */
    result += ((!(darr[0] < darr[1]) && !(darr[0] > darr[1])) ? 8 : 0);
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Alternative: a >= b (unordered version) */
    if (a >= b) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan < b)) {
        result |= 4;
    }
    
    /* Complex with memory operands */
    volatile float* pf = &farr[0];
    if (!(*pf < *(pf + 1))) {
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
    
    /* Direct: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Alternative: a > b (unordered version) */
    if (a > b) {
        result |= 2;
    }
    
    /* Complex expression */
    if (!(fp_struct.d <= darr[2])) {
        result |= 4;
    }
    
    /* Nested in ternary */
    result += (!(ldarr[0] <= ldarr[1]) ? 8 : 0);
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* Alternative: a <= b (unordered version) */
    if (a <= b) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > b)) {
        result |= 4;
    }
    
    /* Inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
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

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    
    int result = 0;
    
    /* Direct: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* Alternative: a < b (unordered version) */
    if (a < b) {
        result |= 2;
    }
    
    /* Complex with struct member */
    if (!(fp_struct.f >= farr[3])) {
        result |= 4;
    }
    
    /* Nested expressions */
    int temp = (!(darr[1] >= darr[2])) ? 8 : 16;
    result |= temp;
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double same = g_one;
    
    int result = 0;
    
    /* Direct: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Alternative: a != b (but ordered) */
    if (a != b && isordered(a, b)) {
        result |= 2;
    }
    
    /* With equal values */
    if ((same < same) || (same > same)) {
        result |= 4;  /* Should not be taken */
    }
    
    /* Complex with memory */
    volatile double* pa = &darr[0];
    volatile double* pb = &darr[1];
    if ((*pa < *pb) || (*pa > *pb)) {
        result |= 8;
    }
    
    /* Inline assembly for LTGT */
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
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

/* Pattern 9: Mixed condition codes in complex expression */
__attribute__((noinline))
int test_mixed(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    
    int result = 0;
    
    /* Complex nested ternary with multiple conditions */
    result = (a != a) ? 1 : 
             ((b < c) ? 2 : 
             ((!(b > c)) ? 3 : 
             ((b == b) ? 4 : 
             ((c != b) ? 5 : 6))));
    
    /* Logical AND/OR combination */
    if ((isunordered(a, b) || (b < c)) && !(c <= b)) {
        result |= 8;
    }
    
    /* Multiple comparisons in single expression */
    int r1 = !(farr[0] >= farr[1]);
    int r2 = !(darr[2] <= darr[3]);
    int r3 = (ldarr[0] < ldarr[1]) || (ldarr[0] > ldarr[1]);
    
    result |= (r1 ? 16 : 0);
    result |= (r2 ? 32 : 0);
    result |= (r3 ? 64 : 0);
    
    sink(result);
    return result;
}

/* Pattern 10: Different floating-point types */
__attribute__((noinline))
int test_types(void) {
    int result = 0;
    
    /* float comparisons */
    volatile float f1 = farr[0];
    volatile float f2 = farr[1];
    if (!(f1 < f2)) result |= 1;
    if (!(f1 > f2)) result |= 2;
    if ((f1 < f2) || (f1 > f2)) result |= 4;
    
    /* double comparisons */
    volatile double d1 = darr[2];
    volatile double d2 = darr[3];
    if (!(d1 <= d2)) result |= 8;
    if (!(d1 >= d2)) result |= 16;
    if (!(d1 < d2) && !(d1 > d2)) result |= 32;
    
    /* long double comparisons */
    volatile long double ld1 = ldarr[0];
    volatile long double ld2 = ldarr[1];
    if (ld1 != ld1) result |= 64;
    if (ld1 == ld1) result |= 128;
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    g_nan = 0.0/0.0;
    g_inf = 1.0/0.0;
    
    /* Call all test patterns */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_types();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int buffer;
    buffer = x;
}

void sink_ptr(void* p) {
    volatile static void* buffer;
    buffer = p;
}
