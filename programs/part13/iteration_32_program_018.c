#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, __builtin_nan(""), 5.0};
    return values[idx % 5];
}

/* Pseudo-random generator for varying conditions */
static uint32_t lcg_state = 42;
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
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED (NaN comparisons) */
    {
        volatile double a = get_value(3); /* NaN */
        volatile double b = get_value(0); /* 1.0 */
        
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Ternary operator version */
        global_counter += (a != a) ? 2 : 0; /* NaN != NaN is true */
        
        /* Inline asm to force condition code evaluation */
        asm volatile ("" : : "r"((int)(a != a)) : "cc");
    }
    
    /* Block 2: ORDERED (normal comparisons) */
    {
        volatile double x = get_value(1); /* 2.0 */
        volatile double y = get_value(2); /* 3.0 */
        
        if (__builtin_isordered(x, y)) {
            global_sum += 4;
        }
        
        /* Mixed integer-FP comparison */
        double converted = (double)(int)volatile_int;
        if (converted == converted) { /* Ordered check */
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        volatile double p = get_value(3); /* NaN */
        volatile double q = get_value(3); /* NaN */
        
        /* NaN == NaN is false, but UNEQ handles unordered case */
        if (!(p > q) && !(p < q)) { /* UNEQ: !(a > b) && !(a < b) */
            global_sum += 16;
        }
        
        /* Using builtin for clarity */
        if (!__builtin_isgreater(p, q) && !__builtin_isless(p, q)) {
            global_counter += 32;
        }
    }
    
    /* Block 4: UNGE (unordered or greater-or-equal) */
    {
        volatile double m = get_value(3); /* NaN */
        volatile double n = get_value(0); /* 1.0 */
        
        if (!(m < n)) { /* UNGE: !(a < b) */
            global_sum += 64;
        }
    }
    
    /* Block 5: UNGT (unordered or greater) */
    {
        volatile double u = get_value(3); /* NaN */
        volatile double v = get_value(0); /* 1.0 */
        
        if (!(u <= v)) { /* UNGT: !(a <= b) */
            global_sum += 128;
        }
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        volatile double c = get_value(3); /* NaN */
        volatile double d = get_value(0); /* 1.0 */
        
        if (!(c > d)) { /* UNLE: !(a > b) */
            global_sum += 256;
        }
    }
    
    /* Block 7: UNLT (unordered or less) */
    {
        volatile double e = get_value(3); /* NaN */
        volatile double f = get_value(0); /* 1.0 */
        
        if (!(e >= f)) { /* UNLT: !(a >= b) */
            global_sum += 512;
        }
    }
    
    /* Block 8: LTGT (less or greater, but not equal/unordered) */
    {
        volatile double g = get_value(1); /* 2.0 */
        volatile double h = get_value(2); /* 3.0 */
        
        if ((g < h) || (g > h)) { /* LTGT: (a < b) || (a > b) */
            global_sum += 1024;
        }
        
        /* Alternative using builtins */
        if (__builtin_isless(g, h) || __builtin_isgreater(g, h)) {
            global_counter += 2048;
        }
    }
    
    /* Loop with varying conditions */
    volatile double arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = get_value(i);
    }
    
    for (int i = 0; i < 100; i++) {
        uint32_t r = lcg_rand();
        volatile double a = arr[r % 8];
        volatile double b = arr[(r >> 3) % 8];
        
        /* Switch based on hash of index to trigger different condition codes */
        switch (r % 8) {
            case 0: /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_sum += 1;
                break;
            case 1: /* ORDERED */
                if (__builtin_isordered(a, b)) global_sum += 2;
                break;
            case 2: /* UNEQ */
                if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) 
                    global_sum += 3;
                break;
            case 3: /* UNGE */
                if (!(a < b)) global_sum += 4;
                break;
            case 4: /* UNGT */
                if (!(a <= b)) global_sum += 5;
                break;
            case 5: /* UNLE */
                if (!(a > b)) global_sum += 6;
                break;
            case 6: /* UNLT */
                if (!(a >= b)) global_sum += 7;
                break;
            case 7: /* LTGT */
                if ((a < b) || (a > b)) global_sum += 8;
                break;
        }
        
        /* Clobber FPU status register periodically */
        if (i % 7 == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                         "st(4)", "st(5)", "st(6)", "st(7)", "cc");
        }
    }
    
    /* Mixed integer-FP comparisons in loop */
    for (int i = 0; i < 50; i++) {
        volatile double conv = (double)(int)(volatile_int + i);
        volatile double fp_val = get_value(i % 5);
        
        /* Use ternary operator to force condition code generation */
        int result = (conv < fp_val) ? 1 : 
                    (conv > fp_val) ? 2 : 
                    (conv == fp_val) ? 3 : 4;
        global_counter += result;
        
        /* Complex condition combining ordered/unordered checks */
        if ((conv != conv) || (fp_val != fp_val) || 
            (__builtin_isless(conv, fp_val) && __builtin_isordered(conv, fp_val))) {
            global_sum += 9;
        }
    }
    
    printf("Final sum: %d, counter: %d\n", global_sum, global_counter);
    
    /* Use result to prevent dead code elimination */
    if (global_sum > 1000) {
        return 0;
    } else {
        return 1;
    }
}
