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
volatile float vf_inf = __builtin_inff();

/* Function to generate UNORDERED condition code (unord) */
int test_unordered(double a, double b) {
    /* Using __builtin_isunordered directly */
    if (__builtin_isunordered(a, b)) {
        return 1;
    }
    /* Alternative: checking if either is NaN */
    if (a != a || b != b) {
        return 2;
    }
    return 0;
}

/* Function to generate ORDERED condition code (ord) */
int test_ordered(float a, float b) {
    /* Ordered means neither is NaN */
    if (!__builtin_isunordered(a, b)) {
        return 1;
    }
    /* Alternative explicit check */
    if (a == a && b == b) {
        return 2;
    }
    return 0;
}

/* Function to generate UNEQ condition code (ueq) */
int test_uneq(double a, double b) {
    /* unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) {
        return 1;
    }
    return 0;
}

/* Function to generate UNGE condition code (nlt) */
int test_unge(float a, float b) {
    /* unordered or not less than => not less than (nlt) */
    if (!(a < b)) {  /* This generates nlt when optimized */
        return 1;
    }
    return 0;
}

/* Function to generate UNGT condition code (nle) */
int test_ungt(double a, double b) {
    /* unordered or greater than => not less or equal (nle) */
    if (!(a <= b)) {  /* This generates nle when optimized */
        return 1;
    }
    return 0;
}

/* Function to generate UNLE condition code (ule) */
int test_unle(float a, float b) {
    /* unordered or less or equal */
    if (__builtin_isunordered(a, b) || a <= b) {
        return 1;
    }
    return 0;
}

/* Function to generate UNLT condition code (ult) */
int test_unlt(double a, double b) {
    /* unordered or less than */
    if (__builtin_isunordered(a, b) || a < b) {
        return 1;
    }
    return 0;
}

/* Function to generate LTGT condition code (une) */
int test_ltgt(float a, float b) {
    /* Using __builtin_islessgreater which checks a < b || a > b, both ordered */
    if (__builtin_islessgreater(a, b)) {
        return 1;
    }
    /* Alternative: ordered and not equal */
    if (!__builtin_isunordered(a, b) && a != b) {
        return 2;
    }
    return 0;
}

/* Mixed precision tests */
int test_mixed_precision(double d, float f) {
    int result = 0;
    
    /* Mixed unordered check */
    if (__builtin_isunordered(d, (double)f)) {
        result |= 1;
    }
    
    /* Mixed ordered comparison with inverse */
    if (!(d < (double)f)) {  /* Should generate nlt */
        result |= 2;
    }
    
    /* Mixed lessgreater */
    if (__builtin_islessgreater(d, (double)f)) {
        result |= 4;
    }
    
    return result;
}

/* Test with function returns that might produce NaN */
double maybe_nan(int flag) {
    if (flag) {
        return sqrt(-1.0);  /* Returns NaN */
    }
    return 3.14;
}

float maybe_nanf(int flag) {
    if (flag) {
        return sqrtf(-1.0f);  /* Returns NaN */
    }
    return 2.71f;
}

/* Test using inline assembly for direct control (advanced) */
int test_asm_ordered(double a, double b) {
    int result;
    /* Using inline asm to potentially trigger ord condition code */
    __asm__ volatile (
        "fucomip %%st(1), %%st\n\t"
        "setnp %0"
        : "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Main driver that exercises all conditions */
int main() {
    int checksum = 0;
    
    /* Test with various combinations */
    
    /* 1. UNORDERED tests */
    checksum += test_unordered(vd_nan, vd1);
    checksum += test_unordered(vd1, vd_nan);
    checksum += test_unordered(vd_nan, vd_nan);
    checksum += test_unordered(vd_inf, vd_inf);
    
    /* 2. ORDERED tests */
    checksum += test_ordered(vf1, vf2);
    checksum += test_ordered(vf_nan, vf1);
    checksum += test_ordered(vf1, vf_nan);
    
    /* 3. UNEQ tests */
    checksum += test_uneq(vd1, vd1);      /* Equal */
    checksum += test_uneq(vd_nan, vd1);   /* Unordered */
    checksum += test_uneq(vd1, vd2);      /* Neither */
    
    /* 4. UNGE tests */
    checksum += test_unge(vf2, vf1);      /* Greater */
    checksum += test_unge(vf_nan, vf1);   /* Unordered */
    checksum += test_unge(vf1, vf2);      /* Less */
    
    /* 5. UNGT tests */
    checksum += test_ungt(vd2, vd1);      /* Greater */
    checksum += test_ungt(vd_nan, vd1);   /* Unordered */
    checksum += test_ungt(vd1, vd2);      /* Less */
    
    /* 6. UNLE tests */
    checksum += test_unle(vf1, vf2);      /* Less */
    checksum += test_unle(vf_nan, vf1);   /* Unordered */
    checksum += test_unle(vf2, vf1);      /* Greater */
    
    /* 7. UNLT tests */
    checksum += test_unlt(vd1, vd2);      /* Less */
    checksum += test_unlt(vd_nan, vd1);   /* Unordered */
    checksum += test_unlt(vd2, vd1);      /* Greater */
    
    /* 8. LTGT tests */
    checksum += test_ltgt(vf1, vf2);      /* Less */
    checksum += test_ltgt(vf2, vf1);      /* Greater */
    checksum += test_ltgt(vf_nan, vf1);   /* Unordered */
    checksum += test_ltgt(vf1, vf1);      /* Equal */
    
    /* 9. Mixed precision tests */
    checksum += test_mixed_precision(vd1, vf2);
    checksum += test_mixed_precision(vd_nan, vf1);
    checksum += test_mixed_precision(vd1, vf_nan);
    
    /* 10. Tests with function calls */
    checksum += test_unordered(maybe_nan(1), maybe_nan(0));
    checksum += test_ordered(maybe_nanf(1), maybe_nanf(0));
    checksum += test_uneq(maybe_nan(1), 3.14);
    
    /* 11. Tests with constants */
    if (!(0.0 < NAN)) checksum += 1;      /* Should generate nlt */
    if (!(NAN <= 1.0)) checksum += 2;     /* Should generate nle */
    if (__builtin_isunordered(-0.0, 0.0)) checksum += 4;
    if (__builtin_islessgreater(INFINITY, 1.0)) checksum += 8;
    
    /* 12. Complex expressions */
    double d1 = vd1;
    double d2 = vd2;
    float f1 = vf1;
    float f2 = vf2;
    
    /* Ternary operator with floating comparison */
    int t1 = (__builtin_isunordered(d1, d2) || d1 < d2) ? 1 : 0;
    int t2 = (!(f1 >= f2)) ? 2 : 0;  /* Should generate nlt */
    int t3 = (!(d1 <= d2)) ? 4 : 0;  /* Should generate nle */
    
    checksum += t1 + t2 + t3;
    
    /* Array indexing based on comparison */
    int array[4] = {10, 20, 30, 40};
    int idx1 = (__builtin_isunordered(d1, d2) || d1 == d2) ? 0 : 1;
    int idx2 = (!(f1 < f2)) ? 2 : 3;  /* Should generate nlt */
    
    checksum += array[idx1] + array[idx2];
    
    /* Output checksum to ensure all code executes */
    printf("Checksum: %d\n", checksum);
    
    /* Try inline assembly if supported */
    #ifdef __GNUC__
    checksum += test_asm_ordered(1.0, 2.0);
    #endif
    
    return checksum != 0 ? 0 : 1;
}
