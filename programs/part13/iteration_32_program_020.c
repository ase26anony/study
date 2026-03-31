#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 1;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED comparisons */
    {
        double a = get_value(0);
        double b = nan_val;
        
        /* Multiple ways to generate UNORDERED */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        if (a != a) {  /* NaN comparison */
            global_sum += 2;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(b, a) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double a = get_value(1);
        double b = get_value(2);
        
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        
        /* Ordered via explicit check */
        if (!__builtin_isunordered(a, b)) {
            global_sum += 5;
        }
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        double a = nan_val;
        double b = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!(a < b) && !(a > b)) {
            global_sum += 6;
        }
        
        /* Using builtin */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_sum += 7;
        }
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        double a = get_value(3);
        double b = get_value(1);
        
        if (!(a < b)) {  /* Not less than includes unordered case */
            global_sum += 8;
        }
        
        if (!__builtin_isless(a, b)) {
            global_sum += 9;
        }
    }
    
    /* Block 5: UNGT (Unordered or Greater Than) */
    {
        double a = inf_val;
        double b = get_value(2);
        
        if (!(a <= b)) {
            global_sum += 10;
        }
        
        if (!__builtin_islessequal(a, b)) {
            global_sum += 11;
        }
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        double a = get_value(0);
        double b = nan_val;
        
        if (!(a > b)) {
            global_sum += 12;
        }
        
        if (!__builtin_isgreater(a, b)) {
            global_sum += 13;
        }
    }
    
    /* Block 7: UNLT (Unordered or Less Than) */
    {
        double a = -inf_val;
        double b = get_value(4);
        
        if (!(a >= b)) {
            global_sum += 14;
        }
        
        if (!__builtin_isgreaterequal(a, b)) {
            global_sum += 15;
        }
    }
    
    /* Block 8: LTGT (Less Than or Greater Than, but not Equal) */
    {
        double a = get_value(1);
        double b = get_value(3);
        
        if (a != b) {
            global_sum += 16;
        }
        
        if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
            global_sum += 17;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(volatile_int % 5);
        
        switch ((volatile_int >> 3) & 7) {
            case 0:
                if (a == b) global_sum += 18;
                break;
            case 1:
                if (a != b) global_sum += 19;
                break;
            case 2:
                if (a < b) global_sum += 20;
                break;
            case 3:
                if (a > b) global_sum += 21;
                break;
            case 4:
                if (a <= b) global_sum += 22;
                break;
            case 5:
                if (a >= b) global_sum += 23;
                break;
            case 6:
                if (__builtin_isunordered(a, b)) global_sum += 24;
                break;
            case 7:
                if (__builtin_isordered(a, b)) global_sum += 25;
                break;
        }
    }
    
    /* Loop with varying conditions */
    {
        double loop_values[10];
        for (int i = 0; i < 10; i++) {
            loop_values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = loop_values[r % 10];
            double b = loop_values[(r >> 8) % 10];
            
            /* Vary condition based on hash */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2: /* UNEQ */
                    if (!(a < b) && !(a > b)) global_sum += 3;
                    break;
                case 3: /* UNGE */
                    if (!(a < b)) global_sum += 4;
                    break;
                case 4: /* UNGT */
                    if (!(a <= b)) global_sum += 5;
                    break;
                case 5: /* UNLE */
                    if (!(a > b)) global_sum += 6;
                    break;
                case 6: /* UNLT */
                    if (!(a >= b)) global_sum += 7;
                    break;
                case 7: /* LTGT */
                    if (a != b) global_sum += 8;
                    break;
            }
            
            /* Occasionally clobber FPU status */
            if ((r & 0xFF) == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                              "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
