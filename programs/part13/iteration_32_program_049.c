#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg = 123456789;
uint32_t rand_lcg() {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main() {
    /* Initialize volatile variables with NaN */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons (NaN involved) */
    {
        asm volatile ("" : : : "st");  /* Clobber FP stack */
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        
        /* Using explicit operators with NaN */
        if (nan_val != nan_val) {  /* Always true for NaN */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons (no NaN) */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        
        /* Ordered comparison using builtin */
        if (!__builtin_isunordered(zero_val, neg_val)) {
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" : : : "st");
        /* Compare NaN with itself - unordered case of UNEQ */
        if (!(nan_val > nan_val) && !(nan_val < nan_val)) {
            global_sum += 16;
        }
        
        /* Using volatile to prevent optimization */
        volatile double a = get_value(0);
        volatile double b = get_value(0);
        if (!(a > b) && !(a < b)) {
            global_sum += 32;
        }
    }
    
    /* Block 4: UNGE (unordered or greater-or-equal) */
    {
        asm volatile ("" : : : "st");
        if (!(normal_val < zero_val)) {
            global_sum += 64;
        }
        
        /* With NaN */
        if (!(nan_val < normal_val)) {
            global_sum += 128;
        }
    }
    
    /* Block 5: UNGT (unordered or greater) */
    {
        asm volatile ("" : : : "st");
        if (!(normal_val <= zero_val)) {
            global_sum += 256;
        }
        
        /* With NaN */
        if (!(nan_val <= normal_val)) {
            global_sum += 512;
        }
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        asm volatile ("" : : : "st");
        if (!(normal_val > zero_val)) {
            global_sum += 1024;
        }
        
        /* Using ternary operator */
        global_sum += (nan_val > normal_val) ? 0 : 2048;
    }
    
    /* Block 7: UNLT (unordered or less) */
    {
        asm volatile ("" : : : "st");
        if (!(normal_val >= zero_val)) {
            global_sum += 4096;
        }
        
        /* Mixed with integer conversion */
        double converted = (double)(int)volatile_int;
        if (!(converted >= normal_val)) {
            global_sum += 8192;
        }
    }
    
    /* Block 8: LTGT (less or greater, but not equal and not unordered) */
    {
        asm volatile ("" : : : "st");
        if (normal_val != zero_val && 
            !__builtin_isunordered(normal_val, zero_val)) {
            global_sum += 16384;
        }
        
        /* Using switch statement */
        switch ((normal_val > zero_val) ? 1 : ((normal_val < zero_val) ? 2 : 0)) {
            case 1:
                global_sum += 32768;
                break;
            case 2:
                global_sum += 65536;
                break;
            default:
                break;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = rand_lcg();
            volatile double a = arr[r % 10];
            volatile double b = arr[(r >> 8) % 10];
            
            /* Select comparison based on hash */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum++;
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
                    if (!(a > b)) global_sum++;
                    break;
                case 6: /* UNLT */
                    if (!(a >= b)) global_sum++;
                    break;
                case 7: /* LTGT */
                    if (a != b && !__builtin_isunordered(a, b)) global_sum++;
                    break;
            }
            
            /* Insert inline assembly to clobber FP status */
            asm volatile ("" : : : "st,st(1),st(2),st(3),st(4),st(5),st(6),st(7)");
            
            /* Mixed integer-FP comparison */
            double mixed = (double)(int)(r % 100);
            if (mixed != a && !__builtin_isunordered(mixed, a)) {
                global_sum += (r & 1);
            }
        }
    }
    
    /* Additional complex comparisons to ensure coverage */
    {
        /* Chain of comparisons */
        volatile double x = get_value(0);
        volatile double y = get_value(1);
        volatile double z = get_value(2);
        
        if ((x < y) && (y < z) && !__builtin_isunordered(x, z)) {
            global_sum += 1;
        }
        
        /* Using signaling NaN if supported */
        #ifdef __FAST_MATH__
        volatile double snan = __builtin_nans("");
        if (__builtin_isunordered(snan, x)) {
            global_sum += 2;
        }
        #endif
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum != 0 ? 0 : 1;
}
