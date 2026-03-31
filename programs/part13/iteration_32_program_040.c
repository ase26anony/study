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
        asm volatile ("" ::: "cc", "memory");
        if (a != a || b != b) {
            global_counter++;
        }
        
        /* Ternary operator with unordered */
        global_sum += (__builtin_isunordered(a, nan_val) ? 2 : 0);
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double x = get_value(1);
        double y = get_value(2);
        
        if (!__builtin_isunordered(x, y)) {
            global_sum += 4;
        }
        
        /* Inline assembly clobber to force re-evaluation */
        asm volatile ("" ::: "cc");
        global_counter += (x == x && y == y) ? 1 : 0;
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        double p = get_value(4);
        double q = get_value(4); /* Same value */
        
        /* This should generate UNEQ */
        if (!(p > q) && !(p < q)) {
            global_sum += 8;
        }
        
        /* With NaN */
        if (!(nan_val > zero_val) && !(nan_val < zero_val)) {
            global_counter += 2;
        }
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        double a = get_value(0);
        double b = get_value(5); /* -1.0 */
        
        /* Using __builtin_isgreaterequal */
        if (__builtin_isgreaterequal(a, b)) {
            global_sum += 16;
        }
        
        /* Explicit with NaN */
        if (!(a < b)) {
            global_counter += 4;
        }
    }
    
    /* Block 5: UNGT (Unordered or Greater) */
    {
        double x = normal_val;
        double y = neg_val;
        
        if (__builtin_isgreater(x, y)) {
            global_sum += 32;
        }
        
        /* Mixed integer-FP comparison */
        double conv_val = (double)(int)volatile_int;
        if (conv_val > y) {
            global_counter += 8;
        }
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        double p = get_value(5); /* -1.0 */
        double q = get_value(0); /* 1.0 */
        
        if (__builtin_islessequal(p, q)) {
            global_sum += 64;
        }
        
        /* With NaN operand */
        if (!(nan_val > q)) {
            global_counter += 16;
        }
    }
    
    /* Block 7: UNLT (Unordered or Less) */
    {
        double a = neg_val;
        double b = normal_val;
        
        if (__builtin_isless(a, b)) {
            global_sum += 128;
        }
        
        /* Using explicit operator */
        asm volatile ("" ::: "cc", "memory");
        if (a < b) {
            global_counter += 32;
        }
    }
    
    /* Block 8: LTGT (Less or Greater, but not Equal and not Unordered) */
    {
        double x = get_value(0);
        double y = get_value(1);
        
        /* This is tricky - need ordered and not equal */
        if ((x < y) || (x > y)) {
            global_sum += 256;
        }
        
        /* Alternative formulation */
        if (x != y && !__builtin_isunordered(x, y)) {
            global_counter += 64;
        }
    }
    
    /* Switch statement with floating comparisons */
    {
        volatile int selector = volatile_int % 4;
        double val1 = get_value(selector);
        double val2 = get_value(selector + 1);
        
        switch (selector) {
            case 0:
                if (val1 < val2) global_sum += 512;
                break;
            case 1:
                if (val1 > val2) global_sum += 1024;
                break;
            case 2:
                if (val1 == val2) global_sum += 2048;
                break;
            case 3:
                if (val1 != val2) global_sum += 4096;
                break;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        double loop_vals[20];
        for (int i = 0; i < 20; i++) {
            loop_vals[i] = get_value(i);
        }
        
        for (int i = 0; i < 20; i++) {
            uint32_t rand_val = lcg_rand();
            double a = loop_vals[i];
            double b = loop_vals[(i + 1) % 20];
            
            /* Select condition based on hash of index */
            switch (rand_val % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1: /* ORDERED */
                    if (!__builtin_isunordered(a, b)) global_sum += 2;
                    break;
                case 2: /* UNEQ */
                    if (!(a > b) && !(a < b)) global_sum += 3;
                    break;
                case 3: /* UNGE */
                    if (!(a < b)) global_sum += 4;
                    break;
                case 4: /* UNGT */
                    if (a > b) global_sum += 5;
                    break;
                case 5: /* UNLE */
                    if (!(a > b)) global_sum += 6;
                    break;
                case 6: /* UNLT */
                    if (a < b) global_sum += 7;
                    break;
                case 7: /* LTGT */
                    if ((a < b) || (a > b)) global_sum += 8;
                    break;
            }
            
            /* Inline assembly clobber every few iterations */
            if (i % 5 == 0) {
                asm volatile ("" ::: "cc", "memory");
            }
        }
    }
    
    /* Mixed comparisons in complex expressions */
    {
        volatile double v1 = get_value(0);
        volatile double v2 = get_value(3); /* NaN */
        volatile double v3 = get_value(6); /* 0.0 */
        
        /* Complex conditional expression */
        int result = (v1 < v3) ? 1 : 
                    (__builtin_isunordered(v2, v3) ? 2 : 
                    (v1 > (double)(int)volatile_int ? 3 : 4));
        
        global_sum += result;
        
        /* Nested comparisons */
        if ((v1 < v3) && !__builtin_isunordered(v1, v3)) {
            global_counter += 128;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    printf("Final counter: %d\n", global_counter);
    
    return 0;
}
