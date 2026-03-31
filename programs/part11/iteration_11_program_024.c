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
    
    /* Direct unordered checks */
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    if (isunordered(nan1, nan2)) {
        result |= 2;
    }
    
    /* Using inline assembly to force condition code */
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unordered_label
    );
    
    result |= 4;
    goto after_unordered;
    
unordered_label:
    result |= 8;

after_unordered:
    
    /* Complex expression with unordered */
    volatile double x = nan1;
    volatile double y = 1.0;
    int r = (x != x) ? 16 : ((y > 0.0) ? 32 : 64);
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
__attribute__((noinline))
int test_ordered(void) {
    volatile double nan = make_nan();
    volatile double a = 1.5;
    volatile double b = 2.5;
    
    int result = 0;
    
    if (isordered(a, b)) {
        result |= 1;
    }
    
    if (isordered(a, a)) {
        result |= 2;
    }
    
    /* Force ordered condition code with assembly */
    volatile double x = 3.0;
    volatile double y = 4.0;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : ordered_label
    );
    
    result |= 4;
    goto after_ordered;
    
ordered_label:
    result |= 8;

after_ordered:
    
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
__attribute__((noinline))
int test_uneq(void) {
    volatile double nan = make_nan();
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which becomes unordered or equal */
    if (!(a != b)) {  /* Should generate UNEQ when optimized */
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan != nan)) {  /* Always true for NaN */
        result |= 2;
    }
    
    /* Complex expression */
    volatile double x = a;
    volatile double y = b;
    if ((x == y) || (x != x) || (y != y)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
__attribute__((noinline))
int test_unge(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double d = 4.0;
    
    int result = 0;
    
    /* Generate UNGE: !(a < b) */
    if (!(a < b)) {  /* a >= b or unordered */
        result |= 1;
    }
    
    if (!(c < b)) {  /* c >= b or unordered */
        result |= 2;
    }
    
    /* With assembly to force condition code */
    volatile double x = 5.0;
    volatile double y = 5.0;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[unge_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unge_label
    );
    
    result |= 4;
    goto after_unge;
    
unge_label:
    result |= 8;

after_unge:
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
__attribute__((noinline))
int test_ungt(void) {
    volatile double a = 4.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    
    int result = 0;
    
    /* Generate UNGT: !(a <= b) */
    if (!(a <= b)) {  /* a > b or unordered */
        result |= 1;
    }
    
    if (!(c <= b)) {  /* c > b or unordered */
        result |= 2;
    }
    
    /* Using different types */
    volatile float f1 = 5.0f;
    volatile float f2 = 4.0f;
    if (!(f1 <= f2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
__attribute__((noinline))
int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    
    int result = 0;
    
    /* Generate UNLE: !(a > b) */
    if (!(a > b)) {  /* a <= b or unordered */
        result |= 1;
    }
    
    if (!(c > b)) {  /* c <= b or unordered */
        result |= 2;
    }
    
    /* With long double */
    volatile long double ld1 = 2.5L;
    volatile long double ld2 = 3.5L;
    if (!(ld1 > ld2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
__attribute__((noinline))
int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 2.0;
    
    int result = 0;
    
    /* Generate UNLT: !(a >= b) */
    if (!(a >= b)) {  /* a < b or unordered */
        result |= 1;
    }
    
    if (!(c >= b)) {  /* c < b or unordered */
        result |= 2;
    }
    
    /* Using memory operands */
    volatile double arr[3] = {1.0, 2.0, 3.0};
    if (!(arr[0] >= arr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (not equal and ordered) */
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Generate LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {  /* Not equal and ordered */
        result |= 1;
    }
    
    if ((b < a) || (b > a)) {  /* Not equal and ordered */
        result |= 2;
    }
    
    /* With NaN (should be false) */
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    volatile double x = 5.0;
    volatile double y = 6.0;
    volatile double z = 7.0;
    int r = ((x < y) || (x > y)) ? 8 : (((y < z) || (y > z)) ? 16 : 32);
    result |= r;
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed types and complex expressions */
__attribute__((noinline))
int test_mixed(void) {
    volatile float f_nan = 0.0f / 0.0f;
    volatile double d_nan = make_nan();
    volatile long double ld_nan = 0.0L / 0.0L;
    
    volatile float f1 = 1.5f;
    volatile double d1 = 2.5;
    volatile long double ld1 = 3.5L;
    
    int result = 0;
    
    /* Cross-type comparisons */
    if (isunordered(f_nan, d1)) {
        result |= 1;
    }
    
    if (!(f1 >= (float)d1)) {  /* UNLT */
        result |= 2;
    }
    
    if (!(ld1 <= (long double)d1)) {  /* UNGT */
        result |= 4;
    }
    
    /* Struct with volatile members */
    struct {
        volatile double x;
        volatile double y;
    } point = {1.0, 2.0};
    
    if (!(point.x > point.y)) {  /* UNLE */
        result |= 8;
    }
    
    /* Array access */
    volatile double arr[4] = {1.0, NAN, 3.0, 4.0};
    if ((arr[0] < arr[2]) || (arr[0] > arr[2])) {  /* LTGT */
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Nested condition codes */
__attribute__((noinline))
int test_nested(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    /* Deeply nested ternary with different condition codes */
    result = (a != a) ? 1 : 
             (!(b < c) ? 2 :  /* UNGE */
             (!(c > a) ? 4 :  /* UNLE */
             ((b < a) || (b > a) ? 8 : 16)));  /* LTGT */
    
    /* Logical AND/OR combinations */
    if ((!(a >= b)) && (!(b <= c))) {  /* UNLT && UNGT */
        result |= 32;
    }
    
    if ((isunordered(nan, a)) || (!(a == b))) {  /* UNORDERED || UNEQ */
        result |= 64;
    }
    
    sink(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile seed */
    volatile int seed = 42;
    srand(seed);
    
    /* Call all pattern functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed();
    checksum ^= test_nested();
    
    /* Print checksum to ensure all code is live */
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
