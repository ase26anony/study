/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

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
int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    volatile double inf = make_inf();
    
    int result = 0;
    
    /* Direct unordered comparisons */
    if (nan1 != nan1) {
        result |= 1;  /* unordered */
    }
    
    /* Using isunordered() */
    if (isunordered(nan1, normal)) {
        result |= 2;
    }
    
    /* Complex expression with unordered */
    if (isunordered(nan1, nan2) || (normal > 0)) {
        result |= 4;
    }
    
    /* Inline assembly to force condition code use */
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    result |= 8;
    goto end;
    
unordered_label:
    result |= 16;
    
end:
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double nan = make_nan();
    volatile double x = 1.5;
    volatile double y = 2.5;
    
    int result = 0;
    
    /* Direct ordered check */
    if (x == x && y == y) {
        result |= 1;  /* both ordered */
    }
    
    /* Using isordered() */
    if (isordered(x, y)) {
        result |= 2;
    }
    
    /* Complex ordered expression */
    if (isordered(x, y) && !isunordered(x, y)) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    result |= 8;
    goto end2;
    
ordered_label:
    result |= 16;
    
end2:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile float f1 = 1.0f;
    volatile float f2 = 1.0f;
    volatile float f3 = 2.0f;
    volatile float f_nan = make_nan();
    
    int result = 0;
    
    /* Generate UNEQ: !(a < b) && !(a > b) */
    if (!(f1 < f2) && !(f1 > f2)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(f_nan < f1) && !(f_nan > f1)) {
        result |= 2;
    }
    
    /* Using == with volatile to prevent optimization */
    volatile double d1 = 3.0;
    volatile double d2 = 3.0;
    if (d1 == d2) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double c = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Direct: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(a < c)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan < a)) {
        result |= 4;
    }
    
    /* Complex expression */
    if (!(a < b) || (a == c)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(void) {
    volatile long double ld1 = 10.0L;
    volatile long double ld2 = 5.0L;
    volatile long double ld3 = 10.0L;
    
    int result = 0;
    
    /* Direct: !(a <= b) */
    if (!(ld1 <= ld2)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if (!(ld1 <= ld3)) {
        result |= 2;
    }
    
    /* Greater case */
    if (!(ld2 <= ld1)) {
        result |= 4;
    }
    
    /* Complex with memory operand */
    volatile long double arr[3] = {1.0L, 2.0L, 3.0L};
    if (!(arr[0] <= arr[2])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double x = 2.0;
    volatile double y = 3.0;
    volatile double z = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Direct: !(a > b) */
    if (!(x > y)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(y > z)) {
        result |= 2;
    }
    
    /* With NaN */
    if (!(nan > x)) {
        result |= 4;
    }
    
    /* Complex ternary expression */
    result |= (!(x > y)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile float fa = 1.0f;
    volatile float fb = 2.0f;
    volatile float fc = 2.0f;
    
    int result = 0;
    
    /* Direct: !(a >= b) */
    if (!(fa >= fb)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if (!(fb >= fc)) {
        result |= 2;
    }
    
    /* Less than case */
    if (!(fb >= fa)) {
        result |= 4;
    }
    
    /* Nested in logical expression */
    if ((!(fa >= fb)) && (fb < 3.0f)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double p = 1.0;
    volatile double q = 2.0;
    volatile double r = 2.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Direct: (a < b) || (a > b) */
    if ((p < q) || (p > q)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if ((q < r) || (q > r)) {
        result |= 2;
    }
    
    /* With NaN (should not trigger) */
    if ((nan < p) || (nan > p)) {
        result |= 4;
    }
    
    /* Complex with memory */
    volatile double mem[2] = {5.0, 10.0};
    if ((mem[0] < mem[1]) || (mem[0] > mem[1])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed types and complex expressions */
__attribute__((noinline))
int test_mixed(void) {
    volatile float f = 1.5f;
    volatile double d = 2.5;
    volatile long double ld = 3.5L;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Cross-type comparisons */
    if (!(f < (float)d)) {
        result |= 1;
    }
    
    if ((double)ld > d || (double)ld < d) {
        result |= 2;
    }
    
    /* Nested ternary with unordered check */
    result += (nan != nan) ? 4 : 
              (!(f >= 2.0f)) ? 8 : 
              ((d < 3.0) || (d > 3.0)) ? 16 : 0;
    
    /* Struct with volatile members */
    struct {
        volatile double x;
        volatile double y;
    } point = {1.0, 2.0};
    
    if (!(point.x > point.y)) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Array-based comparisons */
__attribute__((noinline))
int test_array_comparisons(void) {
    volatile double arr[4] = {1.0, make_nan(), 3.0, 4.0};
    volatile float farr[3] = {1.0f, 2.0f, 3.0f};
    
    int result = 0;
    
    /* Array element comparisons */
    if (isunordered(arr[0], arr[1])) {
        result |= 1;
    }
    
    if (!(arr[2] < arr[3])) {
        result |= 2;
    }
    
    if (!(farr[0] >= farr[2])) {
        result |= 4;
    }
    
    /* Loop with comparisons */
    for (int i = 0; i < 2; i++) {
        if ((arr[i] < arr[i+2]) || (arr[i] > arr[i+2])) {
            result |= (8 << i);
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
    checksum += test_array_comparisons();
    
    /* Print checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int buffer[1024];
    static int index = 0;
    buffer[index++ % 1024] = x;
}

void sink_ptr(void* p) {
    volatile static void* buffer[1024];
    static int index = 0;
    buffer[index++ % 1024] = p;
}
