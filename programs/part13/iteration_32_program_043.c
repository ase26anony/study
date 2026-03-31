#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int seed) {
    volatile double v = (double)seed;
    return v;
}

/* Simple pseudo-random generator for loop variations */
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
    
    /* Force compiler to keep all variables alive */
    asm volatile("" : : "r"(nan_val), "r"(inf_val), "r"(normal_val), 
                   "r"(zero_val), "r"(neg_val), "r"(int_val));
    
    /* 1. UNORDERED - Compare NaN with anything */
    {
        double a = get_value(1);
        double b = nan_val;
        
        /* Use __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use explicit unordered check */
        asm volatile("" ::: "cc", "memory");
        if (a != a || b != b) {  /* NaN check */
            global_sum += 2;
        }
    }
    
    /* 2. ORDERED - Both operands are not NaN */
    {
        double x = get_value(2);
        double y = get_value(3);
        
        /* Use __builtin_isordered */
        if (__builtin_isordered(x, y)) {
            global_sum += 4;
        }
        
        /* Ternary operator with ordered check */
        int result = __builtin_isordered(x, y) ? 8 : 0;
        global_sum += result;
    }
    
    /* 3. UNEQ - Unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Two NaNs are unordered but also "equal" in the UNEQ sense */
        if (!(a < b) && !(a > b)) {  /* UNEQ condition */
            global_sum += 16;
        }
        
        /* Mix with integer conversion */
        double c = (double)(int)int_val;
        if (!(c < c) && !(c > c)) {
            global_sum += 32;
        }
    }
    
    /* 4. UNGE - Not less than (unordered or greater or equal) */
    {
        double p = get_value(4);
        double q = get_value(5);
        
        /* Use !(p < q) which generates UNGE */
        if (!(p < q)) {
            global_sum += 64;
        }
        
        /* With NaN */
        if (!(nan_val < p)) {
            global_sum += 128;
        }
    }
    
    /* 5. UNGT - Not less than or equal (unordered or greater) */
    {
        double m = get_value(6);
        double n = get_value(7);
        
        /* Use !(m <= n) which generates UNGT */
        if (!(m <= n)) {
            global_sum += 256;
        }
        
        /* With explicit __builtin_isgreater */
        if (__builtin_isgreater(m, n)) {
            global_sum += 512;
        }
    }
    
    /* 6. UNLE - Unordered or less or equal */
    {
        double u = get_value(8);
        double v = get_value(9);
        
        /* Use !(u > v) which generates UNLE */
        if (!(u > v)) {
            global_sum += 1024;
        }
        
        /* With __builtin_islessequal */
        if (__builtin_islessequal(u, v)) {
            global_sum += 2048;
        }
    }
    
    /* 7. UNLT - Unordered or less than */
    {
        double r = get_value(10);
        double s = get_value(11);
        
        /* Use !(r >= s) which generates UNLT */
        if (!(r >= s)) {
            global_sum += 4096;
        }
        
        /* With __builtin_isless */
        if (__builtin_isless(r, s)) {
            global_sum += 8192;
        }
    }
    
    /* 8. LTGT - Less than or greater than (ordered and not equal) */
    {
        double d1 = get_value(12);
        double d2 = get_value(13);
        
        /* (d1 < d2) || (d1 > d2) generates LTGT for ordered values */
        if ((d1 < d2) || (d1 > d2)) {
            global_sum += 16384;
        }
        
        /* Alternative formulation */
        if (d1 != d2 && __builtin_isordered(d1, d2)) {
            global_sum += 32768;
        }
    }
    
    /* Loop with varying conditions */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i + 100);
        }
        
        for (int i = 0; i < 16; i++) {
            uint32_t r = lcg_rand();
            double a = values[i];
            double b = values[(i + 1) % 16];
            
            /* Switch based on random bits to generate different condition codes */
            switch (r & 0x7) {
                case 0:
                    /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:
                    /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:
                    /* UNGE - !(a < b) */
                    if (!(a < b)) global_sum += 4;
                    break;
                case 3:
                    /* UNGT - !(a <= b) */
                    if (!(a <= b)) global_sum += 8;
                    break;
                case 4:
                    /* UNLE - !(a > b) */
                    if (!(a > b)) global_sum += 16;
                    break;
                case 5:
                    /* UNLT - !(a >= b) */
                    if (!(a >= b)) global_sum += 32;
                    break;
                case 6:
                    /* LTGT - ordered and not equal */
                    if (a != b && __builtin_isordered(a, b)) global_sum += 64;
                    break;
                case 7:
                    /* UNEQ - !(a < b) && !(a > b) */
                    if (!(a < b) && !(a > b)) global_sum += 128;
                    break;
            }
            
            /* Inline assembly to clobber FP status register */
            asm volatile("" ::: "cc", "memory");
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = 100;
        for (int i = 0; i < 10; i++) {
            double d = (double)(int)vi;
            double f = get_value(i + 50);
            
            /* Various comparisons with converted integer */
            if (d < f) global_sum += 1;
            if (d > f) global_sum += 2;
            if (d <= f) global_sum += 4;
            if (d >= f) global_sum += 8;
            if (d == f) global_sum += 16;
            if (d != f) global_sum += 32;
            
            vi++;
            asm volatile("" ::: "cc", "memory");
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    
    /* Use the result to prevent dead code elimination */
    if (global_sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
