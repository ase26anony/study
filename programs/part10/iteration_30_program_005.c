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

/* Function to generate NaN */
double get_nan() {
    return __builtin_nan("");
}

/* Function to generate infinity */
double get_inf() {
    return __builtin_inf();
}

/* Test UNORDERED condition code (unord) */
int test_unordered() {
    double a = vd_nan;
    double b = vd1;
    float c = vf_nan;
    float d = vf1;
    
    int res = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(a, b)) res |= 1;
    if (__builtin_isunordered(c, d)) res |= 2;
    
    /* Using !(a == a) NaN check */
    double e = get_nan();
    if (!(e == e)) res |= 4;
    
    /* Mixed types */
    if (__builtin_isunordered(vd_nan, (double)vf1)) res |= 8;
    
    return res;
}

/* Test ORDERED condition code (ord) */
int test_ordered() {
    double a = vd1;
    double b = vd2;
    float c = vf1;
    float d = vf2;
    
    int res = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(a, b)) res |= 1;
    if (!__builtin_isunordered(c, d)) res |= 2;
    
    /* Compare with constant */
    if (!__builtin_isunordered(3.14, 2.71)) res |= 4;
    
    /* Function return */
    if (!__builtin_isunordered(sin(0.0), cos(0.0))) res |= 8;
    
    return res;
}

/* Test UNEQ condition code (ueq) */
int test_uneq() {
    double a = vd1;
    double b = vd1;
    double c = vd_nan;
    
    int res = 0;
    
    /* Equality with possible NaN */
    if (a == b) res |= 1;
    
    /* Using builtin */
    if (!__builtin_islessgreater(a, b)) res |= 2;
    
    /* With NaN operand */
    if (!__builtin_islessgreater(c, a)) res |= 4;
    
    /* Float version */
    float f1 = vf1;
    float f2 = vf1;
    if (f1 == f2) res |= 8;
    
    return res;
}

/* Test UNGE condition code (nlt) */
int test_unge() {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    int res = 0;
    
    /* Inverse condition: !(a < b) */
    if (!(a < b)) res |= 1;
    if (!(b < a)) res |= 2;
    
    /* With NaN */
    if (!(c < b)) res |= 4;
    
    /* Float version */
    if (!(vf2 < vf1)) res |= 8;
    
    return res;
}

/* Test UNGT condition code (nle) */
int test_ungt() {
    double a = vd2;
    double b = vd1;
    double c = vd_nan;
    
    int res = 0;
    
    /* Inverse condition: !(a <= b) */
    if (!(a <= b)) res |= 1;
    if (!(b <= a)) res |= 2;
    
    /* With NaN */
    if (!(c <= b)) res |= 4;
    
    /* Mixed precision */
    if (!((double)vf2 <= vd1)) res |= 8;
    
    return res;
}

/* Test UNLE condition code (ule) */
int test_unle() {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int res = 0;
    
    /* Using builtin or direct comparison */
    if (a <= b) res |= 1;
    
    /* With NaN operand */
    if (c <= b) res |= 2;
    
    /* Function call in comparison */
    if (sin(0.0) <= cos(0.0)) res |= 4;
    
    return res;
}

/* Test UNLT condition code (ult) */
int test_unlt() {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int res = 0;
    
    /* Direct less-than with possible NaN */
    if (a < b) res |= 1;
    
    /* With NaN operand */
    if (c < b) res |= 2;
    
    /* Float version */
    if (vf1 < vf2) res |= 4;
    
    /* Compare with zero */
    if (-0.0 < 0.0) res |= 8;
    
    return res;
}

/* Test LTGT condition code (une) */
int test_ltgt() {
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    int res = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(a, b)) res |= 1;
    
    /* Equivalent: (a < b) || (a > b) */
    if ((a < b) || (a > b)) res |= 2;
    
    /* With NaN */
    if (__builtin_islessgreater(c, a)) res |= 4;
    
    /* Float version */
    if (__builtin_islessgreater(vf1, vf2)) res |= 8;
    
    return res;
}

/* Test mixed conditions in control flow */
int test_control_flow() {
    double x = vd1;
    double y = vd2;
    double z = vd_nan;
    int result = 0;
    
    /* Complex if-else chain with various conditions */
    if (__builtin_isunordered(x, y)) {
        result += 1;
    } else if (!(x >= y)) {  /* nlt */
        result += 2;
    } else if (!(x <= y)) {  /* nle */
        result += 4;
    } else if (x == y) {     /* ueq */
        result += 8;
    }
    
    /* While loop with condition */
    int count = 0;
    while (count < 3 && __builtin_islessgreater(x + count, y)) {
        result += 16;
        count++;
    }
    
    /* Ternary operator */
    result += (z != z) ? 32 : 64;  /* NaN check */
    
    return result;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_control_flow();
    
    /* Additional inline tests to vary code generation */
    {
        volatile double a = 0.0;
        volatile double b = -0.0;
        volatile double c = get_nan();
        
        /* Array indexing based on comparisons */
        int arr[4] = {0};
        arr[__builtin_isunordered(a, c) ? 0 : 1] = 1;
        arr[!(a >= b) ? 2 : 3] = 2;  /* nlt */
        arr[!(a <= b) ? 0 : 1] = 3;  /* nle */
        
        for (int i = 0; i < 4; i++) {
            checksum += arr[i];
        }
        
        /* Complex expression */
        double x = sin(1.0);
        double y = cos(1.0);
        checksum += (__builtin_islessgreater(x, y) ? 100 : 200);
        checksum += (!(x < y) ? 300 : 400);  /* nlt */
        checksum += ((x <= y) ? 500 : 600);  /* ule */
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all tests were executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
