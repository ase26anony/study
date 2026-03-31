/* Test program to generate floating-point condition codes for i386 RTL printer */
/* Compile with: gcc -m32 -mfpmath=387 -O2 -fdump-rtl-final test.c -o test */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int result = 0;
    
    /* Force the compiler to keep all variables live */
    asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    
    /* Test 1: UNORDERED comparisons (x != x, isunordered) */
    if (__builtin_isunordered(a, b)) {
        result += 1;  /* UNORDERED */
    }
    
    if (a != a) {  /* This should also generate UNORDERED check */
        result += 2;
    }
    
    /* Test 2: ORDERED comparisons */
    if (!__builtin_isunordered(c, d)) {
        result += 4;  /* ORDERED */
    }
    
    /* Test 3: UNEQ (unordered or equal) */
    /* Generate via: !(a > b) && !(a < b) which includes NaN case */
    if (!(a > b) && !(a < b)) {
        result += 8;  /* UNEQ */
    }
    
    /* Test 4: UNGE (unordered or greater-or-equal) */
    /* Generate via: !(a < b) */
    if (!(a < b)) {
        result += 16;  /* UNGE -> prints as "nlt" */
    }
    
    /* Test 5: UNGT (unordered or greater) */
    /* Generate via: !(a <= b) */
    if (!(a <= b)) {
        result += 32;  /* UNGT -> prints as "nle" */
    }
    
    /* Test 6: UNLE (unordered or less-or-equal) */
    /* Generate via: !(a > b) */
    if (!(a > b)) {
        result += 64;  /* UNLE -> prints as "ule" */
    }
    
    /* Test 7: UNLT (unordered or less) */
    /* Generate via: !(a >= b) */
    if (!(a >= b)) {
        result += 128;  /* UNLT -> prints as "ult" */
    }
    
    /* Test 8: LTGT (less, greater, but not unordered/equal) */
    /* Generate via: (a < b) || (a > b) but we need it as a single comparison */
    /* We'll use inline asm to force specific pattern */
    {
        int ltgt_flag = 0;
        /* This should generate LTGT in some optimization passes */
        if (a < b || a > b) {
            ltgt_flag = 1;
        }
        if (ltgt_flag) {
            result += 256;  /* LTGT -> prints as "une" */
        }
    }
    
    /* Additional complex expression to force multiple condition codes */
    for (int i = 0; i < 3; i++) {
        volatile double x = a + i;
        volatile double y = b + i;
        
        /* Mix of different comparisons */
        if (__builtin_isunordered(x, y)) {
            result += 512;
        } else if (x == y) {
            result += 1024;
        } else if (x < y) {
            result += 2048;
        } else if (x > y) {
            result += 4096;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double inf = make_inf();
    volatile double normal = 3.14159;
    volatile double zero = 0.0;
    volatile double neg = -2.71828;
    
    /* Use inline asm to prevent constant folding */
    asm volatile("" : "+g"(nan1), "+g"(nan2), "+g"(inf), 
                       "+g"(normal), "+g"(zero), "+g"(neg));
    
    /* Call test function multiple times with different combinations */
    int total = 0;
    
    /* Test NaN vs NaN */
    total += fp_test(nan1, nan2, normal, zero);
    
    /* Test NaN vs normal */
    total += fp_test(nan1, normal, zero, inf);
    
    /* Test normal vs normal */
    total += fp_test(normal, neg, inf, nan1);
    
    /* Test normal vs zero */
    total += fp_test(zero, normal, neg, inf);
    
    /* Test infinity vs normal */
    total += fp_test(inf, normal, nan1, zero);
    
    /* Use the result to prevent dead code elimination */
    volatile int output = total;
    
    /* Print to prevent optimization */
    __builtin_printf("Result: %d\n", output);
    
    return output != 0 ? 0 : 1;
}
