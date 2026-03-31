#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double arr[] = {1.0, 2.0, 3.0, __builtin_nan(""), 5.0};
    return arr[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
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
    
    /* Block 1: UNORDERED - unordered comparison with NaN */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(0); /* 1.0 */
        asm volatile ("" ::: "cc", "memory"); /* Clobber flags */
        
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use in ternary */
        global_sum += (a != a) ? 2 : 0; /* NaN != NaN is true */
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        volatile double x = normal_val;
        volatile double y = zero_val;
        asm volatile ("" ::: "cc", "memory");
        
        if (!__builtin_isunordered(x, y)) {
            global_sum += 4;
        }
        
        /* Mixed integer-FP comparison */
        double z = (double)(int)volatile_int;
        if (z == z) { /* Ordered check */
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = get_value(3); /* NaN */
        double b = get_value(3); /* NaN */
        
        /* (a == b) || unordered(a, b) */
        if (!(a > b) && !(a < b)) {
            global_sum += 16;
        }
    }
    
    /* Block 4: UNGE - not less than (greater or equal or unordered) */
    {
        volatile double p = normal_val;
        volatile double q = zero_val;
        
        /* !(p < q) */
        if (!(p < q)) {
            global_sum += 32;
        }
        
        /* With NaN */
        double nan = nan_val;
        if (!(nan < p)) { /* UNGE because NaN < anything is false */
            global_sum += 64;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (greater or unordered) */
    {
        volatile double m = 10.0;
        volatile double n = 5.0;
        
        /* !(m <= n) */
        if (!(m <= n)) {
            global_sum += 128;
        }
        
        /* With NaN operand */
        if (!(nan_val <= m)) {
            global_sum += 256;
        }
    }
    
    /* Block 6: UNLE - unordered or less or equal */
    {
        volatile double r = 1.0;
        volatile double s = 2.0;
        
        /* Using builtin for less or equal with potential unordered */
        if (__builtin_islessequal(r, s)) {
            global_sum += 512;
        }
        
        /* Explicit with NaN */
        if (nan_val <= nan_val) { /* false, but generates UNLE condition */
            global_sum += 0; /* Never taken, but condition checked */
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        volatile double u = 1.0;
        volatile double v = 2.0;
        
        if (__builtin_isless(u, v)) {
            global_sum += 1024;
        }
        
        /* Switch statement with FP comparison */
        switch (volatile_int) {
            case 42:
                if (u < v) global_sum += 2048;
                break;
            default:
                break;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        volatile double c = 7.0;
        volatile double d = 8.0;
        
        /* (c < d) || (c > d) but not equal and ordered */
        if (c != d) {
            global_sum += 4096;
        }
        
        /* More explicit */
        if (__builtin_isless(c, d) || __builtin_isgreater(c, d)) {
            global_sum += 8192;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        volatile double loop_vals[8];
        for (int i = 0; i < 8; i++) {
            loop_vals[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = loop_vals[r % 8];
            double b = loop_vals[(r >> 3) % 8];
            
            /* Select condition based on hash */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1: /* ORDERED */
                    if (!__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 2: /* UNEQ */
                    if (!(a > b) && !(a < b)) global_sum++;
                    break;
                case 3: /* UNGE */
                    if (!(a < b)) global_sum++;
                    break;
                case 4: /* UNGT */
                    if (!(a <= b)) global_sum++;
                    break;
                case 5: /* UNLE */
                    if (__builtin_islessequal(a, b)) global_sum++;
                    break;
                case 6: /* UNLT */
                    if (__builtin_isless(a, b)) global_sum++;
                    break;
                case 7: /* LTGT */
                    if (a != b) global_sum++;
                    break;
            }
            
            /* Clobber FP status register periodically */
            if (i % 7 == 0) {
                asm volatile ("" ::: "st", "st(1)", "st(2)", "st(3)", 
                              "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    /* Mixed integer-FP in loop */
    {
        for (int i = 0; i < 50; i++) {
            volatile int vi = volatile_int + i;
            double converted = (double)(int)vi;
            
            /* This may generate interesting condition codes */
            if (converted != normal_val) {
                global_sum += i;
            }
            
            /* Compare with NaN */
            if (converted == nan_val) {
                global_sum -= i; /* Never true, but condition evaluated */
            }
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    
    /* Use result to prevent dead code elimination */
    if (global_sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
