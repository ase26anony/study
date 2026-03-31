#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 1;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double zero_val = 0.0;
    volatile double one_val = 1.0;
    volatile double neg_one_val = -1.0;
    
    volatile int volatile_int = 42;
    
    /* Force re-evaluation of FP comparisons */
    asm volatile ("" : : : "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 1. UNORDERED - comparison with NaN */
    if (__builtin_isunordered(nan_val, one_val)) {
        global_sum += 1;
    }
    
    /* 2. ORDERED - both operands are not NaN */
    if (__builtin_isordered(one_val, neg_one_val)) {
        global_sum += 2;
    }
    
    /* 3. UNEQ - unordered or equal */
    if (!(nan_val == nan_val) || (one_val == one_val)) {
        global_sum += 3;
    }
    
    /* 4. UNGE - unordered or greater-or-equal */
    if (__builtin_isunordered(one_val, nan_val) || (one_val >= zero_val)) {
        global_sum += 4;
    }
    
    /* 5. UNGT - unordered or greater-than */
    if (__builtin_isunordered(nan_val, inf_val) || (inf_val > one_val)) {
        global_sum += 5;
    }
    
    /* 6. UNLE - unordered or less-or-equal */
    if (__builtin_isunordered(zero_val, nan_val) || (neg_one_val <= zero_val)) {
        global_sum += 6;
    }
    
    /* 7. UNLT - unordered or less-than */
    if (__builtin_isunordered(nan_val, zero_val) || (neg_one_val < zero_val)) {
        global_sum += 7;
    }
    
    /* 8. LTGT - less-than or greater-than (not equal, not unordered) */
    if ((one_val < neg_one_val) || (one_val > neg_one_val)) {
        global_sum += 8;
    }
    
    /* Mixed integer-FP comparisons with conversions */
    double converted = (double)(int)volatile_int;
    if (converted > 10.0) {
        global_sum += 9;
    }
    
    /* Ternary operator forcing condition code generation */
    global_sum += (__builtin_isunordered(nan_val, one_val) ? 10 : 0);
    
    /* Switch statement with FP comparisons */
    switch (volatile_int) {
        case 42:
            if (one_val != nan_val) global_sum += 11;
            break;
        default:
            if (zero_val == zero_val) global_sum += 12;
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        
        /* Use LCG to select different comparison types */
        uint32_t r = lcg_rand();
        
        switch (r % 8) {
            case 0: /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1: /* ORDERED */
                if (__builtin_isordered(a, b)) global_sum++;
                break;
            case 2: /* UNEQ */
                if (!(a == b) || (a == a)) global_sum++;
                break;
            case 3: /* UNGE */
                if (__builtin_isunordered(a, b) || (a >= b)) global_sum++;
                break;
            case 4: /* UNGT */
                if (__builtin_isunordered(a, b) || (a > b)) global_sum++;
                break;
            case 5: /* UNLE */
                if (__builtin_isunordered(a, b) || (a <= b)) global_sum++;
                break;
            case 6: /* UNLT */
                if (__builtin_isunordered(a, b) || (a < b)) global_sum++;
                break;
            case 7: /* LTGT */
                if ((a < b) || (a > b)) global_sum++;
                break;
        }
        
        /* Inline assembly clobbering FP status register */
        asm volatile ("" : : : "cc", "memory");
    }
    
    /* Final output to ensure all code executes */
    printf("Result: %d\n", global_sum);
    
    return 0;
}
