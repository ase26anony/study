#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Function to prevent constant folding */
double get_value(int i) {
    volatile double v = (double)i;
    return v;
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg = 123456789;
static uint32_t lcg_rand(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - comparison involving NaN */
    {
        double a = get_value(1);
        double b = nan_val;
        
        /* Use __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        
        /* Also use explicit comparison with NaN */
        asm volatile ("" ::: "cc", "memory");  /* Clobber flags */
        if (a != b) {  /* This will be unordered with NaN */
            sum += 2;
        }
    }
    
    /* Block 2: ORDERED - both operands are not NaN */
    {
        double x = get_value(2);
        double y = normal_val;
        
        /* Use __builtin_isordered */
        if (__builtin_isordered(x, y)) {
            sum += 4;
        }
        
        /* Ordered comparison in ternary */
        sum += (x < y) ? 8 : 0;
        
        asm volatile ("" ::: "cc");  /* Clobber condition codes */
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double p = nan_val;
        double q = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!(p < q) && !(p > q)) {  /* UNEQ condition */
            sum += 16;
        }
        
        /* Using volatile to prevent optimization */
        volatile double v1 = p;
        volatile double v2 = q;
        if (v1 == v2) {  /* Will be UNEQ for NaNs */
            sum += 32;
        }
    }
    
    /* Block 4: UNGE - not less than (unordered, greater, or equal) */
    {
        double a = get_value(4);
        double b = nan_val;
        
        /* a >= b with NaN operand */
        if (!(a < b)) {  /* UNGE: not less than */
            sum += 64;
        }
        
        /* In switch statement */
        switch ((!(a < b)) ? 1 : 0) {
            case 1: sum += 128; break;
            default: break;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    {
        double x = nan_val;
        double y = get_value(5);
        
        /* x > y with NaN operand */
        if (!(x <= y)) {  /* UNGT: not less than or equal */
            sum += 256;
        }
        
        asm volatile ("" ::: "cc", "memory");
    }
    
    /* Block 6: UNLE - unordered or less than or equal */
    {
        double p = get_value(6);
        double q = nan_val;
        
        /* p <= q with NaN operand */
        if (!(p > q)) {  /* UNLE: not greater than */
            sum += 512;
        }
        
        /* Mixed integer-FP comparison */
        double conv = (double)(int)volatile_int;
        if (!(conv > q)) {
            sum += 1024;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        double a = nan_val;
        double b = get_value(7);
        
        /* a < b with NaN operand */
        if (!(a >= b)) {  /* UNLT: not greater than or equal */
            sum += 2048;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        double x = get_value(8);
        double y = get_value(9);
        
        /* x != y with ordered operands */
        if (x != y) {  /* LTGT when ordered */
            sum += 4096;
        }
        
        /* Alternative: (x < y) || (x > y) */
        if (x < y || x > y) {
            sum += 8192;
        }
    }
    
    /* Loop with varying conditions */
    {
        double values[10];
        for (int i = 0; i < 10; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 10;
            int idx2 = (r >> 8) % 10;
            double v1 = values[idx1];
            double v2 = values[idx2];
            
            /* Select condition based on hash */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(v1, v2)) sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(v1, v2)) sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!(v1 < v2) && !(v1 > v2)) sum += 3;
                    break;
                case 3:  /* UNGE */
                    if (!(v1 < v2)) sum += 4;
                    break;
                case 4:  /* UNGT */
                    if (!(v1 <= v2)) sum += 5;
                    break;
                case 5:  /* UNLE */
                    if (!(v1 > v2)) sum += 6;
                    break;
                case 6:  /* UNLT */
                    if (!(v1 >= v2)) sum += 7;
                    break;
                case 7:  /* LTGT */
                    if (v1 != v2) sum += 8;
                    break;
            }
            
            /* Insert inline assembly to clobber FP status */
            asm volatile ("" ::: "cc", "memory");
            
            /* Occasionally use NaN */
            if ((r >> 16) % 5 == 0) {
                volatile double temp = nan_val;
                if (v1 != temp) sum += 9;
            }
        }
    }
    
    /* Mixed comparisons with integer conversion */
    {
        for (int i = 0; i < 20; i++) {
            volatile_int = i;
            double d = (double)(int)volatile_int;
            double n = (i % 3 == 0) ? nan_val : get_value(i);
            
            /* Various comparisons that may generate different condition codes */
            if (d == n) sum += 1;
            if (d != n) sum += 2;
            if (d < n) sum += 3;
            if (d > n) sum += 4;
            if (d <= n) sum += 5;
            if (d >= n) sum += 6;
            
            asm volatile ("" ::: "cc");
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
