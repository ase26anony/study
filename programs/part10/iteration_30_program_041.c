/* test_conditions.c - Program to trigger x86 floating-point condition codes */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;

/* Function to generate NaN */
double get_nan(void) {
    return __builtin_nan("");
}

float get_nanf(void) {
    return __builtin_nanf("");
}

/* Test UNORDERED condition code */
int test_unordered(void) {
    double nan = get_nan();
    float nanf = get_nanf();
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd1, nan)) result |= 1;
    if (__builtin_isunordered(nan, vd2)) result |= 2;
    if (__builtin_isunordered(vf1, nanf)) result |= 4;
    if (__builtin_isunordered(nanf, vf2)) result |= 8;
    
    /* Using !(a == a) pattern */
    double x = sqrt(-1.0);  /* Produces NaN */
    if (!(x == x)) result |= 16;
    
    return result;
}

/* Test ORDERED condition code */
int test_ordered(void) {
    double nan = get_nan();
    float nanf = get_nanf();
    int result = 0;
    
    /* Ordered checks */
    if (!__builtin_isunordered(vd1, vd2)) result |= 1;
    if (!__builtin_isunordered(vf1, vf2)) result |= 2;
    
    /* Compare with NaN */
    if (!__builtin_isunordered(vd1, nan)) result |= 4;
    if (!__builtin_isunordered(nanf, vf2)) result |= 8;
    
    return result;
}

/* Test UNEQ (unordered or equal) condition code */
int test_uneq(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* This should generate ueq when optimized */
    if (!(a < b) && !(a > b)) result |= 1;
    
    /* With NaN */
    if (!(nan < a) && !(nan > a)) result |= 2;
    
    /* Float version */
    float fa = vf1;
    float fb = vf2;
    if (!(fa < fb) && !(fa > fb)) result |= 4;
    
    return result;
}

/* Test UNGE (not less than) condition code */
int test_unge(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* Inverse condition: !(a < b) */
    if (!(a < b)) result |= 1;
    
    /* With volatile */
    if (!(vd1 < vd2)) result |= 2;
    
    /* Mixed types */
    float fa = vf1;
    if (!(fa < b)) result |= 4;
    
    /* With NaN */
    if (!(nan < a)) result |= 8;
    
    return result;
}

/* Test UNGT (not less than or equal) condition code */
int test_ungt(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* Inverse condition: !(a <= b) */
    if (!(a <= b)) result |= 1;
    
    /* With function return */
    if (!(sin(a) <= cos(b))) result |= 2;
    
    /* Float version */
    float fa = vf1;
    float fb = vf2;
    if (!(fa <= fb)) result |= 4;
    
    /* With NaN */
    if (!(nan <= a)) result |= 8;
    
    return result;
}

/* Test UNLE (unordered or less than or equal) condition code */
int test_unle(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* Using ternary to force code generation */
    result = (a <= b) ? 1 : 0;
    result |= (nan <= a) ? 2 : 0;
    
    /* Float comparison */
    float fa = vf1;
    result |= (fa <= b) ? 4 : 0;
    
    return result;
}

/* Test UNLT (unordered or less than) condition code */
int test_unlt(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* Direct comparison */
    if (a < b) result |= 1;
    
    /* With NaN */
    if (nan < a) result |= 2;
    
    /* Mixed precision */
    if (vf1 < vd2) result |= 4;
    
    /* In while loop to force conditional jump */
    volatile int count = 0;
    while (a < b && count < 3) {
        result |= (1 << (count + 4));
        count++;
    }
    
    return result;
}

/* Test LTGT (less than or greater than, ordered) condition code */
int test_ltgt(void) {
    double a = vd1;
    double b = vd2;
    double nan = get_nan();
    int result = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(a, b)) result |= 1;
    
    /* Manual equivalent: (a < b) || (a > b) */
    if ((a < b) || (a > b)) result |= 2;
    
    /* With NaN */
    if (__builtin_islessgreater(nan, a)) result |= 4;
    
    /* Float version */
    float fa = vf1;
    float fb = vf2;
    if (__builtin_islessgreater(fa, fb)) result |= 8;
    
    return result;
}

/* Test all conditions in complex expressions */
int test_complex_expressions(void) {
    double a = 3.14;
    double b = 2.71;
    double nan = get_nan();
    float fa = 1.618f;
    float fb = 0.577f;
    float nanf = get_nanf();
    
    int result = 0;
    
    /* Complex if-else chain */
    if (__builtin_isunordered(a, nan)) {
        result |= 1;
    } else if (!__builtin_isunordered(b, b)) {
        result |= 2;
    }
    
    /* Nested comparisons */
    if (!(fa < fb) && __builtin_islessgreater(a, b)) {
        result |= 4;
    }
    
    /* Switch based on comparison results */
    int cmp1 = !(a <= b);
    int cmp2 = __builtin_isunordered(nanf, fa);
    int cmp3 = (fa < fb) || (fa > fb);
    
    switch ((cmp1 << 2) | (cmp2 << 1) | cmp3) {
        case 0: result |= 8; break;
        case 1: result |= 16; break;
        case 2: result |= 32; break;
        case 3: result |= 64; break;
        case 4: result |= 128; break;
        case 5: result |= 256; break;
        case 6: result |= 512; break;
        case 7: result |= 1024; break;
    }
    
    return result;
}

/* Main driver function */
int main(void) {
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
    checksum ^= test_complex_expressions();
    
    /* Additional volatile comparisons to prevent dead code elimination */
    volatile double v1 = 1.5;
    volatile double v2 = 2.5;
    volatile float v3 = 3.5f;
    volatile float v4 = 4.5f;
    
    /* More patterns to trigger different code generation */
    for (int i = 0; i < 10; i++) {
        v1 += 0.1;
        v2 -= 0.1;
        
        if (__builtin_isunordered(v1, v2)) checksum += i;
        if (!__builtin_isunordered(v3, v4)) checksum -= i;
        if (!(v1 < v2)) checksum ^= i;
        if (!(v3 <= v4)) checksum |= i;
        if (__builtin_islessgreater(v1, v2)) checksum &= ~i;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates comparisons were executed)\n");
    
    return checksum != 0 ? 0 : 1;
}
