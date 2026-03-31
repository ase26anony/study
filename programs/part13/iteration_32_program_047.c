#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    volatile double values[] = {1.0, 2.0, 3.0, __builtin_nan(""), 4.0, 5.0};
    return values[idx % 6];
}

/* Simple pseudo-random generator for loop conditions */
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
    
    volatile int volatile_int = 7;
    
    /* Force compiler to generate various condition codes */
    
    /* 1. UNORDERED - using __builtin_isunordered */
    if (__builtin_isunordered(nan_val, normal_val)) {
        global_sum += 1;
    }
    
    /* Inline assembly clobber to interfere with optimization */
    asm volatile ("" : : : "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 2. ORDERED - using !__builtin_isunordered */
    if (!__builtin_isunordered(normal_val, zero_val)) {
        global_sum += 2;
    }
    
    /* 3. UNEQ - unordered or equal */
    if (!(nan_val == nan_val)) {  /* NaN != NaN triggers UNEQ */
        global_sum += 3;
    }
    
    /* 4. UNGE - not less than (unordered, greater, or equal) */
    if (!(nan_val < normal_val)) {
        global_sum += 4;
    }
    
    /* 5. UNGT - not less than or equal (unordered or greater) */
    if (!(nan_val <= normal_val)) {
        global_sum += 5;
    }
    
    /* 6. UNLE - unordered or less or equal */
    if (!(nan_val > normal_val)) {
        global_sum += 6;
    }
    
    /* 7. UNLT - unordered or less than */
    if (!(nan_val >= normal_val)) {
        global_sum += 7;
    }
    
    /* 8. LTGT - less than or greater than (ordered and not equal) */
    if (normal_val != zero_val) {
        global_sum += 8;
    }
    
    /* Mixed integer-FP comparisons */
    double converted = (double)(int)volatile_int;
    if (converted > normal_val) {
        global_sum += 9;
    }
    
    /* Ternary operator usage */
    global_sum += (__builtin_isgreater(inf_val, normal_val)) ? 10 : 0;
    global_sum += (__builtin_isless(normal_val, inf_val)) ? 11 : 0;
    
    /* Switch statement with floating comparisons */
    switch (global_counter) {
        case 0:
            if (normal_val == zero_val) global_sum += 12;
            break;
        case 1:
            if (normal_val != zero_val) global_sum += 13;
            break;
        case 2:
            if (normal_val < zero_val) global_sum += 14;
            break;
        case 3:
            if (normal_val > zero_val) global_sum += 15;
            break;
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        uint32_t r = lcg_rand();
        
        /* Different comparison based on pseudo-random value */
        switch (r % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_counter++;
                break;
            case 1:  /* ORDERED */
                if (!__builtin_isunordered(a, b)) global_counter++;
                break;
            case 2:  /* UNEQ */
                if (!(a == b)) global_counter++;
                break;
            case 3:  /* UNGE */
                if (!(a < b)) global_counter++;
                break;
            case 4:  /* UNGT */
                if (!(a <= b)) global_counter++;
                break;
            case 5:  /* UNLE */
                if (!(a > b)) global_counter++;
                break;
            case 6:  /* UNLT */
                if (!(a >= b)) global_counter++;
                break;
            case 7:  /* LTGT */
                if (a != b) global_counter++;
                break;
        }
        
        /* More inline assembly interference */
        if (i % 16 == 0) {
            asm volatile ("" : : : "cc", "st", "st(1)", "st(2)", "st(3)");
        }
    }
    
    /* Complex expression mixing multiple comparisons */
    volatile double x = get_value(0);
    volatile double y = get_value(1);
    volatile double z = get_value(2);
    
    if ((x < y) && !__builtin_isunordered(z, x) && (y != z)) {
        global_sum += 16;
    }
    
    /* Comparisons with signaling NaN if supported */
    #ifdef __SUPPORTS_SNAN__
    volatile double snan_val = __builtin_nans("");
    if (__builtin_isunordered(snan_val, normal_val)) {
        global_sum += 17;
    }
    #endif
    
    printf("Result: %d (counter: %d)\n", global_sum, global_counter);
    
    /* Ensure all code paths are considered */
    return (global_sum > 0) ? 0 : 1;
}
