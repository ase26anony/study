#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
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
    volatile int volatile_int = 42;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 1. UNORDERED - Compare NaN with normal value */
    if (__builtin_isunordered(nan_val, normal_val)) {
        global_sum += 1;
    }
    
    /* 2. ORDERED - Compare two normal values */
    if (__builtin_isordered(normal_val, get_value(1))) {
        global_sum += 2;
    }
    
    /* 3. UNEQ - NaN != NaN? Actually UNEQ is "unordered or equal" */
    /* Use ternary operator to force condition code generation */
    global_sum += (nan_val == nan_val) ? 3 : 0;  /* This will be false for NaN */
    
    /* 4. UNGE - "not less than" (unordered or greater or equal) */
    if (!__builtin_isless(nan_val, normal_val)) {
        global_sum += 4;
    }
    
    /* 5. UNGT - "not less than or equal" (unordered or greater) */
    if (!__builtin_islessequal(nan_val, normal_val)) {
        global_sum += 5;
    }
    
    /* 6. UNLE - "unordered or less or equal" */
    if (__builtin_islessequal(nan_val, normal_val) || __builtin_isunordered(nan_val, normal_val)) {
        global_sum += 6;
    }
    
    /* 7. UNLT - "unordered or less than" */
    if (__builtin_isless(nan_val, normal_val) || __builtin_isunordered(nan_val, normal_val)) {
        global_sum += 7;
    }
    
    /* 8. LTGT - "less than or greater than" (ordered and not equal) */
    if (__builtin_isless(normal_val, get_value(2)) || __builtin_isgreater(normal_val, get_value(2))) {
        global_sum += 8;
    }
    
    /* Mixed integer-FP comparisons */
    double converted = (double)(int)volatile_int;
    if (converted < get_value(3)) {
        global_sum += 9;
    }
    
    /* Switch statement with FP comparisons */
    int selector = volatile_int % 4;
    switch (selector) {
        case 0:
            if (normal_val == zero_val) global_sum += 10;
            break;
        case 1:
            if (normal_val != zero_val) global_sum += 11;
            break;
        case 2:
            if (normal_val < zero_val) global_sum += 12;
            break;
        case 3:
            if (normal_val > zero_val) global_sum += 13;
            break;
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        uint32_t r = lcg_rand();
        
        /* Clobber FPU status register periodically */
        if (i % 7 == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
        }
        
        /* Different comparisons based on pseudo-random value */
        switch (r % 8) {
            case 0:
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1:
                if (__builtin_isordered(a, b)) global_sum += 2;
                break;
            case 2:
                if (!__builtin_isless(a, b)) global_sum += 3;  /* UNGE */
                break;
            case 3:
                if (!__builtin_islessequal(a, b)) global_sum += 4;  /* UNGT */
                break;
            case 4:
                if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) 
                    global_sum += 5;  /* UNLE */
                break;
            case 5:
                if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) 
                    global_sum += 6;  /* UNLT */
                break;
            case 6:
                if (a == b) global_sum += 7;  /* EQ (but may generate UNEQ in some contexts) */
                break;
            case 7:
                if (a != b) global_sum += 8;  /* NEQ (but may generate LTGT in some contexts) */
                break;
        }
        
        /* Mixed comparison with NaN */
        if (i % 3 == 0) {
            if (__builtin_isgreater(a, nan_val)) global_sum--;
        }
    }
    
    /* Additional complex expressions */
    volatile double x = get_value(0);
    volatile double y = get_value(1);
    volatile double z = get_value(2);
    
    /* Chained comparisons */
    if (x < y && y < z) {
        global_sum += 100;
    }
    
    /* Conditional operator with FP comparison */
    double result = (x != y) ? (x + y) : (x - y);
    global_sum += (int)result;
    
    printf("Final sum: %d\n", global_sum);
    
    /* Use the result to prevent dead code elimination */
    if (global_sum > 1000) {
        printf("Unexpectedly large sum\n");
    }
    
    return global_sum > 0 ? 0 : 1;
}
