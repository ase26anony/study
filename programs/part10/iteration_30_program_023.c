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
    int result = 0;
    
    /* Direct unordered checks */
    if (__builtin_isunordered(vd_nan, vd1)) result |= 1;
    if (__builtin_isunordered(vd1, vd_nan)) result |= 2;
    if (__builtin_isunordered(vd_nan, vd_nan)) result |= 4;
    
    /* Using isnan check */
    if (vd_nan != vd_nan) result |= 8;  /* NaN != NaN */
    
    /* Mixed types */
    if (__builtin_isunordered(vf_nan, vf1)) result |= 16;
    
    /* With function returns */
    if (__builtin_isunordered(get_nan(), vd1)) result |= 32;
    
    return result;
}

/* Test ORDERED condition code (ord) */
int test_ordered() {
    int result = 0;
    
    /* Direct ordered checks */
    if (__builtin_isordered(vd1, vd2)) result |= 1;
    if (__builtin_isordered(vd2, vd1)) result |= 2;
    
    /* Negation of unordered */
    if (!__builtin_isunordered(vd1, vd2)) result |= 4;
    
    /* Mixed precision */
    if (__builtin_isordered(vf1, vf2)) result |= 8;
    
    /* With constants */
    if (__builtin_isordered(0.0, -0.0)) result |= 16;
    
    return result;
}

/* Test UNEQ condition code (ueq) */
int test_uneq() {
    int result = 0;
    double a = vd1;
    double b = vd2;
    double nan = vd_nan;
    
    /* Using builtin */
    if (__builtin_isunordered(a, b) || (a == b)) result |= 1;
    
    /* With NaN - should be true when unordered */
    if (__builtin_isunordered(nan, a) || (nan == a)) result |= 2;
    
    /* Float version */
    float fa = vf1;
    float fb = vf2;
    if (__builtin_isunordered(fa, fb) || (fa == fb)) result |= 4;
    
    return result;
}

/* Test UNGE condition code (nlt) */
int test_unge() {
    int result = 0;
    double a = vd1;
    double b = vd2;
    
    /* Inverse of less than */
    if (!(a < b)) result |= 1;      /* Should generate nlt */
    if (!(vd1 < vd2)) result |= 2;
    
    /* With NaN */
    if (!(vd_nan < vd1)) result |= 4;
    
    /* Float version */
    if (!(vf1 < vf2)) result |= 8;
    
    /* Mixed types */
    if (!((double)vf1 < vd2)) result |= 16;
    
    return result;
}

/* Test UNGT condition code (nle) */
int test_ungt() {
    int result = 0;
    
    /* Inverse of less than or equal */
    if (!(vd1 <= vd2)) result |= 1;  /* Should generate nle */
    if (!(vd2 <= vd1)) result |= 2;
    
    /* With function call */
    if (!(get_nan() <= vd1)) result |= 4;
    
    /* Float version */
    if (!(vf1 <= vf2)) result |= 8;
    
    return result;
}

/* Test UNLE condition code (ule) */
int test_unle() {
    int result = 0;
    
    /* Using builtin */
    if (__builtin_isunordered(vd1, vd2) || (vd1 <= vd2)) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(vd_nan, vd1) || (vd_nan <= vd1)) result |= 2;
    
    /* Float version */
    if (__builtin_isunordered(vf1, vf2) || (vf1 <= vf2)) result |= 4;
    
    return result;
}

/* Test UNLT condition code (ult) */
int test_unlt() {
    int result = 0;
    
    /* Using builtin */
    if (__builtin_isunordered(vd1, vd2) || (vd1 < vd2)) result |= 1;
    
    /* With NaN */
    if (__builtin_isunordered(vd_nan, vd1) || (vd_nan < vd1)) result |= 2;
    
    /* Float version */
    if (__builtin_isunordered(vf1, vf2) || (vf1 < vf2)) result |= 4;
    
    return result;
}

/* Test LTGT condition code (une) */
int test_ltgt() {
    int result = 0;
    
    /* Direct builtin */
    if (__builtin_islessgreater(vd1, vd2)) result |= 1;
    if (__builtin_islessgreater(vd2, vd1)) result |= 2;
    
    /* Manual version: ordered and not equal */
    if (__builtin_isordered(vd1, vd2) && (vd1 != vd2)) result |= 4;
    
    /* With NaN - should be false */
    if (__builtin_islessgreater(vd_nan, vd1)) result |= 8;
    
    /* Float version */
    if (__builtin_islessgreater(vf1, vf2)) result |= 16;
    
    /* Mixed with constants */
    if (__builtin_islessgreater(0.0, -0.0)) result |= 32;
    
    return result;
}

/* Test complex control flow with condition codes */
int test_control_flow() {
    int result = 0;
    double a = 0.0;
    double b = -0.0;
    double nan = get_nan();
    
    /* Complex if-else chain */
    if (__builtin_isunordered(a, nan)) {
        result = 1;
    } else if (!(a < b)) {  /* Should generate nlt */
        result = 2;
    } else if (__builtin_islessgreater(a, b)) {  /* Should generate une */
        result = 3;
    }
    
    /* Ternary operator */
    int x = (__builtin_isordered(vd1, vd2)) ? 10 : 20;
    result += x;
    
    /* While loop with condition */
    int count = 0;
    while (count < 3 && !(vd1 <= vd2)) {  /* Should generate nle */
        count++;
    }
    result += count * 100;
    
    return result;
}

/* Test with inline assembly for direct control */
int test_asm() {
    int result = 0;
    double a = vd1;
    double b = vd2;
    int res;
    
    /* Direct fucomip with seta (above, unordered or greater than) */
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "seta %0"
        : "=r"(res)
        : "t"(a), "u"(b)
        : "cc"
    );
    if (res) result |= 1;
    
    /* Test with NaN */
    double nan = vd_nan;
    __asm__ volatile (
        "fucomip %2, %1\n\t"
        "setp %0"  /* Set parity flag for unordered */
        : "=r"(res)
        : "t"(a), "u"(nan)
        : "cc"
    );
    if (res) result |= 2;
    
    return result;
}

/* Main driver function */
int main() {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum += test_unordered();
    printf("test_unordered: %d\n", test_unordered());
    
    checksum += test_ordered();
    printf("test_ordered: %d\n", test_ordered());
    
    checksum += test_uneq();
    printf("test_uneq: %d\n", test_uneq());
    
    checksum += test_unge();
    printf("test_unge: %d\n", test_unge());
    
    checksum += test_ungt();
    printf("test_ungt: %d\n", test_ungt());
    
    checksum += test_unle();
    printf("test_unle: %d\n", test_unle());
    
    checksum += test_unlt();
    printf("test_unlt: %d\n", test_unlt());
    
    checksum += test_ltgt();
    printf("test_ltgt: %d\n", test_ltgt());
    
    checksum += test_control_flow();
    printf("test_control_flow: %d\n", test_control_flow());
    
    checksum += test_asm();
    printf("test_asm: %d\n", test_asm());
    
    printf("Total checksum: %d\n", checksum);
    
    /* Use checksum in a way that can't be optimized away */
    volatile int* dummy = (volatile int*)&checksum;
    return *dummy != 0 ? 0 : 1;
}
