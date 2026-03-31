/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386/x87 backend coverage.
 */

/* Prevent inlining to keep FP comparisons in separate functions */
#define NOINLINE __attribute__((noinline))

/* Global volatile counter to prevent optimization */
volatile int counter = 0;

/* Function with various FP condition checks */
NOINLINE static int fp_test(volatile double a, volatile double b, volatile double c) {
    int local_counter = 0;
    
    /* Loop to create more RTL instructions */
    for (int i = 0; i < 5; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            local_counter += 1;  /* UNORDERED condition */
        }
        
        if (a != a) {  /* This should also generate unordered check for NaN */
            local_counter += 2;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            local_counter += 3;  /* ORDERED condition */
        }
        
        /* Standard comparisons that might generate other condition codes */
        if (a < b) {
            local_counter += 4;  /* LT condition */
        }
        
        if (a > c) {
            local_counter += 5;  /* GT condition */
        }
        
        if (a == b) {
            local_counter += 6;  /* EQ condition */
        }
        
        /* More complex conditions */
        if (__builtin_isnan(a)) {
            local_counter += 7;  /* Another unordered check */
        }
        
        /* UNEQ: unordered or equal */
        if (!(a < b) && !(a > b)) {
            local_counter += 8;
        }
        
        /* UNGE: not less than (unordered or greater or equal) */
        if (!(a < b)) {
            local_counter += 9;
        }
        
        /* UNGT: not less than or equal */
        if (!(a <= b)) {
            local_counter += 10;
        }
        
        /* UNLE: unordered or less or equal */
        if (!(a > b)) {
            local_counter += 11;
        }
        
        /* UNLT: unordered or less than */
        if (!(a >= b)) {
            local_counter += 12;
        }
        
        /* LTGT: less than or greater than (but not equal, not unordered) */
        if ((a < b) || (a > b)) {
            local_counter += 13;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to create data dependencies */
        a = a * 1.0001;
        b = b * 0.9999;
    }
    
    return local_counter;
}

/* Another test function focusing on long double (80-bit x87) */
NOINLINE static int fp_test_long(volatile long double x, volatile long double y) {
    int result = 0;
    
    /* Generate various condition codes with long double */
    if (__builtin_isunordered(x, y)) {
        result += 100;
    }
    
    if (x == y) {
        result += 200;
    }
    
    if (x < y) {
        result += 300;
    }
    
    if (x > y) {
        result += 400;
    }
    
    /* ORDERED check */
    if (!__builtin_isunordered(x, y)) {
        result += 500;
    }
    
    return result;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;      /* Quiet NaN */
    volatile double inf_val = 1.0 / 0.0;      /* Infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Also test with __builtin_nan */
    volatile double explicit_nan = __builtin_nan("");
    
    /* Long double tests */
    volatile long double ld_nan = 0.0L / 0.0L;
    volatile long double ld_normal = 1.23456789L;
    
    /* Call test functions multiple times with different combinations */
    int total = 0;
    
    total += fp_test(nan_val, normal1, normal2);
    total += fp_test(explicit_nan, inf_val, normal1);
    total += fp_test(normal1, nan_val, normal2);
    total += fp_test(normal1, normal2, nan_val);
    
    total += fp_test_long(ld_nan, ld_normal);
    total += fp_test_long(ld_normal, ld_nan);
    
    /* Mix in some non-NaN comparisons */
    total += fp_test(normal1, normal2, 42.0);
    total += fp_test(100.0, 200.0, 300.0);
    
    /* Update global counter to prevent dead code elimination */
    counter = total;
    
    /* Use the result to prevent optimization */
    if (counter > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
