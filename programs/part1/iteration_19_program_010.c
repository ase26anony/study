/* gcc -O2 -march=x86-64 -mfpmath=sse -ffast-math -S -fverbose-asm test.c -o test.s */
/* Also try: gcc -O3 -m32 -mfpmath=387 -march=i686 test.c -o test_32 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile double vd_nan = NAN;
volatile double vd_inf = INFINITY;
volatile long double vld1 = 1.0L;
volatile long double vld2 = 2.0L;
volatile long double vld_nan = NAN;

/* Test 1: Various unordered comparisons using standard operators */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    
    /* UNORDERED: Compare NaN with anything using != */
    results[0] = (vd_nan != vd1) ? 1 : 0;
    
    /* ORDERED: Compare normal numbers using == */
    results[1] = (vd1 == vd2) ? 0 : 1;
    
    /* UNEQ: unordered or equal - simulate with isunordered || == */
    results[2] = (isunordered(vd_nan, vd1) || (vd_nan == vd1)) ? 1 : 0;
    
    /* UNGE: unordered or greater-or-equal */
    results[3] = (isunordered(vd_nan, vd1) || (vd_nan >= vd1)) ? 1 : 0;
    
    /* UNGT: unordered or greater-than */
    results[4] = (isunordered(vd_nan, vd1) || (vd_nan > vd1)) ? 1 : 0;
    
    /* UNLE: unordered or less-or-equal */
    results[5] = (isunordered(vd_nan, vd1) || (vd_nan <= vd1)) ? 1 : 0;
    
    /* UNLT: unordered or less-than */
    results[6] = (isunordered(vd_nan, vd1) || (vd_nan < vd1)) ? 1 : 0;
    
    /* LTGT: less-than or greater-than (but not equal, not unordered) */
    results[7] = (vd1 < vd2 || vd1 > vd2) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    int result = 0;
    double a = vd1;
    double b = vd2;
    double c = vd_nan;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    
    int result2 = 0;
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    return result + result2;
}

/* Test 3: Loop with various comparison macros */
NOINLINE int test_comparison_macros_loop(void) {
    volatile double arr1[4] = {1.0, NAN, 3.0, INFINITY};
    volatile double arr2[4] = {2.0, NAN, 3.0, -INFINITY};
    int counts[7] = {0};
    
    for (int i = 0; i < 4; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;    /* UNORDERED */
        counts[1] += isgreater(arr1[i], arr2[i]) ? 1 : 0;      /* GT */
        counts[2] += isless(arr1[i], arr2[i]) ? 1 : 0;         /* LT */
        counts[3] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0; /* GE */
        counts[4] += islessequal(arr1[i], arr2[i]) ? 1 : 0;    /* LE */
        counts[5] += (isunordered(arr1[i], arr2[i]) || 
                     (arr1[i] == arr2[i])) ? 1 : 0;            /* UNEQ */
        counts[6] += ((arr1[i] < arr2[i]) || 
                     (arr1[i] > arr2[i])) ? 1 : 0;             /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double a = vld1;
    volatile long double b = vld2;
    volatile long double c = vld_nan;
    int results = 0;
    
    /* Force x87 unordered compare */
    if (a != b) results |= 1;
    if (c == c) results |= 2;  /* NaN == NaN is false */
    if (c != c) results |= 4;  /* NaN != NaN is true (unordered) */
    
    /* Mixed comparisons that might generate different condition codes */
    if (a > b) results |= 8;
    if (a < b) results |= 16;
    if (a >= b) results |= 32;
    if (a <= b) results |= 64;
    
    /* Explicit unordered check */
    if (isunordered(c, a)) results |= 128;
    
    return results;
}

/* Test 5: Switch based on fpclassify results */
NOINLINE int test_fpclassify_switch(void) {
    volatile double vals[5] = {1.0, NAN, INFINITY, -INFINITY, 0.0};
    int result = 0;
    
    for (int i = 0; i < 5; i++) {
        switch (fpclassify(vals[i])) {
            case FP_NAN:
                result += 1;
                /* Force unordered comparison in switch context */
                if (isunordered(vals[i], vals[(i+1)%5])) result += 10;
                break;
            case FP_INFINITE:
                result += 2;
                if (vals[i] > 0) result += 20;
                break;
            case FP_ZERO:
                result += 3;
                if (vals[i] == 0.0) result += 30;
                break;
            case FP_SUBNORMAL:
                result += 4;
                break;
            case FP_NORMAL:
                result += 5;
                if (vals[i] < 100.0) result += 50;
                break;
        }
    }
    return result;
}

/* Test 6: Direct use of builtin for SSE2 unordered compare */
NOINLINE int test_builtin_ucomisd(void) {
    double a = vd1;
    double b = vd_nan;
    double c = vd2;
    
    int r1 = __builtin_ia32_ucomisd(a, b);  /* Should set ZF=0, PF=1, CF=1 (unordered) */
    int r2 = __builtin_ia32_ucomisd(a, c);  /* Should set ZF=0, PF=0, CF=1 (less than) */
    int r3 = __builtin_ia32_ucomisd(a, a);  /* Should set ZF=1, PF=0, CF=0 (equal) */
    
    /* Use results in conditional moves/jumps */
    int result = 0;
    if (r1) result |= 1;
    if (r2) result |= 2;
    if (r3) result |= 4;
    
    /* Force conditional move with unordered compare */
    result = (isunordered(a, b)) ? (result | 8) : (result & ~8);
    
    return result;
}

/* Test 7: Complex branching to prevent optimization */
NOINLINE int test_complex_branching(void) {
    volatile double x = vd1;
    volatile double y = vd2;
    volatile double z = vd_nan;
    int count = 0;
    
    /* Multiple independent branches that can't be easily optimized */
    if (x != y) {
        if (isunordered(x, z)) {
            count += 1;
        } else if (isgreater(x, y)) {
            count += 2;
        }
    }
    
    if (z == z) {  /* Always false for NaN */
        count += 4;
    } else {
        if (isless(z, x)) {
            count += 8;
        }
    }
    
    /* Ternary with unordered comparison */
    count += (isunordered(y, z) || islessequal(x, y)) ? 16 : 32;
    
    /* Nested conditionals */
    if (!isunordered(x, y)) {
        if (x < y) {
            count += 64;
        } else if (x > y) {
            count += 128;
        } else {
            count += 256;
        }
    }
    
    return count;
}

int main(void) {
    int checksum = 0;
    
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_comparison_macros_loop();
    checksum += test_long_double_comparisons();
    checksum += test_fpclassify_switch();
    checksum += test_builtin_ucomisd();
    checksum += test_complex_branching();
    
    printf("Checksum: %d\n", checksum);
    
    /* Also print some values to prevent dead code elimination */
    printf("vd_nan: %f\n", vd_nan);
    printf("vd_inf: %f\n", vd_inf);
    
    return 0;
}
