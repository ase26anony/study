#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 42;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    /* Initialize volatile variables with NaN values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        if (nan_val != nan_val) { /* NaN != NaN triggers unordered */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 3;
        }
        if (!__builtin_isunordered(normal_val, normal_val)) {
            global_sum += 4;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" : : : "st");
        double a = get_value(0);
        double b = get_value(1);
        if (!__builtin_islessgreater(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 5;
        }
        /* Using ternary operator */
        global_sum += (a == b || __builtin_isunordered(a, b)) ? 6 : 0;
    }
    
    /* Block 4: UNGE (unordered or greater-or-equal) */
    {
        asm volatile ("" : : : "st");
        volatile double x = get_value(2);
        volatile double y = get_value(3);
        if (!__builtin_isless(x, y) || __builtin_isunordered(x, y)) {
            global_sum += 7;
        }
        if (x >= y || __builtin_isunordered(x, y)) {
            global_sum += 8;
        }
    }
    
    /* Block 5: UNGT (unordered or greater) */
    {
        asm volatile ("" : : : "st");
        volatile double p = get_value(1);
        volatile double q = get_value(4);
        if (!__builtin_islessequal(p, q) || __builtin_isunordered(p, q)) {
            global_sum += 9;
        }
        if (p > q || __builtin_isunordered(p, q)) {
            global_sum += 10;
        }
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        asm volatile ("" : : : "st");
        volatile double m = get_value(3);
        volatile double n = get_value(0);
        if (!__builtin_isgreater(m, n) || __builtin_isunordered(m, n)) {
            global_sum += 11;
        }
        if (m <= n || __builtin_isunordered(m, n)) {
            global_sum += 12;
        }
    }
    
    /* Block 7: UNLT (unordered or less) */
    {
        asm volatile ("" : : : "st");
        volatile double u = get_value(4);
        volatile double v = get_value(2);
        if (!__builtin_isgreaterequal(u, v) || __builtin_isunordered(u, v)) {
            global_sum += 13;
        }
        if (u < v || __builtin_isunordered(u, v)) {
            global_sum += 14;
        }
    }
    
    /* Block 8: LTGT (less or greater, but not equal and not unordered) */
    {
        asm volatile ("" : : : "st");
        volatile double r = get_value(0);
        volatile double s = get_value(2);
        if (__builtin_islessgreater(r, s) && !__builtin_isunordered(r, s)) {
            global_sum += 15;
        }
        if ((r < s || r > s) && !__builtin_isunordered(r, s)) {
            global_sum += 16;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" : : : "st");
        double converted = (double)(int)volatile_int;
        if (converted > normal_val) {
            global_sum += 17;
        }
        if (converted < inf_val) {
            global_sum += 18;
        }
    }
    
    /* Switch statement with FP comparisons */
    {
        asm volatile ("" : : : "st");
        volatile double a = get_value(lcg_rand() % 5);
        volatile double b = get_value(lcg_rand() % 5);
        
        int cmp_result = 0;
        if (__builtin_isunordered(a, b)) cmp_result = 1;
        else if (a == b) cmp_result = 2;
        else if (a < b) cmp_result = 3;
        else cmp_result = 4;
        
        switch (cmp_result) {
            case 1: global_sum += 19; break;
            case 2: global_sum += 20; break;
            case 3: global_sum += 21; break;
            case 4: global_sum += 22; break;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 10; i++) {
            asm volatile ("" : : : "st");
            uint32_t rand_val = lcg_rand();
            volatile double x = arr[i];
            volatile double y = arr[(i + 1) % 10];
            
            /* Different comparison based on hash of index */
            switch (rand_val % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(x, y)) global_sum += 23;
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(x, y)) global_sum += 24;
                    break;
                case 2: /* UNEQ */
                    if (!__builtin_islessgreater(x, y) || __builtin_isunordered(x, y))
                        global_sum += 25;
                    break;
                case 3: /* UNGE */
                    if (!__builtin_isless(x, y) || __builtin_isunordered(x, y))
                        global_sum += 26;
                    break;
                case 4: /* UNGT */
                    if (!__builtin_islessequal(x, y) || __builtin_isunordered(x, y))
                        global_sum += 27;
                    break;
                case 5: /* UNLE */
                    if (!__builtin_isgreater(x, y) || __builtin_isunordered(x, y))
                        global_sum += 28;
                    break;
                case 6: /* UNLT */
                    if (!__builtin_isgreaterequal(x, y) || __builtin_isunordered(x, y))
                        global_sum += 29;
                    break;
                case 7: /* LTGT */
                    if (__builtin_islessgreater(x, y) && !__builtin_isunordered(x, y))
                        global_sum += 30;
                    break;
            }
        }
    }
    
    /* Final output to ensure all code is executed */
    printf("Final sum: %d\n", global_sum);
    
    return 0;
}
