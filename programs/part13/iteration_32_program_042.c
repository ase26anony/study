#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create side effects and prevent constant folding */
volatile double get_value(int idx) {
    volatile double d = (double)idx;
    /* Clobber FPU status register */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    return d;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED (NaN comparisons) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        if (a != a) {  /* NaN != NaN is true */
            sum += 2;
        }
        
        /* Ternary operator with unordered */
        sum += (__builtin_isunordered(a, b) ? 3 : 0);
    }
    
    /* Block 2: ORDERED */
    {
        volatile double a = normal_val;
        volatile double b = get_value(1);
        
        /* Using __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            sum += 4;
        }
        
        /* Using !__builtin_isunordered */
        if (!__builtin_isunordered(a, b)) {
            sum += 5;
        }
        
        /* Switch statement with ordered check */
        switch (__builtin_isordered(a, b) ? 1 : 0) {
            case 1: sum += 6; break;
            default: sum += 7; break;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Using !__builtin_islessgreater */
        if (!__builtin_islessgreater(a, b)) {
            sum += 8;
        }
        
        /* Using == with NaN (always false for ordered, true for unordered) */
        if (!(a == b)) {  /* Actually triggers UNEQ in some cases */
            sum += 9;
        }
    }
    
    /* Block 4: UNGE (not less than) */
    {
        volatile double a = get_value(2);
        volatile double b = get_value(3);
        
        /* Using !__builtin_isless */
        if (!__builtin_isless(a, b)) {
            sum += 10;
        }
        
        /* Using >= with potential NaN */
        if (a >= b || __builtin_isunordered(a, b)) {
            sum += 11;
        }
    }
    
    /* Block 5: UNGT (not less or equal) */
    {
        volatile double a = get_value(4);
        volatile double b = get_value(5);
        
        /* Using !__builtin_islessequal */
        if (!__builtin_islessequal(a, b)) {
            sum += 12;
        }
        
        /* Using > with potential NaN */
        if (a > b || __builtin_isunordered(a, b)) {
            sum += 13;
        }
    }
    
    /* Block 6: UNLE (unordered or less or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using <= with NaN */
        if (a <= b || __builtin_isunordered(a, b)) {
            sum += 14;
        }
        
        /* Using __builtin_islessequal with unordered handling */
        if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
            sum += 15;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using < with NaN */
        if (a < b || __builtin_isunordered(a, b)) {
            sum += 16;
        }
        
        /* Using __builtin_isless with unordered handling */
        if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
            sum += 17;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        volatile double a = get_value(6);
        volatile double b = get_value(7);
        
        /* Using __builtin_islessgreater */
        if (__builtin_islessgreater(a, b)) {
            sum += 18;
        }
        
        /* Using (a < b || a > b) && !__builtin_isunordered(a, b) */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            sum += 19;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = volatile_int;
        volatile double a = (double)(int)vi;
        volatile double b = get_value(8);
        
        /* Various comparisons with converted int */
        if (a == b) sum += 20;
        if (a != b) sum += 21;
        if (a < b) sum += 22;
        if (a > b) sum += 23;
        if (a <= b) sum += 24;
        if (a >= b) sum += 25;
        
        /* With NaN on one side */
        if (a != nan_val) sum += 26;
        if (nan_val != a) sum += 27;
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[8];
        for (int i = 0; i < 8; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[r % 8];
            volatile double b = arr[(r >> 3) % 8];
            
            /* Select comparison based on hash of iteration */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!__builtin_islessgreater(a, b)) sum += 3;
                    break;
                case 3:  /* UNGE */
                    if (!__builtin_isless(a, b)) sum += 4;
                    break;
                case 4:  /* UNGT */
                    if (!__builtin_islessequal(a, b)) sum += 5;
                    break;
                case 5:  /* UNLE */
                    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) sum += 6;
                    break;
                case 6:  /* UNLT */
                    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) sum += 7;
                    break;
                case 7:  /* LTGT */
                    if (__builtin_islessgreater(a, b)) sum += 8;
                    break;
            }
            
            /* Clobber FPU status register periodically */
            if (i % 13 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
