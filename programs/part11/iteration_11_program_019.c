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

/* Memory locations for varied addressing modes */
typedef struct {
    volatile float f;
    volatile double d;
    volatile long double ld;
} FloatStruct;

volatile FloatStruct fs = {0.0f, 0.0, 0.0L};
volatile double d_array[4] = {0.0, 1.0, 2.0, 3.0};

/* Pattern 1: UNORDERED comparisons with NaN */
__attribute__((noinline))
int pattern_unordered(void) {
    volatile double d = g_nan;
    volatile float f = g_nan;
    volatile long double ld = g_nan;
    int result = 0;
    
    /* Direct unordered checks */
    if (d != d) {  /* Should generate UNORDERED */
        result |= 1;
    }
    
    if (isunordered(f, f)) {  /* Should generate UNORDERED */
        result |= 2;
    }
    
    /* Complex expression with unordered */
    if (isunordered(ld, fs.ld) || (d_array[1] > d_array[2])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int pattern_ordered(void) {
    volatile double d = g_one;
    volatile float f = g_two;
    int result = 0;
    
    if (d == d) {  /* Should generate ORDERED */
        result |= 1;
    }
    
    if (isordered(f, f)) {  /* Should generate ORDERED */
        result |= 2;
    }
    
    /* Ordered in complex expression */
    if (isordered(d_array[0], d_array[1]) && (fs.f < 1.0f)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int pattern_uneq(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_one;
    int result = 0;
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l0\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : taken_label
    );
    
    result |= 1;
    
taken_label:
    /* Alternative: !(a != b) which is UNEQ */
    if (!(a != b)) {
        result |= 2;
    }
    
    /* Complex UNEQ expression */
    if (!(fs.d != d_array[2]) || isunordered(fs.d, d_array[2])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int pattern_unge(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* !(a < b) generates UNGE */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(c < b)) {  /* UNGE because NaN comparison is unordered */
        result |= 2;
    }
    
    /* Complex expression */
    if (!(d_array[0] < d_array[1]) && (fs.f >= 0.0f)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int pattern_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_nan;
    int result = 0;
    
    /* !(a <= b) generates UNGT */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Using inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l0\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : taken_label2
    );
    
    result |= 2;
    
taken_label2:
    /* Complex expression with memory operands */
    if (!(fs.d <= d_array[1]) || (c > b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int pattern_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* Using ternary operator with UNLE condition */
    result = (!(a > b)) ? 1 : 0;  /* !(a > b) is UNLE */
    
    /* Complex nested expression */
    if ((!(fs.d > d_array[3])) && (c <= b)) {
        result |= 2;
    }
    
    /* Mixed types */
    volatile float f1 = fs.f;
    volatile float f2 = 2.0f;
    if (!(f1 > f2) || isunordered(f1, f2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int pattern_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* !(a >= b) generates UNLT */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With memory operand */
    if (!(d_array[1] >= d_array[2])) {
        result |= 2;
    }
    
    /* Complex logical expression */
    if ((!(fs.d >= 3.0)) || (c < b)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
int pattern_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_nan;
    int result = 0;
    
    /* (a < b) || (a > b) generates LTGT */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Using inline assembly for LTGT */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l0\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : taken_label3
    );
    
    result |= 2;
    
taken_label3:
    /* Complex LTGT with memory */
    if ((fs.d < d_array[2]) || (fs.d > d_array[2])) {
        result |= 4;
    }
    
    /* LTGT with different types */
    volatile long double ld1 = fs.ld;
    volatile long double ld2 = 2.0L;
    if ((ld1 < ld2) || (ld1 > ld2)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expressions */
__attribute__((noinline))
int pattern_mixed(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    int result = 0;
    
    /* Nested ternary with multiple condition codes */
    result = (a != a) ? 1 : 
             ((b > c) ? 2 : 
             ((!(b <= c)) ? 3 : 
             ((!(a >= b)) ? 4 : 
             (((b < c) || (b > c)) ? 5 : 6))));
    
    /* Complex logical expression mixing ordered/unordered */
    if ((isunordered(a, b) && !(c >= b)) || 
        (!(fs.d < d_array[1]) && (d_array[0] == d_array[0]))) {
        result |= 8;
    }
    
    /* Multiple comparisons in single expression */
    int r1 = (fs.f != fs.f) ? 1 : 0;
    int r2 = (!(d_array[1] < d_array[2])) ? 2 : 0;
    int r3 = ((fs.d < 1.0) || (fs.d > 1.0)) ? 4 : 0;
    
    result |= r1 | r2 | r3;
    
    sink(result);
    return result;
}

/* Pattern 10: Condition codes with long double (x87 specific) */
__attribute__((noinline))
int pattern_long_double(void) {
    volatile long double a = g_nan;
    volatile long double b = 1.0L;
    volatile long double c = 2.0L;
    int result = 0;
    
    /* Long double comparisons generate x87 condition codes */
    if (a != a) {  /* UNORDERED */
        result |= 1;
    }
    
    if (!(b >= c)) {  /* UNLT */
        result |= 2;
    }
    
    if ((b < c) || (b > c)) {  /* LTGT */
        result |= 4;
    }
    
    if (!(b <= c)) {  /* UNGT */
        result |= 8;
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile memory locations */
    fs.f = 0.5f;
    fs.d = 1.5;
    fs.ld = 2.5L;
    d_array[0] = 0.0/0.0;  /* NaN */
    d_array[1] = 1.0;
    d_array[2] = 2.0;
    d_array[3] = 3.0;
    
    /* Call all pattern functions */
    checksum ^= pattern_unordered();
    checksum ^= pattern_ordered();
    checksum ^= pattern_uneq();
    checksum ^= pattern_unge();
    checksum ^= pattern_ungt();
    checksum ^= pattern_unle();
    checksum ^= pattern_unlt();
    checksum ^= pattern_ltgt();
    checksum ^= pattern_mixed();
    checksum ^= pattern_long_double();
    
    /* Additional calls with different values */
    g_nan = -0.0/0.0;  /* Different NaN */
    fs.d = g_nan;
    checksum ^= pattern_unordered();
    checksum ^= pattern_uneq();
    
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
