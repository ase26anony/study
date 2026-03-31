#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double arr[] = {1.0, 2.0, 3.0, __builtin_nan(""), 5.0};
    return arr[idx % 5];
}

/* Pseudo-random generator for varying conditions */
static uint32_t lcg_state = 42;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED (NaN comparisons) */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(0); /* 1.0 */
        
        /* Force UNORDERED condition */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use in ternary */
        global_counter += (a != a) ? 2 : 0; /* NaN != NaN is true */
        
        /* Clobber FP status register */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    }
    
    /* Block 2: ORDERED */
    {
        double x = get_value(1); /* 2.0 */
        double y = get_value(2); /* 3.0 */
        
        if (__builtin_isordered(x, y)) {
            global_sum += 3;
        }
        
        /* Mixed with inline asm */
        asm volatile ("" : : : "cc", "memory");
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        volatile double d1 = nan_val;
        volatile double d2 = nan_val;
        
        /* NaN == NaN is false, but UNEQ includes unordered case */
        if (!(d1 > d2) && !(d1 < d2)) { /* This yields UNEQ for unordered inputs */
            global_sum += 5;
        }
        
        /* Use in switch */
        int cond = (d1 == d2) ? 1 : ((d1 != d2) ? 2 : 3);
        switch (cond) {
            case 1: global_counter += 1; break;
            case 2: global_counter += 2; break;
            default: global_counter += 3;
        }
    }
    
    /* Block 4: UNGE (Not Less Than, includes unordered) */
    {
        double a = get_value(3); /* NaN */
        double b = normal_val;
        
        if (!(a < b)) { /* UNGE: not less than (includes unordered) */
            global_sum += 7;
        }
        
        /* Mixed integer-FP comparison */
        double converted = (double)(int)volatile_int;
        if (!(converted < b)) {
            global_counter += 4;
        }
    }
    
    /* Block 5: UNGT (Not Less Than or Equal, includes unordered) */
    {
        volatile double v1 = nan_val;
        volatile double v2 = zero_val;
        
        if (!(v1 <= v2)) { /* UNGT: not less than or equal */
            global_sum += 11;
        }
    }
    
    /* Block 6: UNLE (Unordered or Less Than or Equal) */
    {
        double x = get_value(3); /* NaN */
        double y = inf_val;
        
        if (__builtin_islessequal(x, y) || __builtin_isunordered(x, y)) {
            global_sum += 13;
        }
    }
    
    /* Block 7: UNLT (Unordered or Less Than) */
    {
        volatile double a = nan_val;
        volatile double b = 100.0;
        
        if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 17;
        }
    }
    
    /* Block 8: LTGT (Less Than or Greater Than, ordered and not equal) */
    {
        double p = get_value(1); /* 2.0 */
        double q = get_value(2); /* 3.0 */
        
        if ((p < q) || (p > q)) { /* LTGT: ordered and not equal */
            global_sum += 19;
        }
        
        /* With inline asm clobber */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
    }
    
    /* Loop with varying conditions */
    {
        double values[8];
        for (int i = 0; i < 8; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = (r >> 0) & 0x7;
            int idx2 = (r >> 3) & 0x7;
            double v1 = values[idx1];
            double v2 = values[idx2];
            
            /* Switch based on hash of iteration */
            switch ((r >> 6) & 0x7) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(v1, v2)) global_counter++;
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(v1, v2)) global_counter++;
                    break;
                case 2: /* UNEQ */
                    if (!(v1 > v2) && !(v1 < v2)) global_counter++;
                    break;
                case 3: /* UNGE */
                    if (!(v1 < v2)) global_counter++;
                    break;
                case 4: /* UNGT */
                    if (!(v1 <= v2)) global_counter++;
                    break;
                case 5: /* UNLE */
                    if (__builtin_islessequal(v1, v2) || __builtin_isunordered(v1, v2)) global_counter++;
                    break;
                case 6: /* UNLT */
                    if (__builtin_isless(v1, v2) || __builtin_isunordered(v1, v2)) global_counter++;
                    break;
                case 7: /* LTGT */
                    if ((v1 < v2) || (v1 > v2)) global_counter++;
                    break;
            }
            
            /* Mixed integer-FP in loop */
            double mixed = (double)(int)(r & 0xFF);
            if (mixed != v1) {
                global_sum += (int)v2;
            }
        }
    }
    
    printf("Result: global_sum = %d, global_counter = %d\n", 
           global_sum, global_counter);
    
    return 0;
}
