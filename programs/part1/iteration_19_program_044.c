/* test_x86_condition_codes.c
 * Compile with: gcc -O3 -march=x86-64 -mfpmath=sse -ffast-math -S -fverbose-asm test_x86_condition_codes.c -o test.s
 * Also try: gcc -O3 -m32 -mfpmath=387 -march=i686 test_x86_condition_codes.c -o test_32bit
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_neg_inf = -INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

volatile long double g_ld_nan = NAN;
volatile long double g_ld_inf = INFINITY;
volatile long double g_ld_zero = 0.0L;
volatile long double g_ld_one = 1.0L;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED condition codes */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int result = 0;
    
    /* These should generate UNORDERED comparisons */
    if (a != b) result |= 1;      /* NaN != 1.0 -> unordered comparison */
    if (a == d) result |= 2;      /* NaN == NaN -> unordered comparison */
    
    /* These should generate ORDERED comparisons */
    if (b == c) result |= 4;      /* 1.0 == 0.0 -> ordered comparison */
    if (b != c) result |= 8;      /* 1.0 != 0.0 -> ordered comparison */
    
    /* Mixed comparisons */
    if (isunordered(a, b)) result |= 16;
    if (isordered(b, c)) result |= 32;
    
    return result;
}

/* Test 2: Use inline assembly with %C modifier to force condition code output */
NOINLINE int test_asm_condition_codes(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0;
    
    /* UNORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r1)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    /* ORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(r2)
        : "x"(y), "x"(z), "c"(1)
        : "cc"
    );
    
    /* UNEQ comparison (unordered or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C2 %0"
        : "=r"(r3)
        : "x"(x), "x"(x), "c"(2)
        : "cc"
    );
    
    /* UNGE comparison (not less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C3 %0"
        : "=r"(r4)
        : "x"(g_inf), "x"(g_one), "c"(3)
        : "cc"
    );
    
    /* UNGT comparison (not less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C4 %0"
        : "=r"(r5)
        : "x"(g_inf), "x"(g_zero), "c"(4)
        : "cc"
    );
    
    /* UNLE comparison (unordered or less than or equal) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C5 %0"
        : "=r"(r6)
        : "x"(g_neg_inf), "x"(g_one), "c"(5)
        : "cc"
    );
    
    /* UNLT comparison (unordered or less than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C6 %0"
        : "=r"(r7)
        : "x"(g_neg_inf), "x"(g_zero), "c"(6)
        : "cc"
    );
    
    /* LTGT comparison (less than or greater than) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C7 %0"
        : "=r"(r8)
        : "x"(g_one), "x"(g_zero), "c"(7)
        : "cc"
    );
    
    return r1 + (r2 << 1) + (r3 << 2) + (r4 << 3) + 
           (r5 << 4) + (r6 << 5) + (r7 << 6) + (r8 << 7);
}

/* Test 3: Array-based comparisons using math.h macros */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    int results[8] = {0};
    
    /* Initialize with mixed values including NaN */
    arr1[0] = NAN;
    arr2[0] = 1.0;
    arr1[1] = 1.0;
    arr2[1] = NAN;
    arr1[2] = NAN;
    arr2[2] = NAN;
    arr1[3] = 2.0;
    arr2[3] = 1.0;
    arr1[4] = 1.0;
    arr2[4] = 2.0;
    arr1[5] = -1.0;
    arr2[5] = 1.0;
    arr1[6] = INFINITY;
    arr2[6] = 1.0;
    arr1[7] = -INFINITY;
    arr2[7] = 1.0;
    
    int count = 0;
    
    /* Test all the comparison macros that map to different condition codes */
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) {
            results[i] |= 1;      /* UNORDERED */
            count++;
        }
        if (isordered(arr1[i], arr2[i])) {
            results[i] |= 2;      /* ORDERED */
        }
        if (isgreater(arr1[i], arr2[i])) {
            results[i] |= 4;      /* GT (but may use UNLE inverse) */
        }
        if (isless(arr1[i], arr2[i])) {
            results[i] |= 8;      /* LT (but may use UNGE inverse) */
        }
        if (isgreaterequal(arr1[i], arr2[i])) {
            results[i] |= 16;     /* GE (but may use UNLT inverse) */
        }
        if (islessequal(arr1[i], arr2[i])) {
            results[i] |= 32;     /* LE (but may use UNGT inverse) */
        }
        if (!islessgreater(arr1[i], arr2[i])) {
            results[i] |= 64;     /* EQ or UNORDERED (UNEQ) */
        }
        if (islessgreater(arr1[i], arr2[i])) {
            results[i] |= 128;    /* LTGT */
        }
    }
    
    /* Sum results to ensure all code paths are used */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum + count;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double a = g_ld_nan;
    volatile long double b = g_ld_one;
    volatile long double c = g_ld_zero;
    volatile long double d = g_ld_inf;
    
    int result = 0;
    
    /* Direct comparisons with long double - x87 handles these differently */
    if (a != b) result |= 1;
    if (b == c) result |= 2;
    if (a == a) result |= 4;      /* NaN == NaN is false, but may generate UNORDERED */
    
    /* Use builtins for x87 comparisons */
    int cmp1 = __builtin_isunordered(a, b);
    int cmp2 = __builtin_isgreater(b, c);
    int cmp3 = __builtin_isless(c, d);
    int cmp4 = __builtin_islessequal(a, b);
    int cmp5 = __builtin_isgreaterequal(d, b);
    int cmp6 = __builtin_islessgreater(b, c);
    
    result |= (cmp1 << 3);
    result |= (cmp2 << 4);
    result |= (cmp3 << 5);
    result |= (cmp4 << 6);
    result |= (cmp5 << 7);
    result |= (cmp6 << 8);
    
    /* Complex expression that might generate multiple condition codes */
    volatile long double x = g_ld_one;
    for (int i = 0; i < 4; i++) {
        x = x * 2.0L;
        if (x > g_ld_inf) {
            result |= (1 << (12 + i));
        }
        if (x != x) {  /* Check for NaN */
            result |= (1 << (16 + i));
        }
    }
    
    return result;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double vals[6] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0};
    int result = 0;
    
    for (int i = 0; i < 6; i++) {
        int classification = fpclassify(vals[i]);
        
        switch (classification) {
            case FP_NAN:
                result |= 1;
                /* Compare NaN with other values */
                if (vals[i] != 0.0) result |= 2;      /* UNORDERED */
                if (isunordered(vals[i], vals[(i+1)%6])) result |= 4;
                break;
            case FP_INFINITE:
                result |= 8;
                /* Infinite comparisons */
                if (vals[i] > 0.0) result |= 16;      /* GT */
                if (vals[i] < 0.0) result |= 32;      /* LT */
                break;
            case FP_ZERO:
                result |= 64;
                /* Zero comparisons */
                if (vals[i] == 0.0) result |= 128;    /* EQ */
                if (vals[i] >= 0.0) result |= 256;    /* GE */
                break;
            case FP_NORMAL:
                result |= 512;
                /* Normal number comparisons */
                if (vals[i] > vals[(i+1)%6]) result |= 1024;
                if (vals[i] < vals[(i+1)%6]) result |= 2048;
                if (vals[i] != vals[(i+1)%6]) result |= 4096;
                break;
            case FP_SUBNORMAL:
                result |= 8192;
                break;
        }
    }
    
    return result;
}

/* Test 6: Mixed SSE and x87 operations */
NOINLINE int test_mixed_operations(void) {
    /* Use both double and long double in the same function */
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile long double ld1 = g_ld_nan;
    volatile long double ld2 = g_ld_one;
    
    int result = 0;
    
    /* SSE2 double comparisons */
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "jp 1f\n\t"
        "je 2f\n\t"
        "jb 3f\n\t"
        "ja 4f\n\t"
        "1:\n\t"
        "movl $1, %0\n\t"
        "jmp 5f\n\t"
        "2:\n\t"
        "movl $2, %0\n\t"
        "jmp 5f\n\t"
        "3:\n\t"
        "movl $4, %0\n\t"
        "jmp 5f\n\t"
        "4:\n\t"
        "movl $8, %0\n\t"
        "5:\n\t"
        : "=r"(result)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    
    /* x87 long double comparison */
    int ld_result = 0;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(ld_result)
        : "m"(ld1), "m"(ld2), "c"(0)
        : "cc", "st"
    );
    
    return result | (ld_result << 4);
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int checksum = 0;
    
    /* Run all tests and accumulate results */
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_comparisons();
    checksum += test_switch_comparisons();
    checksum += test_mixed_operations();
    
    printf("Final checksum: %d\n", checksum);
    printf("(If non-zero, all tests contributed to the result)\n");
    
    /* Additional forced usage of condition codes in printf to ensure
       they appear in the assembly output */
    volatile double x = NAN;
    volatile double y = 1.0;
    
    /* Force generation of condition code names in assembly */
    if (x != y) printf("unordered\n");
    if (isgreater(x, y)) printf("greater\n");
    if (isless(x, y)) printf("less\n");
    if (isunordered(x, y)) printf("isunordered\n");
    if (isordered(x, y)) printf("isordered\n");
    
    return checksum != 0 ? 0 : 1;
}
