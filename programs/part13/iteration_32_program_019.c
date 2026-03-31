#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
volatile double get_value(void) {
    static volatile double counter = 0.0;
    return counter++ + 1.5;
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - unordered comparison with NaN */
    {
        volatile double a = nan_val;
        volatile double b = get_value();
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Using inline asm to clobber FP status */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        
        /* Ternary operator with unordered */
        global_sum += (a != a) ? 1 : 0;  /* NaN != NaN is true */
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        volatile double a = normal_val;
        volatile double b = get_value();
        
        /* Using __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 2;
        }
        
        /* Mixed integer-FP comparison */
        double converted = (double)(int)volatile_int;
        if (converted == converted && b == b) {  /* Both are ordered */
            global_sum += 1;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Two NaNs are unordered equal (both unordered) */
        if (!(a < b) && !(a > b)) {
            global_sum += 3;
        }
        
        /* Using switch with side effect */
        switch ((a == b) ? 1 : 0) {  /* NaN == NaN is false */
            case 0: global_sum += 1; break;
            default: break;
        }
    }
    
    /* Block 4: UNGE - not less than (unordered, greater, or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using __builtin_isgreaterequal */
        if (__builtin_isgreaterequal(a, b)) {
            global_sum += 4;
        }
        
        /* Equivalent: !(a < b) */
        if (!(a < b)) {
            global_sum += 1;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using __builtin_isgreater */
        if (__builtin_isgreater(a, b)) {
            global_sum += 5;
        }
        
        /* Equivalent: !(a <= b) */
        if (!(a <= b)) {
            global_sum += 1;
        }
    }
    
    /* Block 6: UNLE - unordered or less than or equal */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        
        /* Using __builtin_islessequal */
        if (__builtin_islessequal(a, b)) {
            global_sum += 6;
        }
        
        /* Direct comparison */
        if (a <= b) {
            global_sum += 1;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        
        /* Using __builtin_isless */
        if (__builtin_isless(a, b)) {
            global_sum += 7;
        }
        
        /* Direct comparison */
        if (a < b) {
            global_sum += 1;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        volatile double a = normal_val;
        volatile double b = get_value();
        
        /* Ordered and not equal */
        if ((a < b) || (a > b)) {
            global_sum += 8;
        }
        
        /* Using inline asm to force re-evaluation */
        asm volatile ("" : : : "cc");
    }
    
    /* Loop with varying conditions */
    {
        volatile double array[8];
        for (int i = 0; i < 8; i++) {
            array[i] = get_value();
            if (i == 2) array[i] = nan_val;  /* Insert a NaN */
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = array[r % 8];
            volatile double b = array[(r >> 3) % 8];
            
            /* Switch based on hash of index to use different conditions */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum++;
                    break;
                case 2:  /* UNEQ */
                    if (!(a < b) && !(a > b)) global_sum++;
                    break;
                case 3:  /* UNGE */
                    if (!(a < b)) global_sum++;
                    break;
                case 4:  /* UNGT */
                    if (!(a <= b)) global_sum++;
                    break;
                case 5:  /* UNLE */
                    if (a <= b) global_sum++;
                    break;
                case 6:  /* UNLT */
                    if (a < b) global_sum++;
                    break;
                case 7:  /* LTGT */
                    if ((a < b) || (a > b)) global_sum++;
                    break;
            }
            
            /* Mixed integer-FP comparison in loop */
            double mixed = (double)(int)(r % 100);
            if (mixed < a) {
                global_sum += (r & 1);
            }
        }
    }
    
    /* Additional complex expression combining multiple conditions */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        volatile double c = get_value();
        
        /* Complex conditional expression */
        int complex_cond = (__builtin_isunordered(a, b) ? 1 : 0) +
                          (__builtin_isgreater(c, b) ? 2 : 0) +
                          (!(a <= c) ? 4 : 0);
        
        /* Use in switch to force jump table generation */
        switch (complex_cond & 7) {
            case 0: global_sum += 10; break;
            case 1: global_sum += 20; break;
            case 2: global_sum += 30; break;
            case 3: global_sum += 40; break;
            case 4: global_sum += 50; break;
            case 5: global_sum += 60; break;
            case 6: global_sum += 70; break;
            case 7: global_sum += 80; break;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
