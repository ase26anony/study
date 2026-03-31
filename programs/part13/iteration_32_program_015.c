#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double arr[] = {1.0, 2.0, 3.0, __builtin_nan(""), 5.0, -1.0, 0.0};
    return arr[idx % 7];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons */
    {
        double a = get_value(0);
        double b = get_value(3); /* NaN */
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        asm volatile ("" ::: "cc", "memory"); /* Clobber FP status */
        if (a != a || b != b) { /* NaN check */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double x = get_value(1);
        double y = get_value(2);
        
        /* Using __builtin_isordered */
        if (__builtin_isordered(x, y)) {
            global_sum += 4;
        }
        
        /* Ordered check via negation */
        asm volatile ("" ::: "cc", "memory");
        if (!__builtin_isunordered(x, y)) {
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        double p = get_value(4);
        double q = get_value(4); /* Same value */
        
        /* Using ternary operator */
        int result = (p == q || __builtin_isunordered(p, q)) ? 16 : 0;
        global_sum += result;
        
        /* Mixed with integer conversion */
        asm volatile ("" ::: "cc", "memory");
        if ((double)(int)volatile_int == p || __builtin_isunordered((double)(int)volatile_int, p)) {
            global_sum += 32;
        }
    }
    
    /* Block 4: UNGE (unordered or not less than) */
    {
        double u = get_value(5);
        double v = get_value(0);
        
        if (!(u < v) || __builtin_isunordered(u, v)) {
            global_sum += 64;
        }
        
        /* Using __builtin_isgreaterequal */
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isgreaterequal(u, v)) {
            global_sum += 128;
        }
    }
    
    /* Block 5: UNGT (unordered or not less than or equal) */
    {
        double m = get_value(2);
        double n = get_value(1);
        
        if (!(m <= n) || __builtin_isunordered(m, n)) {
            global_sum += 256;
        }
        
        /* Using __builtin_isgreater */
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isgreater(m, n)) {
            global_sum += 512;
        }
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        double r = get_value(6);
        double s = get_value(4);
        
        if ((r <= s) || __builtin_isunordered(r, s)) {
            global_sum += 1024;
        }
        
        /* Using __builtin_islessequal */
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_islessequal(r, s)) {
            global_sum += 2048;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        double d1 = get_value(0);
        double d2 = get_value(5);
        
        if ((d1 < d2) || __builtin_isunordered(d1, d2)) {
            global_sum += 4096;
        }
        
        /* Using __builtin_isless */
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isless(d1, d2)) {
            global_sum += 8192;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, ordered) */
    {
        double a = get_value(1);
        double b = get_value(2);
        
        /* LTGT: ordered and not equal */
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            global_sum += 16384;
        }
        
        /* Alternative formulation */
        asm volatile ("" ::: "cc", "memory");
        if (a != b && !__builtin_isunordered(a, b)) {
            global_sum += 32768;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        volatile double loop_accum = 0.0;
        double values[10];
        
        /* Initialize array with mixed values */
        for (int i = 0; i < 10; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 10;
            int idx2 = (r >> 8) % 10;
            
            /* Switch on condition type */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 1.0;
                    }
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(values[idx1], values[idx2])) {
                        loop_accum += 2.0;
                    }
                    break;
                case 2: /* UNEQ */
                    if (values[idx1] == values[idx2] || 
                        __builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 3.0;
                    }
                    break;
                case 3: /* UNGE */
                    if (!(values[idx1] < values[idx2]) || 
                        __builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 4.0;
                    }
                    break;
                case 4: /* UNGT */
                    if (!(values[idx1] <= values[idx2]) || 
                        __builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 5.0;
                    }
                    break;
                case 5: /* UNLE */
                    if ((values[idx1] <= values[idx2]) || 
                        __builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 6.0;
                    }
                    break;
                case 6: /* UNLT */
                    if ((values[idx1] < values[idx2]) || 
                        __builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 7.0;
                    }
                    break;
                case 7: /* LTGT */
                    if (values[idx1] != values[idx2] && 
                        !__builtin_isunordered(values[idx1], values[idx2])) {
                        loop_accum += 8.0;
                    }
                    break;
            }
            
            /* Insert inline assembly to clobber FP status register */
            asm volatile ("" ::: "cc", "memory");
            
            /* Mixed integer-FP comparison */
            if ((double)(int)volatile_int > values[idx1] || 
                __builtin_isunordered((double)(int)volatile_int, values[idx1])) {
                global_counter++;
            }
        }
        
        /* Use loop result to prevent dead code elimination */
        global_sum += (int)loop_accum;
    }
    
    /* Final output to ensure all code executes */
    printf("Final sum: %d\n", global_sum);
    printf("Counter: %d\n", global_counter);
    
    return 0;
}
