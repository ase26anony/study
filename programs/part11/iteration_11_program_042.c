/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization of intermediate values */
extern void sink(int value);
extern void sink_ptr(const void *ptr);

/* Volatile variables to prevent constant folding */
volatile double g_nan = 0.0 / 0.0;
volatile double g_inf = 1.0 / 0.0;
volatile double g_neg_inf = -1.0 / 0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Volatile arrays for memory operands */
volatile double arr[10] = {0.0};
volatile float farr[10] = {0.0f};
volatile long double ldarr[10] = {0.0L};

/* Struct with volatile members for varied addressing */
struct fp_pair {
    volatile double a;
    volatile double b;
    volatile float c;
    volatile float d;
    volatile long double e;
    volatile long double f;
};

volatile struct fp_pair fp_struct;

/* Function prototypes with noinline to isolate patterns */
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
__attribute__((noinline)) double make_nan(void) {
    volatile double x = 0.0;
    volatile double y = 0.0;
    return x / y;  /* Produces NaN */
}

/* Helper to generate infinity */
__attribute__((noinline)) double make_inf(void) {
    volatile double x = 1.0;
    volatile double y = 0.0;
    return x / y;  /* Produces inf */
}

/* Test UNORDERED condition codes */
__attribute__((noinline)) int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    /* Direct unordered comparisons */
    if (nan1 != nan1) {
        result |= 1;  /* unordered */
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
        : "st", "st(1)", "cc"
        : unordered_label
    );
    
    /* Not taken path */
    result |= 4;
    goto done;
    
unordered_label:
    /* Taken path - unordered */
    result |= 8;
    
done:
    sink(result);
    return result;
}

/* Test ORDERED condition codes */
__attribute__((noinline)) int test_ordered(void) {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Direct ordered comparisons */
    if (normal1 == normal1) {
        result |= 1;  /* ordered */
    }
    
    /* Using isordered() */
    if (isordered(normal1, normal2)) {
        result |= 2;
    }
    
    /* Inline assembly with ordered condition */
    volatile double a = normal1;
    volatile double b = normal2;
    
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : ordered_label
    );
    
    /* Not taken path */
    result |= 4;
    goto done2;
    
ordered_label:
    /* Taken path - ordered */
    result |= 8;
    
done2:
    sink(result);
    return result;
}

/* Test UNEQ (unordered or equal) */
__attribute__((noinline)) int test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is a == b OR unordered */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* Another UNEQ pattern with NaN */
    if (!(nan != nan)) {  /* This is true for NaN */
        result |= 2;
    }
    
    /* Complex expression generating UNEQ */
    result |= ((a == b) || (nan != nan)) ? 4 : 0;
    
    sink(result);
    return result;
}

/* Test UNGE (not less than) */
__attribute__((noinline)) int test_unge(void) {
    volatile double a = 2.0;
    volatile double b = 1.0;
    volatile double c = 1.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;  /* a >= b */
    }
    
    if (!(b < a)) {
        result |= 2;  /* b >= a */
    }
    
    /* With NaN - should be unordered */
    if (!(nan < a)) {
        result |= 4;
    }
    
    /* Complex expression */
    result |= ((a >= b) && !(c < b)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Test UNGT (not less than or equal) */
__attribute__((noinline)) int test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 2.0;
    int result = 0;
    
    /* Generate UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;  /* a > b */
    }
    
    if (!(b <= a)) {
        result |= 2;  /* b > a (false) */
    }
    
    /* Using > directly should also work */
    if (a > b) {
        result |= 4;
    }
    
    /* Complex expression with ternary */
    result |= (!(c <= b) ? 8 : 0);
    
    sink(result);
    return result;
}

/* Test UNLE (unordered or less than or equal) */
__attribute__((noinline)) int test_unle(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;  /* a <= b */
    }
    
    if (!(b > a)) {
        result |= 2;  /* b <= a (false) */
    }
    
    /* With NaN */
    if (!(nan > a)) {
        result |= 4;
    }
    
    /* Complex: (a <= b) || isunordered(a, b) */
    result |= ((a <= b) || (nan != nan)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Test UNLT (unordered or less than) */
__attribute__((noinline)) int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;  /* a < b */
    }
    
    if (!(b >= a)) {
        result |= 2;  /* b < a (false) */
    }
    
    /* With NaN */
    if (!(nan >= a)) {
        result |= 4;
    }
    
    /* Complex expression */
    result |= ((a < b) && !(c >= b)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Test LTGT (not equal and ordered) */
__attribute__((noinline)) int test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    /* Generate LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;  /* a != b and ordered */
    }
    
    if ((b < c) || (b > c)) {
        result |= 2;  /* false, b == c */
    }
    
    /* With NaN - should be false */
    if ((nan < a) || (nan > a)) {
        result |= 4;  /* shouldn't execute */
    }
    
    /* Complex: !(a == b) && !isunordered(a, b) */
    result |= (!(a == b) && (a == a)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Test with mixed floating-point types */
__attribute__((noinline)) int test_mixed_types(void) {
    volatile float f1 = 1.0f;
    volatile float f2 = 2.0f;
    volatile double d1 = 3.0;
    volatile double d2 = 4.0;
    volatile long double ld1 = 5.0L;
    volatile long double ld2 = 6.0L;
    int result = 0;
    
    /* Float comparisons */
    if (!(f1 < f2)) result |= 1;   /* UNGE */
    if (!(f1 > f2)) result |= 2;   /* UNLE */
    
    /* Double comparisons */
    if (!(d1 <= d2)) result |= 4;  /* UNGT */
    if (!(d1 >= d2)) result |= 8;  /* UNLT */
    
    /* Long double comparisons */
    if ((ld1 < ld2) || (ld1 > ld2)) result |= 16; /* LTGT */
    
    /* Mixed type comparisons */
    if (!((double)f1 < d1)) result |= 32;
    if (!(d2 > (float)ld1)) result |= 64;
    
    sink(result);
    return result;
}

/* Test complex nested expressions */
__attribute__((noinline)) int test_complex_expressions(void) {
    volatile double x = make_nan();
    volatile double y = 1.0;
    volatile double z = 2.0;
    int result = 0;
    
    /* Nested ternary with unordered check */
    result = (x != x) ? 1 : ((y > z) ? 2 : 3);
    
    /* Complex logical expression */
    int r2 = ((x == x) && (y < z)) || (!(z >= y) && (x != x));
    result |= (r2 << 4);
    
    /* Multiple condition codes combined */
    int r3 = (!(y < z) ? 1 : 0) + (!(z > y) ? 2 : 0) + 
             ((y < z) || (y > z) ? 4 : 0);
    result |= (r3 << 8);
    
    sink(result);
    return result;
}

/* Test with memory operands in various addressing modes */
__attribute__((noinline)) int test_memory_operands(void) {
    int result = 0;
    
    /* Initialize array with special values */
    arr[0] = make_nan();
    arr[1] = 1.0;
    arr[2] = 2.0;
    arr[3] = make_inf();
    
    farr[0] = 0.0f / 0.0f;  /* NaN */
    farr[1] = 1.5f;
    farr[2] = 2.5f;
    
    ldarr[0] = 0.0L / 0.0L;  /* Long double NaN */
    ldarr[1] = 3.14L;
    
    /* Struct member access */
    fp_struct.a = make_nan();
    fp_struct.b = 2.0;
    fp_struct.c = 1.0f;
    fp_struct.d = 3.0f;
    fp_struct.e = 4.0L;
    fp_struct.f = make_nan();
    
    /* Array element comparisons */
    if (!(arr[1] < arr[2])) result |= 1;      /* UNGE */
    if (!(farr[1] > farr[2])) result |= 2;    /* UNLE */
    if ((ldarr[0] < ldarr[1]) || (ldarr[0] > ldarr[1])) result |= 4; /* LTGT */
    
    /* Struct member comparisons */
    if (!(fp_struct.a >= fp_struct.b)) result |= 8;   /* UNLT */
    if (!(fp_struct.c <= fp_struct.d)) result |= 16;  /* UNGT */
    
    /* Complex addressing */
    volatile double *ptr = &arr[0];
    if (!(ptr[1] < ptr[2])) result |= 32;
    
    sink(result);
    sink_ptr(ptr);
    return result;
}

/* Dummy sink functions to prevent optimization */
void sink(int value) {
    /* Use inline assembly to prevent removal */
    __asm__ volatile ("" : : "r"(value) : "memory");
}

void sink_ptr(const void *ptr) {
    __asm__ volatile ("" : : "r"(ptr) : "memory");
}

int main(void) {
    int checksum = 0;
    
    printf("Testing i386 condition code patterns...\n");
    
    /* Initialize global volatile values */
    g_nan = make_nan();
    g_inf = make_inf();
    g_neg_inf = -make_inf();
    g_zero = 0.0;
    g_one = 1.0;
    g_two = 2.0;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        arr[i] = (double)i;
        farr[i] = (float)i;
        ldarr[i] = (long double)i;
    }
    
    /* Call each test function and accumulate results */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed_types();
    checksum += test_complex_expressions();
    checksum += test_memory_operands();
    
    /* Print final checksum to ensure all code is live */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
