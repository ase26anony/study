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
    /* Volatile variables to prevent optimization */
    volatile double v1, v2, v3, v4;
    volatile int vi1, vi2;
    
    /* Initialize with NaN and normal values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    
    /* Set volatile values */
    v1 = 1.5;
    v2 = 2.5;
    v3 = nan_val;
    v4 = 3.5;
    vi1 = 10;
    vi2 = 20;
    
    /* 1. UNORDERED - Compare NaN with normal value */
    if (__builtin_isunordered(v3, v1)) {
        global_sum += 1;
    }
    
    /* Inline assembly to clobber FP status register */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* 2. ORDERED - Compare two normal values */
    if (__builtin_isordered(v1, v2)) {
        global_sum += 2;
    }
    
    /* 3. UNEQ - Unordered or equal (NaN == NaN is false, but unordered) */
    /* Use ternary operator to force condition code generation */
    global_sum += (v3 != v3) ? 3 : 0;  /* NaN != NaN is true */
    
    /* 4. UNGE - Unordered or greater than or equal */
    /* Compare NaN with normal value using >= */
    if (!(v1 >= v3)) {  /* This generates nlt (not less than) */
        global_sum += 4;
    }
    
    asm volatile ("" : : : "cc", "memory");
    
    /* 5. UNGT - Unordered or greater than */
    /* Compare NaN with normal value using > */
    if (!(v1 > v3)) {  /* This generates nle (not less than or equal) */
        global_sum += 5;
    }
    
    /* 6. UNLE - Unordered or less than or equal */
    /* Use mixed integer-FP comparison */
    double converted = (double)(int)vi1;
    if (converted <= v3 || __builtin_isunordered(converted, v3)) {
        global_sum += 6;
    }
    
    /* 7. UNLT - Unordered or less than */
    if (v3 < v1 || __builtin_isunordered(v3, v1)) {
        global_sum += 7;
    }
    
    /* 8. LTGT - Less than or greater than (ordered and not equal) */
    if (v1 != v2 && __builtin_isordered(v1, v2)) {
        global_sum += 8;
    }
    
    /* Switch statement with floating point comparisons */
    int switch_val = vi1 % 4;
    switch (switch_val) {
        case 0:
            if (v1 < v2) global_sum += 10;
            break;
        case 1:
            if (v1 > v2) global_sum += 11;
            break;
        case 2:
            if (v1 == v2) global_sum += 12;
            break;
        case 3:
            if (v1 != v2) global_sum += 13;
            break;
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        
        /* Use different comparisons based on LCG state */
        uint32_t r = lcg_rand();
        
        switch (r % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1:  /* ORDERED */
                if (__builtin_isordered(a, b)) global_sum++;
                break;
            case 2:  /* UNEQ - using != with potential NaN */
                global_sum += (a != b) ? 1 : 0;
                break;
            case 3:  /* UNGE - using >= */
                if (a >= b) global_sum++;
                break;
            case 4:  /* UNGT - using > */
                if (a > b) global_sum++;
                break;
            case 5:  /* UNLE - using <= */
                if (a <= b) global_sum++;
                break;
            case 6:  /* UNLT - using < */
                if (a < b) global_sum++;
                break;
            case 7:  /* LTGT - ordered and not equal */
                if (a != b && __builtin_isordered(a, b)) global_sum++;
                break;
        }
        
        /* More complex expression with ternary operator */
        double c = (r & 1) ? nan_val : a;
        global_sum += (c < b) ? 1 : 0;
        
        /* Clobber FPU occasionally */
        if (i % 7 == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
        }
    }
    
    /* Additional comparisons with signaling NaN possibility */
    volatile double *ptr = &v1;
    if (*ptr == v2) {
        global_sum += 100;
    }
    
    /* Force generation of conditional moves */
    double d1 = (vi1 > vi2) ? v1 : v2;
    double d2 = (vi1 < vi2) ? v3 : v4;
    
    if (__builtin_isgreater(d1, d2)) {
        global_sum += 200;
    }
    
    if (__builtin_isless(d1, d2)) {
        global_sum += 300;
    }
    
    /* Print result to ensure execution */
    printf("Result: %d\n", global_sum);
    
    return 0;
}
