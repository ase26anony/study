#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg = 123456789;
uint32_t rand_lcg() {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main() {
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
    vi1 = 42;
    vi2 = -7;
    
    /* 1. UNORDERED: Compare NaN with normal value */
    if (__builtin_isunordered(v3, v1)) {
        global_sum += 1;
        /* Inline assembly clobbering FP status */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    }
    
    /* 2. ORDERED: Compare two normal values */
    if (!__builtin_isunordered(v1, v2)) {
        global_sum += 2;
    }
    
    /* 3. UNEQ: Unordered or equal */
    /* Using explicit comparison with NaN */
    if (v3 != v3 || v1 == v2) {  /* v3 != v3 is true for NaN */
        global_sum += 3;
    }
    
    /* 4. UNGE: Unordered or greater-or-equal */
    /* Using builtin and explicit operators */
    if (__builtin_isunordered(v3, v1) || v2 >= v1) {
        global_sum += 4;
        asm volatile ("" : : : "cc");
    }
    
    /* 5. UNGT: Unordered or greater-than */
    if (__builtin_isunordered(v3, v4) || v4 > v1) {
        global_sum += 5;
    }
    
    /* 6. UNLE: Unordered or less-or-equal */
    if (__builtin_isunordered(v1, v3) || v1 <= v2) {
        global_sum += 6;
    }
    
    /* 7. UNLT: Unordered or less-than */
    if (__builtin_isunordered(v3, v2) || v1 < v2) {
        global_sum += 7;
    }
    
    /* 8. LTGT: Less-than or greater-than (ordered and not equal) */
    if (v1 < v2 || v2 > v1) {  /* Both conditions are true and ordered */
        global_sum += 8;
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)");
    }
    
    /* Mixed integer-FP comparisons */
    double converted = (double)(int)vi1;
    if (converted != v1) {
        global_sum += 9;
    }
    
    /* Ternary operator with FP comparison */
    int result = (v2 > v1) ? 10 : 0;
    global_sum += result;
    
    /* Switch statement based on FP comparison */
    switch ((v1 < v2) ? 1 : ((v1 > v2) ? 2 : 3)) {
        case 1:
            global_sum += 11;
            break;
        case 2:
            global_sum += 12;
            break;
        default:
            global_sum += 13;
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        
        /* Use pseudo-random to select different comparisons */
        uint32_t r = rand_lcg();
        
        switch (r % 8) {
            case 0:
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1:
                if (!__builtin_isunordered(a, b)) global_sum++;
                break;
            case 2:
                if (a == b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 3:
                if (a >= b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 4:
                if (a > b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 5:
                if (a <= b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 6:
                if (a < b || __builtin_isunordered(a, b)) global_sum++;
                break;
            case 7:
                if ((a < b || a > b) && !__builtin_isunordered(a, b)) global_sum++;
                break;
        }
        
        /* Occasionally clobber FP registers */
        if (i % 7 == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        }
    }
    
    /* Complex expression mixing multiple conditions */
    volatile double v5 = get_value(0);
    volatile double v6 = get_value(1);
    
    if ((v5 < v6 && !__builtin_isunordered(v5, v6)) || 
        (__builtin_isunordered(v5, v3) && v5 != v6)) {
        global_sum += 100;
    }
    
    /* Comparisons with infinity */
    if (v1 < inf_val) {
        global_sum += 200;
    }
    
    if (v3 > -inf_val) {  /* NaN comparison */
        global_sum += 300;
    }
    
    printf("Final sum: %d\n", global_sum);
    
    /* Additional forced comparisons in return statement */
    return (v1 < v2) ? 0 : 
           (v1 > v2) ? 1 : 
           (v1 == v2) ? 2 : 
           (__builtin_isunordered(v1, v2)) ? 3 : 4;
}
