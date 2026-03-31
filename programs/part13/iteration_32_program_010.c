#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int seed) {
    volatile double v = (double)seed;
    asm volatile ("" : "+m" (v));  /* Prevent optimization */
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
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    
    volatile int int_val = 42;
    
    /* Block 1: UNORDERED - unordered comparison with NaN */
    {
        double a = get_value(1);
        double b = nan_val;
        
        /* Force unordered comparison */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use in ternary */
        global_sum += (a != a) ? 2 : 0;  /* NaN self-comparison */
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        double a = get_value(2);
        double b = normal_val;
        
        /* Ordered comparison */
        if (!__builtin_isunordered(a, b)) {
            global_sum += 4;
        }
        
        /* Mixed with inline assembly clobber */
        asm volatile ("fwait" ::: "cc");
        global_sum += (a == a) ? 8 : 0;  /* Non-NaN self-comparison */
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = get_value(3);
        double b = nan_val;
        
        /* Use explicit operators to generate UNEQ */
        if (!(a < b) && !(a > b)) {  /* UNEQ: !(a < b) && !(a > b) */
            global_sum += 16;
        }
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        double a = get_value(4);
        double b = normal_val;
        
        /* Generate UNGE: !(a < b) */
        if (!(a < b)) {
            global_sum += 32;
        }
    }
    
    /* Block 5: UNGT - unordered or greater */
    {
        double a = get_value(5);
        double b = normal_val;
        
        /* Generate UNGT: !(a <= b) */
        if (!(a <= b)) {
            global_sum += 64;
        }
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        double a = get_value(6);
        double b = normal_val;
        
        /* Generate UNLE: !(a > b) */
        if (!(a > b)) {
            global_sum += 128;
        }
    }
    
    /* Block 7: UNLT - unordered or less */
    {
        double a = get_value(7);
        double b = normal_val;
        
        /* Generate UNLT: !(a >= b) */
        if (!(a >= b)) {
            global_sum += 256;
        }
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    {
        double a = get_value(8);
        double b = normal_val + 1.0;
        
        /* Generate LTGT: (a < b) || (a > b) */
        if ((a < b) || (a > b)) {
            global_sum += 512;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = int_val;
        double a = (double)(int)vi;  /* Conversion path */
        double b = get_value(9);
        
        /* Various comparisons with converted value */
        if (a == b) global_sum += 1024;
        if (a != b) global_sum += 2048;
        if (a < b)  global_sum += 4096;
        if (a > b)  global_sum += 8192;
        
        /* Clobber FP status register */
        asm volatile ("fstsw %%ax" : : : "ax", "cc");
    }
    
    /* Loop with varying conditions */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i + 100);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 16;
            int idx2 = (r >> 8) % 16;
            double a = values[idx1];
            double b = values[idx2];
            
            /* Switch based on hash of iteration */
            switch (r % 8) {
                case 0:
                    /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1:
                    /* ORDERED */
                    if (!__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 2:
                    /* UNEQ - using explicit operators */
                    if (!(a < b) && !(a > b)) global_sum++;
                    break;
                case 3:
                    /* UNGE */
                    if (!(a < b)) global_sum++;
                    break;
                case 4:
                    /* UNGT */
                    if (!(a <= b)) global_sum++;
                    break;
                case 5:
                    /* UNLE */
                    if (!(a > b)) global_sum++;
                    break;
                case 6:
                    /* UNLT */
                    if (!(a >= b)) global_sum++;
                    break;
                case 7:
                    /* LTGT */
                    if ((a < b) || (a > b)) global_sum++;
                    break;
            }
            
            /* Occasionally introduce NaN */
            if ((r % 13) == 0) {
                a = nan_val;
                if (__builtin_isunordered(a, b)) global_sum += 2;
            }
            
            /* Clobber occasionally */
            if ((r % 17) == 0) {
                asm volatile ("fwait" ::: "cc");
            }
        }
    }
    
    /* Final computation to ensure all code is executed */
    {
        volatile double a = get_value(999);
        volatile double b = get_value(1000);
        
        /* Use all condition codes in a complex expression */
        int result = 0;
        result += __builtin_isunordered(a, b) ? 1 : 0;
        result += !__builtin_isunordered(a, b) ? 2 : 0;
        result += (!(a < b) && !(a > b)) ? 4 : 0;
        result += !(a < b) ? 8 : 0;
        result += !(a <= b) ? 16 : 0;
        result += !(a > b) ? 32 : 0;
        result += !(a >= b) ? 64 : 0;
        result += ((a < b) || (a > b)) ? 128 : 0;
        
        global_sum += result;
    }
    
    printf("Result: %d\n", global_sum);
    return 0;
}
