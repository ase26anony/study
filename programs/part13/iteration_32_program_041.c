#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to prevent constant folding */
volatile double get_value(void) {
    static volatile double counter = 0.0;
    counter += 0.1;
    return counter;
}

int main(void) {
    /* Initialize volatile variables with NaN and normal values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED (NaN comparisons) */
    {
        asm volatile ("" ::: "cc");  /* Clobber condition codes */
        if (__builtin_isunordered(nan_val, normal_val)) {
            sum += 1;
        }
        /* Alternative using explicit operators */
        if (nan_val != nan_val) {  /* NaN != NaN is true */
            sum += 2;
        }
    }
    
    /* Block 2: ORDERED (normal comparisons) */
    {
        asm volatile ("" ::: "cc");
        if (__builtin_isgreater(normal_val, zero_val)) {
            sum += 4;
        }
        if (!__builtin_isunordered(normal_val, get_value())) {
            sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        volatile double a = get_value();
        volatile double b = get_value();
        asm volatile ("" ::: "cc");
        /* Using ternary to force condition code generation */
        sum += (a == b || __builtin_isunordered(a, b)) ? 16 : 0;
    }
    
    /* Block 4: UNGE (not less than: !(a < b) including unordered) */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        asm volatile ("" ::: "cc");
        if (!__builtin_isless(x, y)) {  /* UNGE: nlt */
            sum += 32;
        }
    }
    
    /* Block 5: UNGT (not less than or equal: !(a <= b) including unordered) */
    {
        volatile double p = get_value();
        volatile double q = get_value();
        asm volatile ("" ::: "cc");
        sum += (!__builtin_islessequal(p, q)) ? 64 : 0;
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        volatile double m = nan_val;
        volatile double n = normal_val;
        asm volatile ("" ::: "cc");
        if (__builtin_islessequal(m, n) || __builtin_isunordered(m, n)) {
            sum += 128;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        volatile double u = nan_val;
        volatile double v = normal_val;
        asm volatile ("" ::: "cc");
        if (__builtin_isless(u, v) || __builtin_isunordered(u, v)) {
            sum += 256;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        volatile double r = get_value();
        volatile double s = get_value() + 1.0;
        asm volatile ("" ::: "cc");
        /* LTGT: (a < b) || (a > b) but not unordered */
        if ((r < s || r > s) && !__builtin_isunordered(r, s)) {
            sum += 512;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = volatile_int;
        volatile double d = get_value();
        asm volatile ("" ::: "cc");
        
        /* Force conversion path */
        if ((double)vi < d) {
            sum += 1024;
        }
        if (d > (double)(vi + 1)) {
            sum += 2048;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double array[8];
        for (int i = 0; i < 8; i++) {
            array[i] = get_value() + i;
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = array[r % 8];
            volatile double b = array[(r >> 3) % 8];
            
            /* Switch based on random bits to generate different condition codes */
            switch (r & 0x7) {
                case 0:
                    if (__builtin_isunordered(a, b)) sum += 1;  /* UNORDERED */
                    break;
                case 1:
                    if (!__builtin_isunordered(a, b)) sum += 2; /* ORDERED */
                    break;
                case 2:
                    if (a == b || __builtin_isunordered(a, b)) sum += 3; /* UNEQ */
                    break;
                case 3:
                    if (!__builtin_isless(a, b)) sum += 4;      /* UNGE */
                    break;
                case 4:
                    if (!__builtin_islessequal(a, b)) sum += 5; /* UNGT */
                    break;
                case 5:
                    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) 
                        sum += 6;                               /* UNLE */
                    break;
                case 6:
                    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) 
                        sum += 7;                               /* UNLT */
                    break;
                case 7:
                    if ((a < b || a > b) && !__builtin_isunordered(a, b)) 
                        sum += 8;                               /* LTGT */
                    break;
            }
            
            /* Additional inline assembly to clobber FP status */
            asm volatile ("fwait" ::: "cc");
        }
    }
    
    /* Complex nested comparisons */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        volatile double z = inf_val;
        
        /* This should generate multiple condition code checks */
        if ((x < y && y < z) || (x > y && y > z)) {
            sum += 4096;
        }
        
        /* Chained comparisons */
        sum += (x != y) ? 8192 : 0;
        sum += (y >= zero_val) ? 16384 : 0;
        sum += (z <= inf_val) ? 32768 : 0;
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
