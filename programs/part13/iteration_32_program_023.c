#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int side_effect = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double v = (double)idx;
    /* Clobber FPU to force re-evaluation */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    return v;
}

/* Pseudo-random generator for varying conditions */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Force FPU operations */
    asm volatile ("" : : : "memory");
    
    /* ====== Target each specific condition code ====== */
    
    /* 1. UNORDERED - unordered comparison */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        /* Also in ternary form */
        side_effect += (a != b) ? 1 : 0;  /* != with NaN */
    }
    
    /* 2. ORDERED - ordered comparison */
    {
        volatile double a = normal_val;
        volatile double b = zero_val;
        if (!__builtin_isunordered(a, b)) {
            global_sum += 2;
        }
        /* Using explicit operators */
        if (a < b || a >= b) {  /* Always true for ordered values */
            side_effect += 2;
        }
    }
    
    /* 3. UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        /* Two NaNs are unordered equal (both unordered) */
        if (!(a == b) && __builtin_isunordered(a, b)) {
            global_sum += 3;
        }
        /* Force through conversion */
        double c = (double)(int)volatile_int;
        if (!(c != c)) {  /* c == c, but compiler might use UNEQ */
            side_effect += 3;
        }
    }
    
    /* 4. UNGE - unordered or greater-or-equal */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a < b)) {  /* Not less includes unordered and greater-or-equal */
            global_sum += 4;
        }
        /* Alternative using builtin */
        if (!__builtin_isless(a, b)) {
            side_effect += 4;
        }
    }
    
    /* 5. UNGT - unordered or greater */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a <= b)) {  /* Not less-or-equal includes unordered and greater */
            global_sum += 5;
        }
        if (!__builtin_islessequal(a, b)) {
            side_effect += 5;
        }
    }
    
    /* 6. UNLE - unordered or less-or-equal */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (!(a > b)) {  /* Not greater includes unordered and less-or-equal */
            global_sum += 6;
        }
        if (!__builtin_isgreater(a, b)) {
            side_effect += 6;
        }
    }
    
    /* 7. UNLT - unordered or less */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (!(a >= b)) {  /* Not greater-or-equal includes unordered and less */
            global_sum += 7;
        }
        if (!__builtin_isgreaterequal(a, b)) {
            side_effect += 7;
        }
    }
    
    /* 8. LTGT - less, greater, or unordered (but not equal) */
    {
        volatile double a = normal_val;
        volatile double b = zero_val;
        if (a != b) {  /* Not equal, but both ordered */
            global_sum += 8;
        }
        /* With NaN */
        volatile double c = nan_val;
        if (c != normal_val) {  /* Unequal and unordered */
            side_effect += 8;
        }
    }
    
    /* ====== Loop with varying conditions ====== */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i);
            if (i == 7) values[i] = nan_val;
            if (i == 11) values[i] = -inf_val;
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = (r >> 0) & 0xF;
            int idx2 = (r >> 4) & 0xF;
            double a = values[idx1];
            double b = values[idx2];
            
            /* Switch on condition type */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (!__builtin_isunordered(a, b)) global_sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!(a == b) && __builtin_isunordered(a, b)) global_sum += 3;
                    break;
                case 3:  /* UNGE */
                    if (!(a < b)) global_sum += 4;
                    break;
                case 4:  /* UNGT */
                    if (!(a <= b)) global_sum += 5;
                    break;
                case 5:  /* UNLE */
                    if (!(a > b)) global_sum += 6;
                    break;
                case 6:  /* UNLT */
                    if (!(a >= b)) global_sum += 7;
                    break;
                case 7:  /* LTGT */
                    if (a != b) global_sum += 8;
                    break;
            }
            
            /* Mixed integer-FP comparison */
            double conv_a = (double)(int)(r & 0xFF);
            if (conv_a != b) {
                side_effect += 1;
            }
            
            /* Clobber FPU status */
            asm volatile ("" : : : "cc");
        }
    }
    
    /* ====== Additional complex cases ====== */
    {
        /* Nested comparisons */
        volatile double x = nan_val;
        volatile double y = get_value(1);
        volatile double z = get_value(2);
        
        /* Complex expression that might generate multiple condition codes */
        int result = (x < y) ? 1 : ((y > z) ? 2 : ((x != x) ? 3 : 4));
        global_sum += result;
        
        /* Comparison in function argument */
        side_effect += (x == y) + (y != z) + (z >= x);
    }
    
    printf("Result: %d (side: %d)\n", global_sum, side_effect);
    return global_sum > 0 ? 0 : 1;
}
