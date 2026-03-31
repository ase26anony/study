#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int side_effect = 0;

/* Function to create side effects and prevent constant folding */
double get_value(int idx) {
    side_effect += idx;
    return (double)(side_effect % 100) / 10.0;
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 42;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    
    volatile int volatile_int = 42;
    
    /* Force compiler to keep all variables alive */
    asm volatile("" : : "r"(nan_val), "r"(inf_val), "r"(normal_val), "r"(zero_val));
    
    /* Block 1: UNORDERED comparisons */
    {
        double a = get_value(1);
        double b = nan_val;
        
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Using explicit comparison with NaN */
        if (a != a) {  /* Always false for non-NaN, but compiler might generate unord */
            global_sum += 2;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(b, normal_val) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double a = get_value(2);
        double b = get_value(3);
        
        /* Using __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        
        /* Ordered check via negation */
        if (!__builtin_isunordered(a, b)) {
            global_sum += 5;
        }
        
        /* Switch statement with ordered condition */
        switch ((int)__builtin_isordered(a, b)) {
            case 1: global_sum += 6; break;
            default: break;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        double a = get_value(4);
        double b = a;  /* Same value */
        
        /* This should generate UNEQ */
        if (!(a > b) && !(a < b)) {
            global_sum += 7;
        }
        
        /* With NaN */
        if (!(nan_val > normal_val) && !(nan_val < normal_val)) {
            global_sum += 8;
        }
    }
    
    /* Block 4: UNGE (not less than, includes unordered) */
    {
        double a = get_value(5);
        double b = get_value(6);
        
        /* Using __builtin_isgreaterequal */
        if (__builtin_isgreaterequal(a, b)) {
            global_sum += 9;
        }
        
        /* Explicit: !(a < b) */
        if (!(a < b)) {
            global_sum += 10;
        }
    }
    
    /* Block 5: UNGT (not less than or equal, includes unordered) */
    {
        double a = get_value(7);
        double b = get_value(8);
        
        /* Using __builtin_isgreater */
        if (__builtin_isgreater(a, b)) {
            global_sum += 11;
        }
        
        /* Explicit: !(a <= b) */
        if (!(a <= b)) {
            global_sum += 12;
        }
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        double a = get_value(9);
        double b = get_value(10);
        
        /* Using __builtin_islessequal */
        if (__builtin_islessequal(a, b)) {
            global_sum += 13;
        }
        
        /* With NaN */
        if (__builtin_islessequal(nan_val, b)) {
            global_sum += 14;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        double a = get_value(11);
        double b = get_value(12);
        
        /* Using __builtin_isless */
        if (__builtin_isless(a, b)) {
            global_sum += 15;
        }
        
        /* With NaN */
        if (__builtin_isless(nan_val, b)) {
            global_sum += 16;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, ordered and not equal) */
    {
        double a = get_value(13);
        double b = get_value(14);
        
        /* LTGT: (a < b) || (a > b) but both ordered */
        if ((a < b) || (a > b)) {
            global_sum += 17;
        }
        
        /* Alternative formulation */
        if (a != b && __builtin_isordered(a, b)) {
            global_sum += 18;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(15);
        
        if (a > b) global_sum += 19;
        if (a < b) global_sum += 20;
        if (a == b) global_sum += 21;
        if (a != b) global_sum += 22;
    }
    
    /* Loop with varying conditions */
    {
        double values[20];
        for (int i = 0; i < 20; i++) {
            values[i] = get_value(i + 100);
        }
        
        for (int i = 0; i < 20; i++) {
            uint32_t r = lcg_rand();
            double a = values[i];
            double b = values[(i + 1) % 20];
            
            /* Use hash of index to select different comparisons */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, nan_val)) global_sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!(a > b) && !(a < b)) global_sum += 3;
                    break;
                case 3:  /* UNGE */
                    if (__builtin_isgreaterequal(a, b)) global_sum += 4;
                    break;
                case 4:  /* UNGT */
                    if (__builtin_isgreater(a, b)) global_sum += 5;
                    break;
                case 5:  /* UNLE */
                    if (__builtin_islessequal(a, b)) global_sum += 6;
                    break;
                case 6:  /* UNLT */
                    if (__builtin_isless(a, b)) global_sum += 7;
                    break;
                case 7:  /* LTGT */
                    if ((a < b) || (a > b)) global_sum += 8;
                    break;
            }
            
            /* Inline assembly to clobber FP status register */
            asm volatile("fwait" : : : "cc");
        }
    }
    
    /* Final computation to ensure all code is executed */
    {
        volatile double a = get_value(999);
        volatile double b = get_value(1000);
        
        /* Complex nested conditionals */
        int result = (a < b) ? 
                    ((__builtin_isunordered(a, b)) ? 1 : 2) :
                    ((a > b) ? 3 : 4);
        
        global_sum += result;
        
        /* Another switch with FP comparisons */
        switch ((a > b) + (a < b) * 2 + (a == b) * 4) {
            case 1: global_sum += 10; break;  /* a > b */
            case 2: global_sum += 20; break;  /* a < b */
            case 4: global_sum += 30; break;  /* a == b */
            default: global_sum += 40; break; /* unordered or multiple true */
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    
    /* Use the result to prevent dead code elimination */
    if (global_sum > 1000) {
        printf("Unexpectedly large sum\n");
    }
    
    return global_sum > 0 ? 0 : 1;
}
