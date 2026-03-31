/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = __builtin_nan("");
volatile double vd_inf = __builtin_inf();
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = __builtin_nanf("");

/* Function to generate UNORDERED condition code (unord) */
int test_unordered(double a, double b) {
    /* Using __builtin_isunordered directly */
    int res1 = __builtin_isunordered(a, b);
    
    /* Alternative: comparing with NaN */
    int res2 = !(a == a) || !(b == b);
    
    /* Using volatile to force code generation */
    volatile double x = a;
    volatile double y = b;
    int res3 = __builtin_isunordered(x, y);
    
    return res1 + res2 + res3;
}

/* Function to generate ORDERED condition code (ord) */
int test_ordered(float a, float b) {
    /* Ordered is the opposite of unordered */
    int res1 = !__builtin_isunordered(a, b);
    
    /* Direct ordered check */
    int res2 = (a == a) && (b == b);
    
    /* Using in control flow */
    if (__builtin_isunordered(a, b)) {
        return 0;
    } else {
        return 1 + res2;
    }
}

/* Function to generate UNEQ condition code (ueq) */
int test_uneq(double a, double b) {
    /* UNEQ: unordered or equal */
    int res = 0;
    
    /* This should generate ueq when used in conditional */
    if (!(a > b) && !(a < b)) {
        res = 1;
    }
    
    /* Alternative using builtin */
    res += __builtin_isunordered(a, b) || (a == b);
    
    return res;
}

/* Function to generate UNGE condition code (nlt) */
int test_unge(float a, float b) {
    /* UNGE: unordered or not less than (nlt) */
    int res = 0;
    
    /* Using !(a < b) which should generate nlt */
    if (!(a < b)) {
        res = 1;
    }
    
    /* With volatile */
    volatile float x = a;
    volatile float y = b;
    res += !(x < y);
    
    return res;
}

/* Function to generate UNGT condition code (nle) */
int test_ungt(double a, double b) {
    /* UNGT: unordered or not less or equal (nle) */
    int res = 0;
    
    /* Using !(a <= b) which should generate nle */
    res = !(a <= b);
    
    /* In ternary operator */
    return res ? 2 : 1;
}

/* Function to generate UNLE condition code (ule) */
int test_unle(float a, float b) {
    /* UNLE: unordered or less or equal */
    int res = 0;
    
    /* This pattern might generate ule */
    if (__builtin_isunordered(a, b) || (a <= b)) {
        res = 1;
    }
    
    return res;
}

/* Function to generate UNLT condition code (ult) */
int test_unlt(double a, double b) {
    /* UNLT: unordered or less than */
    int res = 0;
    
    if (__builtin_isunordered(a, b) || (a < b)) {
        res = 1;
    }
    
    return res;
}

/* Function to generate LTGT condition code (une) */
int test_ltgt(float a, float b) {
    /* LTGT: less than or greater than (ordered and not equal) */
    int res = 0;
    
    /* Using __builtin_islessgreater */
    res = __builtin_islessgreater(a, b);
    
    /* Alternative: (a < b) || (a > b) with ordered check */
    if ((a < b) || (a > b)) {
        res += 1;
    }
    
    return res;
}

/* Mixed precision tests */
int test_mixed_precision(void) {
    int res = 0;
    float f = vf1;
    double d = vd1;
    
    /* float vs double comparisons */
    res += !(f < d);      /* nlt */
    res += !(f <= d);     /* nle */
    res += __builtin_isunordered(f, d); /* unord */
    
    return res;
}

/* Tests with function returns that might produce NaN */
double maybe_nan(int flag) {
    return flag ? __builtin_nan("") : 3.14;
}

float maybe_nanf(int flag) {
    return flag ? __builtin_nanf("") : 2.71f;
}

int test_with_function_calls(void) {
    int res = 0;
    
    /* Compare function results */
    double d1 = maybe_nan(0);
    double d2 = maybe_nan(1);
    
    res += __builtin_isunordered(d1, d2);  /* unord */
    res += !__builtin_isunordered(d1, d2); /* ord */
    res += !(d1 < d2);                     /* nlt */
    
    return res;
}

/* Array indexing based on FP comparisons */
int test_array_indexing(void) {
    static const int array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int idx = 0;
    
    /* Use comparisons to compute array index */
    if (!(vd1 < vd2)) idx += 1;      /* nlt */
    if (!(vd1 <= vd2)) idx += 2;     /* nle */
    if (__builtin_isunordered(vd1, vd_nan)) idx += 4; /* unord */
    
    return array[idx & 7];
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test with NaN */
    checksum += test_unordered(vd1, vd_nan);
    checksum += test_ordered(vf1, vf_nan);
    checksum += test_uneq(vd_nan, vd_nan);
    checksum += test_unge(vf1, vf_nan);
    checksum += test_ungt(vd_nan, vd1);
    checksum += test_unle(vf_nan, vf1);
    checksum += test_unlt(vd1, vd_nan);
    checksum += test_ltgt(vf1, vf2);
    
    /* Test with normal values */
    checksum += test_unordered(vd1, vd2);
    checksum += test_ordered(vf1, vf2);
    checksum += test_uneq(vd1, vd1);
    checksum += test_unge(vf2, vf1);
    checksum += test_ungt(vd2, vd1);
    checksum += test_unle(vf1, vf2);
    checksum += test_unlt(vd1, vd2);
    checksum += test_ltgt(vf2, vf1);
    
    /* Test with infinity */
    checksum += test_unordered(vd_inf, vd_nan);
    checksum += test_ordered(vd_inf, vd1);
    
    /* Mixed tests */
    checksum += test_mixed_precision();
    checksum += test_with_function_calls();
    checksum += test_array_indexing();
    
    /* Additional volatile tests to force code generation */
    volatile double a = 0.0;
    volatile double b = -0.0;
    volatile int r;
    
    /* These should generate various condition codes */
    r = !__builtin_isunordered(a, b); checksum += r;  /* ord */
    r = !(a >= b); checksum += r;                     /* nlt? */
    r = !(a > b); checksum += r;                      /* nle? */
    r = __builtin_islessgreater(a, b); checksum += r; /* une */
    
    printf("Checksum: %d\n", checksum);
    
    /* Return non-zero if any NaN comparisons succeeded */
    return checksum == 0 ? 0 : 1;
}
