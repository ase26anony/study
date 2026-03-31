#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to prevent constant folding */
volatile double get_value(void) {
    static volatile double counter = 0.5;
    counter += 0.1;
    return counter;
}

int main(void) {
    volatile double x, y, z;
    volatile int vi = 7;
    
    /* Initialize with NaN */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    
    /* Block 1: UNORDERED - unordered comparison with NaN */
    x = nan_val;
    y = get_value();
    if (__builtin_isunordered(x, y)) {
        sum += 1;
    }
    
    /* Block 2: ORDERED - ordered comparison */
    x = get_value();
    y = get_value();
    if (!__builtin_isunordered(x, y)) {
        sum += 2;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    x = nan_val;
    y = nan_val;
    if (!(x < y) && !(x > y)) {  /* UNEQ: !(a < b) && !(a > b) */
        sum += 3;
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    x = nan_val;
    y = get_value();
    if (!(x < y)) {  /* UNGE: !(a < b) */
        sum += 4;
    }
    
    /* Block 5: UNGT - unordered or greater */
    x = nan_val;
    y = get_value();
    if (!(x <= y)) {  /* UNGT: !(a <= b) */
        sum += 5;
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    x = nan_val;
    y = get_value();
    if (!(x > y)) {  /* UNLE: !(a > b) */
        sum += 6;
    }
    
    /* Block 7: UNLT - unordered or less */
    x = nan_val;
    y = get_value();
    if (!(x >= y)) {  /* UNLT: !(a >= b) */
        sum += 7;
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    x = get_value();
    y = get_value() + 1.0;
    if ((x < y) || (x > y)) {  /* LTGT: (a < b) || (a > b) */
        sum += 8;
    }
    
    /* Mixed integer-FP comparisons */
    x = (double)(int)vi;
    y = get_value();
    if (x != y) {
        sum += 9;
    }
    
    /* Ternary operator with FP comparison */
    z = (__builtin_isless(x, y) ? x : y);
    sum += (int)z;
    
    /* Switch statement based on FP comparison */
    switch ((x > y) ? 1 : (x < y) ? 2 : 0) {
        case 1: sum += 10; break;
        case 2: sum += 20; break;
        default: sum += 30; break;
    }
    
    /* Loop with varying conditions */
    volatile double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = get_value() + i;
    }
    
    for (int i = 0; i < 10; i++) {
        uint32_t r = lcg_rand();
        x = arr[i];
        y = arr[(i + 1) % 10];
        
        /* Inline assembly to clobber FP status register */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", 
                      "st(4)", "st(5)", "st(6)", "st(7)");
        
        /* Different comparisons based on pseudo-random value */
        switch (r % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(x, y)) sum += 100;
                break;
            case 1:  /* ORDERED */
                if (!__builtin_isunordered(x, y)) sum += 200;
                break;
            case 2:  /* UNEQ */
                if (!(x < y) && !(x > y)) sum += 300;
                break;
            case 3:  /* UNGE */
                if (!(x < y)) sum += 400;
                break;
            case 4:  /* UNGT */
                if (!(x <= y)) sum += 500;
                break;
            case 5:  /* UNLE */
                if (!(x > y)) sum += 600;
                break;
            case 6:  /* UNLT */
                if (!(x >= y)) sum += 700;
                break;
            case 7:  /* LTGT */
                if ((x < y) || (x > y)) sum += 800;
                break;
        }
        
        /* Additional inline assembly to interfere with optimization */
        asm volatile ("fwait" : : : "memory");
    }
    
    /* Force NaN comparisons in loop */
    for (int i = 0; i < 5; i++) {
        x = (i & 1) ? nan_val : get_value();
        y = (i & 2) ? nan_val : get_value();
        
        /* Complex expression to force condition code generation */
        int cond = (x == y) ? 1 : (x != y) ? 2 : 
                  (x < y) ? 3 : (x > y) ? 4 : 
                  (x <= y) ? 5 : (x >= y) ? 6 : 0;
        sum += cond;
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}
