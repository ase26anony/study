#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int side_effect = 0;

/* Function to prevent constant folding */
double get_nan(void) {
    volatile double nan = __builtin_nan("");
    return nan;
}

double get_value(int i) {
    volatile double vals[] = {1.0, 2.0, 3.0, 0.0, -1.0, -2.0};
    return vals[i % 6];
}

/* Linear congruential generator for pseudo-random sequence */
static uint32_t lcg_state = 1;
uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan1 = get_nan();
    volatile double nan2 = __builtin_nan("0xdead");
    volatile double x = 1.5;
    volatile double y = 2.5;
    volatile double z = 0.0;
    volatile int vi = 42;
    
    /* Force compiler to keep variables in memory */
    asm volatile("" : : "r"(nan1), "r"(nan2), "r"(x), "r"(y), "r"(z), "r"(vi));
    
    /* Block 1: UNORDERED - unordered comparison */
    if (__builtin_isunordered(nan1, x)) {
        global_sum += 1;
        side_effect = 1;
    }
    
    /* Block 2: ORDERED - ordered comparison */
    if (!__builtin_isunordered(x, y)) {
        global_sum += 2;
        side_effect = 2;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    /* Using ternary operator to force condition code generation */
    global_sum += (nan1 != nan1) ? 3 : 0;  /* NaN != NaN is true for unordered */
    
    /* Block 4: UNGE - not less than (unordered or greater/equal) */
    if (!__builtin_isless(x, y)) {
        global_sum += 4;
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    if (!__builtin_islessequal(x, y)) {
        global_sum += 5;
    }
    
    /* Block 6: UNLE - unordered or less/equal */
    if (__builtin_islessequal(nan1, x) || __builtin_islessequal(x, nan1)) {
        global_sum += 6;
    }
    
    /* Block 7: UNLT - unordered or less than */
    if (__builtin_isless(nan1, x) || __builtin_isless(x, nan1)) {
        global_sum += 7;
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    if (__builtin_isless(x, y) || __builtin_isgreater(x, y)) {
        global_sum += 8;
    }
    
    /* Mixed integer-FP comparisons with conversions */
    if ((double)(int)vi < y) {
        global_sum += 9;
    }
    
    /* Switch statement with floating comparisons */
    volatile int selector = 3;
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
    }
    
    /* Loop with varying conditions */
    volatile double arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = get_value(i);
        
        /* Use LCG to select different comparison types */
        uint32_t r = lcg_rand();
        
        /* Clobber FP status register */
        asm volatile("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        
        switch (r % 8) {
            case 0: /* UNORDERED */
                if (__builtin_isunordered(arr[i], nan1)) {
                    global_sum += i;
                }
                break;
            case 1: /* ORDERED */
                if (!__builtin_isunordered(arr[i], arr[(i+1)%20])) {
                    global_sum += i * 2;
                }
                break;
            case 2: /* UNEQ */
                if (!(arr[i] == arr[(i+2)%20])) {
                    global_sum += i * 3;
                }
                break;
            case 3: /* UNGE */
                if (!(arr[i] < arr[(i+3)%20])) {
                    global_sum += i * 4;
                }
                break;
            case 4: /* UNGT */
                if (!(arr[i] <= arr[(i+4)%20])) {
                    global_sum += i * 5;
                }
                break;
            case 5: /* UNLE */
                if (arr[i] <= arr[(i+5)%20]) {
                    global_sum += i * 6;
                }
                break;
            case 6: /* UNLT */
                if (arr[i] < arr[(i+6)%20]) {
                    global_sum += i * 7;
                }
                break;
            case 7: /* LTGT */
                if (arr[i] != arr[(i+7)%20]) {
                    global_sum += i * 8;
                }
                break;
        }
        
        /* Another inline asm to prevent optimization */
        asm volatile("" : "+m" (global_sum));
    }
    
    /* Complex expression mixing multiple comparisons */
    volatile double a = 1.0, b = 2.0, c = 3.0;
    if ((a < b) && (b < c) && !__builtin_isunordered(a, nan1)) {
        global_sum += 100;
    }
    
    /* Print result to ensure execution */
    printf("Result: %d\n", global_sum);
    
    return 0;
}
