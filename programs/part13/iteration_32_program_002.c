#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    /* Initialize volatile variables with NaN and normal values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED comparisons */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        if (a != a) {  /* NaN != NaN is true */
            global_sum += 2;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(a, b) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        volatile double a = normal_val;
        volatile double b = get_value(0);
        
        /* Using __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        
        /* Ordered check via negation */
        if (!__builtin_isunordered(a, b)) {
            global_sum += 5;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Unordered or equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_sum += 6;
        }
        
        /* Using explicit operators */
        if (a == b || __builtin_isunordered(a, b)) {
            global_sum += 7;
        }
    }
    
    /* Block 4: UNGE (not less than, unordered allowed) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using __builtin_isgreaterequal with NaN */
        if (!__builtin_isless(a, b)) {
            global_sum += 8;
        }
        
        /* Explicit check */
        if (a >= b || __builtin_isunordered(a, b)) {
            global_sum += 9;
        }
    }
    
    /* Block 5: UNGT (not less than or equal, unordered allowed) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!__builtin_islessequal(a, b)) {
            global_sum += 10;
        }
        
        if (a > b || __builtin_isunordered(a, b)) {
            global_sum += 11;
        }
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!__builtin_isgreater(a, b)) {
            global_sum += 12;
        }
        
        if (a <= b || __builtin_isunordered(a, b)) {
            global_sum += 13;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!__builtin_isgreaterequal(a, b)) {
            global_sum += 14;
        }
        
        if (a < b || __builtin_isunordered(a, b)) {
            global_sum += 15;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, ordered) */
    {
        volatile double a = normal_val;
        volatile double b = get_value(1);
        
        if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
            global_sum += 16;
        }
        
        if (a != b && __builtin_isordered(a, b)) {
            global_sum += 17;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = volatile_int;
        volatile double a = (double)(int)vi;
        volatile double b = get_value(2);
        
        /* Force conversion and comparison */
        if (a < b) {
            global_sum += 18;
        }
        
        if ((double)(int)vi > b) {
            global_sum += 19;
        }
    }
    
    /* Switch statement with FP comparisons */
    {
        volatile double a = get_value(3);
        volatile double b = get_value(4);
        int selector = 0;
        
        /* Determine selector based on comparison */
        if (__builtin_isunordered(a, b)) selector = 1;
        else if (a == b) selector = 2;
        else if (a < b) selector = 3;
        else selector = 4;
        
        switch (selector) {
            case 1:
                global_sum += 20;  /* UNORDERED */
                break;
            case 2:
                global_sum += 21;  /* EQ */
                break;
            case 3:
                global_sum += 22;  /* LT */
                break;
            case 4:
                global_sum += 23;  /* GT */
                break;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double array[] = {1.0, nan_val, 3.0, inf_val, -inf_val, 0.0};
        const int n = sizeof(array) / sizeof(array[0]);
        
        for (int i = 0; i < n; i++) {
            volatile double a = array[i];
            volatile double b = array[(i + 1) % n];
            
            /* Use LCG to select different comparisons */
            uint32_t r = lcg_rand();
            
            switch (r % 8) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum += i;  /* UNORDERED */
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) global_sum += i + 1;  /* ORDERED */
                    break;
                case 2:
                    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) 
                        global_sum += i + 2;  /* UNEQ */
                    break;
                case 3:
                    if (!__builtin_isless(a, b)) global_sum += i + 3;  /* UNGE */
                    break;
                case 4:
                    if (!__builtin_islessequal(a, b)) global_sum += i + 4;  /* UNGT */
                    break;
                case 5:
                    if (!__builtin_isgreater(a, b)) global_sum += i + 5;  /* UNLE */
                    break;
                case 6:
                    if (!__builtin_isgreaterequal(a, b)) global_sum += i + 6;  /* UNLT */
                    break;
                case 7:
                    if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) 
                        global_sum += i + 7;  /* LTGT */
                    break;
            }
            
            /* Clobber FPU status register periodically */
            if (i % 2 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                              "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    /* Final computation to ensure all code is executed */
    printf("Final sum: %d\n", global_sum);
    
    /* Additional forced comparisons in return statement */
    return (__builtin_isunordered(nan_val, normal_val) ? 0 : 1) +
           (__builtin_isordered(zero_val, inf_val) ? 0 : 1);
}
