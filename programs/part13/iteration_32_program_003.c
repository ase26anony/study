#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    return (idx % 3 == 0) ? __builtin_nan("") : (double)(idx * 1.5);
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
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - unordered comparison */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        asm volatile ("" ::: "cc");  /* Clobber condition codes */
        
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use in ternary */
        global_counter += (a != a) ? 2 : 0;  /* NaN != NaN is true */
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        volatile double a = normal_val;
        volatile double b = normal_val * 2.0;
        asm volatile ("" ::: "cc");
        
        if (!__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Mixed integer-FP comparison */
        double converted = (double)(int)volatile_int;
        global_counter += (converted == converted) ? 1 : 0;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* (a == b) || (a != a && b != b) */
        if (!(a < b) && !(a > b)) {
            global_sum += 1;
        }
        
        /* Using builtin */
        global_counter += __builtin_isunordered(a, b) ? 1 : 0;
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        asm volatile ("" ::: "cc");
        
        if (!(a < b)) {  /* nlt = not less than */
            global_sum += 1;
        }
        
        /* Alternative formulation */
        global_counter += (a >= b || a != a || b != b) ? 1 : 0;
    }
    
    /* Block 5: UNGT - unordered or greater */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!(a <= b)) {  /* nle = not less-or-equal */
            global_sum += 1;
        }
        
        /* With function call to prevent optimization */
        double fa = get_value(1);
        double fb = get_value(2);
        global_counter += (fa > fb || fa != fa || fb != fb) ? 1 : 0;
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        
        if (a <= b || a != a || b != b) {
            global_sum += 1;
        }
        
        /* Using builtin */
        global_counter += __builtin_islessequal(a, b) ? 1 : 0;
    }
    
    /* Block 7: UNLT - unordered or less */
    {
        volatile double a = normal_val;
        volatile double b = nan_val;
        
        if (a < b || a != a || b != b) {
            global_sum += 1;
        }
        
        /* Mixed comparison */
        double mixed = (double)(int)volatile_int;
        global_counter += (mixed < b || mixed != mixed || b != b) ? 1 : 0;
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    {
        volatile double a = normal_val;
        volatile double b = normal_val * 1.1;
        
        if ((a < b) || (a > b)) {  /* une = not equal and ordered */
            global_sum += 1;
        }
        
        /* Alternative with NaN check */
        global_counter += (a != b && a == a && b == b) ? 1 : 0;
    }
    
    /* Loop with varying conditions */
    volatile double loop_accumulator = 0.0;
    double values[16];
    
    /* Initialize array with mix of NaN and normal values */
    for (int i = 0; i < 16; i++) {
        values[i] = get_value(i);
    }
    
    for (int i = 0; i < 16; i++) {
        uint32_t rand_val = lcg_rand();
        double a = values[i];
        double b = values[(i + 1) % 16];
        
        /* Switch based on pseudo-random value to generate different condition codes */
        switch (rand_val % 8) {
            case 0: /* UNORDERED */
                if (__builtin_isunordered(a, b)) loop_accumulator += 1.0;
                break;
            case 1: /* ORDERED */
                if (!__builtin_isunordered(a, b)) loop_accumulator += 2.0;
                break;
            case 2: /* UNEQ */
                if (!(a < b) && !(a > b)) loop_accumulator += 3.0;
                break;
            case 3: /* UNGE */
                if (!(a < b)) loop_accumulator += 4.0;
                break;
            case 4: /* UNGT */
                if (!(a <= b)) loop_accumulator += 5.0;
                break;
            case 5: /* UNLE */
                if (a <= b || a != a || b != b) loop_accumulator += 6.0;
                break;
            case 6: /* UNLT */
                if (a < b || a != a || b != b) loop_accumulator += 7.0;
                break;
            case 7: /* LTGT */
                if ((a < b) || (a > b)) loop_accumulator += 8.0;
                break;
        }
        
        /* Clobber FP status register periodically */
        if (i % 4 == 0) {
            asm volatile ("" ::: "cc");
        }
    }
    
    /* Use switch statement with floating comparisons */
    volatile int selector = volatile_int % 4;
    switch (selector) {
        case 0:
            if (nan_val == normal_val || nan_val != nan_val) global_sum += 10;
            break;
        case 1:
            if (normal_val > zero_val && normal_val == normal_val) global_sum += 20;
            break;
        case 2:
            if (inf_val >= normal_val || inf_val != inf_val) global_sum += 30;
            break;
        case 3:
            if (!(zero_val < nan_val) && zero_val == zero_val) global_sum += 40;
            break;
    }
    
    /* Final output to ensure all code is executed */
    printf("Result: global_sum = %d, global_counter = %d, loop_accumulator = %f\n",
           global_sum, global_counter, (double)loop_accumulator);
    
    return 0;
}
