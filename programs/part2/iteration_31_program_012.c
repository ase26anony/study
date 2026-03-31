/* test_i386_condition_codes.c
 * Program to generate RTL with various floating-point condition codes
 * for i386 x87 backend coverage testing.
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_nan = 0.0/0.0;  /* Generate NaN */

/* Function to create complex floating-point comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double nan_val) {
    volatile int local_counter = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    /* Use inline assembly to create data dependencies */
    asm volatile("" : "+g"(x), "+g"(y), "+g"(z) : : "memory");
    
    /* Loop to create multiple basic blocks with FP comparisons */
    for (int i = 0; i < 10; i++) {
        /* 1. UNORDERED comparisons (should generate UNORDERED condition code) */
        if (__builtin_isunordered(x, nan_val)) {
            local_counter += 1;
        }
        
        /* 2. ORDERED comparisons (should generate ORDERED condition code) */
        if (!__builtin_isunordered(y, z)) {
            local_counter += 2;
        }
        
        /* 3. Direct NaN check (may generate UNORDERED or ORDERED) */
        if (x != x) {  /* NaN check */
            local_counter += 3;
        }
        
        /* 4. UNEQ (unordered or equal) - using isnan */
        if (__builtin_isnan(x) || x == y) {
            local_counter += 4;
        }
        
        /* 5. UNGE (not less than) - unordered or greater-or-equal */
        if (!(x < y)) {
            local_counter += 5;
        }
        
        /* 6. UNGT (not less or equal) - unordered or greater */
        if (!(x <= y)) {
            local_counter += 6;
        }
        
        /* 7. UNLE (unordered or less-or-equal) */
        if (__builtin_isunordered(x, y) || x <= y) {
            local_counter += 7;
        }
        
        /* 8. UNLT (unordered or less than) */
        if (__builtin_isunordered(x, y) || x < y) {
            local_counter += 8;
        }
        
        /* 9. LTGT (less than or greater than, but not equal/unordered) */
        if (x < y || x > y) {
            local_counter += 9;
        }
        
        /* Mix values to create different comparison scenarios */
        asm volatile("" : : "g"(local_counter) : "memory");
        x += 1.0;
        y -= 0.5;
        z *= 1.1;
    }
    
    return local_counter;
}

/* Another test function with different comparison patterns */
NOINLINE static int fp_test2(volatile double p, volatile double q) {
    volatile int counter = 0;
    
    /* Create various comparison chains */
    if (__builtin_isunordered(p, q)) {
        counter += 100;
    } else if (p == q) {
        counter += 200;
    } else if (p < q) {
        counter += 300;
    } else if (p > q) {
        counter += 400;
    }
    
    /* Switch-like structure with FP comparisons */
    for (int j = 0; j < 5; j++) {
        volatile double temp = p + j;
        
        if (__builtin_isnan(temp)) {
            counter += 1000;
        } else if (temp == 0.0) {
            counter += 2000;
        } else if (temp < 0.0) {
            counter += 3000;
        } else {
            counter += 4000;
        }
        
        /* Ordered comparison check */
        if (!__builtin_isunordered(temp, q)) {
            counter += 5000;
        }
    }
    
    return counter;
}

int main(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan1 = global_nan;
    volatile double nan2 = __builtin_nan("0xdead");
    
    /* Create initial NaN */
    volatile double my_nan = 0.0 / 0.0;
    
    /* Call test functions multiple times with different values */
    int result1 = fp_test(a, b, c, my_nan);
    int result2 = fp_test(b, nan1, a, nan2);
    int result3 = fp_test(nan1, a, b, my_nan);
    int result4 = fp_test2(a, nan1);
    int result5 = fp_test2(nan2, b);
    
    /* Use results to prevent dead code elimination */
    global_counter = result1 + result2 + result3 + result4 + result5;
    
    /* Print to ensure side effects */
    printf("Result: %d\n", global_counter);
    
    return global_counter > 0 ? 0 : 1;
}
