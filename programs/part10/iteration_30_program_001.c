/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile int result = 0;

/* Function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

/* Function to generate infinity */
static double make_inf(void) {
    return __builtin_inf();
}

/* Test UNORDERED condition (unord) */
int test_unordered(void) {
    double nan1 = make_nan();
    double nan2 = __builtin_nan("0x1234");
    float nanf = __builtin_nanf("");
    
    int res = 0;
    
    /* Direct unordered checks */
    res += __builtin_isunordered(nan1, vd1);
    res += __builtin_isunordered(vd1, nan1);
    res += __builtin_isunordered(nan1, nan2);
    
    /* Using !(a == a) pattern */
    res += !(nan1 == nan1);
    res += !(nanf == nanf);
    
    /* Mixed types */
    res += __builtin_isunordered(nan1, (double)vf1);
    
    return res;
}

/* Test ORDERED condition (ord) */
int test_ordered(void) {
    double nan = make_nan();
    double inf = make_inf();
    
    int res = 0;
    
    /* Ordered checks on normal numbers */
    res += !__builtin_isunordered(vd1, vd2);
    res += !__builtin_isunordered(vf1, vf2);
    
    /* Ordered checks with infinity */
    res += !__builtin_isunordered(inf, vd1);
    res += !__builtin_isunordered(vd1, inf);
    
    /* Ordered check with NaN should be false */
    res += !__builtin_isunordered(nan, vd1) ? 0 : 1;
    
    return res;
}

/* Test UNEQ condition (ueq) */
int test_uneq(void) {
    double nan = make_nan();
    double a = 3.14159;
    double b = 3.14159;
    
    int res = 0;
    
    /* UNEQ: unordered or equal */
    if (__builtin_isunordered(a, b) || a == b) res += 1;
    if (__builtin_isunordered(nan, b) || nan == b) res += 1;
    if (__builtin_isunordered(a, nan) || a == nan) res += 1;
    
    /* Using volatile to force code generation */
    volatile double x = 1.0;
    volatile double y = 1.0;
    if (__builtin_isunordered(x, y) || x == y) res += 1;
    
    return res;
}

/* Test UNGE condition (nlt) */
int test_unge(void) {
    double nan = make_nan();
    double a = 2.5;
    double b = 1.5;
    
    int res = 0;
    
    /* UNGE: unordered or not less than (nlt) */
    if (__builtin_isunordered(a, b) || !(a < b)) res += 1;
    if (__builtin_isunordered(nan, b) || !(nan < b)) res += 1;
    
    /* Inverse condition: !(a < b) for ordered case */
    if (!(vd1 < vd2)) res += 1;
    if (!(vf1 < vf2)) res += 2;
    
    /* Mixed precision */
    if (!((double)vf1 < vd2)) res += 1;
    
    return res;
}

/* Test UNGT condition (nle) */
int test_ungt(void) {
    double nan = make_nan();
    double a = 3.0;
    double b = 2.0;
    
    int res = 0;
    
    /* UNGT: unordered or not less or equal (nle) */
    if (__builtin_isunordered(a, b) || !(a <= b)) res += 1;
    if (__builtin_isunordered(nan, b) || !(nan <= b)) res += 1;
    
    /* Inverse condition: !(a <= b) */
    if (!(vd1 <= vd2)) res += 1;
    if (!(1.0 <= 2.0)) res += 1;  /* Constant folding should avoid this */
    
    /* With function return values */
    if (!(sin(0.0) <= cos(0.0))) res += 1;
    
    return res;
}

/* Test UNLE condition (ule) */
int test_unle(void) {
    double nan = make_nan();
    double a = 1.0;
    double b = 2.0;
    
    int res = 0;
    
    /* UNLE: unordered or less or equal */
    if (__builtin_isunordered(a, b) || a <= b) res += 1;
    if (__builtin_isunordered(nan, b) || nan <= b) res += 1;
    
    /* Direct comparison that might generate ule */
    volatile float x = 1.0f;
    volatile float y = 2.0f;
    if (x <= y) res += 1;
    
    /* With constants */
    if (0.0 <= 1.0) res += 1;
    
    return res;
}

/* Test UNLT condition (ult) */
int test_unlt(void) {
    double nan = make_nan();
    double a = 1.0;
    double b = 2.0;
    
    int res = 0;
    
    /* UNLT: unordered or less than */
    if (__builtin_isunordered(a, b) || a < b) res += 1;
    if (__builtin_isunordered(nan, b) || nan < b) res += 1;
    
    /* Direct less-than comparisons */
    if (vd1 < vd2) res += 1;
    if (vf1 < vf2) res += 1;
    
    /* Mixed types */
    if ((float)vd1 < vf2) res += 1;
    
    return res;
}

/* Test LTGT condition (une) */
int test_ltgt(void) {
    double nan = make_nan();
    double a = 1.0;
    double b = 2.0;
    double c = 1.0;
    
    int res = 0;
    
    /* LTGT: less or greater (unordered equal is false) */
    if (__builtin_islessgreater(a, b)) res += 1;      /* true */
    if (__builtin_islessgreater(a, c)) res += 1;      /* false */
    if (__builtin_islessgreater(nan, b)) res += 1;    /* false */
    
    /* Alternative: (a < b) || (a > b) for ordered values */
    if ((vd1 < vd2) || (vd1 > vd2)) res += 1;
    if ((vf1 < vf2) || (vf1 > vf2)) res += 1;
    
    /* With builtin */
    if (__builtin_islessgreater(vd1, vd2)) res += 1;
    
    return res;
}

/* Test with inline assembly for direct control */
int test_asm_direct(void) {
    double a = 1.5;
    double b = 2.5;
    int res;
    
    /* Using inline assembly to potentially trigger condition codes */
    __asm__ volatile (
        "fcomip %2, %1\n\t"
        "seta %0"
        : "=r"(res)
        : "t"(a), "u"(b)
        : "cc"
    );
    
    return res;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Call all test functions */
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    
    /* Try inline assembly version */
    checksum += test_asm_direct();
    
    /* Additional complex patterns */
    {
        volatile double x = 0.0;
        volatile double y = -0.0;
        
        /* Test with signed zeros */
        if (__builtin_isunordered(x, y) || x == y) checksum += 1;
        
        /* Test with infinity */
        double inf = make_inf();
        if (!(inf < inf)) checksum += 1;  /* Should generate nlt? */
        
        /* Test sqrt(-1) gives NaN */
        double nan_sqrt = sqrt(-1.0);
        if (__builtin_isunordered(nan_sqrt, nan_sqrt)) checksum += 1;
    }
    
    /* Use result in control flow */
    if (checksum > 0) {
        printf("Checksum: %d\n", checksum);
    } else {
        printf("No conditions matched\n");
    }
    
    /* Prevent dead code elimination */
    result = checksum;
    
    return checksum != 0 ? 0 : 1;
}
