/* test_conditions.c - Program to trigger x86 floating-point condition code generation */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile float vf1 = 1.0f;
volatile float vf2 = 2.0f;
volatile float vf_nan = 0.0f / 0.0f;  /* Generate NaN */
volatile double vd1 = 3.0;
volatile double vd2 = 4.0;
volatile double vd_nan = 0.0 / 0.0;   /* Generate NaN */

/* Function prototypes for different condition code patterns */
void test_unordered(void);
void test_ordered(void);
void test_uneq(void);
void test_unge(void);
void test_ungt(void);
void test_unle(void);
void test_unlt(void);
void test_ltgt(void);
void test_mixed_precision(void);
void test_with_constants(void);
void test_with_function_calls(void);

/* Helper function to generate NaN */
static double make_nan(void) {
    return __builtin_nan("");
}

/* Helper function to generate infinity */
static double make_inf(void) {
    return __builtin_inf();
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Initialize volatile NaN values */
    vf_nan = make_nan();
    vd_nan = make_nan();
    
    /* Test each condition code pattern */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_precision();
    test_with_constants();
    test_with_function_calls();
    
    /* Compute checksum based on comparison results to ensure all are evaluated */
    {
        float f1 = 1.5f, f2 = 2.5f;
        double d1 = 3.5, d2 = 4.5;
        float f_nan = make_nan();
        double d_nan = make_nan();
        
        /* UNORDERED checks */
        checksum += __builtin_isunordered(f1, f_nan);
        checksum += __builtin_isunordered(d_nan, d2);
        
        /* ORDERED checks */
        checksum += __builtin_isordered(f1, f2);
        checksum += __builtin_isordered(d1, d2);
        
        /* UNEQ: unordered or equal */
        checksum += (f1 == f1) || __builtin_isunordered(f1, f1);
        checksum += (d_nan == d_nan) || __builtin_isunordered(d_nan, d_nan);
        
        /* UNGE: not less than (unordered or greater or equal) */
        checksum += !(f1 < f2);
        checksum += !(d_nan < d1);
        
        /* UNGT: not less than or equal (unordered or greater) */
        checksum += !(f1 <= f2);
        checksum += !(d_nan <= d1);
        
        /* UNLE: unordered or less or equal */
        checksum += (f2 <= f1) || __builtin_isunordered(f2, f1);
        checksum += (d_nan <= d1) || __builtin_isunordered(d_nan, d1);
        
        /* UNLT: unordered or less than */
        checksum += (f1 < f2) || __builtin_isunordered(f1, f2);
        checksum += (d_nan < d1) || __builtin_isunordered(d_nan, d1);
        
        /* LTGT: less than or greater than (ordered and not equal) */
        checksum += __builtin_islessgreater(f1, f2);
        checksum += __builtin_islessgreater(d1, d_nan);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}

/* Test UNORDERED condition code */
void test_unordered(void) {
    volatile float a = vf1;
    volatile float b = vf_nan;
    volatile double c = vd1;
    volatile double d = vd_nan;
    
    /* Direct unordered checks */
    int res1 = __builtin_isunordered(a, b);
    int res2 = __builtin_isunordered(c, d);
    int res3 = __builtin_isunordered(b, a);
    int res4 = __builtin_isunordered(d, c);
    
    /* Use in control flow */
    if (__builtin_isunordered(a, b)) {
        printf("UNORDERED: a and b are unordered\n");
    }
    
    if (__builtin_isunordered(c, d)) {
        printf("UNORDERED: c and d are unordered\n");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4;
    (void)dummy;
}

/* Test ORDERED condition code */
void test_ordered(void) {
    volatile float a = vf1;
    volatile float b = vf2;
    volatile double c = vd1;
    volatile double d = vd2;
    
    /* Direct ordered checks */
    int res1 = __builtin_isordered(a, b);
    int res2 = __builtin_isordered(c, d);
    
    /* Ordered check via !unordered */
    int res3 = !__builtin_isunordered(a, b);
    int res4 = !__builtin_isunordered(c, d);
    
    /* Use in control flow */
    while (__builtin_isordered(a, b)) {
        printf("ORDERED: a and b are ordered\n");
        break;
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4;
    (void)dummy;
}

/* Test UNEQ (unordered or equal) condition code */
void test_uneq(void) {
    volatile float a = vf1;
    volatile float b = vf1;  /* Same value */
    volatile float c = vf_nan;
    
    /* UNEQ: unordered or equal */
    /* This should generate "ueq" condition code */
    int res1 = (a == b) || __builtin_isunordered(a, b);
    int res2 = (c == c) || __builtin_isunordered(c, c);  /* NaN == NaN is false, but unordered is true */
    
    /* Use in ternary operator */
    int x = (a == b) || __builtin_isunordered(a, b) ? 1 : 0;
    int y = (c == a) || __builtin_isunordered(c, a) ? 2 : 0;
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + x + y;
    (void)dummy;
}

/* Test UNGE (not less than) condition code */
void test_unge(void) {
    volatile float a = vf2;
    volatile float b = vf1;
    volatile float c = vf_nan;
    
    /* UNGE: !(a < b) - generates "nlt" */
    int res1 = !(a < b);      /* 2.0 < 1.0 is false, so !false = true */
    int res2 = !(b < a);      /* 1.0 < 2.0 is true, so !true = false */
    int res3 = !(c < b);      /* NaN < 1.0 is false (unordered), so !false = true */
    
    /* Use in if statement */
    if (!(a < b)) {
        printf("UNGE: a is not less than b\n");
    }
    
    if (!(c < b)) {
        printf("UNGE: c is not less than b (unordered case)\n");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3;
    (void)dummy;
}

/* Test UNGT (not less than or equal) condition code */
void test_ungt(void) {
    volatile double a = vd2;
    volatile double b = vd1;
    volatile double c = vd_nan;
    
    /* UNGT: !(a <= b) - generates "nle" */
    int res1 = !(a <= b);     /* 4.0 <= 3.0 is false, so !false = true */
    int res2 = !(b <= a);     /* 3.0 <= 4.0 is true, so !true = false */
    int res3 = !(c <= a);     /* NaN <= 4.0 is false, so !false = true */
    
    /* Use in while loop */
    while (!(a <= b)) {
        printf("UNGT: a is not less than or equal to b\n");
        break;
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3;
    (void)dummy;
}

/* Test UNLE (unordered or less or equal) condition code */
void test_unle(void) {
    volatile float a = vf1;
    volatile float b = vf2;
    volatile float c = vf_nan;
    
    /* UNLE: (a <= b) || __builtin_isunordered(a, b) */
    int res1 = (a <= b) || __builtin_isunordered(a, b);  /* 1.0 <= 2.0 is true */
    int res2 = (b <= a) || __builtin_isunordered(b, a);  /* 2.0 <= 1.0 is false, unordered is false */
    int res3 = (c <= a) || __builtin_isunordered(c, a);  /* NaN <= 1.0 is false, but unordered is true */
    
    /* Use in array indexing */
    int array[3] = {0};
    int idx = ((a <= b) || __builtin_isunordered(a, b)) ? 0 : 1;
    array[idx] = 1;
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + array[0];
    (void)dummy;
}

/* Test UNLT (unordered or less than) condition code */
void test_unlt(void) {
    volatile double a = vd1;
    volatile double b = vd2;
    volatile double c = vd_nan;
    
    /* UNLT: (a < b) || __builtin_isunordered(a, b) */
    int res1 = (a < b) || __builtin_isunordered(a, b);  /* 3.0 < 4.0 is true */
    int res2 = (b < a) || __builtin_isunordered(b, a);  /* 4.0 < 3.0 is false, unordered is false */
    int res3 = (c < a) || __builtin_isunordered(c, a);  /* NaN < 3.0 is false, but unordered is true */
    
    /* Use in if-else chain */
    if ((a < b) || __builtin_isunordered(a, b)) {
        printf("UNLT: a is less than b or unordered\n");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3;
    (void)dummy;
}

/* Test LTGT (less than or greater than) condition code */
void test_ltgt(void) {
    volatile float a = vf1;
    volatile float b = vf2;
    volatile float c = vf_nan;
    
    /* LTGT: __builtin_islessgreater(a, b) - ordered and not equal */
    int res1 = __builtin_islessgreater(a, b);  /* 1.0 and 2.0 are ordered and not equal */
    int res2 = __builtin_islessgreater(b, a);  /* 2.0 and 1.0 are ordered and not equal */
    int res3 = __builtin_islessgreater(c, a);  /* NaN and 1.0 are unordered, so false */
    int res4 = __builtin_islessgreater(a, a);  /* 1.0 and 1.0 are equal, so false */
    
    /* Alternative: (a < b) || (a > b) with ordered check */
    int res5 = ((a < b) || (a > b)) && __builtin_isordered(a, b);
    
    /* Use in complex expression */
    int result = __builtin_islessgreater(a, b) ? 100 : 
                 __builtin_islessgreater(c, a) ? 200 : 300;
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4 + res5 + result;
    (void)dummy;
}

/* Test mixed precision comparisons */
void test_mixed_precision(void) {
    float f = 1.5f;
    double d = 2.5;
    float f_nan = make_nan();
    double d_nan = make_nan();
    
    /* Mixed float/double comparisons (will promote float to double) */
    int res1 = __builtin_isunordered(f, d_nan);
    int res2 = __builtin_isordered(d, f_nan);
    int res3 = !(f < d);           /* Should generate "nlt" for mixed precision */
    int res4 = !(d <= f);          /* Should generate "nle" for mixed precision */
    int res5 = (f <= d) || __builtin_isunordered(f, d_nan);
    int res6 = __builtin_islessgreater(d, f);
    
    /* Use in control flow with mixed types */
    if (__builtin_isunordered(f, d_nan)) {
        printf("Mixed: float and double NaN are unordered\n");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4 + res5 + res6;
    (void)dummy;
}

/* Test with constants */
void test_with_constants(void) {
    /* Comparisons with various constants */
    int res1 = __builtin_isunordered(1.0f, NAN);
    int res2 = __builtin_isordered(2.0, 3.0);
    int res3 = !(0.0f < -0.0f);           /* -0.0 == 0.0, so !(0.0 < -0.0) is true */
    int res4 = !(INFINITY <= 1.0e100);
    int res5 = (1.0 <= 2.0) || __builtin_isunordered(1.0, NAN);
    int res6 = __builtin_islessgreater(3.0f, 4.0f);
    
    /* Use constants in ternary expressions */
    int x = __builtin_isunordered(1.0, NAN) ? 1 : 0;
    int y = !(2.0 < 1.0) ? 2 : 0;          /* Should generate "nlt" */
    int z = !(3.0 <= 2.0) ? 3 : 0;         /* Should generate "nle" */
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4 + res5 + res6 + x + y + z;
    (void)dummy;
}

/* Test with function calls */
void test_with_function_calls(void) {
    /* Use function calls that may return NaN */
    double nan1 = make_nan();
    double inf1 = make_inf();
    double sqrt_neg = sqrt(-1.0);  /* Returns NaN */
    double log_zero = log(0.0);    /* Returns -inf */
    
    /* Comparisons with function results */
    int res1 = __builtin_isunordered(nan1, 1.0);
    int res2 = __builtin_isordered(inf1, log_zero);
    int res3 = !(sqrt_neg < 0.0);
    int res4 = !(inf1 <= log_zero);
    int res5 = (nan1 <= inf1) || __builtin_isunordered(nan1, inf1);
    int res6 = __builtin_islessgreater(inf1, log_zero);
    
    /* Chain of comparisons */
    if (__builtin_isunordered(sqrt_neg, 0.0)) {
        if (!(inf1 < log_zero)) {
            if ((nan1 <= inf1) || __builtin_isunordered(nan1, inf1)) {
                printf("Function calls: triggered multiple conditions\n");
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = res1 + res2 + res3 + res4 + res5 + res6;
    (void)dummy;
}
