/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization of intermediate values */
extern void sink(int value);
extern void sink_ptr(void *ptr);

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

/* ========== UNORDERED/ORDERED patterns ========== */

__attribute__((noinline))
int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = fs.f;
    volatile long double d = fs.ld;
    
    int result = 0;
    
    /* UNORDERED: Using isunordered() */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* UNORDERED: Direct NaN comparison */
    if (a != a) {
        result |= 2;
    }
    
    /* ORDERED: Using isordered() */
    if (isordered(b, c)) {
        result |= 4;
    }
    
    /* ORDERED: Direct comparison with non-NaN */
    if (b == b) {
        result |= 8;
    }
    
    /* Mixed types for ORDERED */
    if (isordered(d, (long double)b)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* ========== UNEQ (unordered or equal) ========== */

__attribute__((noinline))
int test_uneq(void) {
    volatile double x = d_array[0];
    volatile double y = d_array[1];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNEQ: !(a != b) which includes unordered cases */
    if (!(x != y)) {
        result |= 1;
    }
    
    /* UNEQ with NaN operand */
    if (!(nan != nan)) {  /* Always true for NaN */
        result |= 2;
    }
    
    /* Complex expression generating UNEQ */
    if ((x == y) || (x != x) || (y != y)) {
        result |= 4;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_or_equal
    );
    
    result |= 8;
    goto after_label;
    
unordered_or_equal:
    result |= 16;
    
after_label:
    sink(result);
    return result;
}

/* ========== UNGE (not less than) ========== */

__attribute__((noinline))
int test_unge(void) {
    volatile double a = d_array[1];
    volatile double b = d_array[2];
    volatile float c = fs.f;
    
    int result = 0;
    
    /* UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* UNGE with mixed types */
    if (!((double)c < b)) {
        result |= 2;
    }
    
    /* Using inline assembly */
    double temp = a;
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        : "=t"(temp)
        : "m"(a), "m"(b)
        : "cc"
    );
    
    /* Branch based on condition code */
    __asm__ goto (
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : /* no clobbers */
        : not_less_than
    );
    
    result |= 4;
    goto after_unge;
    
not_less_than:
    result |= 8;
    
after_unge:
    sink(result);
    return result;
}

/* ========== UNGT (not less than or equal) ========== */

__attribute__((noinline))
int test_ungt(void) {
    volatile double x = g_one;
    volatile double y = g_two;
    volatile long double z = fs.ld;
    
    int result = 0;
    
    /* UNGT: !(a <= b) */
    if (!(x <= y)) {
        result |= 1;
    }
    
    /* Complex expression */
    if ((x > y) && !isunordered(x, y)) {
        result |= 2;
    }
    
    /* Nested ternary generating UNGT */
    int r = (x != x) ? 0 : ((!(x <= y)) ? 1 : 2);
    result |= (r << 2);
    
    sink(result);
    return result;
}

/* ========== UNLE (unordered or less than or equal) ========== */

__attribute__((noinline))
int test_unle(void) {
    volatile double a = d_array[0];
    volatile double b = d_array[3];
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* UNLE with NaN */
    if (!(nan > b)) {  /* Always true */
        result |= 2;
    }
    
    /* Using inline assembly with memory operand */
    __asm__ goto (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : "m"(a), "m"(b)
        : "st", "st(1)", "cc"
        : not_greater_than
    );
    
    result |= 4;
    goto after_unle;
    
not_greater_than:
    result |= 8;
    
after_unle:
    sink(result);
    return result;
}

/* ========== UNLT (unordered or less than) ========== */

__attribute__((noinline))
int test_unlt(void) {
    volatile double x = g_two;
    volatile double y = g_one;
    volatile float f = fs.f;
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(x >= y)) {
        result |= 1;
    }
    
    /* Mixed type comparison */
    if (!((double)f >= y)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    if ((x < y) || (x != x) || (y != y)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* ========== LTGT (less than or greater than, ordered) ========== */

__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* LTGT with !(a == b) for ordered values */
    if (!(a == b) && isordered(a, b)) {
        result |= 2;
    }
    
    /* LTGT should be false for NaN */
    if ((nan < nan) || (nan > nan)) {  /* Always false */
        result |= 0;  /* No change */
    } else {
        result |= 4;
    }
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : lt_or_gt
    );
    
    result |= 8;
    goto after_ltgt;
    
lt_or_gt:
    result |= 16;
    
after_ltgt:
    sink(result);
    return result;
}

/* ========== Complex combined patterns ========== */

__attribute__((noinline))
int test_combined(void) {
    volatile double v1 = d_array[0];
    volatile double v2 = d_array[1];
    volatile double v3 = d_array[2];
    volatile double v4 = d_array[3];
    
    int result = 0;
    
    /* Nested comparisons generating multiple condition codes */
    result = (v1 != v1) ? 1 : 
             ((!(v2 < v3)) ? 2 : 
             ((!(v3 <= v4)) ? 3 : 
             ((!(v1 > v2)) ? 4 : 
             ((!(v2 >= v3)) ? 5 : 
             (((v3 < v4) || (v3 > v4)) ? 6 : 7)))));
    
    /* Logical AND/OR of comparisons */
    if ((!(v1 < v2)) && (!(v3 > v4)) && ((v2 < v3) || (v2 > v3))) {
        result += 8;
    }
    
    /* Memory-to-memory comparison */
    volatile double *p1 = &d_array[0];
    volatile double *p2 = &d_array[1];
    if (!(*p1 == *p2) && isordered(*p1, *p2)) {
        result += 16;
    }
    
    sink(result);
    return result;
}

/* ========== Main function ========== */

int main(void) {
    int checksum = 0;
    
    /* Initialize with potentially NaN values */
    fs.f = 0.0f / 0.0f;
    fs.d = 0.0 / 0.0;
    fs.ld = 0.0L / 0.0L;
    
    /* Call all test functions */
    checksum += test_unordered_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_combined();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int value) {
    volatile static int sink_var = 0;
    sink_var = value;
}

void sink_ptr(void *ptr) {
    volatile static void *sink_ptr_var = NULL;
    sink_ptr_var = ptr;
}
