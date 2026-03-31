/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386 x87
 * Compile with: gcc -m32 -mfpmath=387 -O2 -da test_i386_condition_codes.c -o test
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks (should generate "unord" in RTL) */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* ORDERED checks (should generate "ord" in RTL) */
        if (!__builtin_isunordered(c, d)) {
            counter += 2;
            asm volatile("" : : : "memory");
        }
        
        /* UNEQ (unordered or equal) - via explicit checks */
        if (__builtin_isunordered(a, c) || a == c) {
            counter += 3;
        }
        
        /* UNGE (not less than) - unordered or greater-or-equal */
        if (!(a < b)) {
            counter += 4;
        }
        
        /* UNGT (not less-or-equal) */
        if (!(a <= b)) {
            counter += 5;
        }
        
        /* UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(a, d) || a <= d) {
            counter += 6;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(b, c) || b < c) {
            counter += 7;
        }
        
        /* LTGT (less than or greater than, but not equal/unordered) */
        if ((a < b) || (a > b)) {
            counter += 8;
        }
        
        /* Standard comparisons mixed in */
        if (a == b) counter--;
        if (a != c) counter++;
        if (a < d) counter += 2;
        if (b > c) counter += 3;
        
        /* NaN checks */
        if (__builtin_isnan(a)) counter += 10;
        if (!__builtin_isnan(b)) counter += 20;
        
        /* Modify values to create data dependencies */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c), "+g"(d));
    }
    
    return counter;
}

/* Another test function with different patterns */
NOINLINE static int fp_test2(volatile double x, volatile double y) {
    volatile int result = 0;
    
    /* Complex conditional expressions */
    result += (__builtin_isunordered(x, y) ? 1 : 0);
    result += ((x < y) && !__builtin_isunordered(x, y)) ? 2 : 0;
    result += ((x > y) || __builtin_isunordered(x, y)) ? 4 : 0;
    result += (!__builtin_isunordered(x, y) && (x == y)) ? 8 : 0;
    
    /* Switch-like structure with FP conditions */
    if (__builtin_isnan(x)) {
        result += 16;
    } else if (__builtin_isinf(x)) {
        result += 32;
    } else if (x == 0.0 && __builtin_signbit(x)) {
        result += 64;  /* negative zero */
    }
    
    return result;
}

int main(void) {
    /* Initialize with various special FP values */
    volatile double nan1 = make_nan();
    volatile double nan2 = __builtin_nan("");
    volatile double inf_pos = make_inf();
    volatile double inf_neg = -make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    volatile double neg_zero = -0.0;
    
    /* Prevent constant folding */
    asm volatile("" : "+g"(nan1), "+g"(nan2), "+g"(inf_pos), 
                       "+g"(inf_neg), "+g"(normal1), "+g"(normal2),
                       "+g"(zero), "+g"(neg_zero));
    
    /* Call test functions with various combinations */
    int result1 = fp_test(nan1, normal1, inf_pos, nan2);
    int result2 = fp_test2(normal2, inf_neg);
    int result3 = fp_test(zero, neg_zero, nan1, normal1);
    int result4 = fp_test2(inf_pos, nan2);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3 + result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
