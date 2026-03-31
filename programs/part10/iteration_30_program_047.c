/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f / 0.0f;  /* Generate NaN */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = 0.0 / 0.0;

/* Function to generate NaN */
static double get_nan(void) {
    return __builtin_nan("");
}

static float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code (unord) */
int test_unordered(float a, float b) {
    int result = 0;
    /* Direct unordered check - should generate "unord" */
    if (__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Alternative unordered check */
    if (a != a || b != b) {  /* NaN check */
        result |= 2;
    }
    
    /* Mixed types */
    double da = a;
    if (__builtin_isunordered(da, vd_nan)) {
        result |= 4;
    }
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered(float a, float b) {
    int result = 0;
    
    /* Ordered check - should generate "ord" */
    if (!__builtin_isunordered(a, b)) {
        result |= 1;
    }
    
    /* Ordered comparison with constants */
    if (a == a && b == b) {  /* Both are not NaN */
        result |= 2;
    }
    
    /* With function return */
    float f = get_nanf();
    if (!__builtin_isunordered(a, f)) {
        result |= 4;
    }
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq(float a, float b) {
    int result = 0;
    
    /* Unordered or equal - should generate "ueq" */
    if (__builtin_isunordered(a, b) || a == b) {
        result |= 1;
    }
    
    /* Using volatile to prevent constant folding */
    volatile float v1 = a;
    volatile float v2 = b;
    if (v1 != v1 || v2 != v2 || v1 == v2) {
        result |= 2;
    }
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge(float a, float b) {
    int result = 0;
    
    /* Unordered or not less than - should generate "nlt" */
    if (__builtin_isunordered(a, b) || !(a < b)) {
        result |= 1;
    }
    
    /* Alternative: !(a < b) with NaN possibility */
    if (!(a < b)) {
        result |= 2;
    }
    
    /* With double precision */
    double da = a;
    double db = b;
    if (__builtin_isunordered(da, db) || !(da < db)) {
        result |= 4;
    }
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt(float a, float b) {
    int result = 0;
    
    /* Unordered or not less or equal - should generate "nle" */
    if (__builtin_isunordered(a, b) || !(a <= b)) {
        result |= 1;
    }
    
    /* Using builtin */
    if (!__builtin_islessequal(a, b)) {
        result |= 2;
    }
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle(float a, float b) {
    int result = 0;
    
    /* Unordered or less or equal - should generate "ule" */
    if (__builtin_isunordered(a, b) || a <= b) {
        result |= 1;
    }
    
    /* With builtin */
    if (__builtin_isunordered(a, b) || __builtin_islessequal(a, b)) {
        result |= 2;
    }
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt(float a, float b) {
    int result = 0;
    
    /* Unordered or less than - should generate "ult" */
    if (__builtin_isunordered(a, b) || a < b) {
        result |= 1;
    }
    
    /* Using builtin */
    if (__builtin_isunordered(a, b) || __builtin_isless(a, b)) {
        result |= 2;
    }
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt(float a, float b) {
    int result = 0;
    
    /* Less or greater (unordered not equal) - should generate "une" */
    if (__builtin_islessgreater(a, b)) {
        result |= 1;
    }
    
    /* Alternative: ordered and not equal */
    if (!__builtin_isunordered(a, b) && a != b) {
        result |= 2;
    }
    
    /* Mixed with double */
    double da = a;
    double db = b;
    if (__builtin_islessgreater(da, db)) {
        result |= 4;
    }
    
    return result;
}

/* Test with various special values */
int test_special_values(void) {
    int result = 0;
    
    float inf = 1.0f / 0.0f;
    float neg_inf = -1.0f / 0.0f;
    float zero = 0.0f;
    float neg_zero = -0.0f;
    
    /* Test with infinity */
    result += test_unordered(inf, vf_nan);
    result += test_ordered(inf, 1.0f);
    result += test_uneq(inf, inf);
    result += test_unge(inf, 1.0f);
    result += test_ungt(inf, 1.0f);
    result += test_unle(1.0f, inf);
    result += test_unlt(1.0f, inf);
    result += test_ltgt(inf, neg_inf);
    
    /* Test with zero and negative zero */
    result += test_unordered(zero, neg_zero);
    result += test_uneq(zero, neg_zero);
    
    /* Test with NaN on both sides */
    result += test_unordered(vf_nan, vf_nan);
    result += test_ordered(vf_nan, vf_nan);
    result += test_uneq(vf_nan, vf_nan);
    
    return result;
}

/* Test using inline assembly for direct control */
int test_asm_direct(void) {
    int result = 0;
    float a = 1.5f;
    float b = 2.5f;
    float c = get_nanf();
    int res1, res2, res3;
    
    /* Direct assembly with fucomi - may generate various condition codes */
    __asm__ volatile (
        "flds %1\n\t"
        "flds %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "seta %0\n\t"
        : "=r" (res1)
        : "m" (a), "m" (b)
        : "cc", "st", "st(1)"
    );
    
    __asm__ volatile (
        "flds %1\n\t"
        "flds %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "setp %0\n\t"
        : "=r" (res2)
        : "m" (a), "m" (c)
        : "cc", "st", "st(1)"
    );
    
    result = res1 + res2;
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Test with normal values */
    checksum += test_unordered(1.0f, 2.0f);
    checksum += test_ordered(1.0f, 2.0f);
    checksum += test_uneq(1.0f, 1.0f);
    checksum += test_unge(3.0f, 2.0f);
    checksum += test_ungt(3.0f, 2.0f);
    checksum += test_unle(1.0f, 2.0f);
    checksum += test_unlt(1.0f, 2.0f);
    checksum += test_ltgt(1.0f, 2.0f);
    
    /* Test with NaN */
    checksum += test_unordered(1.0f, get_nanf());
    checksum += test_ordered(1.0f, get_nanf());
    checksum += test_uneq(get_nanf(), get_nanf());
    checksum += test_unge(get_nanf(), 2.0f);
    checksum += test_ungt(get_nanf(), 2.0f);
    checksum += test_unle(1.0f, get_nanf());
    checksum += test_unlt(1.0f, get_nanf());
    checksum += test_ltgt(1.0f, get_nanf());
    
    /* Test with volatile variables */
    checksum += test_unordered(vf1, vf2);
    checksum += test_ordered(vf1, vf_nan);
    checksum += test_uneq(vf_nan, vf_nan);
    checksum += test_unge(vf2, vf1);
    checksum += test_ungt(vf2, vf1);
    checksum += test_unle(vf1, vf2);
    checksum += test_unlt(vf1, vf2);
    checksum += test_ltgt(vf1, vf2);
    
    /* Test special values */
    checksum += test_special_values();
    
    /* Test assembly */
    checksum += test_asm_direct();
    
    /* Use result in control flow to ensure code isn't dead */
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    } else {
        printf("All tests passed (checksum zero)\n");
    }
    
    return checksum != 0 ? 0 : 1;
}
