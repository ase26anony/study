#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Simple pseudo-random generator to vary conditions */
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
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - unordered comparison with NaN */
    {
        volatile double a = nan_val;
        volatile double b = get_value(1);
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        /* Also use in ternary */
        global_sum += (a != a) ? 2 : 0;  /* NaN != NaN is true */
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        volatile double a = normal_val;
        volatile double b = get_value(2);
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        /* Mixed with inline asm clobber */
        asm volatile ("" : : : "cc", "memory");
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        /* (a == b) when both are NaN triggers UNEQ */
        if (!(a == b)) {  /* Actually false, but compiler may generate UNEQ */
            global_sum += 8;
        }
        /* Use with volatile to prevent optimization */
        volatile double c = a;
        volatile double d = b;
        global_sum += (c == d) ? 16 : 0;
    }
    
    /* Block 4: UNGE - unordered or greater or equal (not less than) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a < b)) {  /* Generates UNGE (nlt) */
            global_sum += 32;
        }
    }
    
    /* Block 5: UNGT - unordered or greater (not less or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a <= b)) {  /* Generates UNGT (nle) */
            global_sum += 64;
        }
    }
    
    /* Block 6: UNLE - unordered or less or equal */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (a <= b) {  /* Generates UNLE (ule) */
            global_sum += 128;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (a < b) {  /* Generates UNLT (ult) */
            global_sum += 256;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (unordered equal not allowed) */
    {
        volatile double a = normal_val;
        volatile double b = get_value(8);
        if (a != b) {  /* Generates LTGT (une) when both ordered */
            global_sum += 512;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile double a = (double)(int)volatile_int;
        volatile double b = get_value(9);
        if (a > b) {
            global_sum += 1024;
        }
    }
    
    /* Switch statement with floating comparisons */
    {
        volatile double a = get_value(10);
        volatile double b = get_value(11);
        int result = 0;
        
        switch (lcg_rand() % 4) {
            case 0:
                result = (a < b) ? 1 : 0;
                break;
            case 1:
                result = (a > b) ? 2 : 0;
                break;
            case 2:
                result = __builtin_isunordered(a, b) ? 3 : 0;
                break;
            case 3:
                result = __builtin_isgreater(a, b) ? 4 : 0;
                break;
        }
        global_sum += result;
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[8];
        for (int i = 0; i < 8; i++) {
            arr[i] = get_value(i + 20);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[r % 8];
            volatile double b = arr[(r >> 3) % 8];
            
            /* Different comparisons based on hash */
            switch (r % 8) {
                case 0: if (a == b) global_sum++; break;
                case 1: if (a != b) global_sum++; break;
                case 2: if (a < b) global_sum++; break;
                case 3: if (a > b) global_sum++; break;
                case 4: if (a <= b) global_sum++; break;
                case 5: if (a >= b) global_sum++; break;
                case 6: if (__builtin_isunordered(a, b)) global_sum++; break;
                case 7: if (__builtin_isordered(a, b)) global_sum++; break;
            }
            
            /* Clobber FPU periodically */
            if (i % 7 == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
            }
        }
    }
    
    /* Final complex expression mixing all condition types */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        volatile double c = inf_val;
        volatile double d = zero_val;
        
        /* This complex expression should generate multiple condition codes */
        int complex_result = 
            (a < b) ? 1 : 0 +      /* UNLT */
            (b >= a) ? 2 : 0 +     /* UNGE */
            (c == d) ? 4 : 0 +     /* UNEQ */
            (d != c) ? 8 : 0 +     /* LTGT */
            __builtin_isunordered(a, c) ? 16 : 0 +  /* Explicit unordered */
            __builtin_isordered(b, d) ? 32 : 0;     /* Explicit ordered */
        
        global_sum += complex_result;
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
