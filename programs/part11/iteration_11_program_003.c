/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization of values */
extern void sink(int);
extern void sink_ptr(const void*);

/* Force functions to not be inlined */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Arrays for memory operand variations */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[2] = {0.0L, 1.0L};

/* Struct for complex memory access */
struct fp_pair {
    volatile float f;
    volatile double d;
};

volatile struct fp_pair fp_struct = {0.0f, 0.0};

/* Pattern 1: UNORDERED comparisons with NaN */
NOINLINE int test_unordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile float c = farr[0];
    volatile long double d = ldarr[0];
    
    int result = 0;
    
    /* Direct NaN comparison - should generate UNORDERED */
    if (a != a) {
        result |= 1;
    }
    
    /* Using isunordered() */
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    /* Complex expression with unordered check */
    result |= (isunordered(c, c) ? 4 : 0);
    
    /* Inline assembly to force condition code use */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
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
NOINLINE int test_ordered(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (a == a) {
        result |= 1;
    }
    
    /* Using isordered() */
    if (isordered(a, b)) {
        result |= 2;
    }
    
    /* Ordered check with NaN */
    if (!isordered(a, nan)) {
        result |= 4;
    }
    
    /* Inline assembly with ordered condition */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l1"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)"
        : ordered_label
    );
    result |= 8;
    goto skip_ordered;
    
ordered_label:
    result |= 16;
    
skip_ordered:
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
NOINLINE int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_one;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Should generate UNEQ: !(a > b) && !(a < b) */
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    /* With NaN - unordered case */
    if (!(nan > a) && !(nan < a)) {
        result |= 2;
    }
    
    /* Complex ternary expression */
    result |= ((!(a > b) && !(a < b)) ? 4 : 0);
    
    /* Memory operand variation */
    volatile double* pa = &darr[0];
    volatile double* pb = &darr[1];
    if (!(*pa > *pb) && !(*pa < *pb)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) */
NOINLINE int test_unge(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_one;
    
    int result = 0;
    
    /* Direct: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* Equal case */
    if (!(c < b)) {
        result |= 2;
    }
    
    /* With struct member */
    if (!(fp_struct.d < b)) {
        result |= 4;
    }
    
    /* Nested in larger expression */
    int temp = (!(a < b)) + (!(b < a));
    result |= (temp << 3);
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) */
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    volatile double c = g_one;
    
    int result = 0;
    
    /* Direct: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if (!(c <= b)) {
        result |= 2;
    }
    
    /* Complex logical expression */
    if (!(a <= b) && (a > b)) {
        result |= 4;
    }
    
    /* Array access */
    if (!(darr[2] <= darr[1])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
NOINLINE int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan > a)) {
        result |= 2;
    }
    
    /* In ternary operator */
    result |= (!(a > b) ? 4 : 0);
    
    /* Combined with other condition */
    if (!(a > b) || (a == b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
NOINLINE int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan >= a)) {
        result |= 2;
    }
    
    /* Complex expression */
    if (!(a >= b) && (a != b)) {
        result |= 4;
    }
    
    /* Memory operand from array */
    if (!(darr[0] >= darr[2])) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
NOINLINE int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double c = g_one;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Direct: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* Equal case should not trigger */
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    /* With NaN - should not trigger */
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    /* Complex nested expression */
    result |= (((a < b) || (a > b)) ? 8 : 0);
    
    /* Using different types */
    volatile float fa = farr[0];
    volatile float fb = farr[2];
    if ((fa < fb) || (fa > fb)) {
        result |= 16;
    }
    
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex function */
NOINLINE int test_mixed_conditions(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    
    int result = 0;
    
    /* Complex expression combining multiple condition codes */
    if (isunordered(x, y)) {
        result |= 1;  /* UNORDERED */
    } else if (!(y > z)) {
        result |= 2;  /* UNLE */
    } else if (!(z < y)) {
        result |= 4;  /* UNGE */
    }
    
    /* Nested ternary with different comparisons */
    int r = (x != x) ? 1 : ((y > z) ? 2 : 3);
    result |= (r << 3);
    
    /* Logical AND/OR combination */
    if ((!(y >= z)) && (!(z <= y))) {
        result |= 32;  /* UNLT && UNGT */
    }
    
    /* Multiple memory accesses */
    volatile double* px = &darr[0];
    volatile double* py = &darr[1];
    volatile double* pz = &darr[2];
    
    if (!(*px > *py) && !(*px < *py)) {
        result |= 64;  /* UNEQ */
    }
    
    if ((*py < *pz) || (*py > *pz)) {
        result |= 128;  /* LTGT */
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Long double operations */
NOINLINE int test_long_double(void) {
    volatile long double a = 0.0L / 0.0L;  /* NaN */
    volatile long double b = 1.0L;
    volatile long double c = 2.0L;
    
    int result = 0;
    
    /* Long double unordered */
    if (isunordered(a, b)) {
        result |= 1;
    }
    
    /* Long double ordered */
    if (isordered(b, c)) {
        result |= 2;
    }
    
    /* Long double comparisons */
    if (!(b > c)) {
        result |= 4;  /* UNLE */
    }
    
    if (!(c < b)) {
        result |= 8;  /* UNGE */
    }
    
    if ((b < c) || (b > c)) {
        result |= 16;  /* LTGT */
    }
    
    sink(result);
    return result;
}

/* Main function with checksum */
int main(void) {
    int checksum = 0;
    
    /* Initialize with volatile operations to prevent optimization */
    volatile int seed = 0;
    sink_ptr(&seed);
    
    /* Call each pattern function */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed_conditions();
    checksum ^= test_long_double();
    
    /* Additional complex calls with different arguments */
    for (int i = 0; i < 3; i++) {
        darr[0] = i * 1.5;
        darr[1] = (i + 1) * 1.5;
        checksum += test_mixed_conditions();
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Dummy sink functions to prevent optimization */
void sink(int x) {
    volatile static int dummy;
    dummy = x;
}

void sink_ptr(const void* p) {
    volatile static const void* dummy;
    dummy = p;
}
