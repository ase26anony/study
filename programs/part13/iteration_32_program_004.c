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
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        if (a != a || b != b) {
            global_sum += 2;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(a, nan_val) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double a = get_value(1);
        double b = get_value(2);
        
        if (!__builtin_isunordered(a, b)) {
            global_sum += 4;
        }
        
        /* Using __builtin_isfinite to ensure ordered */
        if (__builtin_isfinite(a) && __builtin_isfinite(b)) {
            global_sum += 5;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        double a = nan_val;
        double b = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!(a < b) && !(a > b)) {
            global_sum += 6;
        }
        
        /* Using volatile to prevent optimization */
        volatile double v1 = nan_val;
        volatile double v2 = nan_val;
        global_sum += (v1 == v2) ? 7 : 0;  /* Should be false for NaN */
    }
    
    /* Block 4: UNGE (not less than, includes unordered) */
    {
        double a = get_value(3);
        double b = nan_val;
        
        if (!(a < b)) {  /* UNGE: not less than (nlt) */
            global_sum += 8;
        }
        
        /* Using __builtin_isgreaterequal with NaN */
        global_sum += __builtin_isgreaterequal(a, b) ? 9 : 0;
    }
    
    /* Block 5: UNGT (not less than or equal, includes unordered) */
    {
        double a = nan_val;
        double b = get_value(4);
        
        if (!(a <= b)) {  /* UNGT: not less than or equal (nle) */
            global_sum += 10;
        }
        
        /* Using __builtin_isgreater with NaN */
        global_sum += __builtin_isgreater(a, b) ? 11 : 0;
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        double a = get_value(0);
        double b = nan_val;
        
        if (__builtin_islessequal(a, b)) {  /* UNLE: unordered or less than or equal */
            global_sum += 12;
        }
        
        /* Explicit comparison */
        global_sum += (a <= b) ? 13 : 0;
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        double a = nan_val;
        double b = get_value(1);
        
        if (__builtin_isless(a, b)) {  /* UNLT: unordered or less than */
            global_sum += 14;
        }
        
        /* Explicit comparison */
        global_sum += (a < b) ? 15 : 0;
    }
    
    /* Block 8: LTGT (less than or greater than, ordered and not equal) */
    {
        double a = get_value(2);
        double b = get_value(3);
        
        if ((a < b) || (a > b)) {  /* LTGT: less than or greater than (une) */
            global_sum += 16;
        }
        
        /* Using != for ordered values */
        if (a != b) {
            global_sum += 17;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(4);
        
        /* Various comparisons with converted integer */
        if (a < b) global_sum += 18;
        if (a > b) global_sum += 19;
        if (a <= b) global_sum += 20;
        if (a >= b) global_sum += 21;
        if (a == b) global_sum += 22;
        if (a != b) global_sum += 23;
    }
    
    /* Switch statement with floating comparisons */
    {
        double a = get_value(0);
        double b = nan_val;
        int result = 0;
        
        switch (__builtin_isunordered(a, b) ? 1 : 
                __builtin_isless(a, b) ? 2 :
                __builtin_isgreater(a, b) ? 3 : 0) {
            case 1: result = 24; break;  /* UNORDERED */
            case 2: result = 25; break;  /* UNLT */
            case 3: result = 26; break;  /* UNGT */
            default: result = 27; break; /* ORDERED equal */
        }
        global_sum += result;
    }
    
    /* Loop with varying conditions */
    {
        volatile double array[10];
        for (int i = 0; i < 10; i++) {
            array[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = array[r % 10];
            double b = array[(r >> 8) % 10];
            
            /* Vary NaN usage based on random bits */
            if (r & 0x100) a = nan_val;
            if (r & 0x200) b = nan_val;
            
            /* Select comparison based on random bits */
            switch (r & 0x7) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:
                    if (!__builtin_isunordered(a, b)) global_sum += 2;
                    break;
                case 2:
                    if (!(a < b) && !(a > b)) global_sum += 3;
                    break;
                case 3:
                    if (!(a < b)) global_sum += 4;  /* UNGE */
                    break;
                case 4:
                    if (!(a <= b)) global_sum += 5; /* UNGT */
                    break;
                case 5:
                    if (a <= b) global_sum += 6;    /* UNLE */
                    break;
                case 6:
                    if (a < b) global_sum += 7;     /* UNLT */
                    break;
                case 7:
                    if ((a < b) || (a > b)) global_sum += 8; /* LTGT */
                    break;
            }
            
            /* Clobber FPU status register periodically */
            if ((i % 17) == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                                      "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
