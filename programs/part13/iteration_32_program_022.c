#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double arr[] = {1.0, 2.0, 3.0, __builtin_nan(""), 5.0};
    return arr[idx % 5];
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 42;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double x = 2.5;
    volatile double y = 3.5;
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = 1.0 / 0.0;
    volatile int volatile_int = 7;
    
    int local_sum = 0;
    
    /* Block 1: UNORDERED - unordered comparison */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(0); /* 1.0 */
        if (__builtin_isunordered(a, b)) {
            local_sum += 1;
        }
        /* Inline asm to clobber FP status */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        double a = get_value(0); /* 1.0 */
        double b = get_value(1); /* 2.0 */
        if (!__builtin_isunordered(a, b)) {
            local_sum += 2;
        }
        /* Alternative: using explicit operators with volatile */
        volatile double v1 = x;
        volatile double v2 = y;
        if (v1 < v2) { /* This should be ordered */
            local_sum += 1;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(3); /* NaN */
        /* Two NaNs are unordered equal (both NaN, neither less/greater) */
        if (!(a < b) && !(a > b)) {
            local_sum += 3;
        }
        /* Using ternary operator */
        local_sum += (a == b) ? 0 : 1; /* NaN == NaN is false, but UNEQ is true */
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(0); /* 1.0 */
        if (!(a < b)) { /* not less than (greater, equal, or unordered) */
            local_sum += 4;
        }
    }
    
    /* Block 5: UNGT - unordered or greater-than */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(1); /* 2.0 */
        if (!(a <= b)) { /* not less-or-equal (greater or unordered) */
            local_sum += 5;
        }
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        double a = get_value(2); /* 3.0 */
        double b = get_value(3); /* NaN */
        if (!(a > b)) { /* not greater than (less, equal, or unordered) */
            local_sum += 6;
        }
    }
    
    /* Block 7: UNLT - unordered or less-than */
    {
        double a = get_value(0); /* 1.0 */
        double b = get_value(3); /* NaN */
        if (!(a >= b)) { /* not greater-or-equal (less or unordered) */
            local_sum += 7;
        }
    }
    
    /* Block 8: LTGT - less-than or greater-than (ordered and not equal) */
    {
        double a = get_value(0); /* 1.0 */
        double b = get_value(1); /* 2.0 */
        if (a < b || a > b) { /* ordered and not equal */
            local_sum += 8;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = volatile_int;
        double converted = (double)(int)vi;
        if (converted > x) {
            local_sum += 1;
        }
        if (converted < y) {
            local_sum += 1;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        double loop_values[10];
        for (int i = 0; i < 10; i++) {
            loop_values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = loop_values[r % 10];
            double b = loop_values[(r >> 8) % 10];
            
            /* Switch on hash to use different comparisons */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) local_sum++;
                    break;
                case 1: /* ORDERED */
                    if (!__builtin_isunordered(a, b)) local_sum++;
                    break;
                case 2: /* UNEQ */
                    if (!(a < b) && !(a > b)) local_sum++;
                    break;
                case 3: /* UNGE */
                    if (!(a < b)) local_sum++;
                    break;
                case 4: /* UNGT */
                    if (!(a <= b)) local_sum++;
                    break;
                case 5: /* UNLE */
                    if (!(a > b)) local_sum++;
                    break;
                case 6: /* UNLT */
                    if (!(a >= b)) local_sum++;
                    break;
                case 7: /* LTGT */
                    if (a < b || a > b) local_sum++;
                    break;
            }
            
            /* Clobber FPU occasionally */
            if (i % 13 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
            }
        }
    }
    
    /* Use switch statement with floating comparisons */
    {
        volatile double v = x;
        switch (volatile_int % 4) {
            case 0:
                if (v < y) local_sum += 1;
                break;
            case 1:
                if (v > y) local_sum += 2;
                break;
            case 2:
                if (v <= y) local_sum += 3;
                break;
            case 3:
                if (v >= y) local_sum += 4;
                break;
        }
    }
    
    /* Complex expression mixing multiple conditions */
    {
        double a = get_value(0);
        double b = get_value(3); /* NaN */
        double c = get_value(1);
        
        /* This should generate multiple condition code checks */
        int result = (a < b) ? 1 : 
                    (b > c) ? 2 :
                    (a == c) ? 3 :
                    (a != b) ? 4 : 5;
        local_sum += result;
    }
    
    global_sum = local_sum;
    printf("Result: %d\n", global_sum);
    
    return global_sum > 0 ? 0 : 1;
}
