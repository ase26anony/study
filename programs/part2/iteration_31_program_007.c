/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386 x87
 * Compile with: gcc -m32 -mfpmath=387 -O2 -fdump-rtl-final test.c -o test
 */

/* Prevent inlining to keep function boundaries clear */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static volatile double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static volatile double make_inf(void) {
    return 1.0 / 0.0;
}

/* Function with various FP comparisons that should generate different condition codes */
NOINLINE static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create more RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED condition: check if either operand is NaN */
        if (__builtin_isunordered(a, b)) {
            counter++;
            /* Use inline asm to prevent optimization */
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* ORDERED condition: check if both operands are not NaN */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* UNEQ condition: unordered or equal */
        /* This might generate UNEQ in some RTL representations */
        if (a == b || __builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* UNGE condition: unordered or greater than or equal */
        if (a >= b || __builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* UNGT condition: unordered or greater than */
        if (a > b || __builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* UNLE condition: unordered or less than or equal */
        if (a <= b || __builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* UNLT condition: unordered or less than */
        if (a < b || __builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* LTGT condition: less than or greater than (ordered and not equal) */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* Standard ordered comparisons */
        if (a < c) {
            counter++;
        }
        
        if (a > c) {
            counter++;
        }
        
        if (a == c) {
            counter++;
        }
        
        if (a != c) {
            counter++;
        }
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c));
    }
    
    return counter;
}

/* Another test function focusing on direct NaN checks */
NOINLINE static int nan_test(volatile double x, volatile double y) {
    volatile int result = 0;
    
    /* Direct NaN checks that should generate UNORDERED conditions */
    if (x != x) {  /* Classic NaN test */
        result |= 1;
    }
    
    if (__builtin_isnan(x)) {
        result |= 2;
    }
    
    if (__builtin_isnan(y)) {
        result |= 4;
    }
    
    /* Ordered comparison with potential NaN */
    if (x < y) {
        result |= 8;
    }
    
    /* Check if both are ordered (not NaN) */
    if (!__builtin_isunordered(x, y)) {
        result |= 16;
    }
    
    return result;
}

int main(void) {
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    int result1, result2, result3;
    
    /* Test 1: NaN vs normal number */
    result1 = fp_test(nan_val, normal1, normal2);
    
    /* Test 2: Infinity vs NaN */
    result2 = fp_test(inf_val, nan_val, normal1);
    
    /* Test 3: Normal vs normal with NaN introduced */
    result3 = nan_test(normal1, nan_val);
    
    /* Test 4: More combinations */
    volatile double temp = normal1;
    for (int i = 0; i < 5; i++) {
        /* Mix ordered and unordered comparisons */
        if (__builtin_isunordered(temp, nan_val)) {
            result1++;
        }
        
        if (temp < inf_val) {
            result2++;
        }
        
        /* Create LTGT condition */
        if (temp != zero && !__builtin_isunordered(temp, zero)) {
            result3++;
        }
        
        /* Modify temp to create different values */
        asm volatile("" : "+g"(temp));
        temp += 1.0;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = result1 + result2 + result3;
    
    /* Print to ensure code isn't optimized away */
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
