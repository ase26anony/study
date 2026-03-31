#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double arr[] = {1.0, 2.0, 3.0, 4.0, __builtin_nan(""), 5.0};
    return arr[idx % 6];
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    /* Volatile variables to prevent optimization */
    volatile double v1, v2, v3, v4;
    volatile int vi1, vi2;
    
    /* Initialize with NaN values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    
    /* Force initialization */
    v1 = 1.5;
    v2 = 2.5;
    v3 = nan_val;
    v4 = 3.5;
    vi1 = 10;
    vi2 = 20;
    
    /* Block 1: UNORDERED - using NaN comparison */
    {
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        if (__builtin_isunordered(v1, v3)) {
            global_sum += 1;
        }
    }
    
    /* Block 2: ORDERED - normal comparison */
    {
        asm volatile ("" : : : "cc", "memory");
        if (__builtin_isordered(v1, v2)) {
            global_sum += 2;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        volatile double a = get_value(0);
        volatile double b = get_value(4); /* NaN */
        if (!(a != b)) {  /* This can generate UNEQ */
            global_sum += 3;
        }
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        volatile double x = v1;
        volatile double y = v3; /* NaN */
        if (__builtin_isgreaterequal(x, y)) {
            global_sum += 4;
        }
    }
    
    /* Block 5: UNGT - unordered or greater */
    {
        volatile double a = v4;
        volatile double b = nan_val;
        if (__builtin_isgreater(a, b)) {
            global_sum += 5;
        }
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        volatile double a = v2;
        volatile double b = nan_val;
        if (__builtin_islessequal(a, b)) {
            global_sum += 6;
        }
    }
    
    /* Block 7: UNLT - unordered or less */
    {
        volatile double a = v1;
        volatile double b = nan_val;
        if (__builtin_isless(a, b)) {
            global_sum += 7;
        }
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    {
        volatile double a = v1;
        volatile double b = v2;
        if (a != b && !__builtin_isunordered(a, b)) {
            global_sum += 8;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int iv = vi1;
        volatile double dv = v1;
        if ((double)iv != dv) {
            global_sum += 9;
        }
    }
    
    /* Ternary operator usage */
    {
        volatile double a = get_value(1);
        volatile double b = get_value(4);
        int result = __builtin_isunordered(a, b) ? 10 : 11;
        global_sum += result;
    }
    
    /* Switch statement with FP comparisons */
    {
        volatile double a = v1;
        volatile double b = v3;
        switch ((__builtin_isunordered(a, b) << 1) | (a > b)) {
            case 0: global_sum += 12; break;
            case 1: global_sum += 13; break;
            case 2: global_sum += 14; break;
            case 3: global_sum += 15; break;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double loop_vals[8];
        for (int i = 0; i < 8; i++) {
            loop_vals[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = loop_vals[r % 8];
            volatile double b = loop_vals[(r >> 3) % 8];
            
            /* Different comparisons based on random bits */
            switch (r % 8) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:
                    if (!(a == b)) global_sum += 3;  /* une */
                    break;
                case 3:
                    if (a >= b) global_sum += 4;  /* nlt */
                    break;
                case 4:
                    if (a > b) global_sum += 5;   /* nle */
                    break;
                case 5:
                    if (a <= b) global_sum += 6;  /* ule */
                    break;
                case 6:
                    if (a < b) global_sum += 7;   /* ult */
                    break;
                case 7:
                    if (a != b && !__builtin_isunordered(a, b)) global_sum += 8;
                    break;
            }
            
            /* Inline assembly clobber */
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                         "st(4)", "st(5)", "st(6)", "st(7)", "cc");
        }
    }
    
    /* Complex expression mixing multiple conditions */
    {
        volatile double x = get_value(0);
        volatile double y = get_value(4);
        volatile double z = get_value(2);
        
        if ((__builtin_isunordered(x, y) || (x > y)) && 
            (__builtin_isordered(z, x) || (z <= y))) {
            global_sum += 100;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}
