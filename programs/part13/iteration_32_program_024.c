#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double vals[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return vals[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 42;
uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Initialize volatile variables with NaN and normal values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile int volatile_int = 42;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED comparisons */
    {
        double a = nan_val;
        double b = normal1;
        
        /* Using builtin for unordered check */
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        if (a != a) {  /* NaN check */
            sum += 2;
        }
        
        /* Ternary operator with unordered */
        sum += __builtin_isunordered(a, b) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double a = normal1;
        double b = normal2;
        
        if (!__builtin_isunordered(a, b)) {
            sum += 4;
        }
        
        /* Mixed with inline assembly clobber */
        asm volatile ("" : : : "st", "st(1)");
        sum += (a == a && b == b) ? 5 : 0;  /* Both are ordered */
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        double a = nan_val;
        double b = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!(a < b) && !(a > b)) {
            sum += 6;
        }
        
        /* Using volatile to prevent optimization */
        volatile double v1 = a;
        volatile double v2 = b;
        sum += (v1 == v2 || __builtin_isunordered(v1, v2)) ? 7 : 0;
    }
    
    /* Block 4: UNGE (not less than, includes unordered) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a < b)) {  /* UNGE: not less than (nlt) */
            sum += 8;
        }
        
        /* Using builtin with explicit check */
        sum += (!__builtin_isless(a, b)) ? 9 : 0;
    }
    
    /* Block 5: UNGT (not less than or equal, includes unordered) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a <= b)) {  /* UNGT: not less than or equal (nle) */
            sum += 10;
        }
        
        sum += (!__builtin_islessequal(a, b)) ? 11 : 0;
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        double a = normal1;
        double b = nan_val;
        
        if (a <= b || __builtin_isunordered(a, b)) {
            sum += 12;
        }
        
        /* Using switch statement */
        switch ((a <= b || __builtin_isunordered(a, b)) ? 1 : 0) {
            case 1: sum += 13; break;
            default: break;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        double a = normal1;
        double b = nan_val;
        
        if (a < b || __builtin_isunordered(a, b)) {
            sum += 14;
        }
        
        sum += (__builtin_isless(a, b) || __builtin_isunordered(a, b)) ? 15 : 0;
    }
    
    /* Block 8: LTGT (less than or greater than, ordered and not equal) */
    {
        double a = normal1;
        double b = normal2;
        
        if ((a < b) || (a > b)) {  /* LTGT: less than or greater than (une) */
            sum += 16;
        }
        
        /* Ensure not equal and ordered */
        sum += (!__builtin_isunordered(a, b) && a != b) ? 17 : 0;
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(volatile_int % 3);
        
        if (a != b && !__builtin_isunordered(a, b)) {
            sum += 18;
        }
        
        /* Complex expression */
        sum += ((a > b) != (a < b) && !__builtin_isunordered(a, b)) ? 19 : 0;
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        double values[10];
        for (int i = 0; i < 10; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 10;
            int idx2 = (r >> 8) % 10;
            double a = values[idx1];
            double b = values[idx2];
            
            /* Insert NaN occasionally */
            if ((r >> 16) % 7 == 0) {
                a = nan_val;
            }
            if ((r >> 20) % 11 == 0) {
                b = nan_val;
            }
            
            /* Varying comparison types based on random bits */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    sum += __builtin_isunordered(a, b) ? 1 : 0;
                    break;
                case 1:  /* ORDERED */
                    sum += !__builtin_isunordered(a, b) ? 1 : 0;
                    break;
                case 2:  /* UNEQ */
                    sum += (!(a < b) && !(a > b)) ? 1 : 0;
                    break;
                case 3:  /* UNGE */
                    sum += !(a < b) ? 1 : 0;
                    break;
                case 4:  /* UNGT */
                    sum += !(a <= b) ? 1 : 0;
                    break;
                case 5:  /* UNLE */
                    sum += (a <= b || __builtin_isunordered(a, b)) ? 1 : 0;
                    break;
                case 6:  /* UNLT */
                    sum += (a < b || __builtin_isunordered(a, b)) ? 1 : 0;
                    break;
                case 7:  /* LTGT */
                    sum += ((a < b) || (a > b)) ? 1 : 0;
                    break;
            }
            
            /* Clobber FPU registers periodically */
            if (i % 13 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
            }
        }
    }
    
    /* Final mixed comparisons with infinity */
    {
        volatile double a = inf_val;
        volatile double b = -inf_val;
        
        /* These should generate various condition codes */
        sum += (a > b) ? 20 : 0;      /* GT, ordered */
        sum += (a >= b) ? 21 : 0;     /* GE, ordered */
        sum += (a == inf_val) ? 22 : 0; /* EQ, ordered */
        sum += (a != b) ? 23 : 0;     /* NEQ, ordered (LTGT) */
        
        /* Compare NaN with infinity */
        double c = nan_val;
        sum += (c == a) ? 0 : 24;     /* Should be false, UNORDERED */
        sum += (c != a) ? 25 : 0;     /* Should be true, UNORDERED */
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
