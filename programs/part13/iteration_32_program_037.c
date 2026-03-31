#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple LCG for pseudo-random sequence */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double v = 3.14159 * idx;
    return v;
}

int main(void) {
    /* Initialize volatile variables with NaN and normal values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 42.0;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 7;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED - comparison with NaN */
    {
        double a = nan_val;
        double b = get_value(1);
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        /* Also use ternary operator */
        sum += (a != b) ? 0 : 1;  /* This may generate unordered checks */
    }
    
    /* Block 2: ORDERED - both operands are not NaN */
    {
        double a = normal_val;
        double b = get_value(2);
        if (!__builtin_isunordered(a, b)) {
            sum += 2;
        }
        /* Mixed comparison */
        if (a == b) {
            sum += 0;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = nan_val;
        double b = nan_val;
        /* Using explicit operators that may generate UNEQ */
        if (!(a < b) && !(a > b)) {
            sum += 3;
        }
    }
    
    /* Block 4: UNGE - not less than (unordered or greater or equal) */
    {
        double a = get_value(4);
        double b = nan_val;
        if (!__builtin_isless(a, b)) {
            sum += 4;
        }
        /* Alternative using >= operator with NaN */
        if (a >= b) {
            sum += 0;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    {
        double a = nan_val;
        double b = get_value(5);
        if (!__builtin_islessequal(a, b)) {
            sum += 5;
        }
        /* Using > operator */
        if (a > b) {
            sum += 0;
        }
    }
    
    /* Block 6: UNLE - unordered or less or equal */
    {
        double a = get_value(6);
        double b = nan_val;
        if (!__builtin_isgreater(a, b)) {
            sum += 6;
        }
        /* Using <= operator */
        if (a <= b) {
            sum += 0;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        double a = nan_val;
        double b = get_value(7);
        if (!__builtin_isgreaterequal(a, b)) {
            sum += 7;
        }
        /* Using < operator */
        if (a < b) {
            sum += 0;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        double a = get_value(8);
        double b = get_value(9);
        if (__builtin_islessgreater(a, b)) {
            sum += 8;
        }
        /* Alternative: (a < b) || (a > b) for ordered values */
        if ((a < b) || (a > b)) {
            sum += 0;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(10);
        if (a != b) {
            sum += 9;
        }
        if (a < b) {
            sum += 0;
        }
    }
    
    /* Switch statement with floating comparisons */
    {
        double a = get_value(11);
        double b = nan_val;
        int cmp_result = 0;
        
        /* Force multiple condition code evaluations */
        if (__builtin_isunordered(a, b)) cmp_result = 1;
        else if (a < b) cmp_result = 2;
        else if (a > b) cmp_result = 3;
        else cmp_result = 4;
        
        switch (cmp_result) {
            case 1: sum += 10; break;
            case 2: sum += 11; break;
            case 3: sum += 12; break;
            case 4: sum += 13; break;
        }
    }
    
    /* Loop with varying conditions */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 16;
            int idx2 = (r >> 8) % 16;
            double a = values[idx1];
            double b = values[idx2];
            
            /* Select comparison based on hash of iteration */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) sum += 1;
                    break;
                case 1: /* ORDERED */
                    if (!__builtin_isunordered(a, b)) sum += 2;
                    break;
                case 2: /* UNEQ-like */
                    if (!(a < b) && !(a > b)) sum += 3;
                    break;
                case 3: /* UNGE-like */
                    if (!__builtin_isless(a, b)) sum += 4;
                    break;
                case 4: /* UNGT-like */
                    if (!__builtin_islessequal(a, b)) sum += 5;
                    break;
                case 5: /* UNLE-like */
                    if (!__builtin_isgreater(a, b)) sum += 6;
                    break;
                case 6: /* UNLT-like */
                    if (!__builtin_isgreaterequal(a, b)) sum += 7;
                    break;
                case 7: /* LTGT-like */
                    if (__builtin_islessgreater(a, b)) sum += 8;
                    break;
            }
            
            /* Occasionally mix in NaN */
            if ((r >> 16) % 5 == 0) {
                if (__builtin_isunordered(a, nan_val)) sum += 9;
            }
            
            /* Clobber FPU registers periodically */
            if (i % 7 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
