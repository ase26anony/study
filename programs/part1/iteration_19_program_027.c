/* Test program to trigger x86 floating-point condition code printing */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Prevent constant folding */
volatile double vnan = NAN;
volatile double vinf = INFINITY;
volatile double vnum = 3.14159;
volatile double vzero = 0.0;

volatile long double ld_nan = NAN;
volatile long double ld_inf = INFINITY;
volatile long double ld_num = 3.14159265358979323846L;

/* Test 1: Various unordered comparisons using double */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    double nan = vnan;
    double inf = vinf;
    double num = vnum;
    double zero = vzero;
    
    /* These should generate various condition codes */
    results[0] = (nan != nan);      /* UNORDERED */
    results[1] = (nan == nan);      /* ORDERED (false for NaN) */
    results[2] = !(nan < inf);      /* UNGE or NLT */
    results[3] = !(nan <= num);     /* UNGT or NLE */
    results[4] = (nan <= nan);      /* UNLE */
    results[5] = (nan < nan);       /* UNLT */
    results[6] = (inf != inf);      /* LTGT or UNE */
    results[7] = (num == num);      /* UNEQ */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    double nan = vnan;
    double inf = vinf;
    double num = vnum;
    
    int results[6] = {0};
    
    results[0] = isunordered(nan, num);   /* UNORDERED */
    results[1] = isgreater(num, inf);     /* UNLE? Actually generates GT */
    results[2] = isless(inf, num);        /* UNGE? Actually generates LT */
    results[3] = islessequal(nan, nan);   /* UNLE */
    results[4] = isgreaterequal(inf, nan); /* UNLT */
    results[5] = !islessgreater(num, num); /* UNEQ */
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    double a = vnum;
    double b = vnan;
    double c = vinf;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(result2)
        : "x"(b), "x"(b), "i"(ORDERED)
        : "cc"
    );
    
    /* Test UNEQ condition */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C2 %0"
        : "=r"(result3)
        : "x"(a), "x"(a), "i"(UNEQ)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Array comparison with mixed values */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8], arr2[8];
    
    /* Initialize with mixed values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) {
            arr1[i] = vnan;
            arr2[i] = i * 1.0;
        } else if (i % 4 == 1) {
            arr1[i] = i * 1.0;
            arr2[i] = vnan;
        } else if (i % 4 == 2) {
            arr1[i] = vinf;
            arr2[i] = -vinf;
        } else {
            arr1[i] = i * 1.0;
            arr2[i] = i * 2.0;
        }
    }
    
    int counts[7] = {0};  /* For different comparison types */
    
    for (int i = 0; i < 8; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Force multiple different comparisons */
        if (isunordered(a, b)) counts[0]++;      /* UNORDERED */
        if (!isunordered(a, b)) counts[1]++;     /* ORDERED */
        if (!(a < b)) counts[2]++;               /* UNGE/NLT */
        if (!(a <= b)) counts[3]++;              /* UNGT/NLE */
        if (a <= b) counts[4]++;                 /* UNLE (when NaN) */
        if (a < b) counts[5]++;                  /* UNLT (when NaN) */
        if (a != b) counts[6]++;                 /* LTGT/UNE */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    volatile long double a = ld_nan;
    volatile long double b = ld_num;
    volatile long double c = ld_inf;
    
    int results = 0;
    
    /* These should use x87 fucom instructions */
    results += (a != a);      /* UNORDERED */
    results += (b == b);      /* ORDERED/UNEQ */
    results += !(a < b);      /* UNGE/NLT */
    results += !(a <= b);     /* UNGT/NLE */
    results += (a <= a);      /* UNLE */
    results += (a < a);       /* UNLT */
    results += (c != c);      /* LTGT/UNE */
    
    return results;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    double vals[] = {vnan, vinf, -vinf, vnum, vzero};
    int result = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            double a = vals[i];
            double b = vals[j];
            
            /* Complex switch to force multiple condition codes */
            int cmp;
            if (isunordered(a, b)) {
                cmp = 0;  /* UNORDERED */
            } else if (a == b) {
                cmp = 1;  /* UNEQ */
            } else if (a < b) {
                cmp = 2;  /* LT/UNGE inverse */
            } else if (a > b) {
                cmp = 3;  /* GT/UNLE inverse */
            } else {
                cmp = 4;  /* Shouldn't happen */
            }
            
            switch (cmp) {
                case 0: result += 1; break;  /* UNORDERED */
                case 1: result += 2; break;  /* UNEQ */
                case 2: result += 3; break;  /* LT */
                case 3: result += 4; break;  /* GT */
                default: result += 5; break;
            }
        }
    }
    
    return result;
}

/* Test 7: Direct builtin usage */
NOINLINE int test_builtins(void) {
    double a = vnum;
    double b = vnan;
    int res = 0;
    
    /* Use GCC x86 builtins */
    res = __builtin_ia32_ucomisd(a, b);
    /* Result in EFLAGS, force conditional move */
    int tmp;
    __asm__ volatile (
        "cmovc %1, %0"
        : "+r"(res)
        : "r"(tmp)
        : "cc"
    );
    
    return res;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    checksum += test_unordered_comparisons();
    checksum += test_math_macros();
    checksum += test_inline_asm();
    checksum += test_array_comparisons();
    checksum += test_long_double();
    checksum += test_switch_comparisons();
    checksum += test_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
