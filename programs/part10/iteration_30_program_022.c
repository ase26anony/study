/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Helper to generate NaN values */
static double get_nan(void) {
    return __builtin_nan("");
}

static float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code (unord) */
void test_unordered(void) {
    volatile double d1 = get_nan();
    volatile double d2 = 3.14;
    volatile float f1 = get_nanf();
    volatile float f2 = 2.71f;
    
    int res1 = __builtin_isunordered(d1, d2);  /* Should generate unord check */
    int res2 = __builtin_isunordered(f1, f2);  /* Should generate unord check */
    int res3 = !(d1 == d1);                    /* Alternative NaN check */
    
    printf("UNORDERED tests: %d %d %d\n", res1, res2, res3);
}

/* Test ORDERED condition code (ord) */
void test_ordered(void) {
    volatile double d1 = 1.0;
    volatile double d2 = 2.0;
    volatile float f1 = 1.0f;
    volatile float f2 = get_nanf();
    
    int res1 = !__builtin_isunordered(d1, d2);  /* Ordered check */
    int res2 = (d1 == d1) && (d2 == d2);        /* Both are numbers */
    int res3 = !__builtin_isunordered(f1, f2);  /* One is NaN, should be false */
    
    printf("ORDERED tests: %d %d %d\n", res1, res2, res3);
}

/* Test UNEQ condition code (ueq) */
void test_uneq(void) {
    volatile double a = 0.0;
    volatile double b = -0.0;  /* -0.0 equals 0.0 but has different bit pattern */
    volatile double c = get_nan();
    
    /* UNEQ: unordered or equal - true if either unordered OR equal */
    int res1 = __builtin_isunordered(a, c) || (a == b);
    int res2 = !(a != b);  /* Alternative: not less/greater and not unordered? */
    
    /* Using inline asm for explicit control */
    int res3;
    double x = 1.0, y = 1.0;
    asm volatile (
        "fucomip %1, %0\n\t"
        "setp %%al\n\t"
        "sete %%ah\n\t"
        "or %%ah, %%al\n\t"
        "movzbl %%al, %2"
        : "=t"(x), "+u"(y), "=r"(res3)
        : "0"(x), "1"(y)
        : "eax"
    );
    
    printf("UNEQ tests: %d %d %d\n", res1, res2, res3);
}

/* Test UNGE condition code (nlt) */
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double c = get_nan();
    
    /* UNGE: unordered or not less than (nlt) */
    int res1 = __builtin_isunordered(a, c) || !(a < b);
    int res2 = !(a < b);  /* Just the nlt part */
    
    /* Using ! operator on < to get nlt */
    int res3 = !(b < a);  /* 3.0 < 5.0 is true, so !(3.0 < 5.0) is false */
    
    printf("UNGE tests: %d %d %d\n", res1, res2, res3);
}

/* Test UNGT condition code (nle) */
void test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double c = get_nan();
    
    /* UNGT: unordered or not less than or equal (nle) */
    int res1 = __builtin_isunordered(a, c) || !(a <= b);
    int res2 = !(a <= b);  /* Just the nle part */
    
    /* Test with different values */
    volatile float f1 = 7.0f;
    volatile float f2 = 6.0f;
    int res3 = !(f2 <= f1);  /* !(6.0 <= 7.0) = false */
    
    printf("UNGT tests: %d %d %d\n", res1, res2, res3);
}

/* Test UNLE condition code (ule) */
void test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double nan = get_nan();
    
    /* UNLE: unordered or less than or equal */
    int res1 = __builtin_isunordered(a, nan) || (a <= b);
    int res2 = (a <= b) || __builtin_isunordered(b, nan);
    
    /* Mixed float/double */
    volatile float f1 = 4.0f;
    volatile double d1 = 4.0;
    int res3 = (f1 <= d1) || __builtin_isunordered(f1, d1);
    
    printf("UNLE tests: %d %d %d\n", res1, res2, res3);
}

/* Test UNLT condition code (ult) */
void test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = get_nan();
    
    /* UNLT: unordered or less than */
    int res1 = __builtin_isunordered(a, nan) || (a < b);
    int res2 = (a < b) || __builtin_isunordered(a, b);
    
    /* Using function return values */
    int res3 = (sin(a) < cos(b)) || __builtin_isunordered(sin(a), cos(b));
    
    printf("UNLT tests: %d %d %d\n", res1, res2, res3);
}

/* Test LTGT condition code (une) */
void test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = get_nan();
    
    /* LTGT: less than or greater than (ordered and not equal) */
    int res1 = __builtin_islessgreater(a, b);  /* Direct builtin */
    int res2 = (a < b) || (a > b);  /* Ordered comparison */
    int res3 = __builtin_islessgreater(b, c);  /* Equal values, should be false */
    
    /* Test with NaN */
    int res4 = __builtin_islessgreater(a, nan);  /* With NaN, should be false */
    
    printf("LTGT tests: %d %d %d %d\n", res1, res2, res3, res4);
}

/* Complex test mixing multiple conditions in control flow */
void test_control_flow(void) {
    volatile double values[] = {1.0, 2.0, get_nan(), 3.0, 0.0/0.0};  /* Last one is also NaN */
    int results[10];
    int idx = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            /* Use different condition codes in if statements */
            if (__builtin_isunordered(values[i], values[j])) {
                results[idx++] = 1;  /* UNORDERED */
            } else if (__builtin_islessgreater(values[i], values[j])) {
                results[idx++] = 2;  /* LTGT */
            } else if (!(values[i] < values[j])) {
                results[idx++] = 3;  /* UNGE (nlt) */
            } else if (values[i] <= values[j]) {
                results[idx++] = 4;  /* UNLE */
            } else {
                results[idx++] = 0;
            }
        }
    }
    
    printf("Control flow results: ");
    for (int i = 0; i < idx; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
}

/* Main driver that uses all comparison results */
int main(void) {
    unsigned int checksum = 0;
    
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_control_flow();
    
    /* Create a checksum based on actual comparisons to ensure they execute */
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double nan = get_nan();
    
    checksum += __builtin_isunordered(a, nan);
    checksum += !__builtin_isunordered(a, b);
    checksum += __builtin_islessgreater(a, b);
    checksum += !(a >= b);  /* nlt */
    checksum += !(a <= b);  /* nle */
    checksum += (__builtin_isunordered(a, nan) || (a <= b));
    checksum += (__builtin_isunordered(a, nan) || (a < b));
    
    printf("Final checksum: %u\n", checksum);
    
    return 0;
}
