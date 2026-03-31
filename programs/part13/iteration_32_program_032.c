#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Function to prevent constant folding */
double get_value(int seed) {
    volatile double v = 3.14159 * seed;
    return v;
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 1;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 42.0;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 7;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 1. UNORDERED - comparison with NaN */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        /* Also use in ternary */
        sum += (a != a) ? 2 : 0;  /* NaN != NaN is true */
    }
    
    /* 2. ORDERED - both operands are not NaN */
    {
        volatile double a = normal_val;
        volatile double b = get_value(1);
        if (__builtin_isordered(a, b)) {
            sum += 4;
        }
        /* Force through inline asm */
        asm volatile ("" : : "r"(a), "r"(b) : "memory");
    }
    
    /* 3. UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        /* Using explicit operators to trigger UNEQ */
        if (!(a < b) && !(a > b)) {  /* Both false for NaN */
            sum += 8;
        }
    }
    
    /* 4. UNGE - unordered or greater-or-equal */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a < b)) {  /* nlt = not less than = UNGE */
            sum += 16;
        }
    }
    
    /* 5. UNGT - unordered or greater-than */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        if (!(a <= b)) {  /* nle = not less-or-equal = UNGT */
            sum += 32;
        }
    }
    
    /* 6. UNLE - unordered or less-or-equal */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (a <= b || __builtin_isunordered(a, b)) {
            sum += 64;
        }
    }
    
    /* 7. UNLT - unordered or less-than */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        if (a < b || __builtin_isunordered(a, b)) {
            sum += 128;
        }
    }
    
    /* 8. LTGT - less-than or greater-than (ordered and not equal) */
    {
        volatile double a = normal_val;
        volatile double b = get_value(2);
        if (__builtin_islessgreater(a, b)) {
            sum += 256;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile double a = (double)(int)volatile_int;
        volatile double b = get_value(3);
        if (a > b) {
            sum += 512;
        }
        if (a == b) {
            sum += 1024;
        }
    }
    
    /* Loop with varying conditions */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = values[r % 16];
            double b = values[(r >> 4) % 16];
            
            /* Switch based on random bits to generate different condition codes */
            switch (r & 0x7) {
                case 0:
                    if (__builtin_isunordered(a, b)) sum += 1;
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) sum += 2;
                    break;
                case 2:
                    if (!(a < b)) sum += 3;  /* UNGE */
                    break;
                case 3:
                    if (!(a <= b)) sum += 4; /* UNGT */
                    break;
                case 4:
                    if (a <= b || __builtin_isunordered(a, b)) sum += 5;
                    break;
                case 5:
                    if (a < b || __builtin_isunordered(a, b)) sum += 6;
                    break;
                case 6:
                    if (__builtin_islessgreater(a, b)) sum += 7;
                    break;
                case 7:
                    /* Mixed comparison with integer conversion */
                    if ((double)(int)volatile_int > b) sum += 8;
                    break;
            }
            
            /* Clobber FPU status register periodically */
            if ((i & 15) == 0) {
                asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                              "st(4)", "st(5)", "st(6)", "st(7)");
            }
        }
    }
    
    /* Complex nested comparisons */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        volatile double c = inf_val;
        volatile double d = zero_val;
        
        /* Generate multiple condition codes in one expression */
        int result = (a < b) ? 1 : 
                    (__builtin_isunordered(c, d) ? 2 : 
                    (!(b >= c) ? 3 : 
                    (__builtin_islessgreater(d, a) ? 4 : 0)));
        sum += result;
        
        /* Another complex expression */
        sum += ((a == b) && __builtin_isordered(a, b)) ? 16 : 32;
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
