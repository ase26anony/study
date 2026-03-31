#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int seed) {
    volatile double v = (double)seed;
    return v;
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
    volatile double neg_val = -2.71828;
    
    volatile int int_val = 42;
    
    /* Force re-evaluation of FP comparisons */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED (NaN comparisons) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Multiple ways to generate UNORDERED */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        if (a != a) {  /* NaN self-comparison */
            global_sum += 2;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(a, b) ? 3 : 0;
    }
    
    /* Block 2: ORDERED (normal comparisons) */
    {
        volatile double a = normal_val;
        volatile double b = neg_val;
        
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        
        /* Mixed with inline assembly clobber */
        asm volatile ("" : : : "cc");
        global_sum += (a < b) ? 5 : 0;
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        volatile double c = normal_val;
        volatile double d = normal_val;
        
        /* (a == b) where both are NaN gives UNEQ */
        if (!(a == b)) {  /* Note: !(NaN == NaN) is true */
            global_sum += 6;
        }
        
        /* Normal equality that's always false with NaN */
        global_sum += (c == d) ? 7 : 0;
    }
    
    /* Block 4: UNGE (not less than: unordered or greater or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!(a < b)) {  /* UNGE: not less than */
            global_sum += 8;
        }
        
        /* Using builtin for clarity */
        global_sum += !__builtin_isless(a, b) ? 9 : 0;
    }
    
    /* Block 5: UNGT (not less or equal: unordered or greater) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!(a <= b)) {  /* UNGT: not less than or equal */
            global_sum += 10;
        }
        
        global_sum += !__builtin_islessequal(a, b) ? 11 : 0;
    }
    
    /* Block 6: UNLE (unordered or less or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (a <= b) {  /* This is false for NaN, but generates UNLE */
            global_sum += 12;
        }
        
        /* Alternative with builtin */
        global_sum += __builtin_islessequal(a, b) ? 13 : 0;
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (a < b) {  /* Generates UNLT */
            global_sum += 14;
        }
        
        global_sum += __builtin_isless(a, b) ? 15 : 0;
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        volatile double a = normal_val;
        volatile double b = neg_val;
        
        if (a != b) {  /* LTGT when both are ordered and not equal */
            global_sum += 16;
        }
        
        /* Mixed integer-FP comparison */
        volatile double conv_val = (double)(int)int_val;
        global_sum += (conv_val != normal_val) ? 17 : 0;
    }
    
    /* Switch statement with FP comparisons */
    {
        volatile double x = get_value(1);
        volatile double y = get_value(2);
        
        int result = 0;
        if (__builtin_isunordered(x, y)) result = 1;
        else if (__builtin_isless(x, y)) result = 2;
        else if (__builtin_isgreater(x, y)) result = 3;
        else if (__builtin_islessequal(x, y)) result = 4;
        else if (__builtin_isgreaterequal(x, y)) result = 5;
        
        switch (result) {
            case 1: global_sum += 18; break;  /* UNORDERED */
            case 2: global_sum += 19; break;  /* UNLT or LT */
            case 3: global_sum += 20; break;  /* UNGT or GT */
            case 4: global_sum += 21; break;  /* UNLE or LE */
            case 5: global_sum += 22; break;  /* UNGE or GE */
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[8] = {
            nan_val, inf_val, normal_val, zero_val,
            -inf_val, -normal_val, __builtin_nan("0x1234"), 100.0
        };
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 8;
            int idx2 = (r >> 8) % 8;
            volatile double a = arr[idx1];
            volatile double b = arr[idx2];
            
            /* Different comparisons based on hash */
            switch (r % 8) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:
                    if (!(a == b)) global_sum += 3;  /* UNEQ */
                    break;
                case 3:
                    if (!(a < b)) global_sum += 4;   /* UNGE */
                    break;
                case 4:
                    if (!(a <= b)) global_sum += 5;  /* UNGT */
                    break;
                case 5:
                    if (a <= b) global_sum += 6;     /* UNLE */
                    break;
                case 6:
                    if (a < b) global_sum += 7;      /* UNLT */
                    break;
                case 7:
                    if (a != b) global_sum += 8;     /* LTGT */
                    break;
            }
            
            /* Inline assembly to clobber FPU state */
            if (i % 16 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                              "st(4)", "st(5)", "st(6)", "st(7)", "cc");
            }
        }
    }
    
    /* Final mixed comparisons with integer conversions */
    {
        volatile int vi = 100;
        for (int i = 0; i < 10; i++) {
            volatile double a = (double)(int)(vi + i);
            volatile double b = get_value(i);
            
            /* These should generate various condition codes */
            if (a == b) global_sum += 1;
            if (a != b) global_sum += 2;
            if (a < b) global_sum += 3;
            if (a > b) global_sum += 4;
            if (a <= b) global_sum += 5;
            if (a >= b) global_sum += 6;
            
            /* Check for unordered */
            if (__builtin_isunordered(a, b)) global_sum += 7;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
