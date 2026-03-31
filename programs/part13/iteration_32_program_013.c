#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to prevent constant folding */
volatile double get_value(void) {
    static volatile double counter = 0.0;
    return counter += 0.5;
}

int main(void) {
    volatile double x, y, z;
    volatile int vi = 42;
    volatile int condition_result;
    
    /* Initialize with NaN */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 1. UNORDERED: Compare NaN with normal number */
    x = nan_val;
    y = get_value();
    if (__builtin_isunordered(x, y)) {
        global_sum += 1;
    }
    
    /* 2. ORDERED: Compare two normal numbers */
    x = get_value();
    y = get_value();
    if (__builtin_isordered(x, y)) {
        global_sum += 2;
    }
    
    /* 3. UNEQ: Unordered or equal (NaN == NaN is false, but UNEQ handles unordered) */
    x = nan_val;
    y = nan_val;
    condition_result = (x == y) ? 1 : 0;  /* This should be false */
    /* Use ternary to force condition code generation */
    global_sum += __builtin_isunordered(x, y) ? 3 : 0;
    
    /* 4. UNGE: Not less than (unordered or greater/equal) */
    x = nan_val;
    y = get_value();
    if (!__builtin_isless(x, y)) {  /* nlt */
        global_sum += 4;
    }
    
    /* 5. UNGT: Not less than or equal (unordered or greater) */
    x = nan_val;
    y = get_value();
    if (!__builtin_islessequal(x, y)) {  /* nle */
        global_sum += 5;
    }
    
    /* 6. UNLE: Unordered or less/equal */
    x = nan_val;
    y = get_value();
    if (__builtin_islessequal(x, y) || __builtin_isunordered(x, y)) {
        global_sum += 6;
    }
    
    /* 7. UNLT: Unordered or less than */
    x = nan_val;
    y = get_value();
    if (__builtin_isless(x, y) || __builtin_isunordered(x, y)) {
        global_sum += 7;
    }
    
    /* 8. LTGT: Less than or greater than (ordered and not equal) */
    x = get_value();
    y = get_value() + 1.0;
    if (__builtin_isless(x, y) || __builtin_isgreater(x, y)) {
        global_sum += 8;
    }
    
    /* Mixed integer-FP comparisons */
    x = (double)(int)vi;
    y = get_value();
    if (x < y) {
        global_sum += 9;
    }
    
    /* Switch statement with FP comparisons */
    volatile int selector = vi & 0x7;
    switch (selector) {
        case 0:
            if (x == y) global_sum += 10;
            break;
        case 1:
            if (x != y) global_sum += 11;
            break;
        case 2:
            if (x < y) global_sum += 12;
            break;
        case 3:
            if (x > y) global_sum += 13;
            break;
        case 4:
            if (x <= y) global_sum += 14;
            break;
        case 5:
            if (x >= y) global_sum += 15;
            break;
        case 6:
            if (__builtin_isunordered(x, y)) global_sum += 16;
            break;
        case 7:
            if (__builtin_isordered(x, y)) global_sum += 17;
            break;
    }
    
    /* Loop with varying conditions */
    volatile double array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = get_value() + i;
    }
    
    for (int i = 0; i < 100; i++) {
        uint32_t r = lcg_rand();
        volatile double a = array[r % 8];
        volatile double b = array[(r >> 3) % 8];
        
        /* Vary comparison based on bits */
        switch (r & 0x7) {
            case 0:
                if (a == b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 1:
                if (a != b && __builtin_isordered(a, b)) global_sum++;
                break;
            case 2:
                if (a < b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 3:
                if (a > b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 4:
                if (!(a < b)) global_sum++;  /* nlt */
                break;
            case 5:
                if (!(a <= b)) global_sum++;  /* nle */
                break;
            case 6:
                if (__builtin_islessgreater(a, b)) global_sum++;
                break;
            case 7:
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
        }
        
        /* Clobber FPU status register periodically */
        if ((i % 17) == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                         "st(4)", "st(5)", "st(6)", "st(7)");
        }
    }
    
    /* Complex expression with multiple conditions */
    x = get_value();
    y = get_value();
    z = get_value();
    
    if ((x < y && y > z) || (__builtin_isunordered(x, z))) {
        global_sum += 100;
    }
    
    /* Force generation of setcc instructions */
    volatile char bool_result;
    bool_result = (x == y) ? 1 : 0;
    global_sum += bool_result;
    
    bool_result = __builtin_isunordered(x, y) ? 1 : 0;
    global_sum += bool_result;
    
    bool_result = !__builtin_isless(x, y) ? 1 : 0;
    global_sum += bool_result;
    
    printf("Final sum: %d\n", global_sum);
    
    return 0;
}
